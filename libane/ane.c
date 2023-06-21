// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <asm/types.h>
#include <drm.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "ane.h"
#include "drm_ane.h"

#if !defined(LIBANE_CONFIG_STFU_LOG) || !defined(LIBANE_CONFIG_STFU_ERR)
#include <stdio.h>
#endif

#ifndef LIBANE_CONFIG_STFU_LOG
#define ane_log(a, ...) printf("LIBANE: LOG: " a, ##__VA_ARGS__)
#else
#define ane_log(...) \
	do {         \
	} while (0)
#endif

#ifndef LIBANE_CONFIG_STFU_ERR
#define ane_err(a, ...) fprintf(stderr, "LIBANE: ERR: " a, ##__VA_ARGS__)
#else
#define ane_err(...) \
	do {         \
	} while (0)
#endif

#define TILE_SHIFT	   0xEUL
#define TILE_SIZE	   0x4000UL

#define tile_shift(x)	   (((uint64_t)(x)) << TILE_SHIFT)
#define tile_align(x)	   ((((uint64_t)(x)) + TILE_SIZE - 1) & -TILE_SIZE)
#define tile_size(nn, bdx) (tile_shift(to_anec(nn)->tiles[bdx]))

#define ANEC_HEADER_SIZE   0x800
#define src_bdx(nn, idx)   (4 + ane_dst_count(nn) + idx)
#define dst_bdx(nn, idx)   (4 + idx)

#define MAX_ANE_DEVICES	   2
#define MAX_NODE_LEN	   30
#define MAX_NODE_COUNT	   64

static inline void *ane_malloc(const uint64_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL) {
		ane_err("failed to malloc size 0x%lx\n", size);
		return NULL;
	}
	return ptr;
}

static inline void *ane_zmalloc(const uint64_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL) {
		ane_err("failed to malloc size 0x%lx\n", size);
		return NULL;
	}
	memset(ptr, 0, size);
	return ptr;
}

static inline void *ane_memalign(const uint64_t size)
{
	void *ptr = NULL;
	if (posix_memalign(&ptr, TILE_SIZE, size)) {
		ane_err("failed to memalign size 0x%zx\n", size);
		return NULL;
	}
	return ptr;
}

static inline void *ane_zmemalign(const uint64_t size)
{
	void *ptr = NULL;
	if (posix_memalign(&ptr, TILE_SIZE, size)) {
		ane_err("failed to memalign size 0x%lx\n", size);
		return NULL;
	}
	memset(ptr, 0, size);
	return ptr;
}

static inline void set_nid(void *td, int nid)
{
	uint32_t hdr0 = *(uint32_t *)td;
	hdr0 = (hdr0 & 0xf00ffff) | ((nid & 0xff) << 16);
	memcpy(td, &hdr0, sizeof(uint32_t));
}

static inline void set_btsp_and_command(struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);

	memcpy(nn->chans[0].map, nn->data, anec->size);

	/* do not fucking overflow */
	memcpy(nn->btsp_chan.map, nn->data, anec->td_size);
	set_nid(nn->btsp_chan.map, ANE_FIFO_NID);
}

static inline int bo_init(struct ane_nn *nn, struct ane_bo *bo)
{
	struct drm_ane_bo_init args = { .size = bo->size };
	int err = ioctl(nn->fd, DRM_IOCTL_ANE_BO_INIT, &args);
	if (err < 0) {
		ane_err("DRM_IOCTL_ANE_BO_INIT failed with 0x%x\n", err);
		return -EINVAL;
	}

	bo->handle = args.handle;
	bo->offset = args.offset;
	return 0;
}

static inline int bo_free(struct ane_nn *nn, struct ane_bo *bo)
{
	struct drm_ane_bo_free args = { .handle = bo->handle };
	return ioctl(nn->fd, DRM_IOCTL_ANE_BO_FREE, &args);
}

static inline int bo_mmap(struct ane_nn *nn, struct ane_bo *bo)
{
	bo->map = mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED, nn->fd,
		       bo->offset);

	if (bo->map == MAP_FAILED) {
		bo->map = NULL;
		ane_err("failed to mmap bo size 0x%lx\n", bo->size);
		return -EINVAL;
	}

	return 0;
}

