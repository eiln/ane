// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_DRV_H__
#define __ANE_DRV_H__

#include "ane_dev.h"

int ane_drv_device_open(struct ane_device *ane);
int ane_drv_device_close(struct ane_device *ane);

int ane_drv_nn_register(struct ane_device *ane, struct ane_nn *nn);
int ane_drv_nn_unregister(struct ane_device *ane, struct ane_nn *nn);

int ane_drv_nn_exec(struct ane_device *ane, struct ane_nn *nn);

#endif /* __ANE_DRV_H__ */
