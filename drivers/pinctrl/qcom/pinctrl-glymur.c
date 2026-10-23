// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm Glymur pinctrl
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"qup2_se5", 1},
	{"pcie3b_clk", 1},
	{"pcie4_clk_req_n", 1},
	{"pcie5_clk_req_n", 1},
	{"pcie6_clk_req_n", 1},
	{"gpio", 0},
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
	[0] = UFS_RESET("ufs_reset", 0x104004, 0x105000),
	[1] = SDC_QDSD_PINGROUP("sdc2_clk", 0xff000, 14, 6),
	[2] = SDC_QDSD_PINGROUP("sdc2_cmd", 0xff000, 11, 3),
	[3] = SDC_QDSD_PINGROUP("sdc2_data", 0xff000, 9, 0),
};

static const char *glymur_get_function_name(struct udevice *dev,
					    unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *glymur_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	if (selector >= 250 && selector <= 253)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 msm_special_pins_data[selector - 250].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int glymur_get_function_mux(__maybe_unused unsigned int pin,
				   unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data glymur_data = {
	.pin_data = {
		.pin_count = 254,
		.special_pins_start = 250,
		.special_pins_data = msm_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = glymur_get_function_name,
	.get_function_mux = glymur_get_function_mux,
	.get_pin_name = glymur_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,glymur-tlmm", .data = (ulong)&glymur_data },
	{ .compatible = "qcom,mahua-tlmm", .data = (ulong)&glymur_data },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pinctrl_glymur) = {
	.name		= "pinctrl_glymur",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};

