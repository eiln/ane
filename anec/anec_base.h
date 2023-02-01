// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANEC_BASE_H__
#define __ANEC_BASE_H__

#include "drm_ane.h"

struct ane_model {
	const char *name;
	const char *fname;
	const int input_count;
	const int output_count;
	const struct anec anec;
};

#define ANEC_PATH "/home/eileen/ane/anec/kernels/"

#endif /* __ANEC_BASE_H__ */