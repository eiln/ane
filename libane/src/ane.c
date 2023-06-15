// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <drm.h>
#include <sys/ioctl.h>

#include "ane_chan.h"
#include "ane_device.h"
#include "ane_mem.h"
#include "ane_priv.h"

#ifdef LIBANE_INDEX_CHECK
#define SRC_INDEX_CHECK(nn, idx, ret)                                              \
	({                                                                         \
		if (idx >= src_count(nn)) {                                        \
			ane_err("attempted to index %d but max is %d; bailing.\n", \
				idx, src_count(nn));                               \
			return ret;                                                \
		}                                                                  \
	})
#else
#define SRC_INDEX_CHECK(nn, idx, ret) \
	do {                          \
	} while (0)
#endif /* LIBANE_INDEX_CHECK */

#ifdef LIBANE_INDEX_CHECK
#define DST_INDEX_CHECK(nn, idx, ret)                                              \
	({                                                                         \
		if (idx >= dst_count(nn)) {                                        \
			ane_err("attempted to index %d but max is %d; bailing.\n", \
				idx, dst_count(nn));                               \
			return ret;                                                \
		}                                                                  \
	})
#else
#define DST_INDEX_CHECK(nn, idx, ret) \
	do {                          \
	} while (0)
#endif /* LIBANE_INDEX_CHECK */

static inline struct ane_nn *__ane_init_fd(const struct ane_model *model,
					   int fd)
{
	int err;

	struct ane_nn *nn = ane_zmalloc(sizeof(struct ane_nn));
	if (!nn) {
		ane_err("failed to alloc space for nn struct\n");
		return NULL;
	}

	nn->model = model;
	nn->ane.fd = fd;

	err = ane_chan_init(&nn->ane, nn);
	if (err) {
		ane_err("ane_chan_init failed with 0x%x\n", err);
		free(nn);
		return NULL;
	}

	ane_log("initialized nn %p\n", (void *)nn);

	return nn;
}

struct ane_nn *__ane_init(const struct ane_model *model, int dev_id)
{
	int fd;
	struct ane_nn *nn;

	fd = ane_open(dev_id);
	if (fd < 0) {
		ane_err("failed to open device file for dev_id %d\n", dev_id);
		return NULL;
	}

	nn = __ane_init_fd(model, fd);
	if (nn == NULL) {
		ane_err("failed to init nn with fd %d\n", fd);
		ane_close(fd);
		return NULL;
	}

	return nn;
}

void __ane_free(struct ane_nn *nn)
{
	ane_log("freeing nn %p\n", (void *)nn);
	ane_chan_free(&nn->ane, nn);
	free(nn);
}

void ane_free(struct ane_nn *nn)
{
	ane_log("freeing nn %p\n", (void *)nn);
	ane_chan_free(&nn->ane, nn);
	ane_close(nn->ane.fd);
	free(nn);
}

int ane_exec(struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);

	struct drm_ane_submit args;
	memset(&args, 0, sizeof(args));

	args.tsk_size = anec->tsk_size;
	args.td_count = anec->td_count;
	args.td_size = anec->td_size;

	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		if (anec->tiles[bdx]) {
			args.handles[bdx] = nn->chans[bdx].handle;
		}
	}
	args.btsp_handle = nn->btsp_chan.handle;

	return ioctl(nn->ane.fd, DRM_IOCTL_ANE_SUBMIT, &args);
}

void __ane_send(struct ane_nn *nn, void *from, const int idx)
{
	SRC_INDEX_CHECK(nn, idx, );
	memcpy(nn->chans[nn->src_bdx[idx]].map, from,
	       tile_size(nn, nn->src_bdx[idx]));
}

void __ane_read(struct ane_nn *nn, void *to, const int idx)
{
	DST_INDEX_CHECK(nn, idx, );
	memcpy(to, nn->chans[nn->dst_bdx[idx]].map,
	       tile_size(nn, nn->dst_bdx[idx]));
}

void *__ane_src_chan(struct ane_nn *nn, const int idx)
{
	SRC_INDEX_CHECK(nn, idx, NULL);
	return nn->chans[nn->src_bdx[idx]].map;
}

void *__ane_dst_chan(struct ane_nn *nn, const int idx)
{
	DST_INDEX_CHECK(nn, idx, NULL);
	return nn->chans[nn->dst_bdx[idx]].map;
}

uint64_t __ane_src_size(struct ane_nn *nn, const int idx)
{
	SRC_INDEX_CHECK(nn, idx, 0);
	return tile_size(nn, nn->src_bdx[idx]);
}

uint64_t __ane_dst_size(struct ane_nn *nn, const int idx)
{
	DST_INDEX_CHECK(nn, idx, 0);
	return tile_size(nn, nn->dst_bdx[idx]);
}
