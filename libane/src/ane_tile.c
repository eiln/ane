// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include "ane_priv.h"

// clang-format off
static inline void ane_tile(void *data, void *tile, const uint64_t N,
			    const uint64_t C, const uint64_t H,
			    const uint64_t W, const uint64_t P,
			    const uint64_t R)
{
	const uint64_t new_H = P / R;
	const uint64_t new_W = R / sizeof(uint16_t);
	const uint64_t stride = W * sizeof(uint16_t);

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				void *src = ((void *)(data)) + ((n*(C*H*W) + c*(H*W) + h*(W)) * sizeof(uint16_t));
				void *dst = ((void *)(tile)) + ((n*(C*new_H*new_W) + c*(new_H*new_W) + h*(new_W)) * sizeof(uint16_t));
				memcpy(dst, src, stride);
			}
		}
	}

	return;
}

static inline void ane_untile(void *data, void *tile, const uint64_t N,
			      const uint64_t C, const uint64_t H,
			      const uint64_t W, const uint64_t P,
			      const uint64_t R)
{
	const uint64_t new_H = P / R;
	const uint64_t new_W = R / sizeof(uint16_t);
	const uint64_t stride = W * sizeof(uint16_t);

	memset(data, 0, N * C * H * W * sizeof(uint16_t));

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				void *src = ((void *)(tile)) + ((n*(C*new_H*new_W) + c*(new_H*new_W) + h*(new_W)) * sizeof(uint16_t));
				void *dst = ((void *)(data)) + ((n*(C*H*W) + c*(H*W) + h*(W)) * sizeof(uint16_t));
				memcpy(dst, src, stride);
			}
		}
	}

	return;
}
// clang-format on

static inline void tile_send(struct ane_nn *nn, void *from, const int idx)
{
	const struct ane_model *model = nn->model;
	const int bdx = nn->src_bdx[idx];

	uint16_t tile[tile_size(nn, bdx) / sizeof(uint16_t)];
	memset(tile, 0, tile_size(nn, bdx));

	ane_tile(from, tile, model->nchw[bdx][0], model->nchw[bdx][1],
		 model->nchw[bdx][2], model->nchw[bdx][3], model->nchw[bdx][4],
		 model->nchw[bdx][5]);
	memcpy(nn->chans[bdx].map, tile, tile_size(nn, bdx));
}

static inline void tile_read(struct ane_nn *nn, void *to, const int idx)
{
	const struct ane_model *model = nn->model;
	const int bdx = nn->dst_bdx[idx];

	uint16_t tile[tile_size(nn, bdx) / sizeof(uint16_t)];
	memcpy(tile, nn->chans[bdx].map, tile_size(nn, bdx));

	ane_untile(to, tile, model->nchw[bdx][0], model->nchw[bdx][1],
		   model->nchw[bdx][2], model->nchw[bdx][3],
		   model->nchw[bdx][4], model->nchw[bdx][5]);
}

void ane_tile_send(struct ane_nn *nn, void *from, const int idx)
{
	tile_send(nn, from, idx);
}

void ane_tile_read(struct ane_nn *nn, void *to, const int idx)
{
	tile_read(nn, to, idx);
}
