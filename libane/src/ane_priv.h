// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_PRIV_H__
#define __ANE_PRIV_H__

#include <errno.h>
#include <stdio.h>

#include "ane_dev.h"

#define to_anec(nn)	   (&nn->model->anec)

#define TILE_SHIFT	   0xEUL
#define TILE_SIZE	   0x4000UL

#define tile_shift(x)	   (((unsigned long)(x)) << TILE_SHIFT)
#define tile_align(x)	   ((((unsigned long)(x)) + TILE_SIZE - 1) & -TILE_SIZE)
#define tile_size(nn, bdx) (tile_shift(to_anec(nn)->tiles[bdx]))

#endif /* __ANE_PRIV_H__ */
