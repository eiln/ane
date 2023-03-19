// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include "ane_chan.h"
#include "ane_drv.h"
#include "ane_mem.h"

static struct ane_device *device_new(void)
{
	int err;

	struct ane_device *ane = ane_zmalloc(sizeof(struct ane_device));
	if (!ane)
		return NULL;

	err = ane_drv_open(ane);
	if (err) {
		fprintf(stderr, "LIBANE: ane_drv_open failed with 0x%x\n", err);
		free(ane);
		return NULL;
	}

	return ane;
}

static void device_del(struct ane_device *ane)
{
	ane_drv_close(ane);
	free(ane);
}

struct ane_nn *ane_init(const struct ane_model *model)
{
	int err;

	struct ane_nn *nn = ane_zmalloc(sizeof(struct ane_nn));
	if (!nn)
		return NULL;

	nn->model = model;

	err = ane_chan_init(nn);
	if (err) {
		fprintf(stderr, "LIBANE: ane_chan_init failed with 0x%x\n",
			err);
		goto free_nn;
	}

	nn->ane = device_new();
	if (!nn->ane)
		goto chan_free;

	err = ane_drv_nn_register(nn->ane, nn);
	if (err) {
		fprintf(stderr,
			"LIBANE: ane_drv_nn_register failed with 0x%x\n", err);
		goto device_del;
	}

	printf("LIBANE: initialized nn %p\n", (void *)nn);

	return nn;

device_del:
	device_del(nn->ane);
chan_free:
	ane_chan_free(nn);
free_nn:
	free(nn);
	return NULL;
}

void ane_free(struct ane_nn *nn)
{
	printf("LIBANE: freeing nn %p\n", (void *)nn);
	ane_drv_nn_unregister(nn->ane, nn);
	device_del(nn->ane);
	ane_chan_free(nn);
	free(nn);
}

int ane_exec(struct ane_nn *nn)
{
	return ane_drv_nn_exec(nn->ane, nn);
}

void ane_send(struct ane_nn *nn, void *from, int idx)
{
	int bdx = nn->src_bdx[idx];
	memcpy(nn->chans[bdx], from, tile_shift(to_anec(nn)->tiles[bdx]));
}

void ane_read(struct ane_nn *nn, void *to, int idx)
{
	int bdx = nn->dst_bdx[idx];
	memcpy(to, nn->chans[bdx], tile_shift(to_anec(nn)->tiles[bdx]));
}
