// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_DEV_H__
#define __ANE_DEV_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

#define TILE_COUNT   0x20

#define ANE_TILE_CMD 0x1
#define ANE_TILE_ITM 0x2
#define ANE_TILE_DST 0x3
#define ANE_TILE_SRC 0x4

struct anec {
	const uint64_t size;
	const uint64_t tsk_size;
	const uint64_t krn_size;
	const uint32_t td_count;
	const uint32_t td_size;
	const uint32_t tiles[TILE_COUNT];
	const uint32_t types[TILE_COUNT];
};

struct ane_model {
	const char *name;
	const void *data;
	const struct anec anec;
	const int input_count;
	const int output_count;
	const uint64_t nchw[TILE_COUNT][6];
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

struct ane_nn {
	struct ane_device ane;
	const struct ane_model *model;
	struct ane_bo chans[TILE_COUNT];
	struct ane_bo btsp_chan;
	int src_bdx[TILE_COUNT];
	int dst_bdx[TILE_COUNT];
};

/* #define LIBANE_STFU_LOG */
/* #define LIBANE_STFU_ERR */
/* #define LIBANE_INDEX_CHECK */
/* #define LIBANE_NO_STATIC_ASSERT */

#if defined(__cplusplus)
}
#endif

#endif /* __ANE_DEV_H__ */