static inline int ane_bo_init(struct ane_nn *nn, struct ane_bo *bo)
{
	int err;

	if (!bo->size)
		return -EINVAL;

	err = bo_init(nn, bo);
	if (err < 0) {
		goto error;
	}

	err = bo_mmap(nn, bo);
	if (err < 0) {
		bo_free(nn, bo);
		goto error;
	}

	return 0;

error:
	bo->handle = 0;
	bo->offset = 0;
	return err;
}

static inline void ane_bo_free(struct ane_nn *nn, struct ane_bo *bo)
{
	if (bo->map) {
		munmap(bo->map, bo->size);
		bo_free(nn, bo);
	}
	bo->map = NULL;
}

static inline void ane_chan_free(struct ane_nn *nn)
{
	ane_bo_free(nn, &nn->btsp_chan);

	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		ane_bo_free(nn, &nn->chans[bdx]);
	}
}

static inline int ane_chan_init(struct ane_nn *nn)
{
	const struct anec *anec = to_anec(nn);
	struct ane_bo *bo;
	int err;

	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		if (anec->tiles[bdx]) {
			bo = &nn->chans[bdx];
			bo->size = tile_size(nn, bdx);
			err = ane_bo_init(nn, bo);
			if (err < 0)
				goto error;
		}
	}

	bo = &nn->btsp_chan;
	bo->size = tile_align(anec->td_size);
	err = ane_bo_init(nn, bo);
	if (err < 0)
		goto error;

	set_btsp_and_command(nn);

	return 0;

error:
	ane_err("failed to init memory-mapped channels\n");
	ane_chan_free(nn);
	return err;
}

static inline int ane_fread(const char *fname, void *data, uint64_t size)
{
	uint64_t done;
	FILE *fp = fopen(fname, "rb");
	if (!fp) {
		ane_err("failed to open file %s", fname);
		return -EINVAL;
	}

	done = fread((char *)data, sizeof(char), size, fp);
	if (done != size) {
		ane_err("only read 0x%zx/0x%zx requested\n", done, size);
	}

	fclose(fp);
	return 0;
}

static inline int ane_fwrite(const char *fname, void *data, uint64_t size)
{
	uint64_t done;
	FILE *fp = fopen(fname, "wb");
	if (!fp) {
		ane_err("failed to open file %s", fname);
		return -EINVAL;
	}

	done = fwrite((char *)data, sizeof(char), size, fp);
	if (done != size) {
		ane_err("only wrote 0x%zx/0x%zx requested\n", done, size);
	}

	fclose(fp);
	return 0;
}

static inline int ane_pread(const char *fname, void *data, uint64_t size,
			    uint64_t offset)
{
	uint64_t done;
	FILE *fp = fopen(fname, "rb");
	if (!fp) {
		ane_err("failed to open file %s", fname);
		return -EINVAL;
	}

	/* Set the file position indicator in front of third double value. */
	if (fseek(fp, offset, SEEK_SET) != 0) {
		ane_err("fseek() failed on file %s", fname);
		fclose(fp);
		return -EINVAL;
	}

	done = fread((char *)data, sizeof(char), size, fp);
	if (done != size) {
		ane_err("only read 0x%zx/0x%zx requested\n", done, size);
	}

	fclose(fp);
	return 0;
}

static inline int is_ane_device(int fd)
{
	int err;

	drm_version_t version = {};
	err = ioctl(fd, DRM_IOCTL_VERSION, &version);
	if (err < 0) {
		ane_err("failed to get drm version with %d", err);
		return -EINVAL;
	}

	if (!version.name_len) {
		return -EINVAL;
	}

	version.name = (char *)ane_malloc(version.name_len + 1);
	version.date_len = 0;
	version.desc_len = 0;

	err = ioctl(fd, DRM_IOCTL_VERSION, &version);
	if (err < 0) {
		ane_err("failed to get drm version with %d", err);
		free(version.name);
		return -EINVAL;
	}

	/* Results might not be null-terminated strings */
	version.name[version.name_len] = '\0';
	if (strcmp(version.name, "ane") != 0) {
		free(version.name);
		return -EINVAL;
	}

	free(version.name);

	return 0;
}

