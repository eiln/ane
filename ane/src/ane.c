// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */
/* IOMMU portions based on gpu/drm/tegra/drm.c */

#define pr_fmt(fmt) "%s: %s: " fmt, KBUILD_MODNAME, __func__

#include <linux/interrupt.h> // sigh
#include <linux/iommu.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_domain.h>
#include <linux/pm_runtime.h>

#include <drm/drm_drv.h>
#include <drm/drm_file.h> // <drm/drm_accel.h>
#include <drm/drm_gem.h>
#include <drm/drm_ioctl.h>

#include "ane.h"
#include "ane_tm.h"
#include "include/drm_ane.h"

/* device-continuous object */
struct ane_node {
	struct drm_mm_node *mm;
	u32 npages;
	struct page **pages;
	dma_addr_t iova;
	u64 userptr;
	u32 type;
};

/* neural network */
struct ane_nn {
	struct drm_gem_object base;
	struct ane_engine_req req;
	struct anec anec;
	struct ane_node tiles[ANE_TILE_COUNT];
	struct ane_node fifo;
	int tile_bdx[ANE_TILE_COUNT];
	int tile_count;
	int mapped;
};

#define to_nn(gem)  container_of(gem, struct ane_nn, base)
#define to_anec(nn) (&nn->anec)

static void ane_mm_free_pages(struct ane_node *node)
{
	if (!node->pages)
		return;

	for (u32 i = 0; i < node->npages && node->pages[i] != NULL; i++) {
		__free_page(node->pages[i]);
	}
	kvfree(node->pages);
}

static int ane_mm_alloc_pages(struct ane_node *node)
{
	if (node->pages)
		return -EBUSY;

	node->pages =
		kvmalloc_array(node->npages, sizeof(*node->pages), GFP_KERNEL);
	if (!node->pages)
		return -ENOMEM;

	for (u32 i = 0; i < node->npages; i++) {
		node->pages[i] = alloc_page(GFP_KERNEL);
		if (node->pages[i] == NULL)
			goto free_pages;
	}

	return 0;

free_pages:
	ane_mm_free_pages(node);
	return -ENOMEM;
}

static void ane_mm_put_user_pages(struct ane_node *node)
{
	if (!node->pages)
		return;

	for (u32 i = 0; i < node->npages; i++) {
		if (node->pages[i])
			put_page(node->pages[i]);
	}
	kvfree(node->pages);
}

static int ane_mm_get_user_pages(struct ane_node *node)
{
	long pinned;

	if (node->pages)
		return -EBUSY;

	if (!node->userptr)
		return -EINVAL;

	node->pages =
		kvmalloc_array(node->npages, sizeof(struct page *), GFP_KERNEL);
	if (!node->pages)
		return -ENOMEM;

	pinned = get_user_pages(node->userptr, node->npages, FOLL_WRITE,
				node->pages, NULL);
	if (pinned != node->npages) {
		ane_mm_put_user_pages(node);
		return -EFAULT;
	}

	return 0;
}

static int ane_iommu_map_pages(struct ane_device *ane, struct ane_node *node)
{
	int err;

	if (node->mm)
		return -EBUSY;

	node->mm = kzalloc(sizeof(*node->mm), GFP_KERNEL);
	if (!node->mm)
		return -ENOMEM;

	mutex_lock(&ane->iommu_lock);

	/* reserve area from ANE address space */
	err = drm_mm_insert_node_generic(&ane->mm, node->mm,
					 node->npages << ane->shift,
					 1UL << ane->shift, 0, 0);
	if (err < 0) {
		dev_err(ane->dev, "out of ANE space: %d\n", err);
		goto unlock;
	}

	node->iova = node->mm->start;

	/* map into ANE address space */
	for (u32 i = 0; i < node->npages; i++) {
		dma_addr_t iova = node->iova + (i << ane->shift);
		err = iommu_map(ane->domain, iova, page_to_phys(node->pages[i]),
				1UL << ane->shift, IOMMU_READ | IOMMU_WRITE);
		if (err < 0) {
			dev_err(ane->dev, "iommu_map failed at 0x%llx", iova);
			while (i-- > 0) {
				iommu_unmap(ane->domain,
					    node->iova + (i << ane->shift),
					    1UL << ane->shift);
			}
			goto remove;
		}
	}

	mutex_unlock(&ane->iommu_lock);

	return 0;

remove:
	drm_mm_remove_node(node->mm);
unlock:
	mutex_unlock(&ane->iommu_lock);
	kfree(node->mm);
	return err;
}

