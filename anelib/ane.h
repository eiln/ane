// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H__
#define __ANE_H__

#include <asm/types.h>
#include <errno.h>
#include <stdint.h>

#include "ane_dev.h"
#include "ane_fp16.h"
#include "ane_utils.h"

struct ane_nn *ane_register(const struct ane_model *model, void *anec_data);
void ane_destroy(struct ane_nn *nn);
int ane_exec(struct ane_nn *nn);

#define input_chan(nn, i)  (nn->chans[nn->imask[i]])
#define output_chan(nn, i) (nn->chans[nn->omask[i]])

#define input_size(nn, i)  (to_anec(nn)->tiles[nn->imask[i]] * TILE_SIZE)
#define output_size(nn, i) (to_anec(nn)->tiles[nn->omask[i]] * TILE_SIZE)

static inline void ane_send(struct ane_nn *nn, void *input_data, int chan)
{
	memcpy(input_chan(nn, chan), input_data, input_size(nn, chan));
	return;
}

static inline void ane_read(struct ane_nn *nn, void *output_data, int chan)
{
	memcpy(output_data, output_chan(nn, chan), output_size(nn, chan));
	return;
}

#endif /* __ANE_H__ */
