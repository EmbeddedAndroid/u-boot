// SPDX-License-Identifier: BSD-3-Clause
/*
 * Qualcomm SPMI bus driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * Loosely based on Little Kernel driver
 */

#include <dm.h>
#include <dm/lists.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/ofnode.h>
#include <spmi/spmi.h>

/* PMIC Arbiter configuration registers */
#define PMIC_ARB_VERSION 0x0000
#define PMIC_ARB_VERSION_V2_MIN 0x20010000
#define PMIC_ARB_VERSION_V3_MIN 0x30000000
#define PMIC_ARB_VERSION_V5_MIN 0x50000000
#define PMIC_ARB_VERSION_V7_MIN	0x70000000
#define PMIC_ARB_VERSION_V8_MIN	0x80000000

#define PMIC_ARB_FEATURES		0x0004
#define PMIC_ARB_FEATURES_PERIPH_MASK	GENMASK(10, 0)

#define APID_MAP_OFFSET_V1_V2_V3 (0x800)
#define APID_MAP_OFFSET_V5 (0x900)
#define APID_MAP_OFFSET_V7 (0x2000)
#define ARB_CHANNEL_OFFSET(n) (0x4 * (n))
#define SPMI_CH_OFFSET(chnl) ((chnl) * 0x8000)
#define SPMI_V5_OBS_CH_OFFSET(chnl) ((chnl) * 0x80)
#define SPMI_V7_OBS_CH_OFFSET(chnl) ((chnl) * 0x20)
#define SPMI_V5_RW_CH_OFFSET(chnl) ((chnl) * 0x10000)
#define SPMI_V7_RW_CH_OFFSET(chnl) ((chnl) * 0x1000)

#define SPMI_OWNERSHIP_PERIPH2OWNER(x)	((x) & 0x7)

#define SPMI_REG_CMD0 0x0
#define SPMI_REG_CONFIG 0x4
#define SPMI_REG_STATUS 0x8
#define SPMI_REG_WDATA 0x10
#define SPMI_REG_RDATA 0x18

#define SPMI_CMD_OPCODE_SHIFT 27
#define SPMI_CMD_SLAVE_ID_SHIFT 20
#define SPMI_CMD_ADDR_SHIFT 12
#define SPMI_CMD_ADDR_OFFSET_SHIFT 4
#define SPMI_CMD_BYTE_CNT_SHIFT 0

#define SPMI_CMD_EXT_REG_WRITE_LONG 0x00
#define SPMI_CMD_EXT_REG_READ_LONG 0x01

#define SPMI_STATUS_DONE 0x1

#define SPMI_MAX_CHANNELS 128
#define SPMI_MAX_CHANNELS_V5	512
#define SPMI_MAX_CHANNELS_V7	1024
#define SPMI_MAX_BUSES_V7	2
#define SPMI_MAX_SLAVES 16
#define SPMI_MAX_PERIPH 256

#define SPMI_CHANNEL_READ_ONLY	BIT(31)
#define SPMI_CHANNEL_VALID	BIT(30)
#define SPMI_CHANNEL_MASK	0xffff

enum arb_ver {
	V1 = 1,
	V2,
	V3,
	V5 = 5,
	V7 = 7
};

/*
 * PMIC arbiter version 5 uses different register offsets for read/write vs
 * observer channels.
 */
enum pmic_arb_channel {
	PMIC_ARB_CHANNEL_RW,
	PMIC_ARB_CHANNEL_OBS,
};

struct msm_spmi_priv {
	phys_addr_t arb_chnl;  /* ARB channel mapping base */
	phys_addr_t spmi_chnls; /* SPMI channels */
	phys_addr_t spmi_obs;  /* SPMI observer */
	phys_addr_t spmi_cnfg;  /* SPMI config */
	u32 owner;	/* Current owner */
	unsigned int base_apid; /* First APID of this bus */
	unsigned int max_channels; /* Number of APIDs of this bus */
	/* SPMI channel map */
	uint32_t channel_map[SPMI_MAX_SLAVES][SPMI_MAX_PERIPH];
	/* SPMI bus arbiter version */
	u32 arb_ver;
};