static void ane_iommu_unmap_pages(struct ane_device *ane, struct ane_node *node)
{
	if (!node->mm)
		return;

	mutex_lock(&ane->iommu_lock);
	for (u32 i = 0; i < node->npages; i++) {
		dma_addr_t iova = node->iova + (i << ane->shift);
		iommu_unmap(ane->domain, iova, 1UL << ane->shift);
	}
	drm_mm_remove_node(node->mm);
	mutex_unlock(&ane->iommu_lock);

	kfree(node->mm);
	return;
}

static void ane_iommu_invalidate_tlb(struct ane_device *ane)
{
	mutex_lock(&ane->iommu_lock);

	iommu_flush_iotlb_all(ane->domain);

	writel(0x1, ane->dart1 + ane->hw->dart.sel);
	writel(ane->hw->dart.inv, ane->dart1 + ane->hw->dart.cmd);
	writel(0x1, ane->dart2 + ane->hw->dart.sel);
	writel(ane->hw->dart.inv, ane->dart2 + ane->hw->dart.cmd);

	writel(ane->hw->dart.inv, ane->dart1 + ane->hw->dart.cmd);
	writel(ane->hw->dart.inv, ane->dart2 + ane->hw->dart.cmd);

	writel(0x1, ane->dart1 + ane->hw->dart.sel);
	writel(ane->hw->dart.inv, ane->dart1 + ane->hw->dart.cmd);
	writel(0x1, ane->dart2 + ane->hw->dart.sel);
	writel(ane->hw->dart.inv, ane->dart2 + ane->hw->dart.cmd);

	writel(ane->hw->dart.inv, ane->dart1 + ane->hw->dart.cmd);
	writel(ane->hw->dart.inv, ane->dart2 + ane->hw->dart.cmd);

	mutex_unlock(&ane->iommu_lock);
}

static int ane_nn_validate_anec(struct ane_device *ane, struct ane_nn *nn)
{
	struct anec *anec = to_anec(nn);
	int i = 0;

	/*
	 * Checks in case the compiler output was messed with...
	 */
	if (!anec->size || !anec->tsk_size || !anec->krn_size ||
	    !anec->td_size || !anec->td_count)
		return -EINVAL;

	if (anec->tsk_size < anec->td_size)
		return -EINVAL;

	if ((anec->td_size * 2) > 1UL << ane->shift)
		return -EINVAL;

	if (anec->size !=
	    (roundup(anec->tsk_size, ANE_CMD_GRAN) + anec->krn_size))
		return -EINVAL;

	if (anec->td_count >= 0xffff)
		return -EINVAL;

	if (!anec->tiles[0] || anec->types[0] != ANE_TILE_CMD || anec->tiles[1])
		return -EINVAL;

	if (anec->tiles[2] && !anec->tiles[3])
		return -EINVAL;

	if (!anec->tiles[4] || !anec->tiles[5])
		return -EINVAL;

	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		if (anec->tiles[bdx])
			nn->tile_bdx[i++] = bdx;

		if (anec->tiles[bdx] && !anec->types[bdx])
			return -EINVAL;

		if (!anec->tiles[bdx] && anec->types[bdx])
			return -EINVAL;

		if (anec->tiles[bdx] > 0xffff)
			return -EINVAL;

		if (anec->tiles[bdx] && anec->types[bdx] != ANE_TILE_CMD &&
		    anec->types[bdx] != ANE_TILE_ITM &&
		    anec->types[bdx] != ANE_TILE_DST &&
		    anec->types[bdx] != ANE_TILE_SRC)
			return -EINVAL;
	}

	nn->tile_count = i;

	return 0;
}

