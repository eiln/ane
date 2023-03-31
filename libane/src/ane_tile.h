// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_TILE_H__
#define __ANE_TILE_H__

#include "ane_dev.h"

void ane_tile_send(struct ane_nn *nn, void *from, const int idx, const int raw);
void ane_tile_read(struct ane_nn *nn, void *to, const int idx, const int raw);

#endif /* __ANE_TILE_H__ */
