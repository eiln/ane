// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <drm.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "ane_priv.h"

static inline void set_nid(void *td, int nid)
{
	uint32_t hdr0 = *(uint32_t *)td;
	hdr0 = (hdr0 & 0xf00ffff) | ((nid & 0xff) << 16);
	memcpy(td, &hdr0, sizeof(uint32_t));
}

static inline void set_command(struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);
	const void *data = nn->model->data;

	memcpy(nn->chans[0].map, data, anec->size);

	/* do not fucking overflow */
	memcpy(nn->btsp_chan.map, data, anec->td_size);
	set_nid(nn->btsp_chan.map, ANE_FIFO_NID);
}

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

static inline int ane_bo_init(struct ane_device *ane, struct ane_bo *bo)
{
	int err;

	if (!bo->size)
		return -EINVAL;

	err = bo_init(ane, bo);
	if (err < 0) {
		ane_err("bo_init failed with 0x%x\n", err);
		goto error;
	}

	err = bo_mmap(ane, bo);
	if (err < 0) {
		bo_free(ane, bo);
		ane_err("bo_mmap failed with 0x%x\n", err);
		goto error;
	}

	return 0;

error:
	bo->handle = 0;
	bo->offset = 0;
	return err;
}

static inline void ane_bo_free(struct ane_device *ane, struct ane_bo *bo)
{
	if (bo->map) {
		munmap(bo->map, bo->size);
		bo_free(ane, bo);
	}
	bo->map = NULL;
}

void ane_chan_free(struct ane_device *ane, struct ane_nn *nn)
{
	ane_bo_free(ane, &nn->btsp_chan);

	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		ane_bo_free(ane, &nn->chans[bdx]);
	}
}

int ane_chan_init(struct ane_device *ane, struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);
	struct ane_bo *bo;
	int err;

	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		if (anec->tiles[bdx]) {
			bo = &nn->chans[bdx];
			bo->size = tile_size(nn, bdx);
			err = ane_bo_init(ane, bo);
			if (err < 0)
				goto error;
		}
	}

	bo = &nn->btsp_chan;
	bo->size = tile_align(anec->td_size);
	err = ane_bo_init(ane, bo);
	if (err < 0)
		goto error;

	set_command(nn);

	return 0;

error:
	ane_err("failed to init memory-mapped channels\n");
	ane_chan_free(ane, nn);
	return err;
}