static const struct vm_operations_struct drm_gem_ane_vm_ops = {
	.open = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};

static const struct drm_gem_object_funcs ane_gem_object_funcs = {
	.vm_ops = &drm_gem_ane_vm_ops,
};

static int ane_nn_create(struct drm_device *drm, void *data,
			 struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	struct drm_ane_nn_create *args = data;
	struct drm_gem_object *gem = NULL;
	struct ane_nn *nn = NULL;
	struct anec *anec = NULL;
	int err;

	if (args->pad)
		return -EINVAL;

	if (!(nn = kzalloc(sizeof(struct ane_nn), GFP_KERNEL)))
		return -ENOMEM;

	anec = to_anec(nn);
	err = copy_from_user(anec, u64_to_user_ptr(args->anec_userptr),
			     sizeof(struct anec));
	if (err) {
		dev_err(ane->dev, "failed to copy anec\n");
		err = -EFAULT;
		goto free_nn;
	}

	err = ane_nn_validate_anec(ane, nn);
	if (err < 0) {
		dev_err(ane->dev, "invalid anec setup\n");
		goto free_nn;
	}

	nn->mapped = 0;
	nn->req.nid = ANE_FIFO_NID;
	nn->req.td_size = anec->td_size;
	nn->req.td_count = anec->td_count;

	pr_info("received nn with size: 0x%llx td_count: 0x%x\n", anec->size,
		nn->req.td_count);

	/* fake mini gem backing for the sake of a handle */
	gem = &nn->base;
	gem->funcs = &ane_gem_object_funcs;
	err = drm_gem_object_init(drm, gem, PAGE_SIZE);
	if (err < 0)
		goto free_nn;

	err = drm_gem_handle_create(file, gem, &args->handle);
	drm_gem_object_put(gem); /* handle holds it now */
	if (err < 0)
		goto release;

	return 0;

release:
	drm_gem_object_release(gem);
free_nn:
	kfree(nn);
	return err;
}

static struct ane_nn *ane_nn_lookup(struct drm_file *file, u32 handle)
{
	struct drm_gem_object *gem;

	gem = drm_gem_object_lookup(file, handle);
	if (!gem)
		return NULL;

	return to_nn(gem);
}

static int ane_nn_free(struct drm_device *drm, void *data,
		       struct drm_file *file)
{
	struct drm_ane_nn_free *args = data;

	struct ane_nn *nn = ane_nn_lookup(file, args->handle);
	if (args->pad || !nn)
		return -EINVAL;

	WARN_ON(nn->mapped);
	pr_info("freeing nn with size: 0x%llx td_count: 0x%x\n",
		to_anec(nn)->size, nn->req.td_count);

	drm_gem_handle_delete(file, args->handle);
	drm_gem_object_release(&nn->base);

	kfree(nn);
	return 0;
}

static void __ane_nn_unmap(struct ane_device *ane, struct ane_nn *nn)
{
	WARN_ON(!nn->mapped);

	for (int i = 0; i < nn->tile_count; i++) {
		int bdx = nn->tile_bdx[i];
		struct ane_node *tile = &nn->tiles[bdx];

		ane_iommu_unmap_pages(ane, tile);

		if (tile->type == ANE_TILE_ITM) {
			ane_mm_free_pages(tile);
		} else {
			ane_mm_put_user_pages(tile);
		}
	}

	ane_iommu_unmap_pages(ane, &nn->fifo);
	ane_mm_put_user_pages(&nn->fifo);

	/* macos invalidates on a per-network basis */
	ane_iommu_invalidate_tlb(ane);

	nn->mapped = 0;
}

