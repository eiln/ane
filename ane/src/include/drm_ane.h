// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __DRM_ANE_H__
#define __DRM_ANE_H__

#if defined(__cplusplus)
extern "C" {
#endif

/* shared constants */
#define ANE_TILE_SIZE  0x4000
#define ANE_TILE_SHIFT 0xe
#define ANE_TILE_COUNT 0x20
#define ANE_FIFO_NID   0x40
#define ANE_CMD_GRAN   0x10

/* tile type enum */
#define ANE_TILE_CMD   0x1
#define ANE_TILE_ITM   0x2
#define ANE_TILE_DST   0x3
#define ANE_TILE_SRC   0x4

struct anec {
	__u64 size;
	__u64 tsk_size;
	__u64 krn_size;
	__u32 td_count;
	__u32 td_size;
	__u32 tiles[ANE_TILE_COUNT];
	__u32 types[ANE_TILE_COUNT];
};

#define DRM_ANE_NN_CREATE 0x1
#define DRM_ANE_NN_FREE	  0x2
#define DRM_ANE_NN_MAP	  0x3
#define DRM_ANE_NN_UNMAP  0x4
#define DRM_ANE_NN_EXEC	  0x5

struct drm_ane_nn_create {
	__u32 handle;
	__u32 pad;
	__u64 anec_userptr;
};

struct drm_ane_nn_free {
	__u32 handle;
	__u32 pad;
};

struct drm_ane_nn_map {
	__u32 handle;
	__u32 pad;
	__u64 tile_userptr[ANE_TILE_COUNT];
	__u64 fifo_userptr;
};

struct drm_ane_nn_unmap {
	__u32 handle;
	__u32 pad;
};

struct drm_ane_nn_exec {
	__u32 handle;
	__u32 pad;
};

#define DRM_IOCTL_ANE_NN_CREATE \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_ANE_NN_CREATE, struct drm_ane_nn_create)
#define DRM_IOCTL_ANE_NN_FREE \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_ANE_NN_FREE, struct drm_ane_nn_free)
#define DRM_IOCTL_ANE_NN_MAP \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_ANE_NN_MAP, struct drm_ane_nn_map)
#define DRM_IOCTL_ANE_NN_UNMAP \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_ANE_NN_UNMAP, struct drm_ane_nn_unmap)
#define DRM_IOCTL_ANE_NN_EXEC \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_ANE_NN_EXEC, struct drm_ane_nn_exec)

#if defined(__cplusplus)
}
#endif

#endif /* __DRM_ANE_H__ */
