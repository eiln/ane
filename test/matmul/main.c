// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <stdio.h>

#include "ane.h"
#include "ane_f16.h"

// clang-format off

int main(void)
{
	int err = 0;

	float A[] = { 0.11, 0.12, 0.13,
                      0.21, 0.22, 0.23 };
	float_to_half_c_array(A, Ah);

	float B[] = { 1011, 1012,
	              1021, 1022,
	              1031, 1032 };
	float_to_half_c_array(B, Bh);

	float C[] = { 0.00, 0.00,
                      0.00, 0.00 };
	init_half_array(Ch, sizeof(C) / sizeof(float));

	struct ane_nn *nn = ane_init("matmul.anec");
	if (nn == NULL) {
		printf("failed to init\n");
		return -1;
	}

	ane_tile_send(nn, Ah, 0);
	ane_tile_send(nn, Bh, 1);

	err = ane_exec(nn);

	ane_tile_read(nn, Ch, 0);

	half_to_float_c_array(Ch, Cf);
	printf("[ %g, %g\n", Cf[0], Cf[1]);
	printf("  %g, %g ]\n", Cf[2], Cf[3]);

	ane_free(nn);

	return err;
}

// clang-format on
