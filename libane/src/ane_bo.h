// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_BO_H__
#define __ANE_BO_H__

#include "ane_dev.h"

struct ane_bo *ane_bo_init(struct ane_device *ane, uint64_t size);
int ane_bo_free(struct ane_device *ane, struct ane_bo *bo);

#endif /* __ANE_BO_H__ */
