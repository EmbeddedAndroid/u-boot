// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm Kaanapali pinctrl
 *
 * Pin data from the Linux pinctrl-kaanapali driver.
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"qup1_se7", 1},
	{"gpio", 0},
	{"pcie0_clk_req_n", 1},
};

#define SDC_QDSD_PINGROUP(pg_name, ctl, pull, drv)	\
	{					        \
		.name = pg_name,			\
		.ctl_reg = ctl,				\
		.io_reg = 0,				\
		.pull_bit = pull,			\
		.drv_bit = drv,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = -1,				\
	}

#define UFS_RESET(pg_name, ctl, io)			\
	{					        \
		.name = pg_name,			\
		.ctl_reg = ctl,				\
		.io_reg = io,				\
		.pull_bit = 3,				\
		.drv_bit = 0,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = 0,				\
	}

static const struct msm_special_pin_data msm_special_pins_data[] = {
	[0] = UFS_RESET("ufs_reset", 0xe8004, 0xe9000),
	[1] = SDC_QDSD_PINGROUP("sdc2_clk", 0xdd000, 14, 6),
	[2] = SDC_QDSD_PINGROUP("sdc2_cmd", 0xdd000, 11, 3),
	[3] = SDC_QDSD_PINGROUP("sdc2_data", 0xdd000, 9, 0),
};

static const char *kaanapali_get_function_name(struct udevice *dev,
					       unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *kaanapali_get_pin_name(struct udevice *dev,
					  unsigned int selector)
{
	if (selector >= 217 && selector <= 220)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 msm_special_pins_data[selector - 217].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int kaanapali_get_function_mux(__maybe_unused unsigned int pin,
				      unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data kaanapali_data = {
	.pin_data = {
		.pin_count = 221,
		.special_pins_start = 217,
		.special_pins_data = msm_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = kaanapali_get_function_name,
	.get_function_mux = kaanapali_get_function_mux,
	.get_pin_name = kaanapali_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,kaanapali-tlmm", .data = (ulong)&kaanapali_data },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pinctrl_kaanapali) = {
	.name		= "pinctrl_kaanapali",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};

