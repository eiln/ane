// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_TILE_H__
#define __ANE_TILE_H__

#include "ane_dev.h"

int ane_tile_send(struct ane_nn *nn, void *from, const int idx);
int ane_tile_read(struct ane_nn *nn, void *to, const int idx);

#define ane_chan_send(nn, from, idx)                    \
	(memcpy(nn->chans[nn->src_bdx[idx]]->map, from, \
		tile_size(nn, nn->src_bdx[idx])))
#define ane_chan_read(nn, to, idx)                    \
	(memcpy(to, nn->chans[nn->dst_bdx[idx]]->map, \
		tile_size(nn, nn->dst_bdx[idx])))

#endif /* __ANE_TILE_H__ */