static int ane_nn_unmap(struct drm_device *drm, void *data,
			struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	struct drm_ane_nn_unmap *args = data;
	struct ane_nn *nn = ane_nn_lookup(file, args->handle);
	if (args->pad || !nn)
		return -EINVAL;
	__ane_nn_unmap(ane, nn);
	return 0;
}

static int ane_nn_map(struct drm_device *drm, void *data, struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	struct drm_ane_nn_map *args = data;
	int err;

	struct anec *anec = NULL;
	struct ane_nn *nn = ane_nn_lookup(file, args->handle);
	if (args->pad || !nn)
		return -EINVAL;

	if (nn->mapped)
		return 0;

	anec = to_anec(nn);

	for (int i = 0; i < nn->tile_count; i++) {
		int bdx = nn->tile_bdx[i];
		struct ane_node *tile = &nn->tiles[bdx];

		tile->npages = anec->tiles[bdx];
		tile->type = anec->types[bdx];
		tile->userptr = args->tile_userptr[bdx];

		/*
		 * Intermediate buffers are just swap space the engine
		 * demands when an operation exceeds L2. They're not BERT
		 * intermediate layers, users don't need them whatsoever.
		 * Back with alloc_page() & keep it in here.
		 */
		if (tile->type == ANE_TILE_ITM) {
			err = ane_mm_alloc_pages(tile);
		} else {
			err = ane_mm_get_user_pages(tile);
		}

		if (err < 0) {
			dev_err(ane->dev, "failed to obtain backing pages\n");
			goto error;
		}

		err = ane_iommu_map_pages(ane, tile);
		if (err < 0) {
			dev_err(ane->dev, "failed to map pages to device\n");
			goto error;
		}

		nn->req.bar[bdx] = lower_32_bits(tile->iova);
	}

	/*
	 * Naturally, Apple stores the compiled neural network as the
	 * TD catted with the kernel @ 16 gran padding. This not only
	 * takes advantage of bank aligned access, but it's also a
	 * convienient serialization method, making one less non-aligned
	 * buffer to worry about. We thus unpack the iova at which the
	 * kernel *should* start.
	 */
	nn->req.bar[1] = nn->req.bar[0] + roundup(anec->tsk_size, ANE_CMD_GRAN);

	/*
	 * Technically the firmware maintains a pool of fifo buffers
	 * with each entry masked to jump to the next. However the
	 * actual pool is just a firmware-level construct, and only two
	 * valid entries, with the first instructed to jump to the second,
	 * is needed to execute the (first) network. We could make this
	 * mini per-network pool here but we can also get it from userspace.
	 */
	nn->fifo.npages = 1;
	nn->fifo.userptr = args->fifo_userptr;
	err = ane_mm_get_user_pages(&nn->fifo);
	if (err < 0) {
		dev_err(ane->dev, "failed to obtain backing pages\n");
		goto error;
	}

	err = ane_iommu_map_pages(ane, &nn->fifo);
	if (err < 0) {
		dev_err(ane->dev, "failed to map pages to device\n");
		goto error;
	}

	nn->req.fifo_addr = lower_32_bits(nn->fifo.iova);

	for (int i = 0; i < ANE_TILE_COUNT; i++) {
		if (nn->req.bar[i]) {
			pr_info("BAR %d: 0x%08x\n", i, nn->req.bar[i]);
		}
	}

	nn->mapped = 1;

	return 0;

error:
	__ane_nn_unmap(ane, nn);
	return err;
}

static int ane_nn_exec(struct drm_device *drm, void *data,
		       struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	struct drm_ane_nn_exec *args = data;
	int err;

	struct ane_nn *nn = ane_nn_lookup(file, args->handle);
	if (args->pad || !nn)
		return -EINVAL;

	if (!nn->mapped)
		return -EINVAL;

	mutex_lock(&ane->engine_lock);

	err = ane_tm_enqueue(ane, &nn->req);
	if (err < 0)
		goto error;

	err = ane_tm_execute(ane, &nn->req);
	if (err < 0)
		goto error;

	mutex_unlock(&ane->engine_lock);

	return 0;

error:
	mutex_unlock(&ane->engine_lock);
	return err;
}

