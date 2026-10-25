// SPDX-License-Identifier: GPL-2.0
/*
 * Qualcomm PMIC eUSB2 repeater
 *
 * Based on the Linux driver:
 * Copyright (c) 2023, Linaro Limited
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define EUSB2_RPTR_STATUS		0x08
#define RPTR_OK				BIT(7)

#define EUSB2_EN_CTL1			0x46
#define EUSB2_RPTR_EN			BIT(7)

#define EUSB2_TUNE_IUSB2		0x51
#define EUSB2_TUNE_RES_FSDIF		0x52
#define EUSB2_TUNE_HSDISC		0x53
#define EUSB2_TUNE_SQUELCH_U		0x54
#define EUSB2_TUNE_USB2_PREEM		0x57

#define EUSB2_RPTR_TIMEOUT_US		10

static const int squelch_detector[] = {
	[0] = -6000,
	[1] = -5000,
	[2] = -4000,
	[3] = -3000,
	[4] = -2000,
	[5] = -1000,
	[6] = 0,
	[7] = 1000,
};

struct eusb2_repeater_init_tbl_reg {
	unsigned int reg;
	unsigned int value;
};

struct eusb2_repeater_cfg {
	const struct eusb2_repeater_init_tbl_reg *init_tbl;
	int init_tbl_num;
};

struct eusb2_repeater_priv {
	unsigned int base;
	struct udevice *vdd18;
	struct udevice *vdd3;
};

static const struct eusb2_repeater_init_tbl_reg pm8550b_init_tbl[] = {
	{ EUSB2_TUNE_IUSB2, 0x8 },
	{ EUSB2_TUNE_SQUELCH_U, 0x3 },
	{ EUSB2_TUNE_USB2_PREEM, 0x5 },
};

static const struct eusb2_repeater_cfg pm8550b_eusb2_cfg = {
	.init_tbl = pm8550b_init_tbl,
	.init_tbl_num = ARRAY_SIZE(pm8550b_init_tbl),
};

static int eusb2_repeater_write(struct udevice *dev, unsigned int reg, u8 val)
{
	struct eusb2_repeater_priv *priv = dev_get_priv(dev);

	return pmic_reg_write(dev->parent, priv->base + reg, val);
}

/* Board tuning from the device tree overrides the PMIC defaults. */
static int eusb2_repeater_tune(struct udevice *dev)
{
	static const struct {
		const char *prop;
		unsigned int reg;
	} tunes[] = {
		{ "qcom,tune-usb2-preem", EUSB2_TUNE_USB2_PREEM },
		{ "qcom,tune-usb2-disc-thres", EUSB2_TUNE_HSDISC },
		{ "qcom,tune-usb2-amplitude", EUSB2_TUNE_IUSB2 },
		{ "qcom,tune-res-fsdif", EUSB2_TUNE_RES_FSDIF },
	};
	s32 squelch;
	u8 val;
	int ret, i;

	for (i = 0; i < ARRAY_SIZE(tunes); i++) {
		if (dev_read_u8(dev, tunes[i].prop, &val))
			continue;
		ret = eusb2_repeater_write(dev, tunes[i].reg, val);
		if (ret)
			return ret;
	}

	if (dev_read_s32(dev, "qcom,squelch-detector-bp", &squelch))
		return 0;

	for (i = 0; i < ARRAY_SIZE(squelch_detector); i++) {
		if (squelch_detector[i] == squelch)
			return eusb2_repeater_write(dev, EUSB2_TUNE_SQUELCH_U, i);
	}

	return 0;
}

static int eusb2_repeater_init(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct eusb2_repeater_priv *priv = dev_get_priv(dev);
	const struct eusb2_repeater_cfg *cfg =
		(const struct eusb2_repeater_cfg *)dev_get_driver_data(dev);
	int ret, val, i;

	ret = regulator_set_enable_if_allowed(priv->vdd18, true);
	if (ret)
		return ret;

	ret = regulator_set_enable_if_allowed(priv->vdd3, true);
	if (ret)
		return ret;

	ret = eusb2_repeater_write(dev, EUSB2_EN_CTL1, EUSB2_RPTR_EN);
	if (ret)
		return ret;

	for (i = 0; i < cfg->init_tbl_num; i++) {
		ret = eusb2_repeater_write(dev, cfg->init_tbl[i].reg,
					   cfg->init_tbl[i].value);
		if (ret)
			return ret;
	}

	ret = eusb2_repeater_tune(dev);
	if (ret)
		return ret;

	ret = read_poll_timeout(pmic_reg_read, val, val < 0 || (val & RPTR_OK),
				1, EUSB2_RPTR_TIMEOUT_US, dev->parent,
				priv->base + EUSB2_RPTR_STATUS);
	if (val < 0)
		return val;
	if (ret)
		dev_err(dev, "initialization timed out\n");

	return ret;
}

static int eusb2_repeater_exit(struct phy *phy)
{
	struct eusb2_repeater_priv *priv = dev_get_priv(phy->dev);

	regulator_set_enable_if_allowed(priv->vdd3, false);

	return regulator_set_enable_if_allowed(priv->vdd18, false);
}

static int eusb2_repeater_probe(struct udevice *dev)
{
	struct eusb2_repeater_priv *priv = dev_get_priv(dev);
	fdt_addr_t base;
	int ret;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;
	priv->base = base;

	ret = device_get_supply_regulator(dev, "vdd18-supply", &priv->vdd18);
	if (ret && ret != -ENOENT)
		return ret;

	ret = device_get_supply_regulator(dev, "vdd3-supply", &priv->vdd3);
	if (ret && ret != -ENOENT)
		return ret;

	return 0;
}

static const struct phy_ops eusb2_repeater_ops = {
	.init = eusb2_repeater_init,
	.exit = eusb2_repeater_exit,
};

static const struct udevice_id eusb2_repeater_ids[] = {
	{
		.compatible = "qcom,pm8550b-eusb2-repeater",
		.data = (ulong)&pm8550b_eusb2_cfg,
	},
	{ }
};

U_BOOT_DRIVER(qcom_eusb2_repeater) = {
	.name = "qcom-eusb2-repeater",
	.id = UCLASS_PHY,
	.of_match = eusb2_repeater_ids,
	.ops = &eusb2_repeater_ops,
	.probe = eusb2_repeater_probe,
	.priv_auto = sizeof(struct eusb2_repeater_priv),
};