static inline int open_fd(const char *node)
{
	int fd = open(node, O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		return -ENODEV;
	}

	if (is_ane_device(fd) < 0) {
		close(fd);
		return -EINVAL;
	}

	return fd;
}

int ane_open(int dev_id)
{
	int fd;
	char node[MAX_NODE_LEN];
	int found = 0;

	if (dev_id < 0 || dev_id >= MAX_ANE_DEVICES) {
		ane_err("invalid dev_id; 0 <= dev_id <= %d\n",
			MAX_ANE_DEVICES - 1);
		return -EINVAL;
	}

	for (int i = 0; i < MAX_NODE_COUNT; i++) {
		snprintf(node, MAX_NODE_LEN, "/dev/accel/accel%d", i);

		fd = open_fd(node);
		if (fd < 0) {
			continue;
		}

		if (dev_id == found) {
			// ane_log("found device %s fd %d\n", node, fd);
			return fd;
		}

		found++;
		close(fd);
	}

	ane_err("failed to find device with dev_id %d\n", dev_id);
	return -ENODEV;
}

void ane_close(int fd)
{
	if (!(fd < 0)) {
		close(fd);
	}
}

static inline int ane_device_open(struct ane_nn *nn, int dev_id)
{
	int fd = ane_open(dev_id);
	if (fd < 0) {
		return -EINVAL;
	}

	nn->fd = fd;

	return 0;
}

static inline void ane_device_close(struct ane_nn *nn)
{
	ane_close(nn->fd);
	nn->fd = 0;
}

static inline int ane_model_init(struct ane_nn *nn, const char *path)
{
	struct anec *anec = to_anec(nn);

	if (ane_fread(path, anec, sizeof(struct anec)) < 0) {
		return -EINVAL;
	}

	if (!anec->size) {
		ane_err("invalid anec at %s\n", path);
		return -EINVAL;
	}

	nn->data = ane_zmemalign(anec->size);
	if (!nn->data) {
		return -ENOMEM;
	}

	if (ane_pread(path, nn->data, anec->size, ANEC_HEADER_SIZE) < 0) {
		free(nn->data);
		return -EINVAL;
	}

	return 0;
}

static inline void ane_model_free(struct ane_nn *nn)
{
	free(nn->data);
}

struct ane_nn *__ane_init(const char *path, int dev_id)
{
	struct ane_nn *nn = ane_zmalloc(sizeof(struct ane_nn));
	if (!nn) {
		ane_err("failed to alloc mem for nn struct\n");
		return NULL;
	}

	if (ane_model_init(nn, path) < 0) {
		ane_err("failed to load model at %s\n", path);
		free(nn);
		return NULL;
	}

	if (ane_device_open(nn, dev_id) < 0) {
		ane_err("failed to open device with dev_id %d\n", dev_id);
		ane_model_free(nn);
		free(nn);
		return NULL;
	}

	if (ane_chan_init(nn) < 0) {
		ane_device_close(nn);
		ane_model_free(nn);
		free(nn);
		return NULL;
	}

	return nn;
}

void __ane_free(struct ane_nn *nn)
{
	ane_chan_free(nn);
	ane_device_close(nn);
	ane_model_free(nn);
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

	return ioctl(nn->fd, DRM_IOCTL_ANE_SUBMIT, &args);
}

#ifndef LIBANE_CONFIG_NO_INDEX_CHECK
#define SRC_INDEX_CHECK(nn, idx, ret)                                          \
	({                                                                     \
		if (idx >= ane_src_count(nn)) {                                \
			ane_err("tried to index %d but max is %d; bailing.\n", \
				idx, ane_src_count(nn));                       \
			return ret;                                            \
		}                                                              \
	})
#else
#define SRC_INDEX_CHECK(nn, idx, ret) \
	do {                          \
	} while (0)
#endif /* LIBANE_CONFIG_NO_INDEX_CHECK */

