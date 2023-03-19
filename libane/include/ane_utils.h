// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_UTILS_H__
#define __ANE_UTILS_H__

#include <fcntl.h>
#include <unistd.h>

static inline size_t ane_file_read(void *src, unsigned long size, char *fpath)
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

static inline size_t ane_file_write(void *src, unsigned long size, char *fpath)
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

#endif /* __ANE_UTILS_H__ */
