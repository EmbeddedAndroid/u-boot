/* SPDX-License-Identifier: GPL-2.0+ */

#if defined(CONFIG_SPL_BUILD)
	b	reset
#else
#if defined(CONFIG_BOOT0_MSM8916_PSCI_WORKAROUND)
#include "msm8916_boot0.h"
#elif defined(CONFIG_QCOM_EL2_GUNYAH_EXIT_SUPPORT) || defined(CONFIG_BOOT0_QCOM_BOOT_CPU)
#if defined(CONFIG_QCOM_EL2_GUNYAH_EXIT_SUPPORT)
#include "gunyah_exit_boot0.h"
#endif
#if defined(CONFIG_BOOT0_QCOM_BOOT_CPU)
#include "boot_cpu_boot0.h"
#endif
	b	reset
#elif defined(CONFIG_QCOM_BOOT0_SNAGBOOT_MODE)
#include "snagboot_boot0.h"
#else
	b	reset
#endif
#endif
