// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_ARR_H__
#define __ANE_ARR_H__

#include "ane_dev.h"
#include "ane_f16.h"
#include "ane_utils.h"

#define _N(nchw) (nchw[0])
#define _C(nchw) (nchw[1])
#define _H(nchw) (nchw[2])
#define _W(nchw) (nchw[3])
#define _P(nchw) (nchw[4])
#define _R(nchw) (nchw[5])

// clang-format off
#define __nchw_data_size(nchw) (_N(nchw) * _C(nchw) * _H(nchw) * _W(nchw) * sizeof(f16))
#define __nchw_tile_size(nchw) (tile_align(_N(nchw) * _C(nchw) * _P(nchw)))
// clang-format on

static inline void ane_tile(void *data, void *tile, uint64_t nchw[6])
{
	uint64_t N = nchw[0], C = nchw[1], H = nchw[2], W = nchw[3],
		 P = nchw[4], R = nchw[5];

	uint64_t new_H = P / R;
	uint64_t new_W = R / sizeof(f16);
	uint64_t stride = W * sizeof(f16);

	f16(*data_a)[N][C][H][W] = (f16(*)[N][C][H][W])data;
	f16(*tile_a)[N][C][new_H][new_W] = (f16(*)[N][C][new_H][new_W])tile;
	memset(tile_a, 0, __nchw_tile_size(nchw));

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				memcpy((*tile_a)[n][c][h], (*data_a)[n][c][h],
				       stride);
			}
		}
	}
}

static inline void ane_untile(void *data, void *tile, uint64_t nchw[6])
{
	uint64_t N = nchw[0], C = nchw[1], H = nchw[2], W = nchw[3],
		 P = nchw[4], R = nchw[5];

	uint64_t new_H = P / R;
	uint64_t new_W = R / sizeof(f16);
	uint64_t stride = W * sizeof(f16);

	f16(*data_a)[N][C][H][W] = (f16(*)[N][C][H][W])data;
	f16(*tile_a)[N][C][new_H][new_W] = (f16(*)[N][C][new_H][new_W])tile;
	memset(data_a, 0, __nchw_data_size(nchw));

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				memcpy((*data_a)[n][c][h], (*tile_a)[n][c][h],
				       stride);
			}
		}
	}
}

static inline struct ane_arr *__arr_init(struct ane_nn *nn, int bdx)
{
	struct ane_arr *arr = ane_zmalloc(sizeof(struct ane_arr));
	if (!arr)
		goto error;
	for (int i = 0; i < 6; i++)
		arr->nchw[i] = nn->model->nchw[bdx][i];
	arr->data = ane_zmemalign(__nchw_data_size(arr->nchw));
	if (!arr->data)
		goto free_arr;
	arr->tile = ane_zmemalign(__nchw_tile_size(arr->nchw));
	if (!arr->tile)
		goto free_data;
	return arr;

free_data:
	free(arr->data);
free_arr:
	free(arr);
error:
	return NULL;
}

static void ane_arrs_free(struct ane_nn *nn)
{
	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		struct ane_arr *arr = nn->arrs[bdx];
		if (arr) {
			free(arr->data);
			free(arr->tile);
			free(arr);
			nn->arrs[bdx] = NULL;
		}
	}
}

static int ane_arrs_init(struct ane_nn *nn)
{
	for (int i = 0; i < input_count(nn); i++) {
		int bdx = nn->src_bdx[i];
		struct ane_arr *arr = __arr_init(nn, bdx);
		if (arr == NULL)
			goto error;
		nn->arrs[bdx] = arr;
	}
	for (int i = 0; i < output_count(nn); i++) {
		int bdx = nn->dst_bdx[i];
		struct ane_arr *arr = __arr_init(nn, bdx);
		if (arr == NULL)
			goto error;
		nn->arrs[bdx] = arr;
	}
	return 0;

error:
	ane_arrs_free(nn);
	return -ENOMEM;
}

#define input_arr(nn, i)    (nn->arrs[nn->src_bdx[i]])
#define output_arr(nn, i)   (nn->arrs[nn->dst_bdx[i]])

#define input_data(nn, i)   (input_arr(nn, i)->data)
#define output_data(nn, i)  (output_arr(nn, i)->data)

#define input_dsize(nn, i)  (__nchw_data_size(input_arr(nn, i)->nchw))
#define output_dsize(nn, i) (__nchw_data_size(output_arr(nn, i)->nchw))

static inline void ane_arrs_tile(struct ane_nn *nn)
{
	for (int i = 0; i < input_count(nn); i++) {
		struct ane_arr *arr = input_arr(nn, i);
		ane_tile(arr->data, arr->tile, arr->nchw);
	}
}

static inline void ane_arrs_untile(struct ane_nn *nn)
{
	for (int i = 0; i < output_count(nn); i++) {
		struct ane_arr *arr = output_arr(nn, i);
		ane_untile(arr->data, arr->tile, arr->nchw);
	}
}

static inline void ane_arrs_send(struct ane_nn *nn)
{
	for (int i = 0; i < input_count(nn); i++) {
		struct ane_arr *arr = input_arr(nn, i);
		ane_send(nn, arr->tile, i);
	}
}

static inline void ane_arrs_read(struct ane_nn *nn)
{
	for (int i = 0; i < output_count(nn); i++) {
		struct ane_arr *arr = output_arr(nn, i);
		ane_read(nn, arr->tile, i);
	}
}

#endif /* __ANE_ARR_H__ */
