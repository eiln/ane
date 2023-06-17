// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_MODEL_H__
#define __ANE_MODEL_H__

#include "ane_dev.h"

struct ane_model *ane_model_init(const char *path);
void ane_model_free(struct ane_model *model);

#endif /* __ANE_MODEL_H__ */
