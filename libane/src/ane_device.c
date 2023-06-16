// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <drm.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ane_mem.h"
#include "ane_priv.h"

#define MAX_ANE_DEVICES 2
#define MAX_NODE_LEN	30
#define MAX_NODE_COUNT	64

static inline bool is_ane_device(int fd)
{
	int err;

	drm_version_t version = {};
	err = ioctl(fd, DRM_IOCTL_VERSION, &version);
	if (err < 0) {
		ane_err("failed to get drm version with %d", err);
		return false;
	}

	if (!version.name_len) {
		return false;
	}

	version.name = (char *)ane_malloc(version.name_len + 1);
	version.date_len = 0;
	version.desc_len = 0;

	err = ioctl(fd, DRM_IOCTL_VERSION, &version);
	if (err < 0) {
		ane_err("failed to get drm version with %d", err);
		free(version.name);
		return false;
	}

	/* Results might not be null-terminated strings */
	version.name[version.name_len] = '\0';
	if (strcmp(version.name, "ane") != 0) {
		free(version.name);
		return false;
	}

	free(version.name);

	return true;
}

static inline int ane_device_open(const char *node)
{
	int fd;

	fd = open(node, O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		// ane_err("failed to open node %s with %d\n", node, fd);
		return -ENODEV;
	}

	if (!is_ane_device(fd)) {
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

		fd = ane_device_open(node);
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
