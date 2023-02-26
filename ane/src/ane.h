// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H__
#define __ANE_H__

#include <linux/iova.h>
#include <drm/drm_device.h>

struct ane_hw;

struct ane_device {
	struct drm_device drm;
	struct device *dev;
	const struct ane_hw *hw;

	struct device **pd_dev;
	struct device_link **pd_link;
	int pd_count;

	void __iomem *engine;
	void __iomem *perf;
	void __iomem *dart1;
	void __iomem *dart2;

	struct iommu_domain *domain;
	struct iova_domain iovad;
	unsigned long shift;
	unsigned long limit;

	int irq;
	int dart_irq;

	struct mutex iommu_lock;
	struct mutex engine_lock;
};

struct ane_hw {
	u64 base;
	u32 ane_type;
	u32 ane_subtype;
	u32 ane_id;
	u32 die_id;
	u32 die_ane_id;

	struct {
		u64 base;
		u64 dart1;
		u64 dart2;
		u64 dapf;
		u32 page_size;
		u64 vm_base;
		u64 vm_size;
		u32 ttbr;
		u32 sel;
		u32 cmd;
		u32 invalidate;
	} dart;
};

#define ANE_BAR_SLOTS 0x20 // same as MAX_TILE_COUNT

struct ane_engine_req {
	int qid;
	int nid;
	u32 td_size;
	u32 td_count;
	dma_addr_t fifo_addr;
	dma_addr_t bar[ANE_BAR_SLOTS];
};

#endif /* __ANE_H__ */
