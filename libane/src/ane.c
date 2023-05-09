// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <drm.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ane_chan.h"
#include "ane_priv.h"

#define ANE_SYSFS_PATH "/dev/dri/renderD129"

static inline int ane_open(struct ane_nn *nn)
{
	int fd = open(ANE_SYSFS_PATH, O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		ane_err("failed to open device sysfs %s\n", ANE_SYSFS_PATH);
		return -ENODEV;
	}

	nn->ane.fd = fd;

	return 0;
}

static inline void ane_close(struct ane_nn *nn)
{
	close(nn->ane.fd);
}

struct ane_nn *ane_init(const struct ane_model *model)
{
	int err;

	/* we malloc once for the nn struct */
	struct ane_nn *nn = malloc(sizeof(struct ane_nn));
	if (!nn) {
		ane_err("out of memory to alloc nn struct\n");
		goto exit;
	}

	memset(nn, 0, sizeof(struct ane_nn));
	nn->model = model;

	err = ane_open(nn);
	if (err)
		goto free;

	err = ane_chan_init(&nn->ane, nn);
	if (err) {
		ane_err("ane_chan_init failed with 0x%x\n", err);
		goto close;
	}

	ane_log("initialized nn %p\n", (void *)nn);

	return nn;

close:
	ane_close(nn);
free:
	free(nn);
exit:
	return NULL;
}

void ane_free(struct ane_nn *nn)
{
	ane_log("freeing nn %p\n", (void *)nn);
	ane_chan_free(&nn->ane, nn);
	ane_close(nn);
	free(nn);
}

int ane_exec(struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);

	struct drm_ane_submit args;
	memset(&args, 0, sizeof(args));

	args.tsk_size = anec->tsk_size;
	args.td_count = anec->td_count;
	args.td_size = anec->td_size;

	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		if (anec->tiles[bdx]) {
			args.handles[bdx] = nn->chans[bdx].handle;
		}
	}
	args.fifo_handle = nn->fifo_chan.handle;

	return ioctl(nn->ane.fd, DRM_IOCTL_ANE_SUBMIT, &args);
}

void ane_send(struct ane_nn *nn, void *from, const int idx)
{
	const int bdx = nn->src_bdx[idx];
	memcpy(nn->chans[bdx].map, from, tile_size(nn, bdx));
}

void ane_read(struct ane_nn *nn, void *to, const int idx)
{
	const int bdx = nn->dst_bdx[idx];
	memcpy(to, nn->chans[bdx].map, tile_size(nn, bdx));
}
