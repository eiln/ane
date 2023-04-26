// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#define pr_fmt(fmt) "%s: %s: " fmt, KBUILD_MODNAME, __func__

#include <linux/interrupt.h> // sigh
#include <linux/iommu.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_domain.h>
#include <linux/pm_runtime.h>

#include <drm/drm_accel.h>
#include <drm/drm_drv.h>
#include <drm/drm_gem.h>
#include <drm/drm_ioctl.h>

#include "ane.h"
#include "ane_tm.h"
#include "include/drm_ane.h"

struct ane_bo {
	struct drm_gem_object base;
	struct drm_mm_node *mm;
	u32 npages;
	struct page **pages;
	dma_addr_t iova;
};

#define to_bo(gem) (container_of(gem, struct ane_bo, base))

static int ane_iommu_map_pages(struct ane_device *ane, struct ane_bo *bo)
{
	int err;

	if (bo->mm)
		return -EBUSY;

	bo->mm = kzalloc(sizeof(*bo->mm), GFP_KERNEL);
	if (!bo->mm)
		return -ENOMEM;

	mutex_lock(&ane->iommu_lock);

	/* reserve area from ANE address space */
	err = drm_mm_insert_node_generic(&ane->mm, bo->mm,
					 bo->npages << ane->shift,
					 1UL << ane->shift, 0, 0);
	if (err < 0) {
		dev_err(ane->dev, "out of ANE space: %d\n", err);
		goto unlock;
	}

	bo->iova = bo->mm->start;

	/* map into ANE address space */
	for (u32 i = 0; i < bo->npages; i++) {
		dma_addr_t iova = bo->iova + (i << ane->shift);
		err = iommu_map(ane->domain, iova, page_to_phys(bo->pages[i]),
				1UL << ane->shift, IOMMU_READ | IOMMU_WRITE);
		if (err < 0) {
			dev_err(ane->dev, "iommu_map failed at 0x%llx", iova);
			while (i-- > 0) {
				iommu_unmap(ane->domain,
					    bo->iova + (i << ane->shift),
					    1UL << ane->shift);
			}
			goto remove;
		}
	}

	mutex_unlock(&ane->iommu_lock);

	return 0;

remove:
	drm_mm_remove_node(bo->mm);
unlock:
	mutex_unlock(&ane->iommu_lock);
	kfree(bo->mm);
	return err;
}

static void ane_iommu_invalidate_tlb(struct ane_device *ane)
{
	mutex_lock(&ane->iommu_lock);

	iommu_flush_iotlb_all(ane->domain);

	writel(0x1, ane->dart1 + ane->hw->dart.sel);
	writel(ane->hw->dart.inv, ane->dart1 + ane->hw->dart.cmd);
	writel(0x1, ane->dart2 + ane->hw->dart.sel);
	writel(ane->hw->dart.inv, ane->dart2 + ane->hw->dart.cmd);

	mutex_unlock(&ane->iommu_lock);
}

static void ane_iommu_unmap_pages(struct ane_device *ane, struct ane_bo *bo)
{
	if (!bo->mm)
		return;

	mutex_lock(&ane->iommu_lock);
	for (u32 i = 0; i < bo->npages; i++) {
		dma_addr_t iova = bo->iova + (i << ane->shift);
		iommu_unmap(ane->domain, iova, 1UL << ane->shift);
	}
	drm_mm_remove_node(bo->mm);
	mutex_unlock(&ane->iommu_lock);

	kfree(bo->mm);

	/* Conservatively invalidate after every unmap batch */
	ane_iommu_invalidate_tlb(ane);
}

static struct ane_bo *ane_bo_lookup(struct drm_file *file, u32 handle)
{
	struct drm_gem_object *gem;

	gem = drm_gem_object_lookup(file, handle);
	if (!gem)
		return NULL;

	return to_bo(gem);
}

static vm_fault_t ane_gem_vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	struct drm_gem_object *gem = vma->vm_private_data;
	struct ane_bo *bo = to_bo(gem);
	struct page *page;
	pgoff_t offset;

	if (!bo->pages)
		return VM_FAULT_SIGBUS;

	offset = (vmf->address - vma->vm_start) >> PAGE_SHIFT;
	page = bo->pages[offset];

	return vmf_insert_page(vma, vmf->address, page);
}

static const struct vm_operations_struct drm_gem_ane_vm_ops = {
	.open = drm_gem_vm_open,
	.close = drm_gem_vm_close,
	.fault = ane_gem_vm_fault,
};

