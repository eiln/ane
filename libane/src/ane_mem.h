// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_MEM_H__
#define __ANE_MEM_H__

#include <stdlib.h>

#include "ane_priv.h"

static inline void *ane_malloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL) {
		ane_err("failed to alloc size 0x%zx\n", size);
		return NULL;
	}
	return ptr;
}

static inline void *ane_zmalloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL) {
		ane_err("failed to alloc size 0x%zx\n", size);
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

#endif /* __ANE_MEM_H__ */
