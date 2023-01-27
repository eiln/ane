// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_UTILS_H__
#define __ANE_UTILS_H__

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static inline void *ane_memalign(size_t size)
{
	void *src;
	int err = posix_memalign(&src, TILE_SIZE, size);
	if (err) {
		fprintf(stderr, "posix_memalign failed with 0x%x\n", err);
		return NULL;
	}
	return src;
}

static inline void *ane_zmemalign(size_t size)
{
	void *src = ane_memalign(size);
	if (src == NULL) {
		return src;
	}
	memset(src, 0, size);
	return src;
}

static inline void *ane_zmalloc(size_t size)
{
	void *src = malloc(size);
	if (src == NULL) {
		return src;
	}
	memset(src, 0, size);
	return src;
}

static inline size_t ane_fread(void *src, size_t size, char *fpath)
{
	FILE *infp = fopen(fpath, "rb");
	if (!infp) {
		fprintf(stderr, "failed to open file %s\n", fpath);
		return -1;
	}
	size_t read = fread(src, 1, size, infp);
	fclose(infp);

	if (read != size) {
		fprintf(stderr, "warning: only read 0x%zx/0x%zx of %s\n", read,
			size, fpath);
	}
	return read;
}

static inline size_t ane_fwrite(void *src, size_t size, char *fpath)
{
	FILE *outfp = fopen(fpath, "wb");
	if (!outfp) {
		fprintf(stderr, "failed to open file %s\n", fpath);
		return -1;
	}
	size_t wrote = fwrite(src, 1, size, outfp);
	fclose(outfp);

	if (wrote != size) {
		fprintf(stderr, "warning: only wrote 0x%zx/0x%zx of %s\n",
			wrote, size, fpath);
	}
	return wrote;
}

#endif /* __ANE_UTILS_H__ */
