// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_UTILS_H__
#define __ANE_UTILS_H__

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TILE_SHIFT	0xEUL
#define TILE_SIZE	0x4000UL

#define tile_shift(x)	(((uint64_t)(x)) << TILE_SHIFT)
#define tile_align(x)	((((uint64_t)(x)) + TILE_SIZE - 1) & -TILE_SIZE)

#define ane_log(a, ...) printf("TEST: LOG: " a, ##__VA_ARGS__)
#define ane_err(a, ...) fprintf(stderr, "TEST: ERR: " a, ##__VA_ARGS__)

static inline void *ane_malloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL) {
		ane_err("failed to malloc size 0x%zx\n", size);
		return NULL;
	}
	return ptr;
}

static inline void *ane_zmalloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL) {
		ane_err("failed to malloc size 0x%zx\n", size);
		return NULL;
	}
	memset(ptr, 0, size);
	return ptr;
}

static inline void *ane_memalign(size_t size)
{
	void *ptr = NULL;
	if (posix_memalign(&ptr, TILE_SIZE, size)) {
		ane_err("failed to memalign size 0x%zx\n", size);
		return NULL;
	}
	return ptr;
}

static inline void *ane_zmemalign(size_t size)
{
	void *ptr = NULL;
	if (posix_memalign(&ptr, TILE_SIZE, size)) {
		ane_err("failed to memalign size 0x%zx\n", size);
		return NULL;
	}
	memset(ptr, 0, size);
	return ptr;
}

#endif /* __ANE_UTILS_H__ */
