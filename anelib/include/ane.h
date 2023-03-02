// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H__
#define __ANE_H__

#include "ane_dev.h"
#include "ane_utils.h"

struct ane_nn *ane_init(const struct ane_model *model, void *cmd_data);
void ane_free(struct ane_nn *nn);
int ane_exec(struct ane_nn *nn);

#define input_chan(nn, i)  (nn->chans[nn->src_bdx[i]])
#define output_chan(nn, i) (nn->chans[nn->dst_bdx[i]])

#define input_size(nn, i)  (tile_size(to_anec(nn)->tiles[nn->src_bdx[i]]))
#define output_size(nn, i) (tile_size(to_anec(nn)->tiles[nn->dst_bdx[i]]))

static inline void ane_send(struct ane_nn *nn, void *input, int chan)
{
	memcpy(input_chan(nn, chan), input, input_size(nn, chan));
}

static inline void ane_read(struct ane_nn *nn, void *output, int chan)
{
	memcpy(output, output_chan(nn, chan), output_size(nn, chan));
}

#endif /* __ANE_H__ */
