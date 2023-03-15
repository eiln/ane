// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_BUF_H__
#define __ANE_BUF_H__

#include "ane_nchw.h"
#include "ane_utils.h"

struct ane_buf {
	int bdx;
	uint64_t nchw[6];
	void *data;
	void *tile;
	size_t data_size;
	size_t tile_size;
};

static inline struct ane_buf *ane_buf_init(struct ane_nn *nn, int bdx)
{
	struct ane_buf *buf = ane_zmalloc(sizeof(struct ane_buf));
	if (!buf)
		return NULL;

	buf->bdx = bdx;
	for (int i = 0; i < 6; i++)
		buf->nchw[i] = nn->model->nchw[bdx][i];

	buf->data_size = nchw_data_size(buf->nchw);
	buf->data = ane_zmemalign(buf->data_size);
	if (!buf->data) {
		free(buf);
		return NULL;
	}

	buf->tile_size = nchw_tile_size(buf->nchw);
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

static inline struct ane_buf *ane_buf_init_input(struct ane_nn *nn, int idx)
{
	if (idx >= input_count(nn))
		return NULL;
	return ane_buf_init(nn, nn->src_bdx[idx]);
}

static inline struct ane_buf *ane_buf_init_output(struct ane_nn *nn, int idx)
{
	if (idx >= output_count(nn))
		return NULL;
	return ane_buf_init(nn, nn->dst_bdx[idx]);
}

static inline void ane_buf_send(struct ane_nn *nn, struct ane_buf *buf)
{
	memcpy(nn->chans[buf->bdx], buf->tile, buf->tile_size);
}

static inline void ane_buf_read(struct ane_nn *nn, struct ane_buf *buf)
{
	memcpy(buf->tile, nn->chans[buf->bdx], buf->tile_size);
}

static inline void ane_buf_tile(struct ane_buf *buf)
{
	nchw_tile(buf->data, buf->tile, buf->nchw);
}

static inline void ane_buf_untile(struct ane_buf *buf)
{
	nchw_untile(buf->data, buf->tile, buf->nchw);
}

#endif /* __ANE_BUF_H__ */
