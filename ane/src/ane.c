// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */
/* IOMMU portions based on iommu/apple-dart.c & gpu/drm/tegra/drm.c */

#define pr_fmt(fmt) "%s: %s: " fmt, KBUILD_MODNAME, __func__

#include <linux/interrupt.h> // sigh
#include <linux/iommu.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_domain.h>
#include <linux/pm_runtime.h>

#include <drm/drm_drv.h>
#include <drm/drm_file.h>
#include <drm/drm_ioctl.h>

#include "ane.h"
#include "ane_tm.h"
#include "include/drm_ane.h"

static void ane_iommu_invalidate_tlb(struct ane_device *ane)
{
	/* 
	 * Figured out exactly 12 minutes ago that TLBs for the other two
	 * "darts" need to be invalidated to *really* flush those old iovas.
	 * There's an odd relationship here; dart1 & dart2 aren't actual
	 * darts with dart capabilities to grant a dart device, but they have 
	 * a register range that closely resembles one, and some of those
	 * registers need to be synced to the main dart for the DMA channels
	 * to not panic. Specifically, TTBR and, now I know, TLB streams.
	 * It's ugly but it works. And I don't think it's worth patching dart.
	 * 
	 */

	mutex_lock(&ane->iova_lock);

	// calls apple_dart_flush_iotlb_all()
	iommu_flush_iotlb_all(ane->domain);

	/* STREAM_SELECT & STREAM_COMMAND (INVALIDATE=1, BUSY=0) */
	// writel(0x1, base + 0x34);
	// writel(0x100000, base + 0x20);
	writel(0x1, ane->dart1 + ane->hw->dart.sel);
	writel(ane->hw->dart.invalidate, ane->dart1 + ane->hw->dart.cmd);
	writel(0x1, ane->dart2 + ane->hw->dart.sel);
	writel(ane->hw->dart.invalidate, ane->dart2 + ane->hw->dart.cmd);

	/* STREAM_COMMAND = 0x100000 (INVALIDATE=1, BUSY=0) */
	// writel(0x100000, base + 0x20);
	writel(ane->hw->dart.invalidate, ane->dart1 + ane->hw->dart.cmd);
	writel(ane->hw->dart.invalidate, ane->dart2 + ane->hw->dart.cmd);

	writel(0x1, ane->dart1 + ane->hw->dart.sel);
	writel(ane->hw->dart.invalidate, ane->dart1 + ane->hw->dart.cmd);
	writel(0x1, ane->dart2 + ane->hw->dart.sel);
	writel(ane->hw->dart.invalidate, ane->dart2 + ane->hw->dart.cmd);

	writel(ane->hw->dart.invalidate, ane->dart1 + ane->hw->dart.cmd);
	writel(ane->hw->dart.invalidate, ane->dart2 + ane->hw->dart.cmd);

	mutex_unlock(&ane->iova_lock);

	return;
}

static void ane_mm_free_pages(struct ane_node *node)
{
	for (int i = 0; i < node->npages && node->pages[i] != NULL; i++) {
		__free_page(node->pages[i]);
	}
	kvfree(node->pages);
	node->pages = NULL;
	return;
}

static int ane_mm_alloc_pages(struct ane_node *node)
{
	node->pages =
		kvmalloc_array(node->npages, sizeof(*node->pages), GFP_KERNEL);
	if (!node->pages)
		return -ENOMEM;

	for (int i = 0; i < node->npages; i++) {
		node->pages[i] = alloc_page(GFP_KERNEL);
		if (node->pages[i] == NULL) {
			goto free_pages;
		}
	}

	return 0;

free_pages:
	ane_mm_free_pages(node);
	return -ENOMEM;
}

static int ane_mm_free_user_pages(struct ane_node *node)
{
	for (int i = 0; i < node->npages; i++) {
		if (node->pages[i]) {
			put_page(node->pages[i]);
			node->pages[i] = NULL;
		}
	}
	kvfree(node->pages);
	node->pages = NULL;
	return 0;
}

static int ane_mm_get_user_pages(struct ane_node *node)
{
	int err;
	long pinned;

	node->pages =
		kvmalloc_array(node->npages, sizeof(struct page *), GFP_KERNEL);
	if (!node->pages)
		return -ENOMEM;

	pinned = get_user_pages(node->userptr, node->npages, FOLL_WRITE,
				node->pages, NULL);
	if (pinned < 0) {
		err = pinned;
		goto free_pages;
	}

	pr_info("pinned %lu/%d pages\n", pinned, node->npages);

	if (pinned < node->npages) {
		err = -ENOMEM;
		goto free_pages;
	}

	return 0;

free_pages:
	ane_mm_free_user_pages(node);
	return err;
}

