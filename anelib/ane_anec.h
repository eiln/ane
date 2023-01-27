// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_ANEC_H__
#define __ANE_ANEC_H__

/* A hack that lets the driver bypass buffers completely. 
 * Nothing to see.
 */

#define ANE_FIFO_WIDTH 0x400 // nxtpow2(0x274)
#define ANE_FIFO_COUNT 0x20

static inline void set_fifo_nid(void *td, int nid)
{
	uint32_t hdr0 = *(uint32_t *)td;
	hdr0 = (hdr0 & 0xf00ffff) | ((nid << 16));
	memcpy(td, &hdr0, sizeof(uint32_t));
	return;
}

static inline void ane_load_anec(struct ane_nn *nn, void *anec_data)
{
	const struct anec *anec = to_anec(nn);

	memcpy(nn->chans[0], anec_data, anec->size);

	/* do not fucking overflow */
	memcpy(nn->fifo_chan, anec_data, anec->td_size);
	memcpy(nn->fifo_chan + ANE_FIFO_WIDTH, anec_data, anec->td_size);

	set_fifo_nid(nn->fifo_chan, FIFO_NID_MAGIC);
	set_fifo_nid(nn->fifo_chan + ANE_FIFO_WIDTH,
		     FIFO_NID_MAGIC + ANE_FIFO_COUNT);
	return;
}

static int ane_init_anec(struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);
	if (!nn->chans[0]) {
		fprintf(stderr, "channels not initiated\n");
		return -EINVAL;
	}

	void *anec_data = ane_zmemalign(anec->size);
	if (!anec_data) {
		return -ENOMEM;
	}

	size_t read =
		ane_fread(anec_data, anec->size, (char *)nn->model->fname);
	if (read != anec->size) {
		fprintf(stderr, "invalid anec backend\n");
		free(anec_data);
		return -EINVAL;
	}

	ane_load_anec(nn, anec_data);

	free(anec_data);

	return 0;
}

#endif /* __ANE_ANEC_H__ */
