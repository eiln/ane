// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_CHAN_H__
#define __ANE_CHAN_H__

#include "ane_dev.h"

int ane_chan_init(struct ane_device *ane, struct ane_nn *nn);
int ane_chan_free(struct ane_device *ane, struct ane_nn *nn);

#endif /* __ANE_CHAN_H__ */
