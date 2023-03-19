// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_BUF_H__
#define __ANE_BUF_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "ane_buf_base.h"
#include "ane_dev.h"
#include "ane_mem.h"

static inline struct ane_buf *ane_buf_alloc_input(struct ane_nn *nn,
						  const int idx)
{
	int bdx = nn->src_bdx[idx];
	const struct ane_model *model = nn->model;
	return ane_buf_alloc(model->nchw[bdx][0], model->nchw[bdx][1],
			     model->nchw[bdx][2], model->nchw[bdx][3],
			     model->nchw[bdx][4], model->nchw[bdx][5]);
}

static inline struct ane_buf *ane_buf_alloc_output(struct ane_nn *nn,
						   const int idx)
{
	int bdx = nn->dst_bdx[idx];
	const struct ane_model *model = nn->model;
	return ane_buf_alloc(model->nchw[bdx][0], model->nchw[bdx][1],
			     model->nchw[bdx][2], model->nchw[bdx][3],
			     model->nchw[bdx][4], model->nchw[bdx][5]);
}

static inline void ane_buf_send(struct ane_nn *nn, const struct ane_buf *buf,
				const int idx)
{
	memcpy(nn->chans[nn->src_bdx[idx]], buf->tile, buf->tile_size);
}

static inline void ane_buf_read(struct ane_nn *nn, const struct ane_buf *buf,
				const int idx)
{
	memcpy(buf->tile, nn->chans[nn->dst_bdx[idx]], buf->tile_size);
}

#if defined(__cplusplus)
}
#endif

#endif /* __ANE_BUF_H__ */
