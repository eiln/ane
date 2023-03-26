// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H__
#define __ANE_H__

#include <drm/drm_device.h>
#include <drm/drm_mm.h>

struct ane_device {
	struct drm_device drm;
	struct device *dev;
	const struct ane_hw *hw;

	struct device **pd_dev; /* Engine/RPM power domains */
	struct device_link **pd_link;
	int pd_count;

	void __iomem *engine; /* Engine MMIO range */
	void __iomem *dart1; /* Auxiliary IOMMU range for DMA */
	void __iomem *dart2; /* Auxiliary IOMMU range for DMA */

	struct drm_mm mm; /* IOMMU space allocator */
	struct iommu_domain *domain;
	unsigned long shift; /* IOMMU shift */

	int irq;
	int dart_irq;

	struct mutex iommu_lock; /* Protects IOMMU space */
	struct mutex engine_lock; /* Protects engine queue */
};

struct ane_hw {
	u64 base;
	u32 ane_type;
	u32 ane_subtype;
	u32 ane_id;
	u32 die_id;
	u32 die_ane_id;

	struct {
		u64 base; /* Main DART base address */
		u64 dart1; /* Aux DART base address */
		u64 dart2; /* Aux DART base address */
		u64 dapf;
		u64 vm_base;
		u64 vm_size;
		u32 page_size;
		u32 ttbr; /* TTBR offset */
		u32 sel; /* Stream select offset */
		u32 cmd; /* Stream command offset */
		u32 inv; /* Stream command TLB invalidation bit */
	} dart;
};

#define ANE_BAR_SLOTS 0x20 // same as ANE_TILE_COUNT

struct ane_engine_req {
	int qid;
	u32 nid;
	u32 td_size;
	u32 td_count;
	u32 fifo_addr;
	u32 bar[ANE_BAR_SLOTS];
};

#endif /* __ANE_H__ */