/* Arbiter resources shared by the buses of a multi-bus arbiter */
struct msm_spmi_arb_priv {
	phys_addr_t core;
	phys_addr_t spmi_chnls;
	phys_addr_t spmi_obs;
	u32 owner;
};

static u32 pmic_arb_fmt_cmd_v1(u8 opc, u8 sid, u8 pid, u8 off)
{
	return (opc << 27) | (sid << 20) | (pid << 12) | (off << 4) | 1;
}

static u32 pmic_arb_fmt_cmd_v2(u8 opc, u8 off)
{
	return (opc << 27) | (off << 4) | 1;
}

static int msm_spmi_write(struct udevice *dev, int usid, int pid, int off,
			  uint8_t val)
{
	struct msm_spmi_priv *priv = dev_get_priv(dev);
	unsigned channel;
	unsigned int ch_offset;
	uint32_t reg = 0;

	if (usid >= SPMI_MAX_SLAVES)
		return -EIO;
	if (pid >= SPMI_MAX_PERIPH)
		return -EIO;
	if (!(priv->channel_map[usid][pid] & SPMI_CHANNEL_VALID))
		return -EINVAL;
	if (priv->channel_map[usid][pid] & SPMI_CHANNEL_READ_ONLY)
		return -EPERM;

	channel = priv->channel_map[usid][pid] & SPMI_CHANNEL_MASK;

	debug("%s: [%d:%d] %s: channel %d\n", dev->name, usid, pid, __func__, channel);

	switch (priv->arb_ver) {
	case V1:
		ch_offset = SPMI_CH_OFFSET(channel);

		reg = pmic_arb_fmt_cmd_v1(SPMI_CMD_EXT_REG_WRITE_LONG,
					  usid, pid, off);
		break;

	case V2:
		ch_offset = SPMI_CH_OFFSET(channel);

		reg = pmic_arb_fmt_cmd_v2(SPMI_CMD_EXT_REG_WRITE_LONG, off);
		break;

	case V5:
		ch_offset = SPMI_V5_RW_CH_OFFSET(channel);

		reg = pmic_arb_fmt_cmd_v2(SPMI_CMD_EXT_REG_WRITE_LONG, off);
		break;

	case V7:
		ch_offset = SPMI_V7_RW_CH_OFFSET(channel);

		reg = pmic_arb_fmt_cmd_v2(SPMI_CMD_EXT_REG_WRITE_LONG, off);
		break;
	}

	/* Disable IRQ mode for the current channel*/
	writel(0x0, priv->spmi_chnls + ch_offset + SPMI_REG_CONFIG);

	/* Write single byte */
	writel(val, priv->spmi_chnls + ch_offset + SPMI_REG_WDATA);

	/* Send write command */
	writel(reg, priv->spmi_chnls + ch_offset + SPMI_REG_CMD0);

	/* Wait till CMD DONE status */
	reg = 0;
	while (!reg) {
		reg = readl(priv->spmi_chnls + ch_offset +
			    SPMI_REG_STATUS);
	}

	if (reg ^ SPMI_STATUS_DONE) {
		printf("SPMI write failure.\n");
		return -EIO;
	}

	return 0;
}

