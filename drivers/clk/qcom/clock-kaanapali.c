// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm Kaanapali
 *
 * Register offsets from the Linux gcc-kaanapali and tcsrcc-kaanapali drivers.
 */

#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,kaanapali-gcc.h>
#include <dt-bindings/clock/qcom,sm8750-tcsr.h>

#include "clock-qcom.h"

/* RPMh CXO: the 76.8 MHz board XO divided by 2 */
#define TCXO_RATE	38400000

/* bi_tcxo_div2, the GCC reference */
#define TCXO_DIV2_RATE	(TCXO_RATE / 2)

static const struct freq_tbl ftbl_gcc_qupv3_wrap1_s4_clk_src[] = {
	F(7372800, CFG_CLK_SRC_GPLL0_EVEN, 1, 384, 15625),
	F(14745600, CFG_CLK_SRC_GPLL0_EVEN, 1, 768, 15625),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(29491200, CFG_CLK_SRC_GPLL0_EVEN, 1, 1536, 15625),
	F(32000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 75),
	F(48000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 25),
	F(51200000, CFG_CLK_SRC_GPLL0_EVEN, 1, 64, 375),
	F(60000000, CFG_CLK_SRC_GPLL0_EVEN, 5, 0, 0),
	F(64000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 16, 75),
	F(66666667, CFG_CLK_SRC_GPLL0, 9, 0, 0),
	F(75000000, CFG_CLK_SRC_GPLL0_EVEN, 4, 0, 0),
	F(80000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 15),
	F(96000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 25),
	F(100000000, CFG_CLK_SRC_GPLL0, 6, 0, 0),
	F(102400000, CFG_CLK_SRC_GPLL0_EVEN, 1, 128, 375),
	F(112000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 28, 75),
	F(117964800, CFG_CLK_SRC_GPLL0_EVEN, 1, 6144, 15625),
	F(120000000, CFG_CLK_SRC_GPLL0, 5, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_usb30_prim_master_clk_src[] = {
	F(66666667, CFG_CLK_SRC_GPLL0_EVEN, 4.5, 0, 0),
	F(133333333, CFG_CLK_SRC_GPLL0, 4.5, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	F(240000000, CFG_CLK_SRC_GPLL0, 2.5, 0, 0),
	{ }
};

static ulong kaanapali_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	switch (clk->id) {
	case GCC_QUPV3_WRAP1_S7_CLK: /* UART7 */
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap1_s4_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x18790,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_USB30_PRIM_MASTER_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x39034,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB30_PRIM_MOCK_UTMI_CLK:
		clk_rcg_set_rate(priv->base, 0x3904c, 0, 0);
		return TCXO_DIV2_RATE;
	case GCC_USB3_PRIM_PHY_AUX_CLK_SRC:
		clk_rcg_set_rate(priv->base, 0x39078, 0, 0);
		return TCXO_DIV2_RATE;
	default:
		return 0;
	}
}

static const struct gate_clk kaanapali_clks[] = {
	GATE_CLK_POLLED(GCC_AGGRE_UFS_PHY_AXI_CLK,	0x770f0, BIT(0), 0x770f0),
	GATE_CLK_POLLED(GCC_AGGRE_USB3_PRIM_AXI_CLK,	0x39094, BIT(0), 0x39094),
	GATE_CLK_POLLED(GCC_CFG_NOC_USB3_PRIM_AXI_CLK,	0x39090, BIT(0), 0x39090),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S7_CLK,		0x52010, BIT(16), 0x18780),
	GATE_CLK_POLLED(GCC_UFS_PHY_AHB_CLK,		0x77028, BIT(0), 0x77028),
	GATE_CLK_POLLED(GCC_UFS_PHY_AXI_CLK,		0x77018, BIT(0), 0x77018),
	GATE_CLK_POLLED(GCC_UFS_PHY_ICE_CORE_CLK,	0x7707c, BIT(0), 0x7707c),
	GATE_CLK_POLLED(GCC_UFS_PHY_PHY_AUX_CLK,	0x770bc, BIT(0), 0x770bc),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_0_CLK,		0x77030, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_1_CLK,		0x770d8, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_TX_SYMBOL_0_CLK,		0x7702c, BIT(0)),
	GATE_CLK_POLLED(GCC_UFS_PHY_UNIPRO_CORE_CLK,	0x7706c, BIT(0), 0x7706c),
	GATE_CLK_POLLED(GCC_USB30_PRIM_MASTER_CLK,	0x39018, BIT(0), 0x39018),
	GATE_CLK_POLLED(GCC_USB30_PRIM_MOCK_UTMI_CLK,	0x3902c, BIT(0), 0x3902c),
	GATE_CLK_POLLED(GCC_USB30_PRIM_SLEEP_CLK,	0x39028, BIT(0), 0x39028),
	GATE_CLK_POLLED(GCC_USB3_PRIM_PHY_AUX_CLK,	0x39068, BIT(0), 0x39068),
	GATE_CLK_POLLED(GCC_USB3_PRIM_PHY_COM_AUX_CLK,	0x3906c, BIT(0), 0x3906c),
	GATE_CLK(GCC_USB3_PRIM_PHY_PIPE_CLK,		0x39070, BIT(0)),
};

