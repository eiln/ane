// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_BACKEND_H__
#define __ANE_BACKEND_H__

#define FIFO_WIDTH 0x400 // nxtpow2(0x274)
#define FIFO_COUNT 0x20

static inline void set_fifo_nid(void *td, int nid)
{
	uint32_t hdr0 = *(uint32_t *)td;
	hdr0 = (hdr0 & 0xf00ffff) | (nid << 16);
	memcpy(td, &hdr0, sizeof(uint32_t));
}

static void ane_load_anec(struct ane_nn *nn, void *anec_data)
{
	const struct anec *anec = to_anec(nn);

	memcpy(nn->chans[0], anec_data, anec->size);

	/* do not fucking overflow */
	memcpy(nn->fifo_chan, anec_data, anec->td_size);
	memcpy(nn->fifo_chan + FIFO_WIDTH, anec_data, anec->td_size);

	set_fifo_nid(nn->fifo_chan, FIFO_NID_MAGIC);
	set_fifo_nid(nn->fifo_chan + FIFO_WIDTH, FIFO_NID_MAGIC + FIFO_COUNT);
}

static void ane_free_chans(struct ane_nn *nn)
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
	}
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
		fprintf(stderr, "ANELIB: invalid src/dst setup\n");
		return -EINVAL;
	}

	nn->chans[0] = ane_zmemalign(tile_sz(anec->tiles[0]));
	if (!nn->chans[0]) {
		return -ENOMEM;
	}

	for (int i = 0; i < input_count(nn); i++) {
		int bdx = nn->imask[i];
		size_t size = tile_sz(anec->tiles[bdx]);
		nn->chans[bdx] = ane_zmemalign(size);
		if (!nn->chans[bdx]) {
			goto error;
		}
		printf("ANELIB: allocated input chan %d size 0x%zx\n", bdx,
		       size);
	}

	for (int i = 0; i < output_count(nn); i++) {
		int bdx = nn->omask[i];
		size_t size = tile_sz(anec->tiles[bdx]);
		nn->chans[bdx] = ane_zmemalign(size);
		if (!nn->chans[bdx]) {
			goto error;
		}
		printf("ANELIB: allocated output chan %d size 0x%zx\n", bdx,
		       size);
	}

	nn->fifo_chan = ane_zmemalign(TILE_SIZE);
	if (!nn->fifo_chan) {
		goto error;
	}

	return 0;

error:
	ane_free_chans(nn);
	return -ENOMEM;
}

static int ane_inst_backend(struct ane_nn *nn, void *anec_data)
{
	int err = ane_alloc_chans(nn);
	if (err) {
		fprintf(stderr, "ANELIB: failed to alloc chans, 0x%x\n", err);
		return err;
	}

	ane_load_anec(nn, anec_data);

	return 0;
}

static int ane_free_backend(struct ane_nn *nn)
{
	ane_free_chans(nn);
	return 0;
}

#endif /* __ANE_BACKEND_H__ */