static const struct drm_ioctl_desc ane_drm_ioctls[] = {
	DRM_IOCTL_DEF_DRV(ANE_NN_CREATE, ane_nn_create, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(ANE_NN_FREE, ane_nn_free, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(ANE_NN_MAP, ane_nn_map, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(ANE_NN_UNMAP, ane_nn_unmap, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(ANE_NN_EXEC, ane_nn_exec, DRM_RENDER_ALLOW),
};

static int ane_drm_open(struct drm_device *drm, struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	int err;

	/* need to bring up power immediately if opening device */
	err = pm_runtime_resume_and_get(ane->dev);
	if (err < 0 && err != -EACCES) {
		pm_runtime_put_autosuspend(ane->dev);
		return err;
	}

	pm_runtime_mark_last_busy(ane->dev);
	pm_runtime_put_autosuspend(ane->dev);

	return err;
}

static void ane_drm_postclose(struct drm_device *drm, struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	pm_runtime_resume_and_get(ane->dev);

	pm_runtime_mark_last_busy(ane->dev);
	pm_runtime_put_autosuspend(ane->dev);
}

static long ane_drm_unlocked_ioctl(struct file *file, unsigned int cmd,
				   unsigned long arg)
{
	struct drm_file *filp = file->private_data;
	struct drm_device *drm = filp->minor->dev;
	struct ane_device *ane = drm->dev_private;
	long err;

	err = pm_runtime_resume_and_get(ane->dev);
	if (err < 0 && err != -EACCES) {
		pm_runtime_put_autosuspend(ane->dev);
		return err;
	}

	err = drm_ioctl(file, cmd, arg);

	pm_runtime_mark_last_busy(ane->dev);
	pm_runtime_put_autosuspend(ane->dev);

	return err;
}

static const struct file_operations ane_drm_fops = {
	.owner = THIS_MODULE,
	.open = drm_open, // accel_open
	.release = drm_release,
	.unlocked_ioctl = ane_drm_unlocked_ioctl,
	.mmap = drm_gem_mmap,
	.poll = drm_poll,
	.read = drm_read,
	.llseek = noop_llseek,
};

static const struct drm_driver ane_drm_driver = {
	.driver_features = DRIVER_GEM | DRIVER_RENDER, // DRIVER_COMPUTE_ACCEL
	.open = ane_drm_open,
	.postclose = ane_drm_postclose,
	.ioctls = ane_drm_ioctls,
	.num_ioctls = ARRAY_SIZE(ane_drm_ioctls),
	.fops = &ane_drm_fops,
	.name = "ane",
	.desc = "Apple Neural Engine driver",
	.date = "20230103",
	.major = 1,
	.minor = 0,
};

static int ane_iommu_remap_ttbr(struct ane_device *ane)
{
	int err = 0;
	u32 ttbr;

	void __iomem *ttbr_reg =
		ioremap(ane->hw->dart.base + ane->hw->dart.ttbr, sizeof(u32));
	if (IS_ERR(ttbr_reg))
		return PTR_ERR(ttbr_reg);

	ttbr = readl_relaxed(ttbr_reg);
	if (!ttbr) {
		dev_err(ane->dev, "base dart not initialized\n");
		err = -EPROBE_DEFER;
		goto unmap;
	}

	/* remap ttbr so DMA chans can do their thing */
	writel_relaxed(ttbr, ane->dart1 + ane->hw->dart.ttbr);
	writel_relaxed(ttbr, ane->dart2 + ane->hw->dart.ttbr);

unmap:
	iounmap(ttbr_reg);
	return err;
}

static void ane_iommu_domain_free(struct ane_device *ane)
{
	drm_mm_takedown(&ane->mm);
}

static int ane_iommu_domain_init(struct ane_device *ane)
{
	unsigned long order, iommu_page_size;
	dma_addr_t min_iova, max_iova;
	int err;

	struct iommu_domain *domain = iommu_get_domain_for_dev(ane->dev);
	if (!domain)
		return -EPROBE_DEFER;

	ane->domain = domain;

	/* ANE objects must be 16K */
	order = __ffs(ane->domain->pgsize_bitmap);
	iommu_page_size = 1UL << order; // 16K
	if (iommu_page_size != ane->hw->dart.page_size) {
		dev_err(ane->dev, "invalid iommu page size\n");
		return -EINVAL;
	}
	ane->shift = order;

	err = ane_iommu_remap_ttbr(ane);
	if (err < 0)
		return err;

	/* DMA chans can't access iovas past the limit */
	/* likely a kernel prefetch distance constraint */
	min_iova = ane->hw->dart.vm_base;

	/* a page before as to not reach real limit */
	max_iova = min_iova + ane->hw->dart.vm_size - iommu_page_size;

	drm_mm_init(&ane->mm, min_iova, max_iova);

	return 0;
}

static void ane_detach_genpd(struct ane_device *ane)
{
	if (ane->pd_count <= 1)
		return;

	for (int i = ane->pd_count - 1; i >= 0; i--) {
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

	for (int i = 0; i < ane->pd_count; i++) {
		ane->pd_dev[i] = dev_pm_domain_attach_by_id(dev, i);
		if (IS_ERR(ane->pd_dev[i])) {
			ane_detach_genpd(ane);
			return PTR_ERR(ane->pd_dev[i]);
		}

		ane->pd_link[i] =
			device_link_add(dev, ane->pd_dev[i],
					DL_FLAG_STATELESS | DL_FLAG_PM_RUNTIME |
						DL_FLAG_RPM_ACTIVE);
		if (!ane->pd_link[i]) {
			ane_detach_genpd(ane);
			return -EINVAL;
		}
	}

	return 0;
}

static int ane_platform_probe(struct platform_device *pdev)
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
	ane->hw = of_device_get_match_data(ane->dev);

	drm = &ane->drm;
	drm->dev_private = ane;

	err = ane_attach_genpd(ane);
	if (err < 0) {
		dev_err(ane->dev, "failed to attatch power domains\n");
		return err;
	}

	ane->irq = platform_get_irq_byname(pdev, "ane");
	if (ane->irq < 0) {
		err = -ENODEV;
		goto detach_genpd;
	}

	ane->dart_irq = platform_get_irq_byname(pdev, "dart");
	if (ane->dart_irq < 0) {
		err = -ENODEV;
		goto detach_genpd;
	}
	disable_irq(ane->dart_irq); // sigh

	ane->engine = devm_platform_ioremap_resource_byname(pdev, "engine");
	if (IS_ERR(ane->engine)) {
		err = PTR_ERR(ane->engine);
		goto detach_genpd;
	}

	ane->dart1 = devm_platform_ioremap_resource_byname(pdev, "dart1");
	if (IS_ERR(ane->dart1)) {
		err = PTR_ERR(ane->dart1);
		goto detach_genpd;
	}

	ane->dart2 = devm_platform_ioremap_resource_byname(pdev, "dart2");
	if (IS_ERR(ane->dart2)) {
		err = PTR_ERR(ane->dart2);
		goto detach_genpd;
	}

	mutex_init(&ane->iommu_lock);
	mutex_init(&ane->engine_lock);

	err = ane_iommu_domain_init(ane);
	if (err < 0)
		goto detach_genpd;

	ane_tm_enable(ane);

	/* measured 3sec on macos */
	pm_runtime_set_autosuspend_delay(ane->dev, 3000);
	pm_runtime_use_autosuspend(ane->dev);

	pm_runtime_get_noresume(ane->dev);
	pm_runtime_set_active(ane->dev);
	pm_runtime_enable(ane->dev);

	pm_runtime_mark_last_busy(ane->dev);
	pm_runtime_put_autosuspend(ane->dev);

	err = drm_dev_register(drm, 0);
	if (err < 0)
		goto disable_pm;

	pr_info("loaded ane!\n");

	return 0;

disable_pm:
	pm_runtime_disable(ane->dev);
	pm_runtime_dont_use_autosuspend(ane->dev);
	ane_iommu_domain_free(ane);
detach_genpd:
	ane_detach_genpd(ane);
	return err;
}

static int ane_platform_remove(struct platform_device *pdev)
{
	struct ane_device *ane = platform_get_drvdata(pdev);
	drm_dev_unregister(&ane->drm);
	pm_runtime_disable(ane->dev);
	pm_runtime_dont_use_autosuspend(ane->dev);
	ane_iommu_domain_free(ane);
	ane_detach_genpd(ane);
	return 0;
}

static int __maybe_unused ane_runtime_suspend(struct device *dev)
{
	struct ane_device *ane = dev_get_drvdata(dev);
	ane_iommu_invalidate_tlb(ane);
	return 0;
}

static int __maybe_unused ane_runtime_resume(struct device *dev)
{
	struct ane_device *ane = dev_get_drvdata(dev);
	ane_tm_enable(ane);
	return 0;
}

static int __maybe_unused ane_pm_suspend(struct device *dev)
{
	struct ane_device *ane = dev_get_drvdata(dev);
	return pm_runtime_force_suspend(ane->dev);
}

static int __maybe_unused ane_pm_resume(struct device *dev)
{
	struct ane_device *ane = dev_get_drvdata(dev);
	return pm_runtime_force_resume(ane->dev);
}

// clang-format off
static const struct dev_pm_ops ane_pm_ops = {
	SET_RUNTIME_PM_OPS(ane_runtime_suspend, ane_runtime_resume, NULL)
	SET_SYSTEM_SLEEP_PM_OPS(ane_pm_suspend, ane_pm_resume)
};
// clang-format on

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
		.vm_base = 0x4000,
		.vm_size = 0xe0000000,
		.page_size = 0x4000,
		.ttbr = DART_T8020_TTBR,
		.sel = DART_T8020_STREAM_SELECT,
		.cmd = DART_T8020_STREAM_COMMAND,
		.inv = DART_T8020_STREAM_COMMAND_INVALIDATE,
	},
};

static const struct ane_hw ane_hw_t6000_ane0 = {
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
		.vm_base = 0x4000,
		.vm_size = 0xe0000000,
		.page_size = 0x4000,
		.ttbr = DART_T8020_TTBR,
		.sel = DART_T8020_STREAM_SELECT,
		.cmd = DART_T8020_STREAM_COMMAND,
		.inv = DART_T8020_STREAM_COMMAND_INVALIDATE,
	},
};

static const struct ane_hw ane_hw_t6000_ane2 = {
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
		.vm_base = 0x4000,
		.vm_size = 0xe0000000,
		.page_size = 0x4000,
		.ttbr = DART_T8020_TTBR,
		.sel = DART_T8020_STREAM_SELECT,
		.cmd = DART_T8020_STREAM_COMMAND,
		.inv = DART_T8020_STREAM_COMMAND_INVALIDATE,
	},
};

static const struct of_device_id ane_of_match[] = {
	{ .compatible = "apple,t8103-ane", .data = &ane_hw_t8103 },
	{}
};

MODULE_DEVICE_TABLE(of, ane_of_match);

static struct platform_driver ane_platform_driver = {
    .probe  = ane_platform_probe,
    .remove = ane_platform_remove,
    .driver =
	{
	    .name	    = "ane",
	    .pm             = pm_ptr(&ane_pm_ops),
	    .of_match_table = ane_of_match,
	},
};

module_platform_driver(ane_platform_driver);

MODULE_AUTHOR("Eileen Yoon <eyn@gmx.com>");
MODULE_DESCRIPTION("Apple Neural Engine driver");
MODULE_LICENSE("Dual MIT/GPL");
