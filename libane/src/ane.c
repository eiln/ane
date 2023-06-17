// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <drm.h>
#include <sys/ioctl.h>

#include "ane_chan.h"
#include "ane_device.h"
#include "ane_mem.h"
#include "ane_model.h"
#include "ane_priv.h"

struct ane_nn *__ane_init_from_model(struct ane_model *model, int dev_id)
{
	struct ane_nn *nn = NULL;

	int fd = ane_device_open(dev_id);
	if (fd < 0) {
		return NULL;
	}

	nn = ane_zmalloc(sizeof(struct ane_nn));
	if (nn == NULL) {
		ane_device_close(fd);
		return NULL;
	}

	nn->model = model;
	nn->ane.fd = fd;

	if (ane_chan_init(&nn->ane, nn) < 0) {
		free(nn);
		ane_device_close(fd);
		return NULL;
	}

	ane_log("loaded model @ %p!\n", (void *)(nn));

	return nn;
}

struct ane_nn *__ane_init(const char *path, int dev_id)
{
	struct ane_nn *nn = NULL;

	struct ane_model *model = ane_model_init(path);
	if (model == NULL) {
		ane_err("failed to load model at %s\n", path);
		return NULL;
	}

	nn = __ane_init_from_model(model, dev_id);
	if (nn == NULL) {
		ane_model_free(model);
		return NULL;
	}

	return nn;
}

void __ane_free_from_model(struct ane_nn *nn)
{
	ane_log("freeing model @ %p!\n", (void *)(nn));
	ane_chan_free(&nn->ane, nn);
	ane_device_close(nn->ane.fd);
	free(nn);
}

void __ane_free(struct ane_nn *nn)
{
	ane_log("freeing model @ %p!\n", (void *)(nn));
	ane_chan_free(&nn->ane, nn);
	ane_device_close(nn->ane.fd);
	ane_model_free(nn->model);
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

void __ane_send(struct ane_nn *nn, void *from, const int idx)
{
	SRC_INDEX_CHECK(nn, idx, );
	memcpy(nn->chans[src_bdx(nn, idx)].map, from,
	       tile_size(nn, src_bdx(nn, idx)));
}

void __ane_read(struct ane_nn *nn, void *to, const int idx)
{
	DST_INDEX_CHECK(nn, idx, );
	memcpy(to, nn->chans[dst_bdx(nn, idx)].map,
	       tile_size(nn, dst_bdx(nn, idx)));
}

void *__ane_src_chan(struct ane_nn *nn, const int idx)
{
	SRC_INDEX_CHECK(nn, idx, NULL);
	return nn->chans[src_bdx(nn, idx)].map;
}

void *__ane_dst_chan(struct ane_nn *nn, const int idx)
{
	DST_INDEX_CHECK(nn, idx, NULL);
	return nn->chans[dst_bdx(nn, idx)].map;
}

uint64_t __ane_src_size(struct ane_nn *nn, const int idx)
{
	SRC_INDEX_CHECK(nn, idx, 0);
	return tile_size(nn, src_bdx(nn, idx));
}

uint64_t __ane_dst_size(struct ane_nn *nn, const int idx)
{
	DST_INDEX_CHECK(nn, idx, 0);
	return tile_size(nn, dst_bdx(nn, idx));
}
