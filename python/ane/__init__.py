#!/usr/bin/python3

# SPDX-License-Identifier: MIT
# Copyright 2022 Eileen Yoon <eyn@gmx.com>

import os
import atexit
import ctypes
import numpy as np
from ctypes import c_void_p, c_ulong

class Driver:
	def __init__(self, path):
		self.lib = ctypes.cdll.LoadLibrary(path)
		self.lib.pyane_init.restype = c_void_p
		self.lib.pyane_free.argtypes = [c_void_p]
		self.lib.pyane_exec.argtypes = [c_void_p]
		self.lib.pyane_send.argtypes = [c_void_p] + [c_void_p] * 0x20
		self.lib.pyane_read.argtypes = [c_void_p] + [c_void_p] * 0x20
		self.lib.pyane_info.argtypes = [c_void_p] + [ctypes.POINTER(c_ulong)] * (2 + (2 * 0x20 * 6))
		self.handles = {}
		atexit.register(self.cleanup)

	def cleanup(self):
		for handle in self.handles:
			self.lib.pyane_free(handle)

	def register(self):
		handle = self.lib.pyane_init()
		if (handle == None): raise RuntimeError("driver error")
		self.handles[handle] = handle
		return handle

class Model:
	def __init__(self, path):
		self.driver = Driver(os.path.abspath(path))
		self.handle = self.driver.register()
		counts, nchws = [c_ulong(), c_ulong()], [c_ulong() for x in range(2 * 0x20 * 6)]
		self.driver.lib.pyane_info(self.handle, *[ctypes.byref(x) for x in counts + nchws])
		self.src_count, self.dst_count = counts[0].value, counts[1].value
		self.src_nchw = tuple([tuple(x.value for x in nchws[n*6:(n+1)*6]) for n in range(self.src_count)])
		self.dst_nchw = tuple([tuple(x.value for x in nchws[n*6:(n+1)*6]) for n in range(0x20, 0x20 + self.dst_count)])
		self.outputs = [ctypes.create_string_buffer(((nchw[0]*nchw[1]*nchw[4]) + 0x3fff) & -0x4000) for nchw in self.dst_nchw]
		self.inputs_pad, self.outputs_pad = [b''] * (0x20 - self.src_count), [b''] * (0x20 - self.dst_count)

	def predict(self, inarrs):  # list of numpy arrays
		assert(len(inarrs) == self.src_count)
		assert(all(((arr.dtype == np.float16) and (arr.shape == self.src_nchw[idx][:4])) for idx,arr in enumerate(inarrs)))
		self.driver.lib.pyane_send(self.handle, *[arr.tobytes(order='C') for arr in inarrs], *self.inputs_pad)
		self.driver.lib.pyane_exec(self.handle)
		self.driver.lib.pyane_read(self.handle, *self.outputs, *self.outputs_pad)
		return [self.tile2arr(self.outputs[idx], *self.dst_nchw[idx]) for idx in range(self.dst_count)]

	def tile2arr(self, tile, N, C, H, W, P, R):
		return np.frombuffer(tile[:N * C * P], dtype=np.float16).reshape((N, C, P//R, R//2))[:N, :C, :H, :W]
