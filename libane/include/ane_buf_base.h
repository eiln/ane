// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_BUF_BASE_H__
#define __ANE_BUF_BASE_H__

#include <stdint.h>

#include "ane_f16.h"
#include "ane_mem.h"
#include "ane_tile.h"
#include "ane_utils.h"

struct ane_buf {
	void *data;
	void *tile;
	const uint64_t n;
	const uint64_t c;
	const uint64_t h;
	const uint64_t w;
	const uint64_t p;
	const uint64_t r;
	const uint64_t elem;
	const uint64_t data_size;
	const uint64_t tile_size;
};

static inline struct ane_buf *ane_buf_alloc(const uint64_t n, const uint64_t c,
					    const uint64_t h, const uint64_t w,
					    const uint64_t p, const uint64_t r)
{
	struct ane_buf *buf;
	buf = (struct ane_buf *)ane_zmalloc(sizeof(struct ane_buf));
	if (!buf)
		return NULL;

	*(uint64_t *)&buf->n = n;
	*(uint64_t *)&buf->c = c;
	*(uint64_t *)&buf->h = h;
	*(uint64_t *)&buf->w = w;
	*(uint64_t *)&buf->p = p;
	*(uint64_t *)&buf->r = r;

	*(uint64_t *)&buf->elem = n * c * h * w;
	*(uint64_t *)&buf->data_size = n * c * h * w * sizeof(uint16_t);
	*(uint64_t *)&buf->tile_size = tile_align(n * c * p);

	buf->data = ane_zmemalign(buf->data_size);
	if (!buf->data) {
		free(buf);
		return NULL;
	}

	buf->tile = ane_zmemalign(buf->tile_size);
	if (!buf->tile) {
		free(buf->data);
		free(buf);
		return NULL;
	}

	return buf;
}

static inline void ane_buf_free(struct ane_buf *buf)
{
	free(buf->tile);
	free(buf->data);
	free(buf);
}

static inline uint16_t ane_buf_get(const struct ane_buf *buf, const uint64_t n,
				   const uint64_t c, const uint64_t h,
				   const uint64_t w)
{
	uint16_t(*array)[buf->n][buf->c][buf->h][buf->w] =
		(uint16_t(*)[buf->n][buf->c][buf->h][buf->w])buf->data;
	return (*array)[n][c][h][w];
}

static inline void ane_buf_set(const struct ane_buf *buf, const uint64_t n,
			       const uint64_t c, const uint64_t h,
			       const uint64_t w, const uint16_t val)
{
	uint16_t(*array)[buf->n][buf->c][buf->h][buf->w] =
		(uint16_t(*)[buf->n][buf->c][buf->h][buf->w])buf->data;
	(*array)[n][c][h][w] = val;
}

static inline void ane_buf_set_all(const struct ane_buf *buf,
				   const uint16_t val)
{
	uint16_t *data = (uint16_t *)buf->data;
	for (uint64_t i = 0; i < buf->elem; i++) {
		data[i] = val;
	}
}

static inline void ane_buf_zero(const struct ane_buf *buf)
{
	memset(buf->data, 0, buf->data_size);
}

static inline void ane_buf_tile(const struct ane_buf *buf)
{
	ane_tile(buf->data, buf->tile, buf->n, buf->c, buf->h, buf->w, buf->p,
		 buf->r);
}

static inline void ane_buf_untile(const struct ane_buf *buf)
{
	ane_untile(buf->data, buf->tile, buf->n, buf->c, buf->h, buf->w, buf->p,
		   buf->r);
}

static inline void ane_buf_fread(const struct ane_buf *buf, char *fname)
{
	ane_file_read(buf->data, buf->data_size, fname);
}

static inline void ane_buf_fwrite(const struct ane_buf *buf, char *fname)
{
	ane_file_write(buf->data, buf->data_size, fname);
}

static inline void ane_buf_from_float(const struct ane_buf *buf, float *src)
{
	uint16_t *dst = (uint16_t *)buf->data;
	for (uint64_t i = 0; i < buf->elem; i++) {
		dst[i] = float_to_half(src[i]);
	}
}

static inline void ane_buf_to_float(const struct ane_buf *buf, float *dst)
{
	uint16_t *src = (uint16_t *)buf->data;
	for (uint64_t i = 0; i < buf->elem; i++) {
		dst[i] = half_to_float(src[i]);
	}
}

#endif /* __ANE_BUF_BASE_H__ */
