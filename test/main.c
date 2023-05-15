// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <ane.h>

#include "ane_utils.h"

int main(void)
{
	int fd = ane_open(0);
	if (fd < 0) {
		ane_err("failed to open device\n");
		return -1;
	}

	ane_close(fd);

	return 0;
}
