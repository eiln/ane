#!/usr/bin/python3

# SPDX-License-Identifier: MIT
# Copyright 2022 Eileen Yoon <eyn@gmx.com>

import os
import atexit
import ctypes
import numpy as np
from ctypes import c_void_p, c_ulong, byref

TILE_COUNT = 0x20
def align16k(x): return (x + 0x4000 - 1) & -0x4000

class Driver:
	def __init__(self, path):
		self.lib = ctypes.cdll.LoadLibrary(path)
		self.lib.pyane_init.restype = c_void_p
		self.lib.pyane_free.argtypes = [c_void_p]
		self.lib.pyane_exec.argtypes = [c_void_p]
		self.lib.pyane_send.argtypes = [c_void_p] + [c_void_p] * TILE_COUNT
		self.lib.pyane_read.argtypes = [c_void_p] + [c_void_p] * TILE_COUNT
		self.lib.pyane_info.argtypes = [c_void_p] + [ctypes.POINTER(c_ulong)] * (2 + (6 * TILE_COUNT * 2))
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
		counts, nchws = [c_ulong(), c_ulong()], [c_ulong() for x in range(TILE_COUNT * 6 * 2)]
		self.driver.lib.pyane_info(self.handle, *[byref(x) for x in counts + nchws])
		self.src_count, self.dst_count = counts[0].value, counts[1].value
		self.src_nchw = tuple([tuple(x.value for x in nchws[n*6:(n+1)*6]) for n in range(self.src_count)])
		self.dst_nchw = tuple([tuple(x.value for x in nchws[n*6:(n+1)*6]) for n in range(TILE_COUNT, TILE_COUNT + self.dst_count)])
		self.src_size = tuple([align16k(nchw[0] * nchw[1] * nchw[4]) for nchw in self.src_nchw])
		self.dst_size = tuple([align16k(nchw[0] * nchw[1] * nchw[4]) for nchw in self.dst_nchw])
		self.outputs = [ctypes.create_string_buffer(size) for size in self.dst_size] + [b''] * (TILE_COUNT - self.dst_count)

	def predict(self, inarrs):  # list of numpy arrays
		assert(all(((arr.dtype == np.float16) and (arr.shape == self.src_nchw[idx][:4])) for idx,arr in enumerate(inarrs)))
		inputs = [arr.tobytes(order='C') for arr in inarrs]
		self.driver.lib.pyane_send(self.handle, *inputs, *[b''] * (TILE_COUNT - self.src_count))
		self.driver.lib.pyane_exec(self.handle)
		self.driver.lib.pyane_read(self.handle, *self.outputs)
		return [self.tile2arr(self.outputs[idx], idx) for idx in range(self.dst_count)]

	def tile2arr(self, tile, idx):
		N, C, H, W, P, R = self.dst_nchw[idx]
		new_N, new_C, new_H, new_W = N, C, P//R, R//2
		arr = np.frombuffer(tile, dtype=np.float16)[:new_N*new_C*new_H*new_W]
		return arr.reshape((new_N, new_C, new_H, new_W))[:N, :C, :H, :W]
