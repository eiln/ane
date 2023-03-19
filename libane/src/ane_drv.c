// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <drm.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ane_dev.h"

#define ANE_SYSFS_PATH "/dev/dri/renderD129"

int ane_drv_open(struct ane_device *ane)
{
	int fd = open(ANE_SYSFS_PATH, O_RDWR | FD_CLOEXEC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		fprintf(stderr, "LIBANE: failed to open sysfs %s\n",
			ANE_SYSFS_PATH);
		return -ENODEV;
	}
	ane->fd = fd;
	return 0;
}

int ane_drv_close(struct ane_device *ane)
{
	close(ane->fd);
	return 0;
}

static int ane_drv_nn_create(struct ane_device *ane, struct ane_nn *nn)
{
	struct drm_ane_nn_create args = {
		.anec_userptr = (uint64_t)to_anec(nn),
	};
	int err = ioctl(ane->fd, DRM_IOCTL_ANE_NN_CREATE, &args);
	nn->handle = args.handle;
	return err;
}

static int ane_drv_nn_free(struct ane_device *ane, struct ane_nn *nn)
{
	struct drm_ane_nn_free args = { .handle = nn->handle };
	return ioctl(ane->fd, DRM_IOCTL_ANE_NN_FREE, &args);
}

static int ane_drv_nn_map(struct ane_device *ane, struct ane_nn *nn)
{
	struct drm_ane_nn_map args = { .handle = nn->handle };
	for (int i = 0; i < ANE_TILE_COUNT; i++) {
		args.tile_userptr[i] = (uint64_t)nn->chans[i];
	}
	args.fifo_userptr = (uint64_t)nn->fifo_chan;
	return ioctl(ane->fd, DRM_IOCTL_ANE_NN_MAP, &args);
}

static int ane_drv_nn_unmap(struct ane_device *ane, struct ane_nn *nn)
{
	struct drm_ane_nn_unmap args = { .handle = nn->handle };
	return ioctl(ane->fd, DRM_IOCTL_ANE_NN_UNMAP, &args);
}

int ane_drv_nn_register(struct ane_device *ane, struct ane_nn *nn)
{
	int err;

	err = ane_drv_nn_create(ane, nn);
	if (err) {
		fprintf(stderr, "LIBANE: ane_drv_nn_create failed with 0x%x\n",
			err);
		return err;
	}

	err = ane_drv_nn_map(ane, nn);
	if (err) {
		fprintf(stderr, "LIBANE: ane_drv_nn_map failed with 0x%x\n",
			err);
		ane_drv_nn_free(ane, nn);
		return err;
	}

	return 0;
}

int ane_drv_nn_unregister(struct ane_device *ane, struct ane_nn *nn)
{
	int err = 0;
	err |= ane_drv_nn_unmap(ane, nn);
	err |= ane_drv_nn_free(ane, nn);
	return err;
}

int ane_drv_nn_exec(struct ane_device *ane, struct ane_nn *nn)
{
	struct drm_ane_nn_exec args = { .handle = nn->handle };
	return ioctl(ane->fd, DRM_IOCTL_ANE_NN_EXEC, &args);
}
