// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_DEV_H__
#define __ANE_DEV_H__

#include <asm/types.h> // kernel __u types
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "anec.h"
#include "drm_ane.h"

#define ANE_SYSFS_PATH "/dev/dri/renderD129"

struct ane_device {
	int fd;
	int ane_type;
	int ane_subtype;
	int ane_id;
};

struct ane_tile {
	void *data;
	size_t size;
};

struct ane_nn {
	struct ane_device *ane;
	uint32_t handle; // gem handle
	const struct ane_model *model; // anec backend
	void *chans[MAX_TILE_COUNT]; // mapped to ane space
	void *fifo_chan; // mapped to ane space
	int imask[MAX_TILE_COUNT]; // convinience bdx masks
	int omask[MAX_TILE_COUNT];
};

#define to_anec(nn)	 (&nn->model->anec)

#define input_count(nn)	 (nn->model->input_count)
#define output_count(nn) (nn->model->output_count)

#endif /* __ANE_DEV_H__ */
