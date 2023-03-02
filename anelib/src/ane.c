// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include "ane_chan.h"
#include "ane_dev.h"
#include "ane_drv.h"
#include "ane_utils.h"

static struct ane_device *ane_dev_new(void)
{
	int err;
	struct ane_device *ane = ane_zmalloc(sizeof(struct ane_device));
	if (!ane) {
		return NULL;
	}

	err = ane_drv_device_open(ane);
	if (err) {
		fprintf(stderr,
			"ANELIB: ane_drv_device_open failed with 0x%x\n", err);
		free(ane);
		return NULL;
	}
	return ane;
}

static void ane_dev_del(struct ane_device *ane)
{
	ane_drv_device_close(ane);
	free(ane);
}

struct ane_nn *ane_init(const struct ane_model *model, void *cmd_data)
{
	int err;
	struct ane_nn *nn = ane_zmalloc(sizeof(struct ane_nn));
	if (!nn) {
		return nn;
	}
	nn->model = model;

	err = ane_chan_init(nn, cmd_data);
	if (err) {
		fprintf(stderr, "ANELIB: ane_chan_init failed with 0x%x\n",
			err);
		goto free_nn;
	}

	/* now call driver to init nn */
	nn->ane = ane_dev_new();
	if (nn->ane == NULL) {
		goto chan_free;
	}

	err = ane_drv_nn_register(nn->ane, nn);
	if (err) {
		fprintf(stderr,
			"ANELIB: ane_drv_nn_register failed with 0x%x\n", err);
		goto dev_del;
	}

	printf("ANELIB: initialized nn %p\n", (void *)nn);

	return nn;

dev_del:
	ane_dev_del(nn->ane);
chan_free:
	ane_chan_free(nn);
free_nn:
	free(nn);
	return NULL;
}

void ane_free(struct ane_nn *nn)
{
	printf("ANELIB: freeing nn %p\n", (void *)nn);
	ane_drv_nn_unregister(nn->ane, nn);
	ane_dev_del(nn->ane);
	ane_chan_free(nn);
	free(nn);
}

int ane_exec(struct ane_nn *nn)
{
	return ane_drv_nn_exec(nn->ane, nn);
}