static int ane_iommu_map_node(struct ane_device *ane, struct ane_node *node)
{
	int err = 0;

	mutex_lock(&ane->iova_lock);

	for (int i = 0; i < node->npages; i++) {
		struct page *p = node->pages[i];
		dma_addr_t iova = node->iova + (i << ane->shift);

		// calls dart_map_pages()
		err = iommu_map(ane->domain, iova, page_to_phys(p),
				1UL << ane->shift, IOMMU_READ | IOMMU_WRITE);
		if (err) {
			pr_err("iommu_map failed for iova 0x%llx @ 0x%x/0x%x",
			       iova, i, node->npages);
			for (int j = 0; j < i; j++) {
				iommu_unmap(ane->domain,
					    node->iova + (j << ane->shift),
					    1UL << ane->shift);
			}
			break;
		}
	}

	mutex_unlock(&ane->iova_lock);

	return err;
}

static int ane_iommu_unmap_node(struct ane_device *ane, struct ane_node *node)
{
	/* each page was iommu_mapped individually */
	/* so the unmap call also has to be looped */
	mutex_lock(&ane->iova_lock);
	for (int i = 0; i < node->npages; i++) {
		dma_addr_t iova = node->iova + (i << ane->shift);
		iommu_unmap(ane->domain, iova, 1UL << ane->shift);
	}
	mutex_unlock(&ane->iova_lock);
	return 0;
}

static int ane_mm_resv_iova(struct ane_device *ane, struct ane_node *node)
{
	struct iova *alloc =
		alloc_iova(&ane->iovad, node->npages, ane->limit, true);
	if (!alloc) {
		return -ENOMEM;
	}
	node->iova = iova_dma_addr(&ane->iovad, alloc);
	return 0;
}

static void ane_mm_unresv_iova(struct ane_device *ane, struct ane_node *node)
{
	free_iova(&ane->iovad, iova_pfn(&ane->iovad, node->iova));
	return;
}

static void ane_nn_unresv_bar(struct ane_device *ane, struct ane_nn *nn)
{
	mutex_lock(&ane->iova_lock);
	for (int i = 0; i < nn->tcount; i++) {
		int bdx = nn->tmask[i];
		struct ane_tile *tile = nn->tiles[bdx];
		struct ane_node *node = &tile->node;
		if (node->iova) {
			ane_mm_unresv_iova(ane, node);
		}
		nn->req.bar[bdx] = 0;
		kfree(tile);
	}
	mutex_unlock(&ane->iova_lock);
	return;
}

static int ane_nn_resv_bar(struct ane_device *ane, struct ane_nn *nn)
{
	int err;
	struct anec *anec = to_anec(nn);

	/* 
	 * BAR is a 0x20 hardware queue of iovas for DMA chans.
	 * It's "base" in the sense of "starting iova" for the 
	 * rasterizer unit broadcasting tiled segment workloads,
	 * not "physical base" as in translation tables.
	 * 
	 * [0] - register state descriptor
	 * [1] - weights data
	 * [2] - intermediate tile 2, if it exists
	 * [3] - intermediate tile 1, if it exists
	 * [4] - output tile 1
	 * [N] - input tile 1
	 * 
	 * where output tile iovas occupy from 4 to (N - 1) and
	 * input tile iovas occupy from N to (N + input_count).
	 */

	mutex_lock(&ane->iova_lock);

	for (int i = 0; i < nn->tcount; i++) {
		int bdx = nn->tmask[i];
		struct ane_node *node = NULL;
		struct ane_tile *tile =
			kzalloc(sizeof(struct ane_tile), GFP_KERNEL);
		if (!tile) {
			err = -ENOMEM;
			goto unresv_bar;
		}
		node = &tile->node;
		nn->tiles[bdx] = tile;

		tile->type = anec->types[bdx];
		node->npages = anec->tiles[bdx];

		if (tile->type != TILE_SRC_CHAIN) {
			err = ane_mm_resv_iova(ane, node);
			if (err < 0) {
				pr_err("failed to reserve iova\n");
				goto unresv_bar;
			}
		} else {
			pr_info("found chained DRAM dependency @ src %d\n",
				bdx);
			node->iova = 0;
		}
		nn->req.bar[bdx] = node->iova;
	}

	mutex_unlock(&ane->iova_lock);

	/* 
	 * The command buffer is the register state descriptor catted with
	 * the weights data (kernel) @ 64 gran padding. Both are "immutable",
	 * meaning its dedicated DMA channel is a one-way, read-only one.
	 * 
	 * Before inputs are even thought of, kernel coefficients are
	 * prefetched and loaded onto the engine cores as instructed by the
	 * control registers, which are DMA'd to from the register state
	 * descriptor as step 0.
	 * 
	 * I'm pretty sure it's packed this way for efficient bank aligned
	 * access, but it's also a convienient serialization method, making one
	 * less non-aligned buffer to worry about. We thus unpack the iova at
	 * which the kernel _should_ start; if it doesn't the req will fail
	 * and we have the queue resetter for that :).
	 */

	nn->req.bar[1] =
		nn->req.bar[0] + roundup(anec->tsk_size, ane->hw->dma0_gran);

	return 0;

unresv_bar:
	mutex_unlock(&ane->iova_lock);
	ane_nn_unresv_bar(ane, nn);
	return err;
}

