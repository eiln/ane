// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <stdio.h>

#include "ane.h"
#include "ane_f16.h"

int main(void)
{
	int err = 0;

	struct ane_nn *nn = ane_init("rsqrt.anec");
	if (nn == NULL) {
		printf("failed to init\n");
		return -1;
	}

	float Af[] = { 0.25, 0.50, 0.75, 1.00 };
	float Bf[] = { 0.00, 0.00, 0.00, 0.00 };
	float_to_half_c_array(Af, A);
	float_to_half_c_array(Bf, B); // init_half_array(B, 4);

	ane_tile_send(nn, A, 0);
	err = ane_exec(nn);
	ane_tile_read(nn, B, 0);

	for (int i = 0; i < 4; i++) {
		uint16_t x = A[i];
		uint16_t y = B[i];
		float z = half_to_float(x);
		float w = half_to_float(y);
		printf("x: 0x%x (%f) -> y: 0x%x (%f)\n", x, z, y, w);
	}

	ane_free(nn);

	return err;
}
