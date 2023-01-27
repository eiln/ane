// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <drm.h> // libdrm
#include "drm_ane.h"

#include "ane_drv.h"

/* ane_drv_* funcs interface with the driver.
 * all take *ane and return status.
 */

int ane_drv_device_open(struct ane_device *ane)
{
	int fd = open(ANE_SYSFS_PATH, O_RDWR | FD_CLOEXEC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		fprintf(stderr, "failed to open sysfs %s\n", ANE_SYSFS_PATH);
		return -ENODEV;
	}
	ane->fd = fd;
	return 0;
}

int ane_drv_device_close(struct ane_device *ane)
{
	close(ane->fd);
	return 0;
}

static int ane_drv_nn_init(struct ane_device *ane, struct ane_nn *nn)
{
	struct drm_ane_nn_init args = {
		.anecs_userptr = (uint64_t)(to_anec(nn)),
	};

	int err = ioctl(ane->fd, DRM_IOCTL_ANE_NN_INIT, &args);
	if (err) {
		fprintf(stderr, "ane_drv_nn_init failed with 0x%x\n", err);
		return -1;
	}

	nn->handle = args.handle;
	return 0;
}

static int ane_drv_nn_deinit(struct ane_device *ane, struct ane_nn *nn)
{
	struct drm_ane_nn_deinit args = { .handle = nn->handle };
	int err = ioctl(ane->fd, DRM_IOCTL_ANE_NN_DEINIT, &args);
	return err;
}

static int ane_drv_nn_sync(struct ane_device *ane, struct ane_nn *nn)
{
	int err;
	struct drm_ane_nn_sync args = { .handle = nn->handle };
	for (int i = 0; i < MAX_TILE_COUNT; i++) {
		args.userptrs[i] = (uint64_t)nn->chans[i];
	}
	args.fifo_userptr = (uint64_t)nn->fifo_chan;

	err = ioctl(ane->fd, DRM_IOCTL_ANE_NN_SYNC, &args);
	if (err) {
		fprintf(stderr, "ane_drv_nn_sync failed with 0x%x\n", err);
		return err;
	}
	return 0;
}

static int ane_drv_nn_unsync(struct ane_device *ane, struct ane_nn *nn)
{
	struct drm_ane_nn_unsync args = { .handle = nn->handle };
	return ioctl(ane->fd, DRM_IOCTL_ANE_NN_UNSYNC, &args);
}

int ane_drv_nn_register(struct ane_device *ane, struct ane_nn *nn)
{
	// nn_init + nn_sync
	// it really should be a single ioctl call
	// but it makes the args way too complex

	int err;

	err = ane_drv_nn_init(ane, nn);
	if (err) {
		fprintf(stderr, "ane_drv_nn_init failed with 0x%x\n", err);
		goto exit;
	}

	err = ane_drv_nn_sync(ane, nn);
	if (err) {
		fprintf(stderr, "ane_drv_nn_sync failed with 0x%x\n", err);
		goto nn_deinit;
	}

	return 0;

nn_deinit:
	ane_drv_nn_deinit(ane, nn);
exit:
	return err;
}

int ane_drv_nn_unregister(struct ane_device *ane, struct ane_nn *nn)
{
	// nn_deinit + nn_unsync
	int err = 0;
	err |= ane_drv_nn_unsync(ane, nn);
	err |= ane_drv_nn_deinit(ane, nn);
	return err;
}

int ane_drv_nn_exec(struct ane_device *ane, struct ane_nn *nn)
{
	struct drm_ane_nn_exec args = { .handle = nn->handle };
	return ioctl(ane->fd, DRM_IOCTL_ANE_NN_EXEC, &args);
}