static int ane_nn_resv_fifo(struct ane_device *ane, struct ane_nn *nn)
{
	int err;
	struct ane_node *node = kzalloc(sizeof(struct ane_node), GFP_KERNEL);
	if (!node)
		return -ENOMEM;
	nn->fifo_node = node;

	node->npages = 1;
	err = ane_mm_resv_iova(ane, node);
	if (err < 0) {
		pr_err("failed to reserve iova\n");
		goto free_kmem;
	}

	nn->req.nid = FIFO_NID_MAGIC;
	nn->req.fifo_addr = node->iova;

	return 0;

free_kmem:
	kfree(node);
	return 0;
}

static void ane_nn_unresv_fifo(struct ane_device *ane, struct ane_nn *nn)
{
	struct ane_node *node = nn->fifo_node;
	ane_mm_unresv_iova(ane, node);
	kfree(node);
	return;
}

static int ane_nn_validate_args(struct ane_device *ane, struct ane_nn *nn)
{
	struct anec *anec = to_anec(nn);
	int tcount = 0;

	if (!anec->size || !anec->tsk_size || !anec->krn_size ||
	    !anec->td_size || !anec->td_count) {
		return -EINVAL;
	}

	if (anec->tsk_size < anec->td_size || anec->size <= anec->tsk_size ||
	    anec->size <= anec->krn_size) {
		return -EINVAL;
	}

	if (anec->td_count >= 0xffff) {
		pr_err("td_count exceeds limit\n");
		return -EINVAL;
	}

	if ((((anec->td_count - 1) * roundup(anec->td_size, 0x100)) +
	     anec->td_size) != anec->tsk_size) {
		pr_err("invalid tsk size\n");
		return -EINVAL;
	}

	if (!IS_ALIGNED(anec->size, ane->hw->dma0_gran)) {
		pr_err("cmd buf not bank aligned\n");
		return -EINVAL;
	}

	if (!anec->tiles[0] || anec->types[0] != TILE_CMD || anec->tiles[1]) {
		pr_err("invalid cmd stack\n");
		return -EINVAL;
	}

	if (anec->tiles[2] && !anec->tiles[3]) {
		pr_err("invalid itm stack\n");
		return -EINVAL;
	}

	// clang-format off
	for (int i = 0; i < MAX_TILE_COUNT; i++) {
		if (anec->tiles[i]) {
			nn->tmask[tcount] = i;
			tcount++;
		}
		if (anec->tiles[i] >= 0x10000) {
			pr_err("tile size exceeds limit\n");
			return -EINVAL;
		}
		if (anec->tiles[i] && !anec->types[i]) {
			return -EINVAL;
		}
		if (!anec->tiles[i] && anec->types[i]) {
			return -EINVAL;
		}
		if (anec->tiles[i] && 
		    anec->types[i] != TILE_CMD &&
		    anec->types[i] != TILE_ITM && 
		    anec->types[i] != TILE_SRC &&
		    anec->types[i] != TILE_DST &&
		    anec->types[i] != TILE_DST_CHAIN &&
		    anec->types[i] != TILE_SRC_CHAIN) {
			pr_err("invalid tile type\n");
			return -EINVAL;
		}
	}
	// clang-format on

	nn->tcount = tcount;

	return 0;
}

static const struct vm_operations_struct drm_gem_ane_vm_ops = {
	.open = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};

static const struct drm_gem_object_funcs ane_gem_object_funcs = {
	.vm_ops = &drm_gem_ane_vm_ops,
};

static int ane_nn_init(struct drm_device *drm, void *data,
		       struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	struct drm_ane_nn_init *args = data;
	struct drm_gem_object *gem = NULL;
	struct ane_nn *nn = NULL;
	struct anec *anec = NULL;
	int err;

	if (args->pad) {
		return -EINVAL;
	}

	if (!(nn = kzalloc(sizeof(struct ane_nn), GFP_KERNEL)))
		return -ENOMEM;
	if (!(anec = kzalloc(sizeof(struct ane_nn), GFP_KERNEL))) {
		kfree(nn);
		return -ENOMEM;
	}

	err = copy_from_user(anec,
			     (void __user *)(uintptr_t)args->anecs_userptr,
			     sizeof(struct anec));
	if (err) {
		pr_err("failed to copy 0x%x bytes from user\n", err);
		err = -ENOMEM;
		goto free_kmem;
	}
	nn->anecp = anec;

	err = ane_nn_validate_args(ane, nn);
	if (err) {
		pr_err("invalid anec setup\n");
		goto free_kmem;
	}

	nn->mapped = 0;
	nn->req.td_size = anec->td_size;
	nn->req.td_count = anec->td_count;
	pr_info("received nn req with size: 0x%llx td_count: 0x%x\n",
		anec->size, nn->req.td_count);

	/* fake mini gem backing for the sake of a handle */
	gem = &nn->base;
	gem->funcs = &ane_gem_object_funcs;
	err = drm_gem_object_init(drm, gem, PAGE_SIZE);
	if (err < 0)
		goto free_kmem;

	err = drm_gem_handle_create(file, gem, &args->handle);
	drm_gem_object_put(gem); // handle holds it now
	if (err < 0)
		goto object_rel;

	err = ane_nn_resv_bar(ane, nn);
	if (err < 0) {
		pr_err("failed to resv bar\n");
		goto handle_del;
	}

	err = ane_nn_resv_fifo(ane, nn);
	if (err < 0) {
		pr_err("failed to resv fifo\n");
		goto unresv_bar;
	}

	return 0;

unresv_bar:
	ane_nn_unresv_bar(ane, nn);
handle_del:
	drm_gem_handle_delete(file, args->handle);
object_rel:
	drm_gem_object_release(gem);
free_kmem:
	kfree(anec);
	kfree(nn);
	return err;
}

