// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_IO_H__
#define __ANE_IO_H__

#include <unistd.h>

#include "ane_priv.h"

static inline int ane_fread(const char *fname, void *data, size_t size)
{
	size_t done;
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

static inline int ane_fwrite(const char *fname, void *data, size_t size)
{
	size_t done;
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

static inline int ane_pread(const char *fname, void *data, size_t size,
			    size_t offset)
{
	size_t done;
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

#endif /* __ANE_IO_H__ */
