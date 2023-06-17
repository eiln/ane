// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_PRIV_H__
#define __ANE_PRIV_H__

#include <asm/types.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "ane_dev.h"
#include "drm_ane.h"

#if !defined(LIBANE_STFU_LOG) || !defined(LIBANE_STFU_ERR)
#include <stdio.h>
#endif

#define TILE_SHIFT	   0xEUL
#define TILE_SIZE	   0x4000UL

#define tile_shift(x)	   (((uint64_t)(x)) << TILE_SHIFT)
#define tile_align(x)	   ((((uint64_t)(x)) + TILE_SIZE - 1) & -TILE_SIZE)
#define tile_size(nn, bdx) (tile_shift(to_anec(nn)->tiles[bdx]))

#define to_anec(nn)	   (&nn->model->anec)
#define src_count(nn)	   (to_anec(nn)->src_count)
#define dst_count(nn)	   (to_anec(nn)->dst_count)

#define src_bdx(nn, idx)   (4 + to_anec(nn)->dst_count + idx)
#define dst_bdx(nn, idx)   (4 + idx)

#ifndef LIBANE_STFU_LOG
#define ane_log(a, ...) printf("LIBANE: LOG: " a, ##__VA_ARGS__)
#else
#define ane_log(...) \
	do {         \
	} while (0)
#endif

#ifndef LIBANE_STFU_ERR
#define ane_err(a, ...) fprintf(stderr, "LIBANE: ERR: " a, ##__VA_ARGS__)
#else
#define ane_err(...) \
	do {         \
	} while (0)
#endif

#endif /* __ANE_PRIV_H__ */
