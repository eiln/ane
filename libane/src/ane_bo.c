// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <drm.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "ane_priv.h"

static inline int bo_init(struct ane_device *ane, struct ane_bo *bo)
{
	struct drm_ane_bo_init args = { .size = bo->size };
	int err = ioctl(ane->fd, DRM_IOCTL_ANE_BO_INIT, &args);
	bo->handle = args.handle;
	bo->offset = args.offset;
	return err;
}

static inline int bo_free(struct ane_device *ane, struct ane_bo *bo)
{
	struct drm_ane_bo_free args = { .handle = bo->handle };
	return ioctl(ane->fd, DRM_IOCTL_ANE_BO_FREE, &args);
}

static inline int bo_mmap(struct ane_device *ane, struct ane_bo *bo)
{
	bo->map = mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED, ane->fd,
		       bo->offset);

	if (bo->map == MAP_FAILED) {
		bo->map = NULL;
		ane_err("failed to mmap bo\n");
		return -EINVAL;
	}

	return 0;
}

int ane_bo_init(struct ane_device *ane, struct ane_bo *bo)
{
	int err;

	if (!bo->size)
		return -EINVAL;

	err = bo_init(ane, bo);
	if (err < 0) {
		ane_err("bo_init failed with 0x%x\n", err);
		return err;
	}

	err = bo_mmap(ane, bo);
	if (err < 0) {
		bo_free(ane, bo);
		ane_err("bo_mmap failed with 0x%x\n", err);
		return err;
	}

	return 0;
}

void ane_bo_free(struct ane_device *ane, struct ane_bo *bo)
{
	if (bo->map) {
		munmap(bo->map, bo->size);
		bo_free(ane, bo);
	}
}
