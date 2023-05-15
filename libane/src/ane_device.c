// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <fcntl.h>
#include <unistd.h>

#include "ane_priv.h"

#if 0

// drm
find /sys/class/drm -type l -exec sh -c "readlink -f {} | grep '\.ane/drm/renderD' | sed 's/.*\/drm/\/dev\/dri/' | sort " \;

#define ANE_DEVNODE "/dev/dri/renderD129"

#endif

int ane_open(void)
{
	int fd = open(ANE_DEVNODE, O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		ane_err("failed to open devnode %s\n", ANE_DEVNODE);
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
