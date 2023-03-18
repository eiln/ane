// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_UTILS_H__
#define __ANE_UTILS_H__

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static inline void *ane_memalign(size_t size)
{
	void *data;
	int err = posix_memalign(&data, 0x4000, size);
	if (err)
		return NULL;
	return data;
}

static inline void *ane_zmemalign(size_t size)
{
	void *data = ane_memalign(size);
	if (data == NULL)
		return NULL;
	memset(data, 0, size);
	return data;
}

static inline void *ane_zmalloc(size_t size)
{
	void *data = malloc(size);
	if (data == NULL)
		return NULL;
	memset(data, 0, size);
	return data;
}

static inline size_t ane_fread(void *src, size_t size, char *fpath)
{
	size_t read;
	FILE *infp = fopen(fpath, "rb");
	if (!infp) {
		fprintf(stderr, "LIBANE: failed to open file %s\n", fpath);
		return 0;
	}

	read = fread(src, 1, size, infp);
	fclose(infp);
	if (read != size) {
		fprintf(stderr,
			"LIBANE: warning: only read 0x%zx/0x%zx of %s\n", read,
			size, fpath);
	}
	return read;
}

static inline size_t ane_fwrite(void *src, size_t size, char *fpath)
{
	size_t wrote;
	FILE *outfp = fopen(fpath, "wb");
	if (!outfp) {
		fprintf(stderr, "LIBANE: failed to open file %s\n", fpath);
		return 0;
	}

	wrote = fwrite(src, 1, size, outfp);
	fclose(outfp);
	if (wrote != size) {
		fprintf(stderr,
			"LIBANE: warning: only wrote 0x%zx/0x%zx of %s\n",
			wrote, size, fpath);
	}
	return wrote;
}

static inline int ane_fill_random(void *src, size_t size)
{
	int fd = open("/dev/random", O_RDONLY);
	if (fd < 0)
		return fd;
	read(fd, src, size);
	close(fd);
	return 0;
}

#endif /* __ANE_UTILS_H__ */
