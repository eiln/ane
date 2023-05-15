// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H__
#define __ANE_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "ane_dev.h"

#ifndef LIBANE_NO_STATIC_ASSERT
#ifdef __cplusplus
#ifndef _Static_assert
#define _Static_assert static_assert
#endif /* _Static_assert */
#endif /* __cplusplus */
#define STATIC_ASSERT(test_for_true) \
	_Static_assert((test_for_true), "(" #test_for_true ") failed")
#else
#define STATIC_ASSERT(test_for_true) \
	do {                         \
	} while (0)
#endif /* LIBANE_NO_STATIC_ASSERT */

int ane_open(void);
void ane_close(int fd);

struct ane_nn *__ane_init(const struct ane_model *model, int fd);
struct ane_nn *ane_init(const struct ane_model *model);

void __ane_free(struct ane_nn *nn);
void ane_free(struct ane_nn *nn);

int ane_exec(struct ane_nn *nn);

void __ane_send(struct ane_nn *nn, void *from, const int idx);
void __ane_read(struct ane_nn *nn, void *to, const int idx);

void __ane_tile_send(struct ane_nn *nn, void *from, const int idx);
void __ane_tile_read(struct ane_nn *nn, void *to, const int idx);

#define ane_send(nn, from, idx)                  \
	({                                       \
		STATIC_ASSERT(idx < TILE_COUNT); \
		__ane_send(nn, from, idx);       \
	})

#define ane_read(nn, to, idx)                    \
	({                                       \
		STATIC_ASSERT(idx < TILE_COUNT); \
		__ane_read(nn, to, idx);         \
	})

#define ane_tile_send(nn, from, idx)             \
	({                                       \
		STATIC_ASSERT(idx < TILE_COUNT); \
		__ane_tile_send(nn, from, idx);  \
	})

#define ane_tile_read(nn, to, idx)               \
	({                                       \
		STATIC_ASSERT(idx < TILE_COUNT); \
		__ane_tile_read(nn, to, idx);    \
	})

void *__ane_src_chan(struct ane_nn *nn, const int idx);
void *__ane_dst_chan(struct ane_nn *nn, const int idx);

uint64_t __ane_src_size(struct ane_nn *nn, const int idx);
uint64_t __ane_dst_size(struct ane_nn *nn, const int idx);

#define ane_src_chan(nn, idx)                    \
	({                                       \
		STATIC_ASSERT(idx < TILE_COUNT); \
		__ane_src_chan(nn, idx);         \
	})

#define ane_dst_chan(nn, idx)                    \
	({                                       \
		STATIC_ASSERT(idx < TILE_COUNT); \
		__ane_dst_chan(nn, idx);         \
	})

#define ane_src_size(nn, idx)                    \
	({                                       \
		STATIC_ASSERT(idx < TILE_COUNT); \
		__ane_src_size(nn, idx);         \
	})

#define ane_dst_size(nn, idx)                    \
	({                                       \
		STATIC_ASSERT(idx < TILE_COUNT); \
		__ane_dst_size(nn, idx);         \
	})

#if defined(__cplusplus)
}
#endif

#endif /* __ANE_H__ */
