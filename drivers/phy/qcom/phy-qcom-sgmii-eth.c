// SPDX-License-Identifier: GPL-2.0-only
/*
 * Qualcomm DWMAC SGMII SerDes PHY driver
 *
 * Based on the Linux driver drivers/phy/qualcomm/phy-qcom-sgmii-eth.c
 * Copyright (c) 2023, Linaro Limited
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <phy_interface.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#include "phy-qcom-qmp-qserdes-com-v5.h"
#include "phy-qcom-qmp-qserdes-txrx-v5.h"

#define QSERDES_QMP_PLL					0x0
#define QSERDES_RX					0x600
#define QSERDES_TX					0x400
#define QSERDES_PCS					0xc00

#define QPHY_PCS_PHY_START				0x000
#define QPHY_PCS_POWER_DOWN_CONTROL			0x004
#define QPHY_PCS_SW_RESET				0x008
#define QPHY_PCS_LINE_RESET_TIME			0x00c
#define QPHY_PCS_TX_LARGE_AMP_DRV_LVL			0x020
#define QPHY_PCS_TX_SMALL_AMP_DRV_LVL			0x028
#define QPHY_PCS_PCS_READY_STATUS			0x094
#define QPHY_PCS_TX_MID_TERM_CTRL1			0x0d8
#define QPHY_PCS_TX_MID_TERM_CTRL2			0x0dc
#define QPHY_PCS_SGMII_MISC_CTRL8			0x118

#define QSERDES_COM_C_READY				BIT(0)
#define QSERDES_PCS_READY				BIT(0)
#define QSERDES_PCS_SGMIIPHY_READY			BIT(7)
#define QSERDES_COM_C_PLL_LOCKED			BIT(1)

struct qcom_sgmii_reg {
	u16 offset;
	u8 val;
};

struct qcom_sgmii_phy {
	void __iomem *base;
	struct clk refclk;
	phy_interface_t interface;
	bool powered;
};

/* 1.25 Gbps: SGMII and 1000BASE-X */
static const struct qcom_sgmii_reg qcom_sgmii_init_1g[] = {
	{ QSERDES_PCS + QPHY_PCS_SW_RESET, 0x01 },
	{ QSERDES_PCS + QPHY_PCS_POWER_DOWN_CONTROL, 0x01 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_PLL_IVCO, 0x0f },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_CP_CTRL_MODE0, 0x06 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_PLL_RCTRL_MODE0, 0x16 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_PLL_CCTRL_MODE0, 0x36 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_SYSCLK_EN_SEL, 0x1a },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_LOCK_CMP1_MODE0, 0x0a },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_LOCK_CMP2_MODE0, 0x1a },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_DEC_START_MODE0, 0x82 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_DIV_FRAC_START1_MODE0, 0x55 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_DIV_FRAC_START2_MODE0, 0x55 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_DIV_FRAC_START3_MODE0, 0x03 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_VCO_TUNE1_MODE0, 0x24 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_VCO_TUNE2_MODE0, 0x02 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_VCO_TUNE_INITVAL2, 0x00 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_HSCLK_SEL, 0x04 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_HSCLK_HS_SWITCH_SEL, 0x00 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_CORECLK_DIV_MODE0, 0x0a },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_CORE_CLK_EN, 0x00 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_BIN_VCOCAL_CMP_CODE1_MODE0, 0xb9 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_BIN_VCOCAL_CMP_CODE2_MODE0, 0x1e },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_BIN_VCOCAL_HSCLK_SEL, 0x11 },
	{ QSERDES_TX + QSERDES_V5_TX_TX_BAND, 0x05 },
	{ QSERDES_TX + QSERDES_V5_TX_SLEW_CNTL, 0x0a },
	{ QSERDES_TX + QSERDES_V5_TX_RES_CODE_LANE_OFFSET_TX, 0x09 },
	{ QSERDES_TX + QSERDES_V5_TX_RES_CODE_LANE_OFFSET_RX, 0x09 },
	{ QSERDES_TX + QSERDES_V5_TX_LANE_MODE_1, 0x05 },
	{ QSERDES_TX + QSERDES_V5_TX_LANE_MODE_3, 0x00 },
	{ QSERDES_TX + QSERDES_V5_TX_RCV_DETECT_LVL_2, 0x12 },
	{ QSERDES_TX + QSERDES_V5_TX_TRAN_DRVR_EMP_EN, 0x0c },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_FO_GAIN, 0x0a },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_SO_GAIN, 0x06 },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_FASTLOCK_FO_GAIN, 0x0a },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_SO_SATURATION_AND_ENABLE, 0x7f },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_FASTLOCK_COUNT_LOW, 0x00 },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_FASTLOCK_COUNT_HIGH, 0x01 },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_PI_CONTROLS, 0x81 },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_PI_CTRL2, 0x80 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_TERM_BW, 0x04 },
	{ QSERDES_RX + QSERDES_V5_RX_VGA_CAL_CNTRL2, 0x08 },
	{ QSERDES_RX + QSERDES_V5_RX_GM_CAL, 0x0f },
	{ QSERDES_RX + QSERDES_V5_RX_RX_EQU_ADAPTOR_CNTRL1, 0x04 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_EQU_ADAPTOR_CNTRL2, 0x00 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_EQU_ADAPTOR_CNTRL3, 0x4a },
	{ QSERDES_RX + QSERDES_V5_RX_RX_EQU_ADAPTOR_CNTRL4, 0x0a },
	{ QSERDES_RX + QSERDES_V5_RX_RX_IDAC_TSETTLE_LOW, 0x80 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_IDAC_TSETTLE_HIGH, 0x01 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_IDAC_MEASURE_TIME, 0x20 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_EQ_OFFSET_ADAPTOR_CNTRL1, 0x17 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_OFFSET_ADAPTOR_CNTRL2, 0x00 },
	{ QSERDES_RX + QSERDES_V5_RX_SIGDET_CNTRL, 0x0f },
	{ QSERDES_RX + QSERDES_V5_RX_SIGDET_DEGLITCH_CNTRL, 0x1e },
	{ QSERDES_RX + QSERDES_V5_RX_RX_BAND, 0x05 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_00_LOW, 0xe0 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_00_HIGH, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_00_HIGH2, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_00_HIGH3, 0x09 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_00_HIGH4, 0xb1 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_01_LOW, 0xe0 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_01_HIGH, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_01_HIGH2, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_01_HIGH3, 0x09 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_01_HIGH4, 0xb1 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_10_LOW, 0xe0 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_10_HIGH, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_10_HIGH2, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_10_HIGH3, 0x3b },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_10_HIGH4, 0xb7 },
	{ QSERDES_RX + QSERDES_V5_RX_DCC_CTRL1, 0x0c },
	{ QSERDES_PCS + QPHY_PCS_LINE_RESET_TIME, 0x0c },
	{ QSERDES_PCS + QPHY_PCS_TX_LARGE_AMP_DRV_LVL, 0x1f },
	{ QSERDES_PCS + QPHY_PCS_TX_SMALL_AMP_DRV_LVL, 0x03 },
	{ QSERDES_PCS + QPHY_PCS_TX_MID_TERM_CTRL1, 0x83 },
	{ QSERDES_PCS + QPHY_PCS_TX_MID_TERM_CTRL2, 0x08 },
	{ QSERDES_PCS + QPHY_PCS_SGMII_MISC_CTRL8, 0x0c },
	{ QSERDES_PCS + QPHY_PCS_SW_RESET, 0x00 },
	{ QSERDES_PCS + QPHY_PCS_PHY_START, 0x01 },
};

