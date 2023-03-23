// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_DEV_H__
#define __ANE_DEV_H__

#include <asm/types.h>
#include <stdint.h>

#include "drm_ane.h"

struct ane_device {
	int fd;
	int ane_type;
	int ane_subtype;
	int ane_id;
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
	void *chans[ANE_TILE_COUNT];
	void *fifo_chan;
	int src_bdx[ANE_TILE_COUNT];
	int dst_bdx[ANE_TILE_COUNT];
};

#define input_count(nn)	 (nn->model->input_count)
#define output_count(nn) (nn->model->output_count)

#endif /* __ANE_DEV_H__ */