static int msm_spmi_read(struct udevice *dev, int usid, int pid, int off)
{
	struct msm_spmi_priv *priv = dev_get_priv(dev);
	unsigned channel;
	unsigned int ch_offset;
	uint32_t reg = 0;

	if (usid >= SPMI_MAX_SLAVES)
		return -EIO;
	if (pid >= SPMI_MAX_PERIPH)
		return -EIO;
	if (!(priv->channel_map[usid][pid] & SPMI_CHANNEL_VALID))
		return -EINVAL;

	channel = priv->channel_map[usid][pid] & SPMI_CHANNEL_MASK;

	debug("%s: [%d:%d] %s: channel %d\n", dev->name, usid, pid, __func__, channel);

	switch (priv->arb_ver) {
	case V1:
		ch_offset = SPMI_CH_OFFSET(channel);

		/* Prepare read command */
		reg = pmic_arb_fmt_cmd_v1(SPMI_CMD_EXT_REG_READ_LONG,
					  usid, pid, off);
		break;

	case V2:
		ch_offset = SPMI_CH_OFFSET(channel);

		/* Prepare read command */
		reg = pmic_arb_fmt_cmd_v2(SPMI_CMD_EXT_REG_READ_LONG, off);
		break;

	case V5:
		ch_offset = SPMI_V5_OBS_CH_OFFSET(channel);

		/* Prepare read command */
		reg = pmic_arb_fmt_cmd_v2(SPMI_CMD_EXT_REG_READ_LONG, off);
		break;

	case V7:
		ch_offset = SPMI_V7_OBS_CH_OFFSET(channel);

		/* Prepare read command */
		reg = pmic_arb_fmt_cmd_v2(SPMI_CMD_EXT_REG_READ_LONG, off);
		break;
	}

	/* Disable IRQ mode for the current channel*/
	writel(0x0, priv->spmi_obs + ch_offset + SPMI_REG_CONFIG);

	/* Request read */
	writel(reg, priv->spmi_obs + ch_offset + SPMI_REG_CMD0);

	/* Wait till CMD DONE status */
	reg = 0;
	while (!reg) {
		reg = readl(priv->spmi_obs + ch_offset + SPMI_REG_STATUS);
	}

	if (reg ^ SPMI_STATUS_DONE) {
		printf("SPMI read failure.\n");
		return -EIO;
	}

	/* Read the data */
	return readl(priv->spmi_obs + ch_offset +
				SPMI_REG_RDATA) & 0xFF;
}

static struct dm_spmi_ops msm_spmi_ops = {
	.read = msm_spmi_read,
	.write = msm_spmi_write,
};

/*
 * In order to allow multiple EEs to write to a single PPID in arbiter
 * version 5 and 7, there is more than one APID mapped to each PPID.
 * The owner field for each of these mappings specifies the EE which is
 * allowed to write to the APID.
 */
static void msm_spmi_channel_map_v5(struct msm_spmi_priv *priv, unsigned int i,
				    uint8_t slave_id, uint8_t pid)
{
	/* Mark channels read-only when from different owner */
	uint32_t cnfg = readl(priv->spmi_cnfg +
			      ARB_CHANNEL_OFFSET(i - priv->base_apid));
	uint8_t owner = SPMI_OWNERSHIP_PERIPH2OWNER(cnfg);
	bool prev_valid = priv->channel_map[slave_id][pid] & SPMI_CHANNEL_VALID;
	uint32_t prev_read_only = priv->channel_map[slave_id][pid] & SPMI_CHANNEL_READ_ONLY;

	if (!prev_valid) {
		/* First PPID mapping */
		priv->channel_map[slave_id][pid] = i | SPMI_CHANNEL_VALID;
		if (owner != priv->owner)
			priv->channel_map[slave_id][pid] |= SPMI_CHANNEL_READ_ONLY;

	} else if (owner == priv->owner) {
		/*
		 * Found a channel owned by our EE - ALWAYS switch to it!
		 * even if we already have a mapping, we must prefer the one
		 * owned by our EE to avoid hardware access violations.
		 */
		priv->channel_map[slave_id][pid] = i | SPMI_CHANNEL_VALID;
		/* Clear READ_ONLY flag since we own this channel */

	} else if (prev_read_only) {
		/*
		 * Previous mapping was read-only and this one is also not ours.
		 * Update to this channel.
		 */
		priv->channel_map[slave_id][pid] = i | SPMI_CHANNEL_VALID | SPMI_CHANNEL_READ_ONLY;

	}
	/* else: Previous was writable and owned by us, this one isn't - keep previous */
}

static void msm_spmi_scan_channels(struct msm_spmi_priv *priv)
{
	unsigned int i;

	for (i = priv->base_apid; i < priv->base_apid + priv->max_channels; i++) {
		uint32_t periph = readl(priv->arb_chnl + ARB_CHANNEL_OFFSET(i));
		uint8_t slave_id = (periph & 0xf0000) >> 16;
		uint8_t pid = (periph & 0xff00) >> 8;

		switch (priv->arb_ver) {
		case V2:
		case V3:
			priv->channel_map[slave_id][pid] = i | SPMI_CHANNEL_VALID;
			break;

		case V5:
		case V7:
			msm_spmi_channel_map_v5(priv, i, slave_id, pid);
			break;
		}
	}
}