static const struct drm_gem_object_funcs ane_gem_object_funcs = {
	.vm_ops = &drm_gem_ane_vm_ops,
};

static int ane_bo_init(struct drm_device *drm, void *data,
		       struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	struct drm_ane_bo_init *args = data;
	struct drm_gem_object *gem = NULL;
	struct ane_bo *bo = NULL;
	int err;

	if (args->pad)
		return -EINVAL;

	bo = kzalloc(sizeof(struct ane_bo), GFP_KERNEL);
	if (!bo)
		return -ENOMEM;

	gem = &bo->base;
	gem->funcs = &ane_gem_object_funcs;
	err = drm_gem_object_init(drm, gem, round_up(args->size, PAGE_SIZE));
	if (err < 0)
		goto free;

	err = drm_gem_create_mmap_offset(gem);
	if (err < 0)
		goto release;

	args->offset = drm_vma_node_offset_addr(&gem->vma_node);

	bo->npages = gem->size >> PAGE_SHIFT;
	bo->pages = drm_gem_get_pages(gem);
	if (IS_ERR(bo->pages)) {
		err = PTR_ERR(bo->pages);
		goto release;
	}

	err = ane_iommu_map_pages(ane, bo);
	if (err < 0)
		goto put;

	err = drm_gem_handle_create(file, gem, &args->handle);
	drm_gem_object_put(gem); /* handle holds it now */
	if (err < 0)
		goto unmap;

	return 0;

unmap:
	ane_iommu_unmap_pages(ane, bo);
put:
	drm_gem_put_pages(&bo->base, bo->pages, false, false);
release:
	drm_gem_object_release(gem);
free:
	kfree(bo);
	return err;
}

static int ane_bo_free(struct drm_device *drm, void *data,
		       struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	struct drm_ane_bo_free *args = data;
	struct ane_bo *bo = ane_bo_lookup(file, args->handle);
	if (args->pad || !bo)
		return -EINVAL;
	ane_iommu_unmap_pages(ane, bo);
	drm_gem_put_pages(&bo->base, bo->pages, true, true);
	drm_gem_handle_delete(file, args->handle);
	drm_gem_object_release(&bo->base);
	kfree(bo);
	return 0;
}

static int ane_submit(struct drm_device *drm, void *data, struct drm_file *file)
{
	struct ane_device *ane = drm->dev_private;
	struct drm_ane_submit *args = data;
	struct ane_bo *bo = NULL;
	int err;

	struct ane_request req;
	memset(&req, 0, sizeof(req));

	req.nid = ANE_FIFO_NID;
	req.td_size = args->td_size;
	req.td_count = args->td_count;

	for (int bdx = 0; bdx < ANE_TILE_COUNT; bdx++) {
		if (args->handles[bdx]) {
			bo = ane_bo_lookup(file, args->handles[bdx]);
			if (!bo)
				return -EINVAL;
			req.bar[bdx] = lower_32_bits(bo->iova);
		}
	}

	/*
	 * Apple stores the compiled neural network as the TD catted
	 * with the kernel @ 16 gran padding. This 1) takes advantage
	 * of bank aligned access, 2) is a convienient serialization
	 * method, making one less non-aligned buffer to worry about,
	 * 3) makes my life easier :). We thus unpack the iova at which
	 * the kernel *should* start.
	 */
	req.bar[1] = req.bar[0] + round_up(args->tsk_size, ANE_CMD_GRAN);

	bo = ane_bo_lookup(file, args->fifo_handle);
	if (!bo)
		return -EINVAL;
	req.fifo_addr = lower_32_bits(bo->iova);

	/*
	 * Single threaded for now; cores are activated in parallel,
	 * so scheduling would only improve the enqueue time, or
	 * 36 writel()'s.
	 */
	mutex_lock(&ane->engine_lock);

	err = ane_tm_enqueue(ane, &req);
	if (err < 0)
		goto unlock;

	err = ane_tm_execute(ane, &req);
	if (err < 0)
		goto unlock;

unlock:
	mutex_unlock(&ane->engine_lock);
	return err;
}

