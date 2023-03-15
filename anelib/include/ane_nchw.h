// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_NCHW_H__
#define __ANE_NCHW_H__

#include "ane_dev.h"
#include "ane_f16.h"

// clang-format off

// nchw_data_size (N * C * H * W * sizeof(f16))
#define nchw_data_size(nchw) (nchw[0] * nchw[1] * nchw[2] * nchw[3] * sizeof(f16))
// nchw_tile_size (tile_align(N * C * P))
#define nchw_tile_size(nchw) (tile_align(nchw[0] * nchw[1] * nchw[4]))

static inline void nchw_tile(void *data, void *tile, uint64_t nchw[6])
{
	uint64_t N = nchw[0], C = nchw[1], H = nchw[2], W = nchw[3],
		 P = nchw[4], R = nchw[5];

	uint64_t new_H = P / R;
	uint64_t new_W = R / sizeof(f16);
	uint64_t stride = W * sizeof(f16);

	f16(*data_a)[N][C][H][W] = (f16(*)[N][C][H][W])data;
	f16(*tile_a)[N][C][new_H][new_W] = (f16(*)[N][C][new_H][new_W])tile;
	memset(tile_a, 0, nchw_tile_size(nchw));

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				memcpy((*tile_a)[n][c][h], (*data_a)[n][c][h], stride);
			}
		}
	}
}

static inline void nchw_untile(void *data, void *tile, uint64_t nchw[6])
{
	uint64_t N = nchw[0], C = nchw[1], H = nchw[2], W = nchw[3],
		 P = nchw[4], R = nchw[5];

	uint64_t new_H = P / R;
	uint64_t new_W = R / sizeof(f16);
	uint64_t stride = W * sizeof(f16);

	f16(*data_a)[N][C][H][W] = (f16(*)[N][C][H][W])data;
	f16(*tile_a)[N][C][new_H][new_W] = (f16(*)[N][C][new_H][new_W])tile;
	memset(data_a, 0, nchw_data_size(nchw));

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				memcpy((*data_a)[n][c][h], (*tile_a)[n][c][h], stride);
			}
		}
	}
}

// clang-format on

#endif /* __ANE_NCHW_H__ */
