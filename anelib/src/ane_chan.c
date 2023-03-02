// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include "ane_dev.h"
#include "ane_utils.h"

#define FIFO_WIDTH 0x400 // nxtpow2(0x274)
#define FIFO_COUNT 0x20

static inline void set_fifo_nid(void *td, int nid)
{
	uint32_t hdr0 = *(uint32_t *)td;
	hdr0 = (hdr0 & 0xf00ffff) | ((nid & 0xff) << 16);
	memcpy(td, &hdr0, sizeof(uint32_t));
}

static inline void load_cmd_data(struct ane_nn *nn, void *cmd_data)
{
	const struct anec *anec = to_anec(nn);

	memcpy(nn->chans[0], cmd_data, anec->size);

	/* do not fucking overflow */
	memcpy(nn->fifo_chan, cmd_data, anec->td_size);
	memcpy(nn->fifo_chan + FIFO_WIDTH, cmd_data, anec->td_size);

	set_fifo_nid(nn->fifo_chan, ANE_FIFO_NID);
	set_fifo_nid(nn->fifo_chan + FIFO_WIDTH, ANE_FIFO_NID + FIFO_COUNT);
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
		fprintf(stderr, "ANELIB: invalid src/dst setup\n");
		return -EINVAL;
	}

	nn->fifo_chan = ane_zmemalign(ANE_TILE_SIZE);
	if (!nn->fifo_chan)
		goto error;

	nn->chans[0] = ane_zmemalign(tile_size(anec->tiles[0]));
	if (!nn->chans[0])
		goto error;

	for (int i = 0; i < input_count(nn); i++) {
		int bdx = nn->src_bdx[i];
		size_t size = tile_size(anec->tiles[bdx]);

		nn->chans[bdx] = ane_zmemalign(size);
		if (!nn->chans[bdx])
			goto error;

		printf("ANELIB: allocated input chan %d/%d size 0x%zx\n", i + 1,
		       input_count(nn), size);
	}

	for (int i = 0; i < output_count(nn); i++) {
		int bdx = nn->dst_bdx[i];
		size_t size = tile_size(anec->tiles[bdx]);

		nn->chans[bdx] = ane_zmemalign(size);
		if (!nn->chans[bdx])
			goto error;

		printf("ANELIB: allocated output chan %d/%d size 0x%zx\n",
		       i + 1, output_count(nn), size);
	}

	return 0;

error:
	free_chans(nn);
	return -ENOMEM;
}

int ane_chan_init(struct ane_nn *nn, void *cmd_data)
{
	int err = alloc_chans(nn);
	if (err) {
		fprintf(stderr, "ANELIB: failed to alloc chans, 0x%x\n", err);
		return err;
	}

	load_cmd_data(nn, cmd_data);

	return 0;
}

int ane_chan_free(struct ane_nn *nn)
{
	free_chans(nn);
	return 0;
}