static int kaanapali_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

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

static const struct qcom_reset_map kaanapali_gcc_resets[] = {
	[GCC_CAMERA_BCR] = { 0x26000 },
	[GCC_DISPLAY_BCR] = { 0x27000 },
	[GCC_EVA_AXI0_CLK_ARES] = { 0x9f008, 2 },
	[GCC_EVA_AXI0C_CLK_ARES] = { 0x9f01c, 2 },
	[GCC_EVA_BCR] = { 0x9f000 },
	[GCC_GPU_BCR] = { 0x71000 },
	[GCC_PCIE_0_BCR] = { 0x6b000 },
	[GCC_PCIE_0_LINK_DOWN_BCR] = { 0x6c014 },
	[GCC_PCIE_0_NOCSR_COM_PHY_BCR] = { 0x6c020 },
	[GCC_PCIE_0_PHY_BCR] = { 0x6c01c },
	[GCC_PCIE_0_PHY_NOCSR_COM_PHY_BCR] = { 0x6c028 },
	[GCC_PCIE_PHY_BCR] = { 0x6f000 },
	[GCC_PCIE_PHY_CFG_AHB_BCR] = { 0x6f00c },
	[GCC_PCIE_PHY_COM_BCR] = { 0x6f010 },
	[GCC_PCIE_RSCC_BCR] = { 0x11000 },
	[GCC_PDM_BCR] = { 0x33000 },
	[GCC_QUPV3_WRAPPER_1_BCR] = { 0x18000 },
	[GCC_QUPV3_WRAPPER_2_BCR] = { 0x1e000 },
	[GCC_QUPV3_WRAPPER_3_BCR] = { 0xa8000 },
	[GCC_QUPV3_WRAPPER_4_BCR] = { 0xa9000 },
	[GCC_QUPV3_WRAPPER_I2C_BCR] = { 0x17000 },
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x12000 },
	[GCC_QUSB2PHY_SEC_BCR] = { 0x12004 },
	[GCC_SDCC2_BCR] = { 0x14000 },
	[GCC_SDCC4_BCR] = { 0x16000 },
	[GCC_UFS_PHY_BCR] = { 0x77000 },
	[GCC_USB30_PRIM_BCR] = { 0x39000 },
	[GCC_USB3_DP_PHY_PRIM_BCR] = { 0x50008 },
	[GCC_USB3_DP_PHY_SEC_BCR] = { 0x50014 },
	[GCC_USB3_PHY_PRIM_BCR] = { 0x50000 },
	[GCC_USB3_PHY_SEC_BCR] = { 0x5000c },
	[GCC_USB3PHY_PHY_PRIM_BCR] = { 0x50004 },
	[GCC_USB3PHY_PHY_SEC_BCR] = { 0x50010 },
	[GCC_VIDEO_AXI0_CLK_ARES] = { 0x32018, 2 },
	[GCC_VIDEO_AXI1_CLK_ARES] = { 0x3202c, 2 },
	[GCC_VIDEO_BCR] = { 0x32000 },
	[GCC_VIDEO_XO_CLK_ARES] = { 0x32040, 2 },
};

