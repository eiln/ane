// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_TILE_H__
#define __ANE_TILE_H__

#include "ane_f16.h"
#include "ane_mem.h"

static inline void ane_tile(void *data, void *tile, const uint64_t N,
			    const uint64_t C, const uint64_t H,
			    const uint64_t W, const uint64_t P,
			    const uint64_t R)
{
	const uint64_t new_H = P / R;
	const uint64_t new_W = R / sizeof(f16);
	const uint64_t stride = W * sizeof(f16);
	const uint64_t tile_size = tile_align(N * C * P);

	f16(*data_a)[N][C][H][W] = (f16(*)[N][C][H][W])data;
	f16(*tile_a)[N][C][new_H][new_W] = (f16(*)[N][C][new_H][new_W])tile;
	memset(tile, 0, tile_size);

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				memcpy((*tile_a)[n][c][h], (*data_a)[n][c][h],
				       stride);
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
	const uint64_t new_W = R / sizeof(f16);
	const uint64_t stride = W * sizeof(f16);
	const uint64_t data_size = N * C * H * W * sizeof(f16);

	f16(*data_a)[N][C][H][W] = (f16(*)[N][C][H][W])data;
	f16(*tile_a)[N][C][new_H][new_W] = (f16(*)[N][C][new_H][new_W])tile;
	memset(data, 0, data_size);

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				memcpy((*data_a)[n][c][h], (*tile_a)[n][c][h],
				       stride);
			}
		}
	}
	return;
}

#endif /* __ANE_TILE_H__ */
