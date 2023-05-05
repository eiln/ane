// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include "ane_bo.h"
#include "ane_priv.h"

#define FIFO_WIDTH 0x400 // nxtpow2(0x274)
#define FIFO_COUNT 0x20

static inline void set_nid(void *td, int nid)
{
	uint32_t hdr0 = *(uint32_t *)td;
	hdr0 = (hdr0 & 0xf00ffff) | ((nid & 0xff) << 16); /* trust me bro */
	memcpy(td, &hdr0, sizeof(uint32_t));
}

static inline void load_anec(struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);
	const void *anec_data = nn->model->data;

	memcpy(nn->chans[0].map, anec_data, anec->size);

	/* do not fucking overflow */
	memcpy(nn->fifo_chan.map, anec_data, anec->td_size);
	memcpy(nn->fifo_chan.map + FIFO_WIDTH, anec_data, anec->td_size);

	set_nid(nn->fifo_chan.map, ANE_FIFO_NID);
	set_nid(nn->fifo_chan.map + FIFO_WIDTH, ANE_FIFO_NID + FIFO_COUNT);
}

void ane_chan_free(struct ane_device *ane, struct ane_nn *nn)
{
	ane_bo_free(ane, &nn->fifo_chan);

	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		ane_bo_free(ane, &nn->chans[bdx]);
	}
}

int ane_chan_init(struct ane_device *ane, struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);
	struct ane_bo *bo;
	int err;

	/* creating bar index masks to easily iterate over */
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

	if (ic != src_count(nn) || oc != dst_count(nn)) {
		ane_err("invalid src/dst setup\n");
		return -EINVAL;
	}

	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		if (anec->tiles[bdx]) {
			bo = &nn->chans[bdx];
			bo->size = tile_size(nn, bdx);
			err = ane_bo_init(ane, bo);
			if (err < 0)
				goto error;
		}
	}

	bo = &nn->fifo_chan;
	bo->size = tile_align(FIFO_WIDTH * 2);
	err = ane_bo_init(ane, bo);
	if (err < 0)
		goto error;

	load_anec(nn);

	return 0;

error:
	ane_err("failed to init memory-mapped channels\n");
	ane_chan_free(ane, nn);
	return err;
}
