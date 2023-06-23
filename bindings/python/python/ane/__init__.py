#!/usr/bin/python3

# SPDX-License-Identifier: MIT
# Copyright 2022 Eileen Yoon <eyn@gmx.com>

import atexit
import ctypes
import numpy as np
from ctypes import c_void_p
from construct import Struct, Array, Int32ul, Int64ul

class _Driver:
	def __init__(self, lib_path):
		self.lib = ctypes.cdll.LoadLibrary(lib_path)
		self.lib.pyane_init.restype = c_void_p
		self.lib.pyane_init.argtypes = [ctypes.c_char_p, ctypes.c_int]
		self.lib.pyane_free.argtypes = [c_void_p]
		self.lib.pyane_exec.argtypes = [c_void_p]
		self.lib.pyane_send.argtypes = [c_void_p] + [c_void_p] * 0x20
		self.lib.pyane_read.argtypes = [c_void_p] + [c_void_p] * 0x20
		self.handles = {}
		atexit.register(self.cleanup)

	def cleanup(self):
		for handle in self.handles:
			self.lib.pyane_free(handle)

	def register(self, path, dev_id):
		handle = self.lib.pyane_init(path.encode('ascii'), dev_id)
		if (handle == None): raise RuntimeError("driver error")
		self.handles[handle] = handle
		return handle

class model:
	def __init__(self, path, dev_id=0, lib_path="/usr/lib/libane_python.so"):
		self.driver = _Driver(lib_path)
		self.handle = self.driver.register(path, dev_id)
		fmt = Struct("size" / Int64ul,"td_size" / Int32ul, "td_count" / Int32ul, "tsk_size" / Int64ul, "krn_size" / Int64ul, "src_count" / Int32ul, "dst_count" / Int32ul, "tiles" / Array(0x20, Int32ul), "nchw" / Array(0x20 * 6, Int64ul))
		res = fmt.parse(open(path, "rb").read()[:fmt.sizeof()])
		self.src_count, self.dst_count = res.src_count, res.dst_count
		self.dst_nchw, self.src_nchw = [tuple([tuple(x for x in res.nchw[(base+n)*6:(base+n+1)*6]) for n in range(count)]) for (base, count) in ((4, res.dst_count), (4 + res.dst_count, res.src_count))]
		self.outputs = [ctypes.create_string_buffer(nchw[0]*nchw[1]*nchw[2]*nchw[3]*2) for nchw in self.dst_nchw]
		self.inputs_pad, self.outputs_pad = [b''] * (0x20 - res.src_count), [b''] * (0x20 - res.dst_count)

	def predict(self, inarrs):  # list of numpy arrays
		assert(len(inarrs) == self.src_count)
		assert(all(((arr.dtype == np.float16) and (arr.shape == self.src_nchw[idx][:4])) for idx,arr in enumerate(inarrs)))
		self.driver.lib.pyane_send(self.handle, *[arr.tobytes(order='C') for arr in inarrs], *self.inputs_pad)
		self.driver.lib.pyane_exec(self.handle)
		self.driver.lib.pyane_read(self.handle, *self.outputs, *self.outputs_pad)
		return [np.frombuffer(self.outputs[idx], dtype=np.float16).reshape(*self.dst_nchw[idx][:4]) for idx in range(self.dst_count)]
