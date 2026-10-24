// SPDX-License-Identifier: GPL-2.0+
/*
 * Arm SMMUv3 left in bypass by the boot firmware
 *
 * U-Boot does not program the SMMUv3. When it is disabled (SMMU_CR0.SMMUEN
 * clear) and global bypass is not aborting (SMMU_GBPA.ABORT clear), incoming
 * transactions bypass translation, so the devices behind it can be used as
 * they are. Binding this driver lets them probe; an SMMUv3 that is enabled
 * or aborting is refused.
 */

#include <dm.h>
#include <iommu.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>

#define SMMU_CR0		0x20
#define SMMU_CR0_SMMUEN		BIT(0)
#define SMMU_GBPA		0x44
#define SMMU_GBPA_ABORT		BIT(20)

static int arm_smmu_v3_probe(struct udevice *dev)
{
	void __iomem *base = dev_read_addr_ptr(dev);
	u32 cr0, gbpa;

	if (!base)
		return -EINVAL;

	cr0 = readl(base + SMMU_CR0);
	gbpa = readl(base + SMMU_GBPA);
	if ((cr0 & SMMU_CR0_SMMUEN) || (gbpa & SMMU_GBPA_ABORT)) {
		dev_err(dev, "not in bypass (CR0 %#x GBPA %#x)\n", cr0, gbpa);
		return -EBUSY;
	}

	return 0;
}

static const struct iommu_ops arm_smmu_v3_ops = {
};

static const struct udevice_id arm_smmu_v3_ids[] = {
	{ .compatible = "arm,smmu-v3" },
	{ }
};

U_BOOT_DRIVER(arm_smmu_v3) = {
	.name = "arm_smmu_v3",
	.id = UCLASS_IOMMU,
	.of_match = arm_smmu_v3_ids,
	.ops = &arm_smmu_v3_ops,
	.probe = arm_smmu_v3_probe,
};