static const struct drm_ioctl_desc ane_drm_ioctls[] = {
	DRM_IOCTL_DEF_DRV(ANE_BO_INIT, ane_bo_init, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(ANE_BO_FREE, ane_bo_free, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(ANE_SUBMIT, ane_submit, DRM_RENDER_ALLOW),
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

static int ane_drm_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct drm_gem_object *gem = NULL;
	struct ane_bo *bo = NULL;
	int err;

	err = drm_gem_mmap(file, vma);
	if (err < 0)
		return err;

	/*
	 * Set vm_pgoff (used as a fake buffer offset by DRM) to 0 and map the
	 * whole buffer from the start.
	 */
	vma->vm_pgoff = 0;
	gem = vma->vm_private_data;
	bo = to_bo(gem);

	/*
	 * We allocated a struct page table for rk_obj, so clear
	 * VM_PFNMAP flag that was set by drm_gem_mmap_obj()/drm_gem_mmap().
	 */
	vma->vm_flags |= VM_IO | VM_DONTEXPAND | VM_DONTDUMP;
	vma->vm_flags &= ~VM_PFNMAP;

	vma->vm_page_prot =
		pgprot_writecombine(vm_get_page_prot(vma->vm_flags));
	vma->vm_page_prot = pgprot_decrypted(vma->vm_page_prot);

	if (vma_pages(vma) == 0)
		return -ENXIO;
	return vm_map_pages(vma, bo->pages, bo->npages);
}

static const struct file_operations ane_drm_fops = {
	.owner = THIS_MODULE,
	.open = drm_open, // accel_open
	.release = drm_release,
	.unlocked_ioctl = ane_drm_unlocked_ioctl,
	.mmap = ane_drm_mmap,
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

static int ane_iommu_domain_init(struct ane_device *ane)
{
	dma_addr_t min_iova, max_iova;

	struct iommu_domain *domain = iommu_get_domain_for_dev(ane->dev);
	if (!domain)
		return -EPROBE_DEFER;

	ane->domain = domain;
	ane->shift = __ffs(ane->domain->pgsize_bitmap);

	/* DMA chans can't access iovas past the limit */
	/* likely a kernel prefetch distance constraint */
	min_iova = ane->hw->dart.vm_base;

	/* a page before as to not reach real limit */
	max_iova = min_iova + ane->hw->dart.vm_size - (1UL << ane->shift);

	drm_mm_init(&ane->mm, min_iova, max_iova);

	return 0;
}

static void ane_iommu_domain_free(struct ane_device *ane)
{
	drm_mm_takedown(&ane->mm);
}

static void ane_iommu_remap_ttbr(struct ane_device *ane)
{
	/* L2 DMA transfers fail without */
	writel_relaxed(readl_relaxed(ane->ttbr),
		       ane->dart1 + ane->hw->dart.ttbr);
	writel_relaxed(readl_relaxed(ane->ttbr),
		       ane->dart2 + ane->hw->dart.ttbr);
}

static void ane_hw_reset(struct ane_device *ane)
{
	ane_iommu_remap_ttbr(ane);
	ane_tm_enable(ane);
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

	ane->ttbr = devm_ioremap(
		ane->dev, ane->hw->dart.base + ane->hw->dart.ttbr, sizeof(u32));
	if (IS_ERR(ane->ttbr)) {
		err = PTR_ERR(ane->ttbr);
		goto detach_genpd;
	}

	mutex_init(&ane->iommu_lock);
	mutex_init(&ane->engine_lock);

	err = ane_iommu_domain_init(ane);
	if (err < 0)
		goto detach_genpd;

	ane_hw_reset(ane);

	/* measured 3s on macos, but 1s seems more stable */
	pm_runtime_set_autosuspend_delay(ane->dev, 1000);
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
	ane_hw_reset(ane);
	return 0;
}

// clang-format off
static const struct dev_pm_ops ane_pm_ops = {
	SET_RUNTIME_PM_OPS(ane_runtime_suspend, ane_runtime_resume, NULL)
	SET_SYSTEM_SLEEP_PM_OPS(pm_runtime_force_suspend, pm_runtime_force_resume)
};
// clang-format on

/* T8020/T6000 registers */
#define DART_T8020_STREAM_COMMAND	     0x20
#define DART_T8020_STREAM_SELECT	     0x34
#define DART_T8020_TTBR			     0x200
#define DART_T8020_STREAM_COMMAND_INVALIDATE BIT(20)

static const struct ane_hw ane_hw_t8103_ane = {
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
	{ .compatible = "apple,t8103-ane", .data = &ane_hw_t8103_ane },
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