/* 3.125 Gbps: 2500BASE-X */
static const struct qcom_sgmii_reg qcom_sgmii_init_2p5g[] = {
	{ QSERDES_PCS + QPHY_PCS_SW_RESET, 0x01 },
	{ QSERDES_PCS + QPHY_PCS_POWER_DOWN_CONTROL, 0x01 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_PLL_IVCO, 0x0f },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_CP_CTRL_MODE0, 0x06 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_PLL_RCTRL_MODE0, 0x16 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_PLL_CCTRL_MODE0, 0x36 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_SYSCLK_EN_SEL, 0x1a },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_LOCK_CMP1_MODE0, 0x1a },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_LOCK_CMP2_MODE0, 0x41 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_DEC_START_MODE0, 0x7a },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_DIV_FRAC_START1_MODE0, 0x00 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_DIV_FRAC_START2_MODE0, 0x20 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_DIV_FRAC_START3_MODE0, 0x01 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_VCO_TUNE1_MODE0, 0xa1 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_VCO_TUNE2_MODE0, 0x02 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_VCO_TUNE_INITVAL2, 0x00 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_HSCLK_SEL, 0x03 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_HSCLK_HS_SWITCH_SEL, 0x00 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_CORECLK_DIV_MODE0, 0x05 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_CORE_CLK_EN, 0x00 },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_BIN_VCOCAL_CMP_CODE1_MODE0, 0xcd },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_BIN_VCOCAL_CMP_CODE2_MODE0, 0x1c },
	{ QSERDES_QMP_PLL + QSERDES_V5_COM_BIN_VCOCAL_HSCLK_SEL, 0x11 },
	{ QSERDES_TX + QSERDES_V5_TX_TX_BAND, 0x04 },
	{ QSERDES_TX + QSERDES_V5_TX_SLEW_CNTL, 0x0a },
	{ QSERDES_TX + QSERDES_V5_TX_RES_CODE_LANE_OFFSET_TX, 0x09 },
	{ QSERDES_TX + QSERDES_V5_TX_RES_CODE_LANE_OFFSET_RX, 0x02 },
	{ QSERDES_TX + QSERDES_V5_TX_LANE_MODE_1, 0x05 },
	{ QSERDES_TX + QSERDES_V5_TX_LANE_MODE_3, 0x00 },
	{ QSERDES_TX + QSERDES_V5_TX_RCV_DETECT_LVL_2, 0x12 },
	{ QSERDES_TX + QSERDES_V5_TX_TRAN_DRVR_EMP_EN, 0x0c },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_FO_GAIN, 0x0a },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_SO_GAIN, 0x06 },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_FASTLOCK_FO_GAIN, 0x0a },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_SO_SATURATION_AND_ENABLE, 0x7f },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_FASTLOCK_COUNT_LOW, 0x00 },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_FASTLOCK_COUNT_HIGH, 0x01 },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_PI_CONTROLS, 0x81 },
	{ QSERDES_RX + QSERDES_V5_RX_UCDR_PI_CTRL2, 0x80 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_TERM_BW, 0x00 },
	{ QSERDES_RX + QSERDES_V5_RX_VGA_CAL_CNTRL2, 0x08 },
	{ QSERDES_RX + QSERDES_V5_RX_GM_CAL, 0x0f },
	{ QSERDES_RX + QSERDES_V5_RX_RX_EQU_ADAPTOR_CNTRL1, 0x04 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_EQU_ADAPTOR_CNTRL2, 0x00 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_EQU_ADAPTOR_CNTRL3, 0x4a },
	{ QSERDES_RX + QSERDES_V5_RX_RX_EQU_ADAPTOR_CNTRL4, 0x0a },
	{ QSERDES_RX + QSERDES_V5_RX_RX_IDAC_TSETTLE_LOW, 0x80 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_IDAC_TSETTLE_HIGH, 0x01 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_IDAC_MEASURE_TIME, 0x20 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_EQ_OFFSET_ADAPTOR_CNTRL1, 0x17 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_OFFSET_ADAPTOR_CNTRL2, 0x00 },
	{ QSERDES_RX + QSERDES_V5_RX_SIGDET_CNTRL, 0x0f },
	{ QSERDES_RX + QSERDES_V5_RX_SIGDET_DEGLITCH_CNTRL, 0x1e },
	{ QSERDES_RX + QSERDES_V5_RX_RX_BAND, 0x18 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_00_LOW, 0x18 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_00_HIGH, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_00_HIGH2, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_00_HIGH3, 0x0c },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_00_HIGH4, 0xb8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_01_LOW, 0xe0 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_01_HIGH, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_01_HIGH2, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_01_HIGH3, 0x09 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_01_HIGH4, 0xb1 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_10_LOW, 0xe0 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_10_HIGH, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_10_HIGH2, 0xc8 },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_10_HIGH3, 0x3b },
	{ QSERDES_RX + QSERDES_V5_RX_RX_MODE_10_HIGH4, 0xb7 },
	{ QSERDES_RX + QSERDES_V5_RX_DCC_CTRL1, 0x0c },
	{ QSERDES_PCS + QPHY_PCS_LINE_RESET_TIME, 0x0c },
	{ QSERDES_PCS + QPHY_PCS_TX_LARGE_AMP_DRV_LVL, 0x1f },
	{ QSERDES_PCS + QPHY_PCS_TX_SMALL_AMP_DRV_LVL, 0x03 },
	{ QSERDES_PCS + QPHY_PCS_TX_MID_TERM_CTRL1, 0x83 },
	{ QSERDES_PCS + QPHY_PCS_TX_MID_TERM_CTRL2, 0x08 },
	{ QSERDES_PCS + QPHY_PCS_SGMII_MISC_CTRL8, 0x8c },
	{ QSERDES_PCS + QPHY_PCS_SW_RESET, 0x00 },
	{ QSERDES_PCS + QPHY_PCS_PHY_START, 0x01 },
};

