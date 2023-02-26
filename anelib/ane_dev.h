// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_DEV_H__
#define __ANE_DEV_H__

#include <asm/types.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ane_model.h"
#include "drm_ane.h"

#define ANE_SYSFS_PATH "/dev/dri/renderD129"

struct ane_device {
	int fd;
	int ane_type;
	int ane_subtype;
	int ane_id;
};

struct ane_nn {
	struct ane_device *ane;
	uint32_t handle;
	const struct ane_model *model;
	void *chans[MAX_TILE_COUNT];
	void *fifo_chan;
	int imask[MAX_TILE_COUNT];
	int omask[MAX_TILE_COUNT];
};

#define tile_sz(x)	 (x << TILE_SHIFT)
#define to_anec(nn)	 (&nn->model->anec)

#define input_count(nn)	 (nn->model->input_count)
#define output_count(nn) (nn->model->output_count)

#endif /* __ANE_DEV_H__ */
