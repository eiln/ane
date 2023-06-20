// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H__
#define __ANE_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

#define TILE_COUNT 0x20

struct anec {
	const uint64_t size;
	const uint32_t td_size;
	const uint32_t td_count;
	const uint64_t tsk_size;
	const uint64_t krn_size;
	const uint32_t src_count;
	const uint32_t dst_count;
	const uint32_t tiles[TILE_COUNT];
	const uint64_t nchw[TILE_COUNT][6];
} __attribute__((__packed__, aligned(1)));

struct ane_bo {
	void *map;
	uint64_t size;
	uint32_t handle;
	uint64_t offset;
};

struct ane_nn {
	int fd;
	void *data;
	struct anec anec;
	struct ane_bo chans[TILE_COUNT];
	struct ane_bo btsp_chan;
};

/* #define LIBANE_CONFIG_STFU_LOG */
/* #define LIBANE_CONFIG_STFU_ERR */
/* #define LIBANE_CONFIG_INDEX_CHECK */
/* #define LIBANE_CONFIG_NO_STATIC_ASSERT */

#ifndef LIBANE_CONFIG_NO_STATIC_ASSERT
#ifdef __cplusplus
#ifndef _Static_assert
#define _Static_assert static_assert
#endif /* _Static_assert */
#endif /* __cplusplus */
#define LIBANE_STATIC_ASSERT(test_for_true) \
	_Static_assert((test_for_true), "(" #test_for_true ") failed")
#else
#define LIBANE_STATIC_ASSERT(test_for_true) \
	do {                                \
	} while (0)
#endif /* LIBANE_CONFIG_NO_STATIC_ASSERT */

int ane_open(int dev_id);
void ane_close(int fd);

struct ane_nn *__ane_init(const char *path, int dev_id);
#define ane_init(path) (__ane_init(path, 0))

void __ane_free(struct ane_nn *nn);
#define ane_free(nn) (__ane_free(nn))

int ane_exec(struct ane_nn *nn);

#define to_anec(nn)	  (&nn->anec)
#define ane_src_count(nn) (to_anec(nn)->src_count)
#define ane_dst_count(nn) (to_anec(nn)->dst_count)

#define LIBANE_IDX_SAF(func, nn, idx, ...)              \
	({                                              \
		LIBANE_STATIC_ASSERT(idx < TILE_COUNT); \
		func(nn, __VA_ARGS__, idx);             \
	})

// clang-format off
void __ane_send(struct ane_nn *nn, void *from, const int idx);
void __ane_read(struct ane_nn *nn, void *to, const int idx);
#define ane_send(nn, from, idx) LIBANE_IDX_SAF(__ane_send, nn, idx, from)
#define ane_read(nn, to, idx)	LIBANE_IDX_SAF(__ane_read, nn, idx, to)

void __ane_tile_send(struct ane_nn *nn, void *from, const int idx);
void __ane_tile_read(struct ane_nn *nn, void *to, const int idx);
#define ane_tile_send(nn, from, idx) LIBANE_IDX_SAF(__ane_tile_send, nn, idx, from)
#define ane_tile_read(nn, to, idx)   LIBANE_IDX_SAF(__ane_tile_read, nn, idx, to)

void *__ane_src_chan(struct ane_nn *nn, const int idx);
void *__ane_dst_chan(struct ane_nn *nn, const int idx);
#define ane_src_chan(nn, idx) LIBANE_IDX_SAF(__ane_src_chan, nn, idx)
#define ane_dst_chan(nn, idx) LIBANE_IDX_SAF(__ane_dst_chan, nn, idx)

uint64_t __ane_src_size(struct ane_nn *nn, const int idx);
uint64_t __ane_dst_size(struct ane_nn *nn, const int idx);
#define ane_src_size(nn, idx) LIBANE_IDX_SAF(__ane_src_size, nn, idx)
#define ane_dst_size(nn, idx) LIBANE_IDX_SAF(__ane_dst_size, nn, idx)

uint64_t __ane_src_shape_n(struct ane_nn *nn, const int idx);
uint64_t __ane_src_shape_c(struct ane_nn *nn, const int idx);
uint64_t __ane_src_shape_h(struct ane_nn *nn, const int idx);
uint64_t __ane_src_shape_w(struct ane_nn *nn, const int idx);
#define ane_src_shape_n(nn, idx) LIBANE_IDX_SAF(__ane_src_shape_n, nn, idx)
#define ane_src_shape_c(nn, idx) LIBANE_IDX_SAF(__ane_src_shape_c, nn, idx)
#define ane_src_shape_h(nn, idx) LIBANE_IDX_SAF(__ane_src_shape_h, nn, idx)
#define ane_src_shape_w(nn, idx) LIBANE_IDX_SAF(__ane_src_shape_w, nn, idx)

uint64_t __ane_dst_shape_n(struct ane_nn *nn, const int idx);
uint64_t __ane_dst_shape_c(struct ane_nn *nn, const int idx);
uint64_t __ane_dst_shape_h(struct ane_nn *nn, const int idx);
uint64_t __ane_dst_shape_w(struct ane_nn *nn, const int idx);
#define ane_dst_shape_n(nn, idx) LIBANE_IDX_SAF(__ane_dst_shape_n, nn, idx)
#define ane_dst_shape_c(nn, idx) LIBANE_IDX_SAF(__ane_dst_shape_c, nn, idx)
#define ane_dst_shape_h(nn, idx) LIBANE_IDX_SAF(__ane_dst_shape_h, nn, idx)
#define ane_dst_shape_w(nn, idx) LIBANE_IDX_SAF(__ane_dst_shape_w, nn, idx)
// clang-format on

void ane_tile(void *data, void *tile, const uint64_t N, const uint64_t C,
	      const uint64_t H, const uint64_t W, const uint64_t P,
	      const uint64_t R);
void ane_untile(void *data, void *tile, const uint64_t N, const uint64_t C,
		const uint64_t H, const uint64_t W, const uint64_t P,
		const uint64_t R);

#if defined(__cplusplus)
}
#endif

#endif /* __ANE_H__ */