static int msm_spmi_probe(struct udevice *dev)
{
	struct msm_spmi_priv *priv = dev_get_priv(dev);
	phys_addr_t core_addr;
	u32 hw_ver;

	core_addr = dev_read_addr_name(dev, "core");
	priv->spmi_chnls = dev_read_addr_name(dev, "chnls");
	priv->spmi_obs = dev_read_addr_name(dev, "obsrvr");
	dev_read_u32(dev, "qcom,ee", &priv->owner);

	hw_ver = readl(core_addr + PMIC_ARB_VERSION);

	if (hw_ver < PMIC_ARB_VERSION_V3_MIN) {
		priv->arb_ver = V2;
		priv->arb_chnl = core_addr + APID_MAP_OFFSET_V1_V2_V3;
		priv->max_channels = SPMI_MAX_CHANNELS;
	} else if (hw_ver < PMIC_ARB_VERSION_V5_MIN) {
		priv->arb_ver = V3;
		priv->arb_chnl = core_addr + APID_MAP_OFFSET_V1_V2_V3;
		priv->max_channels = SPMI_MAX_CHANNELS;
	} else if (hw_ver < PMIC_ARB_VERSION_V7_MIN) {
		priv->arb_ver = V5;
		priv->arb_chnl = core_addr + APID_MAP_OFFSET_V5;
		priv->max_channels = min_t(u32, readl(core_addr + PMIC_ARB_FEATURES) &
						PMIC_ARB_FEATURES_PERIPH_MASK,
					   SPMI_MAX_CHANNELS_V5);
		priv->spmi_cnfg = dev_read_addr_name(dev, "cnfg");
	} else {
		/* TOFIX: handle second bus */
		priv->arb_ver = V7;
		priv->arb_chnl = core_addr + APID_MAP_OFFSET_V7;
		priv->max_channels = min_t(u32, readl(core_addr + PMIC_ARB_FEATURES) &
						PMIC_ARB_FEATURES_PERIPH_MASK,
					   SPMI_MAX_CHANNELS_V7);
		priv->spmi_cnfg = dev_read_addr_name(dev, "cnfg");
	}

	dev_dbg(dev, "PMIC Arb Version-%d (%#x)\n", hw_ver >> 28, hw_ver);

	if (priv->arb_chnl == FDT_ADDR_T_NONE ||
	    priv->spmi_chnls == FDT_ADDR_T_NONE ||
	    priv->spmi_obs == FDT_ADDR_T_NONE)
		return -EINVAL;

	dev_dbg(dev, "priv->arb_chnl address (%#08llx)\n", priv->arb_chnl);
	dev_dbg(dev, "priv->spmi_chnls address (%#08llx)\n", priv->spmi_chnls);
	dev_dbg(dev, "priv->spmi_obs address (%#08llx)\n", priv->spmi_obs);
	/* Scan peripherals connected to each SPMI channel */
	msm_spmi_scan_channels(priv);

	return 0;
}

static const struct udevice_id msm_spmi_ids[] = {
	{ .compatible = "qcom,spmi-pmic-arb" },
	{ }
};

U_BOOT_DRIVER(msm_spmi) = {
	.name = "msm_spmi",
	.id = UCLASS_SPMI,
	.of_match = msm_spmi_ids,
	.ops = &msm_spmi_ops,
	.probe = msm_spmi_probe,
	.priv_auto = sizeof(struct msm_spmi_priv),
};

/*
 * Arbiter v7 can drive two SPMI buses. The binding then describes the
 * arbiter with one child node per bus; each bus owns a contiguous range
 * of APIDs whose size is given by a per-bus FEATURES register, and its
 * own APID ownership table in its "cnfg" region.
 */
static int msm_spmi_bus_index(struct udevice *dev)
{
	ofnode node;
	int index = 0;

	dev_for_each_subnode(node, dev->parent) {
		if (!ofnode_is_enabled(node))
			continue;
		if (ofnode_equal(node, dev_ofnode(dev)))
			return index;
		index++;
	}

	return -ENODEV;
}

