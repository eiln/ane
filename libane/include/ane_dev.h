// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_DEV_H__
#define __ANE_DEV_H__

#include <asm/types.h>
#include <stdint.h>

#include "drm_ane.h"

#define ANE_TILE_CMD 0x1
#define ANE_TILE_ITM 0x2
#define ANE_TILE_DST 0x3
#define ANE_TILE_SRC 0x4

struct anec {
	__u64 size;
	__u64 tsk_size;
	__u64 krn_size;
	__u32 td_count;
	__u32 td_size;
	__u32 tiles[ANE_TILE_COUNT];
	__u32 types[ANE_TILE_COUNT];
};

struct ane_device {
	int fd;
	int ane_type;
	int ane_subtype;
	int ane_id;
};

struct ane_bo {
	void *map;
	uint64_t size;
	uint32_t handle;
	uint64_t offset;
};

struct ane_model {
	const char *name;
	const void *data;
	const struct anec anec;
	const int input_count;
	const int output_count;
	const uint64_t nchw[ANE_TILE_COUNT][6];
};

struct ane_nn {
	struct ane_device *ane;
	const struct ane_model *model;
	uint32_t handle;
	struct ane_bo *chans[ANE_TILE_COUNT];
	struct ane_bo *fifo_chan;
	int src_bdx[ANE_TILE_COUNT];
	int dst_bdx[ANE_TILE_COUNT];
};

#define input_count(nn)	 (nn->model->input_count)
#define output_count(nn) (nn->model->output_count)

#endif /* __ANE_DEV_H__ */