#ifndef LIBANE_CONFIG_NO_INDEX_CHECK
#define DST_INDEX_CHECK(nn, idx, ret)                                          \
	({                                                                     \
		if (idx >= ane_dst_count(nn)) {                                \
			ane_err("tried to index %d but max is %d; bailing.\n", \
				idx, ane_dst_count(nn));                       \
			return ret;                                            \
		}                                                              \
	})
#else
#define DST_INDEX_CHECK(nn, idx, ret) \
	do {                          \
	} while (0)
#endif /* LIBANE_CONFIG_NO_INDEX_CHECK */

uint64_t __ane_src_size(struct ane_nn *nn, const uint32_t idx)
{
	SRC_INDEX_CHECK(nn, idx, 0);
	return tile_size(nn, src_bdx(nn, idx));
}

uint64_t __ane_dst_size(struct ane_nn *nn, const uint32_t idx)
{
	DST_INDEX_CHECK(nn, idx, 0);
	return tile_size(nn, dst_bdx(nn, idx));
}

uint64_t __ane_src_elem(struct ane_nn *nn, const uint32_t idx)
{
	SRC_INDEX_CHECK(nn, idx, 0);
	return to_anec(nn)->nchw[src_bdx(nn, idx)][0] *
	       to_anec(nn)->nchw[src_bdx(nn, idx)][1] *
	       to_anec(nn)->nchw[src_bdx(nn, idx)][2] *
	       to_anec(nn)->nchw[src_bdx(nn, idx)][3];
}

uint64_t __ane_dst_elem(struct ane_nn *nn, const uint32_t idx)
{
	DST_INDEX_CHECK(nn, idx, 0);
	return to_anec(nn)->nchw[dst_bdx(nn, idx)][0] *
	       to_anec(nn)->nchw[dst_bdx(nn, idx)][1] *
	       to_anec(nn)->nchw[dst_bdx(nn, idx)][2] *
	       to_anec(nn)->nchw[dst_bdx(nn, idx)][3];
}

uint64_t __ane_src_shape_n(struct ane_nn *nn, const uint32_t idx)
{
	SRC_INDEX_CHECK(nn, idx, 0);
	return to_anec(nn)->nchw[src_bdx(nn, idx)][0];
}

uint64_t __ane_src_shape_c(struct ane_nn *nn, const uint32_t idx)
{
	SRC_INDEX_CHECK(nn, idx, 0);
	return to_anec(nn)->nchw[src_bdx(nn, idx)][1];
}

uint64_t __ane_src_shape_h(struct ane_nn *nn, const uint32_t idx)
{
	SRC_INDEX_CHECK(nn, idx, 0);
	return to_anec(nn)->nchw[src_bdx(nn, idx)][2];
}

uint64_t __ane_src_shape_w(struct ane_nn *nn, const uint32_t idx)
{
	SRC_INDEX_CHECK(nn, idx, 0);
	return to_anec(nn)->nchw[src_bdx(nn, idx)][3];
}

uint64_t __ane_dst_shape_n(struct ane_nn *nn, const uint32_t idx)
{
	DST_INDEX_CHECK(nn, idx, 0);
	return to_anec(nn)->nchw[dst_bdx(nn, idx)][0];
}

uint64_t __ane_dst_shape_c(struct ane_nn *nn, const uint32_t idx)
{
	DST_INDEX_CHECK(nn, idx, 0);
	return to_anec(nn)->nchw[dst_bdx(nn, idx)][1];
}

uint64_t __ane_dst_shape_h(struct ane_nn *nn, const uint32_t idx)
{
	DST_INDEX_CHECK(nn, idx, 0);
	return to_anec(nn)->nchw[dst_bdx(nn, idx)][2];
}

uint64_t __ane_dst_shape_w(struct ane_nn *nn, const uint32_t idx)
{
	DST_INDEX_CHECK(nn, idx, 0);
	return to_anec(nn)->nchw[dst_bdx(nn, idx)][3];
}

void *__ane_src_chan(struct ane_nn *nn, const uint32_t idx)
{
	SRC_INDEX_CHECK(nn, idx, NULL);
	return nn->chans[src_bdx(nn, idx)].map;
}

