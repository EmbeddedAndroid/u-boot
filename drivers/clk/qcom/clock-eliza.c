// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm Eliza
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Register data from the Linux gcc-eliza and tcsrcc-eliza drivers.
 */

#include <clk-uclass.h>
#include <dm.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,eliza-gcc.h>
#include <dt-bindings/clock/qcom,eliza-tcsr.h>

#include "clock-qcom.h"

/* RPMh CXO: the 76.8 MHz board XO divided by two */
#define TCXO_RATE	38400000

/* bi_tcxo_div2, the GCC reference */
#define TCXO_DIV2_RATE	(TCXO_RATE / 2)

static const struct freq_tbl ftbl_gcc_qupv3_wrap2_s5_clk_src[] = {
	F(7372800, CFG_CLK_SRC_GPLL0_EVEN, 1, 384, 15625),
	F(14745600, CFG_CLK_SRC_GPLL0_EVEN, 1, 768, 15625),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(29491200, CFG_CLK_SRC_GPLL0_EVEN, 1, 1536, 15625),
	F(32000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 75),
	F(48000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 25),
	F(64000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 16, 75),
	F(80000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 15),
	F(96000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 25),
	F(100000000, CFG_CLK_SRC_GPLL0, 6, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_usb30_prim_master_clk_src[] = {
	F(66666667, CFG_CLK_SRC_GPLL0_EVEN, 4.5, 0, 0),
	F(133333333, CFG_CLK_SRC_GPLL0, 4.5, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	F(240000000, CFG_CLK_SRC_GPLL0, 2.5, 0, 0),
	{ }
};

static ulong eliza_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	switch (clk->id) {
	case GCC_QUPV3_WRAP2_S5_CLK: /* UART13 */
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap2_s5_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x1e640,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_USB30_PRIM_MASTER_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x39030,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB30_PRIM_MOCK_UTMI_CLK:
		clk_rcg_set_rate(priv->base, 0x39048, 0, CFG_CLK_SRC_CXO);
		return TCXO_DIV2_RATE;
	default:
		return 0;
	}
}

static const struct gate_clk eliza_clks[] = {
	GATE_CLK_POLLED(GCC_AGGRE_UFS_PHY_AXI_CLK,	0x770f4, BIT(0), 0x770f4),
	GATE_CLK_POLLED(GCC_AGGRE_USB3_PRIM_AXI_CLK,	0x39094, BIT(0), 0x39094),
	GATE_CLK_POLLED(GCC_CFG_NOC_USB3_PRIM_AXI_CLK,	0x39090, BIT(0), 0x39090),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S0_CLK,		0x52008, BIT(22), 0x18004),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S1_CLK,		0x52008, BIT(23), 0x18140),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S2_CLK,		0x52008, BIT(24), 0x1827c),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S3_CLK,		0x52008, BIT(25), 0x18290),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S4_CLK,		0x52008, BIT(26), 0x183cc),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S5_CLK,		0x52008, BIT(27), 0x18508),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S6_CLK,		0x52008, BIT(28), 0x18644),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S7_CLK,		0x52010, BIT(16), 0x18780),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP2_S0_CLK,		0x52010, BIT(4), 0x1e004),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP2_S1_CLK,		0x52010, BIT(5), 0x1e140),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP2_S2_CLK,		0x52010, BIT(6), 0x1e27c),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP2_S3_CLK,		0x52010, BIT(7), 0x1e3b8),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP2_S4_CLK,		0x52010, BIT(8), 0x1e4f4),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP2_S5_CLK,		0x52010, BIT(9), 0x1e630),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP2_S6_CLK,		0x52010, BIT(10), 0x1e76c),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP2_S7_CLK,		0x52010, BIT(17), 0x1e8a8),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP_1_M_AHB_CLK,	0x52008, BIT(20), 0x23000),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP_1_S_AHB_CLK,	0x52008, BIT(21), 0x23004),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP_2_M_AHB_CLK,	0x52010, BIT(2), 0x23158),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP_2_S_AHB_CLK,	0x52010, BIT(1), 0x2315c),
	GATE_CLK_POLLED(GCC_UFS_PHY_AHB_CLK,		0x77028, BIT(0), 0x77028),
	GATE_CLK_POLLED(GCC_UFS_PHY_AXI_CLK,		0x77018, BIT(0), 0x77018),
	GATE_CLK_POLLED(GCC_UFS_PHY_PHY_AUX_CLK,	0x770bc, BIT(0), 0x770bc),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_0_CLK,		0x77030, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_1_CLK,		0x770d8, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_TX_SYMBOL_0_CLK,		0x7702c, BIT(0)),
	GATE_CLK_POLLED(GCC_UFS_PHY_UNIPRO_CORE_CLK,	0x7706c, BIT(0), 0x7706c),
	GATE_CLK_POLLED(GCC_USB30_PRIM_MASTER_CLK,	0x39018, BIT(0), 0x39018),
	GATE_CLK_POLLED(GCC_USB30_PRIM_MOCK_UTMI_CLK,	0x3902c, BIT(0), 0x3902c),
	GATE_CLK_POLLED(GCC_USB30_PRIM_SLEEP_CLK,	0x39028, BIT(0), 0x39028),
	GATE_CLK(GCC_USB3_PRIM_PHY_AUX_CLK,		0x39064, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK,		0x39068, BIT(0)),
};

