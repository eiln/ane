// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include "ane_io.h"
#include "ane_mem.h"
#include "ane_priv.h"

#define HEADER_SIZE 0x1000

struct ane_model *ane_model_init(const char *path)
{
	struct ane_model *model = ane_zmalloc(sizeof(struct ane_model));
	struct anec *anec = NULL;
	if (!model) {
		return NULL;
	}

	anec = &model->anec;

	if (ane_fread(path, anec, sizeof(struct anec)) < 0) {
		free(model);
		return NULL;
	}

	model->data = ane_zmemalign(anec->size);
	if (!model->data) {
		free(model);
		return NULL;
	}

	if (ane_pread(path, model->data, anec->size, HEADER_SIZE) < 0) {
		free(model->data);
		free(model);
		return NULL;
	}

	return model;
}

void ane_model_free(struct ane_model *model)
{
	free(model->data);
	free(model);
}
