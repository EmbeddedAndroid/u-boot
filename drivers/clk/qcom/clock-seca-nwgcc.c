// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 */

#include <linux/types.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,seca-nwgcc.h>
#include "clock-qcom.h"

/*
 * SecaAU moves both UFS controllers into NW_GCC: UFS_0 at 0x52000 and UFS_1
 * at 0x3d000, with their MEM_PHY GDSCs at 0x3b000 and 0x3c000. The symbol
 * clocks are sourced by the PHY and only run once it is up, so they are
 * not polled.
 */
static const struct gate_clk seca_nwgcc_clks[] = {
	GATE_CLK_POLLED(NW_GCC_AGGRE_NOC_UFS_0_PHY_AXI_CLK, 0x520f8, BIT(0), 0x520f8),
	GATE_CLK_POLLED(NW_GCC_AGGRE_NOC_UFS_1_PHY_AXI_CLK, 0x3d0f8, BIT(0), 0x3d0f8),
	GATE_CLK_POLLED(NW_GCC_UFS_0_PHY_AHB_CLK,          0x52028, BIT(0), 0x52028),
	GATE_CLK_POLLED(NW_GCC_UFS_0_PHY_AXI_CLK,          0x52018, BIT(0), 0x52018),
	GATE_CLK_POLLED(NW_GCC_UFS_0_PHY_ICE_CORE_CLK,     0x5207c, BIT(0), 0x5207c),
	GATE_CLK_POLLED(NW_GCC_UFS_0_PHY_PHY_AUX_CLK,      0x520bc, BIT(0), 0x520bc),
	GATE_CLK_POLLED(NW_GCC_UFS_0_PHY_UNIPRO_CORE_CLK,  0x5206c, BIT(0), 0x5206c),
	GATE_CLK_POLLED(NW_GCC_UFS_1_PHY_AHB_CLK,          0x3d028, BIT(0), 0x3d028),
	GATE_CLK_POLLED(NW_GCC_UFS_1_PHY_AXI_CLK,          0x3d018, BIT(0), 0x3d018),
	GATE_CLK_POLLED(NW_GCC_UFS_1_PHY_ICE_CORE_CLK,     0x3d07c, BIT(0), 0x3d07c),
	GATE_CLK_POLLED(NW_GCC_UFS_1_PHY_PHY_AUX_CLK,      0x3d0bc, BIT(0), 0x3d0bc),
	GATE_CLK_POLLED(NW_GCC_UFS_1_PHY_UNIPRO_CORE_CLK,  0x3d06c, BIT(0), 0x3d06c),

	GATE_CLK(NW_GCC_UFS_0_PHY_RX_SYMBOL_0_CLK, 0x52030, BIT(0)),
	GATE_CLK(NW_GCC_UFS_0_PHY_RX_SYMBOL_1_CLK, 0x520d8, BIT(0)),
	GATE_CLK(NW_GCC_UFS_0_PHY_TX_SYMBOL_0_CLK, 0x5202c, BIT(0)),
	GATE_CLK(NW_GCC_UFS_1_PHY_RX_SYMBOL_0_CLK, 0x3d030, BIT(0)),
	GATE_CLK(NW_GCC_UFS_1_PHY_RX_SYMBOL_1_CLK, 0x3d0d8, BIT(0)),
	GATE_CLK(NW_GCC_UFS_1_PHY_TX_SYMBOL_0_CLK, 0x3d02c, BIT(0)),
};

static const struct qcom_reset_map seca_nwgcc_resets[] = {
	[NW_GCC_UFS_0_PHY_BCR] = { 0x52000 },
	[NW_GCC_UFS_1_PHY_BCR] = { 0x3d000 },
};

static const struct qcom_power_map seca_nwgcc_gdscs[] = {
	[NW_GCC_UFS_0_MEM_PHY_GDSC] = { 0x3b000 },
	[NW_GCC_UFS_0_PHY_GDSC] = { 0x52004 },
	[NW_GCC_UFS_1_MEM_PHY_GDSC] = { 0x3c000 },
	[NW_GCC_UFS_1_PHY_GDSC] = { 0x3d004 },
};

static int seca_nwgcc_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	return qcom_gate_clk_en(priv, clk->id);
}

static struct msm_clk_data seca_nwgcc_data = {
	.resets = seca_nwgcc_resets,
	.num_resets = ARRAY_SIZE(seca_nwgcc_resets),
	.clks = seca_nwgcc_clks,
	.num_clks = ARRAY_SIZE(seca_nwgcc_clks),

	.power_domains = seca_nwgcc_gdscs,
	.num_power_domains = ARRAY_SIZE(seca_nwgcc_gdscs),

	.enable = seca_nwgcc_enable,
};

static const struct udevice_id gcc_seca_nwgcc_of_match[] = {
	{
		.compatible = "qcom,seca-nwgcc",
		.data = (ulong)&seca_nwgcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_seca_nwgcc) = {
	.name		= "gcc_seca_nwgcc",
	.id		= UCLASS_NOP,
	.of_match	= gcc_seca_nwgcc_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