static int qcom_sgmii_poll(struct qcom_sgmii_phy *priv, unsigned int reg,
			   u32 bit)
{
	u32 val;

	return readl_poll_timeout(priv->base + reg, val, val & bit, 750000);
}

static int qcom_sgmii_phy_calibrate(struct phy *phy)
{
	struct qcom_sgmii_phy *priv = dev_get_priv(phy->dev);
	const struct qcom_sgmii_reg *tbl;
	unsigned int i, n;

	switch (priv->interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
		tbl = qcom_sgmii_init_1g;
		n = ARRAY_SIZE(qcom_sgmii_init_1g);
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		tbl = qcom_sgmii_init_2p5g;
		n = ARRAY_SIZE(qcom_sgmii_init_2p5g);
		break;
	default:
		return -EINVAL;
	}

	for (i = 0; i < n; i++)
		writel(tbl[i].val, priv->base + tbl[i].offset);

	if (qcom_sgmii_poll(priv, QSERDES_QMP_PLL + QSERDES_V5_COM_C_READY_STATUS,
			    QSERDES_COM_C_READY)) {
		dev_err(phy->dev, "QSERDES_COM_C_READY_STATUS timed out\n");
		return -ETIMEDOUT;
	}

	if (qcom_sgmii_poll(priv, QSERDES_PCS + QPHY_PCS_PCS_READY_STATUS,
			    QSERDES_PCS_READY)) {
		dev_err(phy->dev, "PCS_READY timed out\n");
		return -ETIMEDOUT;
	}

	if (qcom_sgmii_poll(priv, QSERDES_PCS + QPHY_PCS_PCS_READY_STATUS,
			    QSERDES_PCS_SGMIIPHY_READY)) {
		dev_err(phy->dev, "SGMIIPHY_READY timed out\n");
		return -ETIMEDOUT;
	}

	if (qcom_sgmii_poll(priv, QSERDES_QMP_PLL + QSERDES_V5_COM_CMN_STATUS,
			    QSERDES_COM_C_PLL_LOCKED)) {
		dev_err(phy->dev, "PLL lock timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int qcom_sgmii_phy_power_on(struct phy *phy)
{
	struct qcom_sgmii_phy *priv = dev_get_priv(phy->dev);
	int ret;

	ret = clk_enable(&priv->refclk);
	if (ret)
		return ret;

	ret = qcom_sgmii_phy_calibrate(phy);
	if (ret) {
		clk_disable(&priv->refclk);
		return ret;
	}

	priv->powered = true;

	return 0;
}

static int qcom_sgmii_phy_power_off(struct phy *phy)
{
	struct qcom_sgmii_phy *priv = dev_get_priv(phy->dev);

	writel(0x08, priv->base + QSERDES_PCS + QPHY_PCS_TX_MID_TERM_CTRL2);
	writel(0x01, priv->base + QSERDES_PCS + QPHY_PCS_SW_RESET);
	udelay(100);
	writel(0x00, priv->base + QSERDES_PCS + QPHY_PCS_SW_RESET);
	writel(0x01, priv->base + QSERDES_PCS + QPHY_PCS_PHY_START);

	clk_disable(&priv->refclk);
	priv->powered = false;

	return 0;
}

static int qcom_sgmii_phy_set_mode(struct phy *phy, enum phy_mode mode,
				   int submode)
{
	struct qcom_sgmii_phy *priv = dev_get_priv(phy->dev);

	if (mode != PHY_MODE_ETHERNET)
		return -EINVAL;

	switch (submode) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
	case PHY_INTERFACE_MODE_2500BASEX:
		break;
	default:
		return -EINVAL;
	}

	priv->interface = submode;

	if (!priv->powered)
		return 0;

	return qcom_sgmii_phy_calibrate(phy);
}

static const struct phy_ops qcom_sgmii_phy_ops = {
	.power_on = qcom_sgmii_phy_power_on,
	.power_off = qcom_sgmii_phy_power_off,
	.set_mode = qcom_sgmii_phy_set_mode,
};

static int qcom_sgmii_phy_probe(struct udevice *dev)
{
	struct qcom_sgmii_phy *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_name(dev, "sgmi_ref", &priv->refclk);
	if (ret)
		return ret;

	priv->interface = PHY_INTERFACE_MODE_SGMII;

	return 0;
}

static const struct udevice_id qcom_sgmii_phy_ids[] = {
	{ .compatible = "qcom,sa8775p-dwmac-sgmii-phy" },
	{ }
};

U_BOOT_DRIVER(qcom_sgmii_eth_phy) = {
	.name = "qcom-sgmii-eth-phy",
	.id = UCLASS_PHY,
	.of_match = qcom_sgmii_phy_ids,
	.ops = &qcom_sgmii_phy_ops,
	.probe = qcom_sgmii_phy_probe,
	.priv_auto = sizeof(struct qcom_sgmii_phy),
};