static int ane_nn_deinit(struct drm_device *drm, void *data,
			 struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	struct drm_ane_nn_deinit *args = data;

	struct drm_gem_object *gem = drm_gem_object_lookup(file, args->handle);
	struct ane_nn *nn;
	if (args->pad || gem == NULL)
		return -EINVAL;
	nn = to_nn(gem);

	ane_nn_unresv_fifo(ane, nn);
	ane_nn_unresv_bar(ane, nn);

	drm_gem_handle_delete(file, args->handle);
	drm_gem_object_release(gem);

	kfree(nn->anecp);
	kfree(nn);
	return 0;
}

static int ane_nn_chain(struct drm_device *drm, void *data,
			struct drm_file *file)
{
	/* Not done. At all. Nothing to see. */
	struct drm_ane_nn_chain *args = data;
	struct drm_gem_object *gem;
	struct ane_nn *nn, *nxt;
	int err = 0;

	gem = drm_gem_object_lookup(file, args->handle);
	if (gem == NULL)
		return -EINVAL;
	nn = to_nn(gem);

	gem = drm_gem_object_lookup(file, args->handle_nxt);
	if (gem == NULL)
		return -EINVAL;
	nxt = to_nn(gem);

	for (int i = 0; i < MAX_TILE_COUNT; i++) {
		if (args->chain[i]) {
			struct ane_tile *prev, *next;
			if (args->chain[i] >= MAX_TILE_COUNT || i <= 3 ||
			    args->chain[i] <= 3) { // itm bdx base
				err = -EINVAL;
				goto error;
			}
			prev = nn->tiles[i];
			next = nxt->tiles[args->chain[i]];

			if (!to_tnode(prev)->iova || to_tnode(next)->iova) {
				pr_err("prev not reserved or next reserved\n");
				err = -EINVAL;
				goto error;
			}
#if 0 
			// if we only copy the iova to BAR, which is what
			// actually ends up in the hardware, the noopd
			// dependency can be faked somewhat
			nxt->req.bar[args->chain[i]] = nn->req.bar[i];
			
			// cool msg so it looks like i'm doing something
			pr_info("chaining dst %d to src %d @ iova 0x%llx\n", i,
				args->chain[i], nxt->req.bar[args->chain[i]]);
#endif
		}
	}
error:
	return err;
}

static int __ane_nn_unsync(struct ane_device *ane, struct ane_nn *nn)
{
	if (!nn->mapped) {
		pr_warn("double free?\n");
	}

	for (int i = 0; i < nn->tcount; i++) {
		int bdx = nn->tmask[i];
		struct ane_tile *tile = nn->tiles[bdx];
		struct ane_node *node = to_tnode(tile);

		if (tile->type == TILE_ITM || tile->type == TILE_DST_CHAIN) {
			ane_mm_free_pages(node);
		} else if (tile->type == TILE_CMD || tile->type == TILE_SRC ||
			   tile->type == TILE_DST) {
			ane_mm_free_user_pages(node);
		} else if (tile->type == TILE_SRC_CHAIN) {
			;
		}

		if (tile->type != TILE_SRC_CHAIN) {
			ane_iommu_unmap_node(ane, node);
		}
	}

	ane_mm_free_user_pages(nn->fifo_node);
	ane_iommu_unmap_node(ane, nn->fifo_node);

	ane_iommu_invalidate_tlb(ane);

	nn->mapped = 0;

	return 0;
}

static int ane_nn_unsync(struct drm_device *drm, void *data,
			 struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	struct drm_ane_nn_unsync *args = data;
	struct drm_gem_object *gem = drm_gem_object_lookup(file, args->handle);
	if (args->pad || gem == NULL)
		return -EINVAL;
	__ane_nn_unsync(ane, to_nn(gem));
	return 0;
}

