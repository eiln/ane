// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H__
#define __ANE_H__

#if defined(__cplusplus)
extern "C" {
#endif

/*
// Asahi Neural Engine (ANE) Userspace Library
//
// USAGE:

	Simply drop "ane.h" with libane.a (generated from ane.c).

	#include "ane.h" // only requires <stdint.h>

	int main(void)
	{
		struct ane_nn *nn = ane_init("model.anec"); // init network from anec format
		if (!nn) {
			printf("failed to load model\n");
			return -1;
		}

		void *input0 = malloc(ane_src_size(nn, 0)); // get size of 0th index input
		ane_send(nn, input0, 0); // send 0th index input to device
		ane_send(nn, input1, 1); // send 1st index input to device

		if (ane_exec(nn) < 0) { // call exec after sending all inputs
			printf("execution failed\n");
		}

		ane_read(nn, output0, 0); // receive 0th index output from device

		ane_free(nn); // inverse of ane_init()
		return 0;
	}

	Compiles with gcc or g++:

	gcc -I/usr/include/libane main.c /usr/lib/libane.a  # or -lane

*/

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
	void *map; /* mmap-ed CPU virtual address */
	uint64_t size; /* size of mmap region */
	uint32_t handle; /* drm gem handle */
	uint64_t offset; /* drm gem fake offset for mmap */
};

struct ane_nn {
	int fd; /* file descriptor to accel node (index dev_id) */
	void *data; /* anec content loaded from path */
	struct anec anec; /* anec header loaded from path */
	struct ane_bo chans[TILE_COUNT]; /* mmap-ed tile channels */
	struct ane_bo btsp_chan; /* mmap-ed bootstrap channel */
};

/* #define LIBANE_CONFIG_NO_ERR */
/* #define LIBANE_CONFIG_NO_INDEX_CHECK */
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

struct ane_nn *__ane_init(const char *path, int dev_id);
#define ane_init(path) (__ane_init(path, 0))

void __ane_free(struct ane_nn *nn);
#define ane_free(nn) (__ane_free(nn))

int ane_exec(struct ane_nn *nn);

#define to_anec(nn)	  (&nn->anec)
#define ane_src_count(nn) (to_anec(nn)->src_count)
#define ane_dst_count(nn) (to_anec(nn)->dst_count)

#define _LIBANE_SF(func, nn, idx, ...)                  \
	({                                              \
		LIBANE_STATIC_ASSERT(idx < TILE_COUNT); \
		func(nn, ##__VA_ARGS__, idx);           \
	})

uint64_t __ane_src_size(struct ane_nn *nn, const uint32_t idx);
uint64_t __ane_dst_size(struct ane_nn *nn, const uint32_t idx);
#define ane_src_size(nn, idx) _LIBANE_SF(__ane_src_size, nn, idx)
#define ane_dst_size(nn, idx) _LIBANE_SF(__ane_dst_size, nn, idx)

void __ane_send(struct ane_nn *nn, void *from, const uint32_t idx);
void __ane_read(struct ane_nn *nn, void *to, const uint32_t idx);
#define ane_send(nn, from, idx) _LIBANE_SF(__ane_send, nn, idx, from)
#define ane_read(nn, to, idx)	_LIBANE_SF(__ane_read, nn, idx, to)

void __ane_tile_send(struct ane_nn *nn, void *from, const uint32_t idx);
void __ane_tile_read(struct ane_nn *nn, void *to, const uint32_t idx);
#define ane_tile_send(nn, from, idx) _LIBANE_SF(__ane_tile_send, nn, idx, from)
#define ane_tile_read(nn, to, idx)   _LIBANE_SF(__ane_tile_read, nn, idx, to)

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
