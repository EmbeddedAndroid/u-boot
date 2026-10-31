// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm Eliza pinctrl
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Pin data from the Linux pinctrl-eliza driver.
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"qup2_se5", 1},
	{"gpio", 0},
};

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
	[0] = UFS_RESET("ufs_reset", 0xc9004, 0xca000),
};

static const char *eliza_get_function_name(struct udevice *dev,
					   unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *eliza_get_pin_name(struct udevice *dev,
				      unsigned int selector)
{
	if (selector == 185)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 msm_special_pins_data[0].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int eliza_get_function_mux(__maybe_unused unsigned int pin,
				  unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data eliza_data = {
	.pin_data = {
		.pin_count = 186,
		.special_pins_start = 185,
		.special_pins_data = msm_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = eliza_get_function_name,
	.get_function_mux = eliza_get_function_mux,
	.get_pin_name = eliza_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,eliza-tlmm", .data = (ulong)&eliza_data },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pinctrl_eliza) = {
	.name		= "pinctrl_eliza",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};
