// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <fcntl.h>
#include <unistd.h>

#include "ane_priv.h"

#if 0

// drm
find /sys/class/drm -type l -exec sh -c "readlink -f {} | grep '\.ane/drm/renderD' | sed 's/.*\/drm/\/dev\/dri/' | sort " \;

#define ANE_DEVNODE "/dev/dri/renderD129"

// -DANE_DEVNODE=\"/dev/dri/renderD129\"
// -DANE_MULTI_DEVICE -DANE_DEVNODE_0=\"/dev/dri/renderD129\" -DANE_DEVNODE_2=\"/dev/dri/renderD130\"

#endif

#ifdef ANE_MULTI_DEVICE
#if !defined(ANE_DEVNODE_0) || !defined(ANE_DEVNODE_2) || defined(ANE_DEVNODE)
#error "LIBANE: invalid multi-ane devnode configuration"
#endif
#else
#if defined(ANE_DEVNODE_0) || defined(ANE_DEVNODE_2) || !defined(ANE_DEVNODE)
#error "LIBANE: invalid single-ane devnode configuration"
#endif
#endif /* ANE_MULTI_DEVICE */

int ane_open(int dev_id)
{
	int fd;
	char *devnode;

#ifdef ANE_MULTI_DEVICE
	if (!dev_id) {
		devnode = ANE_DEVNODE_0;
	} else {
		devnode = ANE_DEVNODE_2;
	}
#else
	if (dev_id) {
		ane_err("multi-devices not supported. defaulting to dev_id=0\n");
	}
	devnode = ANE_DEVNODE;

#endif /* ANE_MULTI_DEVICE */

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
