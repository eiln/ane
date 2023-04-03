// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_PRIV_H__
#define __ANE_PRIV_H__

#include <asm/types.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ane_dev.h"
#include "drm_ane.h"

#define TILE_SHIFT	   0xEUL
#define TILE_SIZE	   0x4000UL

#define to_anec(nn)	   (&nn->model->anec)
#define input_count(nn)	   (nn->model->input_count)
#define output_count(nn)   (nn->model->output_count)

#define tile_shift(x)	   (((uint64_t)(x)) << TILE_SHIFT)
#define tile_align(x)	   ((((uint64_t)(x)) + TILE_SIZE - 1) & -TILE_SIZE)
#define tile_size(nn, bdx) (tile_shift(to_anec(nn)->tiles[bdx]))

#endif /* __ANE_PRIV_H__ */
