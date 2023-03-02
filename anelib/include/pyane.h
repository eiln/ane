// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __PYANE_H__
#define __PYANE_H__

#include "ane.h"

int pyane_free(struct ane_nn *nn)
{
	ane_free(nn);
	return 0;
}

int pyane_exec(struct ane_nn *nn, void *inputs[ANE_TILE_COUNT],
	       void *outputs[ANE_TILE_COUNT])
{
	int err;
	for (int i = 0; i < input_count(nn); i++) {
		ane_send(nn, inputs[i], i);
	}
	err = ane_exec(nn);
	for (int i = 0; i < output_count(nn); i++) {
		ane_read(nn, outputs[i], i);
	}
	return err;
}

#endif /* __PYANE_H__ */