static int msm_spmi_bus_probe(struct udevice *dev)
{
	struct msm_spmi_arb_priv *arb = dev_get_priv(dev->parent);
	struct msm_spmi_priv *priv = dev_get_priv(dev);
	unsigned int base_apid = 0, apid_count = 0;
	u32 hw_ver;
	int index, i;

	hw_ver = readl(arb->core + PMIC_ARB_VERSION);
	if (hw_ver < PMIC_ARB_VERSION_V7_MIN ||
	    hw_ver >= PMIC_ARB_VERSION_V8_MIN) {
		dev_err(dev, "unsupported PMIC arbiter version %#x\n", hw_ver);
		return -ENODEV;
	}

	index = msm_spmi_bus_index(dev);
	if (index < 0 || index >= SPMI_MAX_BUSES_V7)
		return -EINVAL;

	for (i = 0; i <= index; i++) {
		base_apid += apid_count;
		apid_count = readl(arb->core + PMIC_ARB_FEATURES + 4 * i) &
			     PMIC_ARB_FEATURES_PERIPH_MASK;
	}

	if (!apid_count || base_apid + apid_count > SPMI_MAX_CHANNELS_V7) {
		dev_err(dev, "invalid APID range %u+%u for bus %d\n",
			base_apid, apid_count, index);
		return -EINVAL;
	}

	priv->spmi_cnfg = dev_read_addr_name(dev, "cnfg");
	if (priv->spmi_cnfg == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->arb_ver = V7;
	priv->arb_chnl = arb->core + APID_MAP_OFFSET_V7;
	priv->spmi_chnls = arb->spmi_chnls;
	priv->spmi_obs = arb->spmi_obs;
	priv->owner = arb->owner;
	priv->base_apid = base_apid;
	priv->max_channels = apid_count;

	dev_dbg(dev, "bus %d: APIDs %u..%u\n", index, base_apid,
		base_apid + apid_count - 1);

	msm_spmi_scan_channels(priv);

	return 0;
}

U_BOOT_DRIVER(msm_spmi_bus) = {
	.name = "msm_spmi_bus",
	.id = UCLASS_SPMI,
	.ops = &msm_spmi_ops,
	.probe = msm_spmi_bus_probe,
	.priv_auto = sizeof(struct msm_spmi_priv),
};

static int msm_spmi_arb_bind(struct udevice *dev)
{
	ofnode node;
	int ret;

	dev_for_each_subnode(node, dev) {
		if (!ofnode_is_enabled(node))
			continue;

		ret = device_bind_driver_to_node(dev, "msm_spmi_bus",
						 ofnode_get_name(node), node,
						 NULL);
		if (ret)
			return ret;
	}

	return 0;
}

static int msm_spmi_arb_probe(struct udevice *dev)
{
	struct msm_spmi_arb_priv *priv = dev_get_priv(dev);

	priv->core = dev_read_addr_name(dev, "core");
	priv->spmi_chnls = dev_read_addr_name(dev, "chnls");
	priv->spmi_obs = dev_read_addr_name(dev, "obsrvr");
	dev_read_u32(dev, "qcom,ee", &priv->owner);

	if (priv->core == FDT_ADDR_T_NONE ||
	    priv->spmi_chnls == FDT_ADDR_T_NONE ||
	    priv->spmi_obs == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct udevice_id msm_spmi_arb_ids[] = {
	{ .compatible = "qcom,x1e80100-spmi-pmic-arb" },
	{ }
};

U_BOOT_DRIVER(msm_spmi_arb) = {
	.name = "msm_spmi_arb",
	.id = UCLASS_NOP,
	.of_match = msm_spmi_arb_ids,
	.bind = msm_spmi_arb_bind,
	.probe = msm_spmi_arb_probe,
	.priv_auto = sizeof(struct msm_spmi_arb_priv),
};

/*
 * PMIC arbiter v8 (Glymur): one arbiter node ("core", "chnls", "obsrvr",
 * "chnl_map") with a child node per SPMI bus ("cnfg", "intr", "chnl_owner").
 * The global APID space is split between the buses in order, each taking
 * the number of APIDs in its PMIC_ARB_FEATURES register. The APID map holds
 * the PPID (SID [12:8], peripheral [7:0]); read/write channels are 0x200
 * apart, observer channels 0x20 apart in a 0x40000 window per EE.
 */
#define PMIC_ARB_V8_FEATURES(n)		(0x0004 + 0x4 * (n))
#define PMIC_ARB_V8_PERIPH_MASK		GENMASK(12, 0)
#define PMIC_ARB_V8_PPID_SID(m)		(((m) >> 8) & 0x1f)
#define PMIC_ARB_V8_PPID_PID(m)		((m) & 0xff)
#define SPMI_V8_RW_CH_OFFSET(apid)	((apid) * 0x200)
#define SPMI_V8_OBS_CH_OFFSET(ee, apid)	((ee) * 0x40000 + (apid) * 0x20)
#define SPMI_V8_MAX_BUSES		4

struct msm_spmi_v8_arb {
	phys_addr_t core;
	phys_addr_t chnls;
	phys_addr_t obsrvr;
	phys_addr_t apid_map;
	u32 ee;
};

struct msm_spmi_v8_bus {
	phys_addr_t owner;
	/* APID per [SID][peripheral] | SPMI_CHANNEL_VALID [| READ_ONLY] */
	uint32_t apid[SPMI_MAX_SLAVES][SPMI_MAX_PERIPH];
};

static int msm_spmi_v8_xfer(struct udevice *dev, int usid, int pid, int off,
			    bool write, uint8_t val)
{
	struct msm_spmi_v8_arb *arb = dev_get_priv(dev_get_parent(dev));
	struct msm_spmi_v8_bus *bus = dev_get_priv(dev);
	phys_addr_t ch;
	uint32_t map, reg;
	unsigned int timeout = 100000;

	if (usid >= SPMI_MAX_SLAVES || pid >= SPMI_MAX_PERIPH)
		return -EIO;
	map = bus->apid[usid][pid];
	if (!(map & SPMI_CHANNEL_VALID))
		return -EINVAL;
	if (write && (map & SPMI_CHANNEL_READ_ONLY))
		return -EPERM;

	if (write)
		ch = arb->chnls + SPMI_V8_RW_CH_OFFSET(map & SPMI_CHANNEL_MASK);
	else
		ch = arb->obsrvr + SPMI_V8_OBS_CH_OFFSET(arb->ee, map & SPMI_CHANNEL_MASK);

	writel(0x0, ch + SPMI_REG_CONFIG);
	if (write)
		writel(val, ch + SPMI_REG_WDATA);
	writel(pmic_arb_fmt_cmd_v2(write ? SPMI_CMD_EXT_REG_WRITE_LONG :
				   SPMI_CMD_EXT_REG_READ_LONG, off),
	       ch + SPMI_REG_CMD0);

	do {
		reg = readl(ch + SPMI_REG_STATUS);
	} while (!reg && --timeout);

	if (reg != SPMI_STATUS_DONE) {
		dev_err(dev, "SPMI %s failure %#x [%d:%#x:%#x]\n",
			write ? "write" : "read", reg, usid, pid, off);
		return -EIO;
	}

	return write ? 0 : readl(ch + SPMI_REG_RDATA) & 0xff;
}

static int msm_spmi_v8_read(struct udevice *dev, int usid, int pid, int off)
{
	return msm_spmi_v8_xfer(dev, usid, pid, off, false, 0);
}

static int msm_spmi_v8_write(struct udevice *dev, int usid, int pid, int off,
			     uint8_t val)
{
	return msm_spmi_v8_xfer(dev, usid, pid, off, true, val);
}

static struct dm_spmi_ops msm_spmi_v8_ops = {
	.read = msm_spmi_v8_read,
	.write = msm_spmi_v8_write,
};

static int msm_spmi_v8_bus_probe(struct udevice *dev)
{
	struct msm_spmi_v8_arb *arb = dev_get_priv(dev_get_parent(dev));
	struct msm_spmi_v8_bus *bus = dev_get_priv(dev);
	ofnode node;
	unsigned int index = 0, base = 0, count, i;

	bus->owner = dev_read_addr_name(dev, "chnl_owner");
	if (bus->owner == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* This bus' position among the arbiter's buses gives its APID range. */
	dev_for_each_subnode(node, dev_get_parent(dev)) {
		if (ofnode_equal(node, dev_ofnode(dev)))
			break;
		index++;
	}
	if (index >= SPMI_V8_MAX_BUSES)
		return -EINVAL;

	for (i = 0; i < index; i++)
		base += readl(arb->core + PMIC_ARB_V8_FEATURES(i)) &
			PMIC_ARB_V8_PERIPH_MASK;
	count = readl(arb->core + PMIC_ARB_V8_FEATURES(index)) &
		PMIC_ARB_V8_PERIPH_MASK;

	for (i = base; i < base + count; i++) {
		uint32_t map = readl(arb->apid_map + 0x4 * i);
		uint32_t owner = SPMI_OWNERSHIP_PERIPH2OWNER(readl(bus->owner +
								   0x4 * (i - base)));
		uint8_t sid = PMIC_ARB_V8_PPID_SID(map);
		uint8_t pid = PMIC_ARB_V8_PPID_PID(map);
		uint32_t prev;

		if (!map || sid >= SPMI_MAX_SLAVES)
			continue;

		/*
		 * A PPID may have an APID per EE: keep the one this EE owns,
		 * else the last one, read only.
		 */
		prev = bus->apid[sid][pid];
		if ((prev & SPMI_CHANNEL_VALID) && !(prev & SPMI_CHANNEL_READ_ONLY))
			continue;
		bus->apid[sid][pid] = i | SPMI_CHANNEL_VALID;
		if (owner != arb->ee)
			bus->apid[sid][pid] |= SPMI_CHANNEL_READ_ONLY;
	}

	dev_dbg(dev, "bus %u: APIDs %u..%u\n", index, base, base + count - 1);

	return 0;
}

U_BOOT_DRIVER(msm_spmi_v8_bus) = {
	.name = "msm_spmi_v8_bus",
	.id = UCLASS_SPMI,
	.ops = &msm_spmi_v8_ops,
	.probe = msm_spmi_v8_bus_probe,
	.priv_auto = sizeof(struct msm_spmi_v8_bus),
};

static int msm_spmi_v8_arb_bind(struct udevice *dev)
{
	ofnode node;
	int ret;

	dev_for_each_subnode(node, dev) {
		ret = device_bind_driver_to_node(dev, "msm_spmi_v8_bus",
						 ofnode_get_name(node), node, NULL);
		if (ret)
			return ret;
	}

	return 0;
}

static int msm_spmi_v8_arb_probe(struct udevice *dev)
{
	struct msm_spmi_v8_arb *arb = dev_get_priv(dev);
	u32 hw_ver;

	arb->core = dev_read_addr_name(dev, "core");
	arb->chnls = dev_read_addr_name(dev, "chnls");
	arb->obsrvr = dev_read_addr_name(dev, "obsrvr");
	arb->apid_map = dev_read_addr_name(dev, "chnl_map");
	if (arb->core == FDT_ADDR_T_NONE || arb->chnls == FDT_ADDR_T_NONE ||
	    arb->obsrvr == FDT_ADDR_T_NONE || arb->apid_map == FDT_ADDR_T_NONE)
		return -EINVAL;
	dev_read_u32(dev, "qcom,ee", &arb->ee);

	hw_ver = readl(arb->core + PMIC_ARB_VERSION);
	if (hw_ver < PMIC_ARB_VERSION_V8_MIN) {
		dev_err(dev, "PMIC arbiter %#x is not v8\n", hw_ver);
		return -ENODEV;
	}

	return 0;
}

static const struct udevice_id msm_spmi_v8_ids[] = {
	{ .compatible = "qcom,glymur-spmi-pmic-arb" },
	{ }
};

U_BOOT_DRIVER(msm_spmi_v8_arb) = {
	.name = "msm_spmi_v8_arb",
	.id = UCLASS_NOP,
	.of_match = msm_spmi_v8_ids,
	.bind = msm_spmi_v8_arb_bind,
	.probe = msm_spmi_v8_arb_probe,
	.priv_auto = sizeof(struct msm_spmi_v8_arb),
};
