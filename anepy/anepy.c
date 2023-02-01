// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <stdio.h>

#include "ane.h"

int anepy_destroy(struct ane_nn *nn)
{
	ane_destroy(nn);
	return 0;
}

void *anepy_exec(struct ane_nn *nn, void *input)
{
	int err;
	ane_write(nn, input, 0);

	err = ane_exec(nn);
	if (err) {
		fprintf(stderr, "execution failed with 0x%x\n", err);
	}

	void *output = ane_zmemalign(output_size(nn, 0));
	ane_read(nn, output, 0);
	return output;
}

int main(void)
{
	printf("hi\n");
	return 0;
}