static int eliza_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	/* No U-Boot driver handles the USB 3 PHY: enable its aux clocks, as SM8650 does */
	switch (clk->id) {
	case GCC_AGGRE_USB3_PRIM_AXI_CLK:
		qcom_gate_clk_en(priv, GCC_USB30_PRIM_MASTER_CLK);
		fallthrough;
	case GCC_USB30_PRIM_MASTER_CLK:
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_AUX_CLK);
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_COM_AUX_CLK);
		break;
	}

	return qcom_gate_clk_en(priv, clk->id);
}

static const struct qcom_reset_map eliza_gcc_resets[] = {
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x12000 },
	[GCC_UFS_PHY_BCR] = { 0x77000 },
	[GCC_USB30_PRIM_BCR] = { 0x39000 },
};

static const struct qcom_power_map eliza_gdscs[] = {
	[GCC_UFS_MEM_PHY_GDSC] = { 0x9e000 },
	[GCC_UFS_PHY_GDSC] = { 0x77004 },
	[GCC_USB30_PRIM_GDSC] = { 0x39004 },
};

static struct msm_clk_data eliza_gcc_data = {
	.resets = eliza_gcc_resets,
	.num_resets = ARRAY_SIZE(eliza_gcc_resets),
	.clks = eliza_clks,
	.num_clks = ARRAY_SIZE(eliza_clks),
	.power_domains = eliza_gdscs,
	.num_power_domains = ARRAY_SIZE(eliza_gdscs),

	.enable = eliza_enable,
	.set_rate = eliza_set_rate,
};

static const struct udevice_id gcc_eliza_of_match[] = {
	{
		.compatible = "qcom,eliza-gcc",
		.data = (ulong)&eliza_gcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_eliza) = {
	.name		= "gcc_eliza",
	.id		= UCLASS_NOP,
	.of_match	= gcc_eliza_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};

/* TCSRCC */

static const struct gate_clk eliza_tcsr_clks[] = {
	GATE_CLK(TCSR_USB2_CLKREF_EN,		0x4, BIT(0)),
};

static struct msm_clk_data eliza_tcsrcc_data = {
	.clks = eliza_tcsr_clks,
	.num_clks = ARRAY_SIZE(eliza_tcsr_clks),
};

static int tcsrcc_eliza_clk_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	return qcom_gate_clk_en(priv, clk->id);
}

static ulong tcsrcc_eliza_clk_get_rate(struct clk *clk)
{
	return TCXO_RATE;
}

static int tcsrcc_eliza_clk_probe(struct udevice *dev)
{
	struct msm_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->data = (struct msm_clk_data *)dev_get_driver_data(dev);

	return 0;
}

static struct clk_ops tcsrcc_eliza_clk_ops = {
	.enable = tcsrcc_eliza_clk_enable,
	.get_rate = tcsrcc_eliza_clk_get_rate,
};

static const struct udevice_id tcsrcc_eliza_of_match[] = {
	{
		.compatible = "qcom,eliza-tcsr",
		.data = (ulong)&eliza_tcsrcc_data,
	},
	{ }
};

U_BOOT_DRIVER(tcsrcc_eliza) = {
	.name		= "tcsrcc_eliza",
	.id		= UCLASS_CLK,
	.of_match	= tcsrcc_eliza_of_match,
	.ops		= &tcsrcc_eliza_clk_ops,
	.priv_auto	= sizeof(struct msm_clk_priv),
	.probe		= tcsrcc_eliza_clk_probe,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
