// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_MODEL_H__
#define __ANE_MODEL_H__

#include <asm/types.h>
#include "drm_ane.h"

struct ane_model {
	const char *name;
	const int input_count;
	const int output_count;
	const struct anec anec;
	const __u64 nchw[MAX_TILE_COUNT][6]; /* N, C, H, W, pS, rS */
};

#endif /* __ANE_MODEL_H__ */
