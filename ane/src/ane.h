// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H__
#define __ANE_H__

#include <linux/iova.h>
#include <drm/drm_device.h>
#include <drm/drm_gem.h>

struct anec;
struct ane_hw;

struct ane_device {
	struct drm_device drm;
	struct device *dev;
	const struct ane_hw *hw;

	void __iomem *engine;
	void __iomem *perf;
	void __iomem *dart1;
	void __iomem *dart2;
	void __iomem *clk;

	struct iommu_domain *domain;
	struct iova_domain iovad;
	unsigned long shift;
	unsigned long limit;

	int irq;
	int dart_irq;

	struct mutex iova_lock;
	struct mutex tm_lock;
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

	u32 max_ane;
	u32 max_ne;
	u32 bar_slots;
	u32 dma0_gran;
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

struct ane_node {
	struct list_head entry;
	u32 npages;
	struct page **pages;
	dma_addr_t iova;
	u64 userptr;
};

struct ane_tile {
	struct ane_node node;
	u32 type;
};

struct ane_nn {
	struct drm_gem_object base;
	struct ane_engine_req req;
	struct anec *anecp;
	struct ane_tile *tiles[ANE_BAR_SLOTS];
	struct ane_node *fifo_node;
	int tmask[ANE_BAR_SLOTS];
	int tcount;
	unsigned int mapped;
};

#define to_nn(gem)     container_of(gem, struct ane_nn, base)
#define to_anec(nn)    (nn->anecp)
#define to_tnode(tile) (&tile->node)

#define ane_get_time(ane, msg)                             \
	(pr_info("TIME 0x%x CNTR %d: %s", readl(ane->clk), \
		 readl(ane->clk + 0x4), msg))

#endif /* __ANE_H__ */