void *__ane_dst_chan(struct ane_nn *nn, const uint32_t idx)
{
	DST_INDEX_CHECK(nn, idx, NULL);
	return nn->chans[dst_bdx(nn, idx)].map;
}

void __ane_send(struct ane_nn *nn, void *from, const uint32_t idx)
{
	SRC_INDEX_CHECK(nn, idx, );
	memcpy(nn->chans[src_bdx(nn, idx)].map, from,
	       tile_size(nn, src_bdx(nn, idx)));
}

void __ane_read(struct ane_nn *nn, void *to, const uint32_t idx)
{
	DST_INDEX_CHECK(nn, idx, );
	memcpy(to, nn->chans[dst_bdx(nn, idx)].map,
	       tile_size(nn, dst_bdx(nn, idx)));
}

void ane_tile(void *data, void *tile, const uint64_t N, const uint64_t C,
	      const uint64_t H, const uint64_t W, const uint64_t P,
	      const uint64_t R)
{
	const uint64_t new_H = P / R;
	const uint64_t new_W = R / sizeof(uint16_t);
	const uint64_t stride = W * sizeof(uint16_t);

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				void *src = ((void *)(data)) +
					    ((n * (C * H * W) + c * (H * W) +
					      h * (W)) *
					     sizeof(uint16_t));
				void *dst =
					((void *)(tile)) +
					((n * (C * new_H * new_W) +
					  c * (new_H * new_W) + h * (new_W)) *
					 sizeof(uint16_t));
				memcpy(dst, src, stride);
			}
		}
	}
}

void ane_untile(void *data, void *tile, const uint64_t N, const uint64_t C,
		const uint64_t H, const uint64_t W, const uint64_t P,
		const uint64_t R)
{
	const uint64_t new_H = P / R;
	const uint64_t new_W = R / sizeof(uint16_t);
	const uint64_t stride = W * sizeof(uint16_t);

	memset(data, 0, N * C * H * W * sizeof(uint16_t));

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				void *src =
					((void *)(tile)) +
					((n * (C * new_H * new_W) +
					  c * (new_H * new_W) + h * (new_W)) *
					 sizeof(uint16_t));
				void *dst = ((void *)(data)) +
					    ((n * (C * H * W) + c * (H * W) +
					      h * (W)) *
					     sizeof(uint16_t));
				memcpy(dst, src, stride);
			}
		}
	}
}

static inline void ___ane_tile_send(struct ane_nn *nn, void *from,
				    const uint32_t idx)
{
	const struct anec *anec = to_anec(nn);
	const int bdx = src_bdx(nn, idx);

	uint16_t tile[tile_size(nn, bdx) / sizeof(uint16_t)];
	memset(tile, 0, tile_size(nn, bdx));

	ane_tile(from, tile, anec->nchw[bdx][0], anec->nchw[bdx][1],
		 anec->nchw[bdx][2], anec->nchw[bdx][3], anec->nchw[bdx][4],
		 anec->nchw[bdx][5]);
	memcpy(nn->chans[bdx].map, tile, tile_size(nn, bdx));
}

static inline void ___ane_tile_read(struct ane_nn *nn, void *to,
				    const uint32_t idx)
{
	const struct anec *anec = to_anec(nn);
	const int bdx = dst_bdx(nn, idx);

	uint16_t tile[tile_size(nn, bdx) / sizeof(uint16_t)];
	memcpy(tile, nn->chans[bdx].map, tile_size(nn, bdx));

	ane_untile(to, tile, anec->nchw[bdx][0], anec->nchw[bdx][1],
		   anec->nchw[bdx][2], anec->nchw[bdx][3], anec->nchw[bdx][4],
		   anec->nchw[bdx][5]);
}

void __ane_tile_send(struct ane_nn *nn, void *from, const uint32_t idx)
{
	SRC_INDEX_CHECK(nn, idx, );
	___ane_tile_send(nn, from, idx);
}

void __ane_tile_read(struct ane_nn *nn, void *to, const uint32_t idx)
{
	DST_INDEX_CHECK(nn, idx, );
	___ane_tile_read(nn, to, idx);
}
