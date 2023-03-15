// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H__
#define __ANE_H__

#include "ane_dev.h"
#include "ane_utils.h"

struct ane_nn *ane_init(const struct ane_model *model, void *anec_buf);
void ane_free(struct ane_nn *nn);
int ane_exec(struct ane_nn *nn);

#define input_size(nn, idx)  (tile_size(to_anec(nn)->tiles[nn->src_bdx[idx]]))
#define output_size(nn, idx) (tile_size(to_anec(nn)->tiles[nn->dst_bdx[idx]]))

static inline void ane_send(struct ane_nn *nn, void *input, int idx)
{
	memcpy(nn->chans[nn->src_bdx[idx]], input, input_size(nn, idx));
}

static inline void ane_read(struct ane_nn *nn, void *output, int idx)
{
	memcpy(output, nn->chans[nn->dst_bdx[idx]], output_size(nn, idx));
}

#endif /* __ANE_H__ */
