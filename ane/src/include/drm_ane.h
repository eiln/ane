// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __DRM_ANE_H__
#define __DRM_ANE_H__

#define TILE_SIZE      0x4000
#define TILE_SHIFT     0xe
#define MAX_TILE_COUNT 0x20

#define TILE_CMD       0x1
#define TILE_ITM       0x2
#define TILE_SRC       0x3
#define TILE_DST       0x4

struct anec {
	__u64 size;
	__u64 tsk_size;
	__u64 krn_size;
	__u32 td_count;
	__u32 td_size;
	__u32 tiles[MAX_TILE_COUNT];
	__u32 types[MAX_TILE_COUNT];
};

#define CMD_GRAN	  16
#define FIFO_NID_MAGIC	  0x40

#define DRM_ANE_NN_INIT	  0x1
#define DRM_ANE_NN_DEINIT 0x2
#define DRM_ANE_NN_SYNC	  0x3
#define DRM_ANE_NN_UNSYNC 0x4
#define DRM_ANE_NN_EXEC	  0x5

struct drm_ane_nn_init {
	__u32 handle;
	__u32 pad;
	__u64 anec_userptr;
};

struct drm_ane_nn_deinit {
	__u32 handle;
	__u32 pad;
};

struct drm_ane_nn_sync {
	__u32 handle;
	__u32 pad;
	__u64 tile_userptr[MAX_TILE_COUNT];
	__u64 fifo_userptr;
};

struct drm_ane_nn_unsync {
	__u32 handle;
	__u32 pad;
};

struct drm_ane_nn_exec {
	__u32 handle;
	__u32 pad;
};

// clang-format off
#define DRM_IOCTL_ANE_NN_INIT DRM_IOWR(DRM_COMMAND_BASE + DRM_ANE_NN_INIT, struct drm_ane_nn_init)
#define DRM_IOCTL_ANE_NN_DEINIT DRM_IOWR(DRM_COMMAND_BASE + DRM_ANE_NN_DEINIT, struct drm_ane_nn_deinit)
#define DRM_IOCTL_ANE_NN_SYNC DRM_IOWR(DRM_COMMAND_BASE + DRM_ANE_NN_SYNC, struct drm_ane_nn_sync)
#define DRM_IOCTL_ANE_NN_UNSYNC DRM_IOWR(DRM_COMMAND_BASE + DRM_ANE_NN_UNSYNC, struct drm_ane_nn_unsync)
#define DRM_IOCTL_ANE_NN_EXEC DRM_IOWR(DRM_COMMAND_BASE + DRM_ANE_NN_EXEC, struct drm_ane_nn_exec)
// clang-format on

#endif /* __DRM_ANE_H__ */
