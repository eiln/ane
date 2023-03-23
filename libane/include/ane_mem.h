// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_MEM_H__
#define __ANE_MEM_H__

#include <stdlib.h>
#include <string.h>

static inline void *ane_zmalloc(unsigned long size)
{
	void *data = malloc(size);
	if (data == NULL)
		return NULL;
	memset(data, 0, size);
	return data;
}

static inline void *ane_memalign(unsigned long size)
{
	void *data;
	if (posix_memalign(&data, 0x4000UL, size))
		return NULL;
	return data;
}

static inline void *ane_zmemalign(unsigned long size)
{
	void *data = ane_memalign(size);
	if (data == NULL)
		return NULL;
	memset(data, 0, size);
	return data;
}

#endif /* __ANE_MEM_H__ */
