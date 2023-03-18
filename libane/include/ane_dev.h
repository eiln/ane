// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_DEV_H__
#define __ANE_DEV_H__

#include <asm/types.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "drm_ane.h"

struct ane_device {
	int fd;
	int ane_type;
	int ane_subtype;
	int ane_id;
};

struct ane_model {
	const char *name;
	const int input_count;
	const int output_count;
	const struct anec anec;
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

#define TILE_SHIFT	 0xe
#define TILE_SIZE	 0x4000

#define tile_size(x)	 (x << TILE_SHIFT)
#define tile_align(x)	 ((x + TILE_SIZE - 1) & -TILE_SIZE)

#define to_anec(nn)	 (&nn->model->anec)
#define input_count(nn)	 (nn->model->input_count)
#define output_count(nn) (nn->model->output_count)

#endif /* __ANE_DEV_H__ */
