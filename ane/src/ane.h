// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H__
#define __ANE_H__

#include <drm/drm_device.h>
#include <drm/drm_mm.h>

#include "include/drm_ane.h"

struct ane_device {
	struct drm_device drm;
	struct device *dev;
	const struct ane_hw *hw; /* HW-specific data */

	struct device **pd_dev; /* Engine/RPM power domains */
	struct device_link **pd_link;
	int pd_count;

	void __iomem *engine; /* Engine MMIO */
	void __iomem *dart0; /* Main IOMMU MMIO shared with IOMMU driver */
	void __iomem *dart1; /* Auxiliary IOMMU for DMA */
	void __iomem *dart2; /* Auxiliary IOMMU for DMA */

	struct drm_mm mm; /* IOMMU space (iova) allocator */
	struct iommu_domain *domain;
	unsigned long shift; /* IOMMU page shift */

	int irq;
	int dart_irq; /* IOMMU IRQ shared with IOMMU driver */

	struct mutex iommu_lock; /* Protects IOMMU space */
	struct mutex engine_lock; /* Protects engine queue */
};

struct ane_hw {
	struct {
		u64 vm_base;
		u64 vm_size;
		u32 ttbr; /* TTBR offset */
		u32 select; /* Stream select offset */
		u32 command; /* Stream command offset */
		u32 invalidate; /* Stream command TLB invalidation bit */
	} dart;
};

struct ane_request {
	int qid;
	u32 nid;
	u32 td_size;
	u32 td_count;
	u32 btsp_iova;
	u32 bar[ANE_TILE_COUNT];
};

#endif /* __ANE_H__ */