static int ane_nn_sync(struct drm_device *drm, void *data,
		       struct drm_file *file)
{
	/* 
	 * Pages need to be mapped to IOMMU space the DMA channels operate on;
	 * these chans expect a continuous _iova_ block. Physically continuous
	 * mem is out of the question given the size of these neural nets.
	 * As the DMA API doesn't let this happen over non-continuous mem,
	 * each page must be iommu_map()'d on its reserved iova offset.
	 * 
	 * Current impl uses user pages pinned by get_user_pages() as 
	 * backings for the continuous iova block to be DMA'd. By this iommu
	 * glue, these mappings become both user and device continuous,
	 * saving a lot of overhead. Corrections were made s.t. the kernel
	 * never accesses these pages.
	 * 
	 * This isn't DRM typical, but Intel's latest VPU patch also has 
	 * pin_user_pages(), and I couldn't think of a better way to do this.
	 * 
	 */

	struct ane_device *ane = drm->dev_private;
	struct drm_ane_nn_sync *args = data;
	int err = 0;

	struct drm_gem_object *gem = drm_gem_object_lookup(file, args->handle);
	struct ane_nn *nn;
	struct anec *anec;
	if (gem == NULL || args->pad)
		return -EINVAL;

	nn = to_nn(gem);
	anec = to_anec(nn);
	if (nn->mapped)
		return -EINVAL;

	for (int i = 0; i < nn->tcount; i++) {
		int bdx = nn->tmask[i];
		struct ane_tile *tile = nn->tiles[bdx];
		struct ane_node *node = to_tnode(tile);

		/* 
		 * Intermediate buffers are just swap space the engine demands
		 * when operation exceeds L2. Users don't need them whatsoever.
		 * Back with alloc_page() & keep it in here.
		 * 
		 */
		if (tile->type == TILE_ITM || tile->type == TILE_DST_CHAIN) {
			if (args->userptrs[bdx]) {
				err = -EINVAL;
				goto exit;
			}
			err = ane_mm_alloc_pages(node);
		} else if (tile->type == TILE_CMD || tile->type == TILE_SRC ||
			   tile->type == TILE_DST) {
			if (!args->userptrs[bdx]) {
				err = -EINVAL;
				goto exit;
			}
			node->userptr = args->userptrs[bdx];
			err = ane_mm_get_user_pages(node);
		} else if (tile->type == TILE_SRC_CHAIN) {
			;
		}

		if (err) {
			pr_err("failed to allocate backing pages\n");
			goto exit;
		}

		if (tile->type != TILE_SRC_CHAIN) {
			err = ane_iommu_map_node(ane, node);
		}

		if (err) {
			pr_err("failed to map pages to device space\n");
			goto exit;
		}
	}

	nn->fifo_node->userptr = args->fifo_userptr;
	err = ane_mm_get_user_pages(nn->fifo_node);
	if (err) {
		pr_err("failed to allocate backing pages\n");
		goto exit;
	}

	err = ane_iommu_map_node(ane, nn->fifo_node);
	if (err) {
		pr_err("failed to map pages to device space\n");
		goto exit;
	}

	for (int i = 0; i < MAX_TILE_COUNT; i++) {
		if (nn->req.bar[i]) {
			pr_info("BAR %d: 0x%llx\n", i, nn->req.bar[i]);
		}
	}
	
	nn->mapped = 1;

	return 0;

exit:
	__ane_nn_unsync(ane, nn);
	return err;
}

static int ane_hw_reset(struct ane_device *ane)
{
	pr_err("hard resetting to recover\n");
	return 0;
}

static int ane_nn_exec(struct drm_device *drm, void *data,
		       struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	struct drm_ane_nn_exec *args = data;
	struct drm_gem_object *gem = drm_gem_object_lookup(file, args->handle);
	struct ane_nn *nn;
	int err;
	if (gem == NULL || args->pad)
		return -EINVAL;

	nn = to_nn(gem);
	if (!nn->mapped) {
		pr_err("not mapped to device space\n");
		goto error;
	}

	/* 
	 * Queues are bottlenecked by the fact that the cores (in parallel)
	 * can only process one request at a time. I'm not sure if it's 
	 * worth implementing scheduling logic for essentially <40 r32's. 
	 * I have yet to see Apple use any of the other queues either.
	 * 
	 */

	ane_get_time(ane, "scheduling request on engine...\n");
	mutex_lock(&ane->tm_lock);

	err = ane_tm_enqueue_tq(ane, &nn->req);
	if (err < 0)
		goto error;

	err = ane_tm_execute_tq(ane, &nn->req);
	if (err < 0)
		goto error;

	mutex_unlock(&ane->tm_lock);
	ane_get_time(ane, "finished execution!\n");

	return 0;

error: // well shit
	mutex_unlock(&ane->tm_lock);
	ane_hw_reset(ane);
	return err;
}