static const struct qcom_power_map kaanapali_gdscs[] = {
	[GCC_PCIE_0_GDSC] = { 0x6b004 },
	[GCC_PCIE_0_PHY_GDSC] = { 0x6c000 },
	[GCC_UFS_MEM_PHY_GDSC] = { 0x9e000 },
	[GCC_UFS_PHY_GDSC] = { 0x77004 },
	[GCC_USB30_PRIM_GDSC] = { 0x39004 },
	[GCC_USB3_PHY_GDSC] = { 0x50018 },
};

static struct msm_clk_data kaanapali_gcc_data = {
	.resets = kaanapali_gcc_resets,
	.num_resets = ARRAY_SIZE(kaanapali_gcc_resets),
	.clks = kaanapali_clks,
	.num_clks = ARRAY_SIZE(kaanapali_clks),
	.power_domains = kaanapali_gdscs,
	.num_power_domains = ARRAY_SIZE(kaanapali_gdscs),

	.enable = kaanapali_enable,
	.set_rate = kaanapali_set_rate,
};

static const struct udevice_id gcc_kaanapali_of_match[] = {
	{
		.compatible = "qcom,kaanapali-gcc",
		.data = (ulong)&kaanapali_gcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_kaanapali) = {
	.name		= "gcc_kaanapali",
	.id		= UCLASS_NOP,
	.of_match	= gcc_kaanapali_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};

/* TCSRCC */

static const struct gate_clk kaanapali_tcsr_clks[] = {
	GATE_CLK(TCSR_PCIE_0_CLKREF_EN,		0x15044, BIT(0)),
	GATE_CLK(TCSR_UFS_CLKREF_EN,		0x15054, BIT(0)),
	GATE_CLK(TCSR_USB2_CLKREF_EN,		0x1505c, BIT(0)),
	GATE_CLK(TCSR_USB3_CLKREF_EN,		0x1504c, BIT(0)),
};

static struct msm_clk_data kaanapali_tcsrcc_data = {
	.clks = kaanapali_tcsr_clks,
	.num_clks = ARRAY_SIZE(kaanapali_tcsr_clks),
};

static int tcsrcc_kaanapali_clk_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	qcom_gate_clk_en(priv, clk->id);

	return 0;
}

static ulong tcsrcc_kaanapali_clk_get_rate(struct clk *clk)
{
	return TCXO_RATE;
}

static int tcsrcc_kaanapali_clk_probe(struct udevice *dev)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(dev);
	struct msm_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->data = data;

	return 0;
}

static struct clk_ops tcsrcc_kaanapali_clk_ops = {
	.enable = tcsrcc_kaanapali_clk_enable,
	.get_rate = tcsrcc_kaanapali_clk_get_rate,
};

static const struct udevice_id tcsrcc_kaanapali_of_match[] = {
	{
		.compatible = "qcom,kaanapali-tcsr",
		.data = (ulong)&kaanapali_tcsrcc_data,
	},
	{ }
};

U_BOOT_DRIVER(tcsrcc_kaanapali) = {
	.name		= "tcsrcc_kaanapali",
	.id		= UCLASS_CLK,
	.of_match	= tcsrcc_kaanapali_of_match,
	.ops		= &tcsrcc_kaanapali_clk_ops,
	.priv_auto	= sizeof(struct msm_clk_priv),
	.probe		= tcsrcc_kaanapali_clk_probe,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
