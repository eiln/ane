// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include "ane_mem.h"
#include "ane_priv.h"

#define FIFO_WIDTH 0x400 // nxtpow2(0x274)
#define FIFO_COUNT 0x20

static inline void set_nid(void *td, int nid)
{
	uint32_t hdr0 = *(uint32_t *)td;
	hdr0 = (hdr0 & 0xf00ffff) | ((nid & 0xff) << 16);
	memcpy(td, &hdr0, sizeof(uint32_t));
}

static inline void load_anec(struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);
	const void *anec_data = nn->model->data;

	memcpy(nn->chans[0], anec_data, anec->size);

	/* do not fucking overflow */
	memcpy(nn->fifo_chan, anec_data, anec->td_size);
	memcpy((char *)nn->fifo_chan + FIFO_WIDTH, anec_data, anec->td_size);

	set_nid(nn->fifo_chan, ANE_FIFO_NID);
	set_nid((char *)nn->fifo_chan + FIFO_WIDTH, ANE_FIFO_NID + FIFO_COUNT);
}

static void free_chans(struct ane_nn *nn)
{
	if (nn->fifo_chan) {
		free(nn->fifo_chan);
		nn->fifo_chan = NULL;
	}

	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		if (nn->chans[bdx]) {
			free(nn->chans[bdx]);
			nn->chans[bdx] = NULL;
		}
	}

	return;
}

static int alloc_chans(struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);

	int ic = 0, oc = 0;
	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		if (anec->types[bdx] == ANE_TILE_SRC) {
			nn->src_bdx[ic] = bdx;
			ic++;
		} else if (anec->types[bdx] == ANE_TILE_DST) {
			nn->dst_bdx[oc] = bdx;
			oc++;
		}
	}

	if (ic != input_count(nn) || oc != output_count(nn)) {
		fprintf(stderr, "LIBANE: invalid src/dst setup\n");
		return -EINVAL;
	}

	nn->fifo_chan = ane_zmemalign(tile_shift(1UL));
	if (!nn->fifo_chan)
		goto error;

	nn->chans[0] = ane_zmemalign(tile_size(nn, 0));
	if (!nn->chans[0])
		goto error;

	for (int i = 0; i < input_count(nn); i++) {
		int bdx = nn->src_bdx[i];
		nn->chans[bdx] = ane_zmemalign(tile_size(nn, bdx));
		if (!nn->chans[bdx])
			goto error;
		printf("LIBANE: allocated input chan %d/%d size 0x%lx\n", i + 1,
		       input_count(nn), tile_size(nn, bdx));
	}

	for (int i = 0; i < output_count(nn); i++) {
		int bdx = nn->dst_bdx[i];
		nn->chans[bdx] = ane_zmemalign(tile_size(nn, bdx));
		if (!nn->chans[bdx])
			goto error;
		printf("LIBANE: allocated output chan %d/%d size 0x%lx\n",
		       i + 1, output_count(nn), tile_size(nn, bdx));
	}

	return 0;

error:
	fprintf(stderr, "LIBANE: out of memory for chans\n");
	free_chans(nn);
	return -ENOMEM;
}

int ane_chan_init(struct ane_nn *nn)
{
	int err;

	err = alloc_chans(nn);
	if (err) {
		fprintf(stderr, "LIBANE: failed to alloc chans, 0x%x\n", err);
		return err;
	}

	load_anec(nn);

	return 0;
}

int ane_chan_free(struct ane_nn *nn)
{
	free_chans(nn);
	return 0;
}
