// SPDX-License-Identifier: GPL-2.0+
/*
 * Mark fused-off CPU cores in the OS device tree.
 *
 * Parts of a SoC can ship with CPU cores disabled in fuses. XBL lists them
 * in the SMEM socinfo item (format 14 and later): a mask of disabled cores
 * per cluster. The firmware refuses PSCI CPU_ON for those cores, so give
 * them status = "fail" and the OS does not count them as present.
 */

#include <fdt_support.h>
#include <linux/libfdt.h>
#include <log.h>
#include <soc/qcom/smem.h>
#include <soc/qcom/socinfo.h>

#define SOCINFO_VERSION_14	((0 << 16) | 14)

int ft_board_setup(void *blob, struct bd_info *bd)
{
	const __le32 *masks;
	struct socinfo *info;
	size_t size;
	u32 num, off, cl, core;
	int cpus, node;

	info = qcom_smem_get(QCOM_SMEM_HOST_ANY, SMEM_HW_SW_BUILD_ID, &size);
	if (IS_ERR(info))
		return 0;
	if (le32_to_cpu(info->fmt) < SOCINFO_VERSION_14 ||
	    size < offsetof(struct socinfo, num_subset_parts))
		return 0;

	num = le32_to_cpu(info->num_clusters);
	off = le32_to_cpu(info->ncluster_array_offset);
	if (!num || off > size || num > (size - off) / sizeof(__le32))
		return 0;
	masks = (const __le32 *)((uintptr_t)info + off);

	cpus = fdt_path_offset(blob, "/cpus");
	if (cpus < 0)
		return 0;

	fdt_for_each_subnode(node, blob, cpus) {
		const fdt32_t *reg;
		u64 mpidr;
		int len;

		reg = fdt_getprop(blob, node, "reg", &len);
		if (!reg || (len != 4 && len != 8))
			continue;
		mpidr = len == 8 ? fdt64_to_cpu(*(const fdt64_t *)reg) :
				   fdt32_to_cpu(*reg);
		cl = (mpidr >> 16) & 0xff;
		core = (mpidr >> 8) & 0xff;
		if (cl >= num || core >= 32)
			continue;
		if (!(le32_to_cpu(masks[cl]) & BIT(core)))
			continue;
		log_info("CPU 0x%llx is fused off\n", mpidr);
		fdt_setprop_string(blob, node, "status", "fail");
	}

	return 0;
}
