// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include "ane.h"
#include "ane_backend.h"
#include "ane_drv.h"

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

struct ane_nn *ane_register(const struct ane_model *model, void *anec_data)
{
	int err;
	struct ane_nn *nn = ane_zmalloc(sizeof(struct ane_nn));
	if (!nn) {
		return nn;
	}
	nn->model = model;

	err = ane_inst_backend(nn, anec_data);
	if (err) {
		fprintf(stderr, "ANELIB: ane_inst_backend failed with 0x%x\n",
			err);
		goto free_nn;
	}

	/* now call driver to init nn */
	nn->ane = ane_dev_new();
	if (nn->ane == NULL) {
		goto free_backend;
	}

	err = ane_drv_nn_register(nn->ane, nn);
	if (err) {
		fprintf(stderr,
			"ANELIB: ane_drv_nn_register failed with 0x%x\n", err);
		goto dev_del;
	}

	printf("ANELIB: registered nn %p\n", (void *)nn);

	return nn;

dev_del:
	ane_dev_del(nn->ane);
free_backend:
	ane_free_backend(nn);
free_nn:
	free(nn);
	return NULL;
}

void ane_destroy(struct ane_nn *nn)
{
	printf("ANELIB: destroying nn %p\n", (void *)nn);
	ane_drv_nn_unregister(nn->ane, nn);
	ane_dev_del(nn->ane);
	ane_free_backend(nn);
	free(nn);
}

int ane_exec(struct ane_nn *nn)
{
	return ane_drv_nn_exec(nn->ane, nn);
}
