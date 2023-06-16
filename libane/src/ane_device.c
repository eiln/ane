// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <fcntl.h>
#include <unistd.h>

#include "ane_priv.h"

#define ANE_DEVNODE "/dev/accel/accel0"

int ane_open(int dev_id)
{
	int fd;
	char *devnode;

	(void)dev_id;
	devnode = ANE_DEVNODE;

	fd = open(devnode, O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		ane_err("failed to open devnode %s\n", devnode);
		return -ENODEV;
	}

	return fd;
}

void ane_close(int fd)
{
	if (!(fd < 0)) {
		close(fd);
	}
}