static const struct drm_ioctl_desc ane_drm_driver_ioctls[] = {
	DRM_IOCTL_DEF_DRV(ANE_NN_INIT, ane_nn_init, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(ANE_NN_DEINIT, ane_nn_deinit, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(ANE_NN_SYNC, ane_nn_sync, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(ANE_NN_UNSYNC, ane_nn_unsync, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(ANE_NN_EXEC, ane_nn_exec, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(ANE_NN_CHAIN, ane_nn_chain, DRM_RENDER_ALLOW),
};

static int ane_drm_open(struct drm_device *drm, struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	int ret;

	/* need to bring up power immediately if opening device */
	ret = pm_runtime_resume_and_get(ane->dev);
	if (ret < 0 && ret != -EACCES) {
		pm_runtime_put_autosuspend(ane->dev);
		return ret;
	}

	pm_runtime_mark_last_busy(ane->dev);
	pm_runtime_put_autosuspend(ane->dev);
	return ret;
}

static void ane_drm_postclose(struct drm_device *drm, struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	pm_runtime_resume_and_get(ane->dev);

	pm_runtime_mark_last_busy(ane->dev);
	pm_runtime_put_autosuspend(ane->dev);
	return;
}

long ane_drm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct drm_file *filp = file->private_data;
	struct drm_device *drm = filp->minor->dev;
	struct ane_device *ane = drm->dev_private;
	long ret;

	pr_info("inside ioctl\n");

	ret = pm_runtime_resume_and_get(ane->dev);
	if (ret < 0 && ret != -EACCES) {
		pm_runtime_put_autosuspend(ane->dev);
		return ret;
	}

	ret = drm_ioctl(file, cmd, arg);

	pm_runtime_mark_last_busy(ane->dev);
	pm_runtime_put_autosuspend(ane->dev);
	return ret;
}

static const struct file_operations ane_drm_driver_fops = {
	.owner = THIS_MODULE,
	.open = drm_open,
	.release = drm_release,
	.unlocked_ioctl = ane_drm_ioctl,
	.mmap = drm_gem_mmap,
	.poll = drm_poll,
	.read = drm_read,
	.llseek = noop_llseek,
};

static const struct drm_driver ane_drm_driver = {
	.driver_features = DRIVER_GEM | DRIVER_RENDER,
	.open = ane_drm_open,
	.postclose = ane_drm_postclose,
	.ioctls = ane_drm_driver_ioctls,
	.num_ioctls = ARRAY_SIZE(ane_drm_driver_ioctls),
	.fops = &ane_drm_driver_fops,
	.name = "ane",
	.desc = "Apple Neural Engine driver",
	.date = "20230103",
	.major = 123,
	.minor = 0,
};

static int ane_iommu_sync_ttbr(struct ane_device *ane)
{
	int err = 0;
	u32 ttbr;

	void __iomem *ttbr_reg =
		ioremap(ane->hw->dart.base + ane->hw->dart.ttbr, sizeof(u32));
	if (IS_ERR(ttbr_reg)) {
		return -ENXIO;
	}

	ttbr = readl_relaxed(ttbr_reg);
	if (ttbr == 0) {
		pr_err("base dart not initialized\n");
		err = -ENXIO;
		goto unmap_reg;
	}

	/* remap ttbr so DMA chans can do their thing */
	writel_relaxed(ttbr, ane->dart1 + ane->hw->dart.ttbr);
	writel_relaxed(ttbr, ane->dart2 + ane->hw->dart.ttbr);

	if (readl_relaxed(ttbr_reg) !=
		    readl_relaxed(ane->dart1 + ane->hw->dart.ttbr) ||
	    readl_relaxed(ttbr_reg) !=
		    readl_relaxed(ane->dart2 + ane->hw->dart.ttbr)) {
		pr_err("failed to remap ttbr to dma channels\n");
		err = -ENXIO;
	}

unmap_reg:
	iounmap(ttbr_reg);
	return err;
}

static void ane_iommu_domain_free(struct ane_device *ane)
{
	put_iova_domain(&ane->iovad);
	return;
}

static int ane_iommu_domain_init(struct ane_device *ane)
{
	unsigned long order, iommu_page_size;
	dma_addr_t min_iova, max_iova;
	int err;

	struct iommu_domain *domain = iommu_get_domain_for_dev(ane->dev);
	if (!domain)
		return -ENODEV;
	ane->domain = domain;

	order = __ffs(ane->domain->pgsize_bitmap);
	iommu_page_size = 1UL << order; // 16K
	if (iommu_page_size != ane->hw->dart.page_size) {
		pr_err("iommu page size doesn't match dart config\n");
		return -ENXIO;
	}

	/* DMA chans can't access iovas past the limit */
	/* I believe it's a prefetch distance issue */
	min_iova = ane->hw->dart.vm_base;
	init_iova_domain(&ane->iovad, iommu_page_size, min_iova >> order);
	ane->shift = iova_shift(&ane->iovad);

	max_iova = min_iova + ane->hw->dart.vm_size - iommu_page_size;
	if (!ane->hw->dart.vm_size)
		max_iova = ane->domain->geometry.aperture_end;

	/* a page before as to not reach real limit */
	max_iova -= iommu_page_size;
	ane->limit = max_iova >> iova_shift(&ane->iovad);

	err = ane_iommu_sync_ttbr(ane);
	if (err < 0) {
		dev_err(ane->dev, "failed to sync ttbr\n");
		ane_iommu_domain_free(ane);
		return err;
	}

	return 0;
}

static void ane_detach_genpd(struct ane_device *ane)
{
	int i;

	if (ane->pd_count <= 1)
		return;

	for (i = ane->pd_count - 1; i >= 0; i--) {
		if (ane->pd_link[i])
			device_link_del(ane->pd_link[i]);
		if (!IS_ERR_OR_NULL(ane->pd_dev[i]))
			dev_pm_domain_detach(ane->pd_dev[i], true);
	}
	return;
}

static int ane_attach_genpd(struct ane_device *ane)
{
	struct device *dev = ane->dev;
	int i;

	ane->pd_count = of_count_phandle_with_args(
		dev->of_node, "power-domains", "#power-domain-cells");
	if (ane->pd_count <= 1)
		return 0;

	ane->pd_dev = devm_kcalloc(dev, ane->pd_count, sizeof(*ane->pd_dev),
				   GFP_KERNEL);
	if (!ane->pd_dev)
		return -ENOMEM;

	ane->pd_link = devm_kcalloc(dev, ane->pd_count, sizeof(*ane->pd_link),
				    GFP_KERNEL);
	if (!ane->pd_link)
		return -ENOMEM;

	for (i = 0; i < ane->pd_count; i++) {
		ane->pd_dev[i] = dev_pm_domain_attach_by_id(dev, i);
		if (IS_ERR(ane->pd_dev[i])) {
			ane_detach_genpd(ane);
			return PTR_ERR(ane->pd_dev[i]);
		}

		ane->pd_link[i] = device_link_add(dev, ane->pd_dev[i],
						  DL_FLAG_STATELESS |
						  DL_FLAG_PM_RUNTIME |
						  DL_FLAG_RPM_ACTIVE);
		if (!ane->pd_link[i]) {
			ane_detach_genpd(ane);
			return -EINVAL;
		}
	}

	return 0;
}

static int ane_pdev_probe(struct platform_device *pdev)
{
	struct ane_device *ane;
	struct drm_device *drm;
	int err;

	ane = devm_drm_dev_alloc(&pdev->dev, &ane_drm_driver, struct ane_device,
				 drm);
	if (IS_ERR(ane))
		return PTR_ERR(ane);

	platform_set_drvdata(pdev, ane);
	ane->dev = &pdev->dev;
	ane->hw = of_device_get_match_data(&pdev->dev);

	drm = &ane->drm;
	drm->dev_private = ane;

	err = ane_attach_genpd(ane);
	if (err < 0) {
		dev_err(ane->dev, "failed to attatch power domains\n");
		return err;
	}

	ane->irq = platform_get_irq_byname(pdev, "ane");
	if (ane->irq < 0){
		err =  -ENODEV;
		goto detach_genpd;
	}

	ane->dart_irq = platform_get_irq_byname(pdev, "dart");
	if (ane->dart_irq < 0){
		err =  -ENODEV;
		goto detach_genpd;
	}
	disable_irq(ane->dart_irq); // sigh

	ane->engine = devm_platform_ioremap_resource_byname(pdev, "engine");
	if (IS_ERR(ane->engine)){
		err = PTR_ERR(ane->engine);
		goto detach_genpd;
	}

	ane->dart1 = devm_platform_ioremap_resource_byname(pdev, "dart1");
	if (IS_ERR(ane->dart1)){
		err = PTR_ERR(ane->dart1);
		goto detach_genpd;
	}

	ane->dart2 = devm_platform_ioremap_resource_byname(pdev, "dart2");
	if (IS_ERR(ane->dart2)){
		err = PTR_ERR(ane->dart2);
		goto detach_genpd;
	}

	ane->perf = devm_platform_ioremap_resource_byname(pdev, "perf");
	if (IS_ERR(ane->perf)){
		err = PTR_ERR(ane->perf);
		goto detach_genpd;
	}

	ane->clk = devm_ioremap(ane->dev, ane->hw->base + 0x1170000UL,
				sizeof(u32) * 2);
	if (IS_ERR(ane->clk)){
		err = PTR_ERR(ane->clk);
		goto detach_genpd;
	}

	mutex_init(&ane->iova_lock);
	mutex_init(&ane->tm_lock);

	err = ane_iommu_domain_init(ane);
	if (err < 0) {
		dev_err(ane->dev, "failed to attatch iommu domain\n");
		goto detach_genpd;
	}

	err = drm_dev_register(drm, 0);
	if (err < 0)
		goto iommu_free;

	pm_runtime_set_autosuspend_delay(ane->dev, 50);
	pm_runtime_use_autosuspend(ane->dev);

	pm_runtime_get_noresume(&pdev->dev);
	pm_runtime_set_active(&pdev->dev);
	pm_runtime_enable(&pdev->dev);

	pm_runtime_mark_last_busy(&pdev->dev);
	pm_runtime_put_autosuspend(&pdev->dev);

	pr_info("loaded ane!\n");

	return 0;

iommu_free:
	ane_iommu_domain_free(ane);
detach_genpd:
	ane_detach_genpd(ane);
	return err;
}

static int ane_pdev_remove(struct platform_device *pdev)
{
	struct ane_device *ane = platform_get_drvdata(pdev);
	pm_runtime_disable(&pdev->dev);
	pm_runtime_dont_use_autosuspend(&pdev->dev);
	drm_dev_unregister(&ane->drm);
	ane_iommu_domain_free(ane);
	ane_detach_genpd(ane);
	return 0;
}

/* T8020/T6000 registers */
#define DART_T8020_STREAM_COMMAND	     0x20
#define DART_T8020_STREAM_SELECT	     0x34
#define DART_T8020_TTBR			     0x200
#define DART_T8020_STREAM_COMMAND_INVALIDATE BIT(20)

static const struct ane_hw ane_hw_t8103 = {
	.base = 0x26a000000,
	.ane_type = 64,
	.ane_subtype = 0,
	.ane_id = 0,
	.die_id = 0,
	.die_ane_id = 0,
	.dart = {
		.base = 0x26b800000,
		.dart1 = 0x26b810000,
		.dart2 = 0x26b820000,
		.dapf = 0x26b804000,
		.vm_base = 0x0,
		.vm_size = 0xe0000000,
		.page_size = 0x4000,
		.ttbr = DART_T8020_TTBR,
		.sel = DART_T8020_STREAM_SELECT,
		.cmd = DART_T8020_STREAM_COMMAND,
		.invalidate = DART_T8020_STREAM_COMMAND_INVALIDATE,
	},
	.max_ane = 1,
	.max_ne = 8,
	.bar_slots = 0x20,
	.dma0_gran = 0x40,
};

static const struct ane_hw ane_hw_t600x_ane0 = {
	.base = 0x284000000,
	.ane_type = 96,
	.ane_subtype = 0,
	.ane_id = 0,
	.die_id = 0,
	.die_ane_id = 0,
	.dart = {
		.base = 0x285800000,
		.dart1 = 0x285810000,
		.dart2 = 0x285820000,
		.dapf = 0x285804000,
		.vm_base = 0x0,
		.vm_size = 0xe0000000,
		.page_size = 0x4000,
		.ttbr = DART_T8020_TTBR,
		.sel = DART_T8020_STREAM_SELECT,
		.cmd = DART_T8020_STREAM_COMMAND,
		.invalidate = DART_T8020_STREAM_COMMAND_INVALIDATE,
	},
	.max_ane = 1,
	.max_ne = 8,
	.bar_slots = 0x20,
	.dma0_gran = 0x40,
};

static const struct ane_hw ane_hw_t600x_ane2 = {
	.base = 0x2284000000,
	.ane_type = 96,
	.ane_subtype = 2,
	.ane_id = 2,
	.die_id = 1,
	.die_ane_id = 0,
	.dart = {
		.base = 0x2285800000,
		.dart1 = 0x2285810000,
		.dart2 = 0x2285820000,
		.dapf = 0x2285804000,
		.vm_base = 0x0,
		.vm_size = 0xe0000000,
		.page_size = 0x4000,
		.ttbr = DART_T8020_TTBR,
		.sel = DART_T8020_STREAM_SELECT,
		.cmd = DART_T8020_STREAM_COMMAND,
		.invalidate = DART_T8020_STREAM_COMMAND_INVALIDATE,
	},
	.max_ane = 1,
	.max_ne = 8,
	.bar_slots = 0x20,
	.dma0_gran = 0x40,
};

static const struct of_device_id ane_pdev_match[] = {
	{ .compatible = "apple,t8103-ane", .data = &ane_hw_t8103 },
	{}
};
MODULE_DEVICE_TABLE(of, ane_pdev_match);

static int __maybe_unused ane_runtime_resume(struct device *dev)
{
	struct ane_device *ane = dev_get_drvdata(dev);
	int err;

	pr_info("test: 0x%x\n", readl(ane->engine));
	err = ane_tm_init_tqs(ane);
	if (err < 0){
		return err;
	}

	return 0;
}

static int __maybe_unused ane_runtime_suspend(struct device *dev)
{
	struct ane_device *ane = dev_get_drvdata(dev);

	pr_info("test: 0x%x\n", readl(ane->engine));
	ane_iommu_invalidate_tlb(ane);

	return 0;
}

static const struct dev_pm_ops ane_pm_ops = {
	SET_RUNTIME_PM_OPS(ane_runtime_suspend, ane_runtime_resume, NULL)
};

static struct platform_driver ane_pdev_driver = {
    .probe  = ane_pdev_probe,
    .remove = ane_pdev_remove,
    .driver =
	{
	    .name	    = "ane",
	    .owner	    = THIS_MODULE,
	    .pm             = pm_ptr(&ane_pm_ops),
	    .of_match_table = ane_pdev_match,
	},
};
module_platform_driver(ane_pdev_driver);

MODULE_AUTHOR("Eileen Yoon <eyn@gmx.com>");
MODULE_DESCRIPTION("Apple Neural Engine driver");
MODULE_LICENSE("Dual MIT/GPL");
