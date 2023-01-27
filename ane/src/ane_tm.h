// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_TM_H__
#define __ANE_TM_H__

#include "ane.h"

int ane_tm_init_tqs(struct ane_device *ane);
int ane_tm_enqueue_tq(struct ane_device *ane, struct ane_engine_req *req);
int ane_tm_execute_tq(struct ane_device *ane, struct ane_engine_req *req);

#endif /* __ANE_TM_H__ */
