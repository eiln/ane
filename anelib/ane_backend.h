// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_BACKEND_H__
#define __ANE_BACKEND_H__

#include "ane_anec.h"

static int ane_free_chans(struct ane_nn *nn)
{
	for (int i = 0; i < input_count(nn); i++) {
		int bdx = nn->imask[i];
		if (nn->chans[bdx]) {
			free(nn->chans[bdx]);
		}
	}
	for (int i = 0; i < output_count(nn); i++) {
		int bdx = nn->omask[i];
		if (nn->chans[bdx]) {
			free(nn->chans[bdx]);
		}
	}
	if (nn->fifo_chan) {
		free(nn->fifo_chan);
		nn->fifo_chan = NULL;
	}
	return 0;
}

static int ane_alloc_chans(struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);

	int ic = 0, oc = 0;
	for (int i = 0; i < MAX_TILE_COUNT; i++) {
		if (anec->types[i] == TILE_SRC) {
			nn->imask[ic] = i;
			ic++;
		} else if (anec->types[i] == TILE_DST) {
			nn->omask[oc] = i;
			oc++;
		}
	}
	if (ic != input_count(nn) || oc != output_count(nn)) {
		fprintf(stderr, "invalid anec setup\n");
		goto error;
	}

	nn->chans[0] = ane_zmemalign(anec->tiles[0] * TILE_SIZE);
	if (!nn->chans[0]) {
		goto error;
	}

	for (int i = 0; i < input_count(nn); i++) {
		int bdx = nn->imask[i];
		size_t size = anec->tiles[bdx] * TILE_SIZE;
		nn->chans[bdx] = ane_zmemalign(size);
		if (!nn->chans[bdx]) {
			goto error;
		}
		printf("allocated input chan %d size 0x%zx\n", bdx, size);
	}

	for (int i = 0; i < output_count(nn); i++) {
		int bdx = nn->omask[i];
		size_t size = anec->tiles[bdx] * TILE_SIZE;
		nn->chans[bdx] = ane_zmemalign(size);
		if (!nn->chans[bdx]) {
			goto error;
		}
		printf("allocated output chan %d size 0x%zx\n", bdx, size);
	}

	nn->fifo_chan = ane_zmemalign(TILE_SIZE);
	if (!nn->fifo_chan) {
		goto error;
	}

	return 0;

error:
	fprintf(stderr, "failed to alloc chans\n");
	ane_free_chans(nn);
	return -ENOMEM;
}

static int ane_inst_backend(struct ane_nn *nn, void *anec_data)
{
	int err;

	err = ane_alloc_chans(nn);
	if (err) {
		fprintf(stderr, "failed to alloc chans, 0x%x\n", err);
		goto error;
	}

	err = ane_init_anec_fromp(nn, anec_data);
	if (err) {
		fprintf(stderr, "failed to load anec backend, 0x%x\n", err);
		goto free_chans;
	}

	return 0;

free_chans:
	ane_free_chans(nn);
error:
	return err;
}

static int ane_free_backend(struct ane_nn *nn)
{
	ane_free_chans(nn);
	return 0;
}

#endif /* __ANE_BACKEND_H__ */
