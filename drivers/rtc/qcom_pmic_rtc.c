// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm SPMI PMIC real-time clock
 *
 * The RTC is a 32-bit seconds counter in the PMIC. On most platforms the
 * counter is read-only to the application processor, so, as in the Linux
 * rtc-pm8xxx driver, the time is set by storing an offset to the counter:
 * in an nvmem cell ("offset") or in the UEFI variable RTCInfo
 * ("qcom,uefi-rtc-info"). The counter itself is written only when the
 * board declares "allow-set-time".
 */

#include <dm.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <log.h>
#include <nvmem.h>
#include <rtc.h>
#include <asm/unaligned.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <power/pmic.h>

#define RTC_WDATA0		0x40
#define RTC_EN_CTL		0x46
#define RTC_RDATA0		0x48
#define RTC_EN			BIT(7)

#define ALARM_EN_CTL		0x46
#define ALARM_EN		BIT(7)

#define RTC_NUM_BYTES		4

/* 1980-01-06, the GPS epoch the RTCInfo offset is relative to */
#define GPS_EPOCH		315964800U

struct qcom_pmic_rtc_data {
	u32 rtc_base;
	u32 alarm_base;
};

struct qcom_pmic_rtc_priv {
	struct udevice *pmic;
	u32 rtc_base;
	u32 alarm_base;
	bool allow_set_time;
	bool has_nvmem_offset;
	bool use_uefi;
	struct nvmem_cell offset_cell;
	u32 offset;
};

struct qcom_uefi_rtc_info {
	__le32 offset_gps;
	u8 reserved[8];
} __packed;

static const efi_guid_t qcom_uefi_rtc_info_guid =
	EFI_GUID(0x882f8c2b, 0x9646, 0x435f,
		 0x8d, 0xe5, 0xf2, 0x08, 0xff, 0x80, 0xc1, 0xbd);

static int qcom_pmic_rtc_read_bytes(struct qcom_pmic_rtc_priv *priv, u8 *val)
{
	int i, ret;

	for (i = 0; i < RTC_NUM_BYTES; i++) {
		ret = pmic_reg_read(priv->pmic, priv->rtc_base + RTC_RDATA0 + i);
		if (ret < 0)
			return ret;
		val[i] = ret;
	}

	return 0;
}

static int qcom_pmic_rtc_read_raw(struct qcom_pmic_rtc_priv *priv, u32 *secs)
{
	u8 val[RTC_NUM_BYTES];
	int ret;

	ret = qcom_pmic_rtc_read_bytes(priv, val);
	if (ret)
		return ret;

	/*
	 * The bytes are read one SPMI transfer at a time; a carry out of
	 * the low byte in between shows as a smaller low byte on a second
	 * read, and then the whole counter is read again.
	 */
	ret = pmic_reg_read(priv->pmic, priv->rtc_base + RTC_RDATA0);
	if (ret < 0)
		return ret;
	if (ret < val[0]) {
		ret = qcom_pmic_rtc_read_bytes(priv, val);
		if (ret)
			return ret;
	}

	*secs = get_unaligned_le32(val);

	return 0;
}

static int qcom_pmic_rtc_write_raw(struct qcom_pmic_rtc_priv *priv, u32 secs)
{
	u8 val[RTC_NUM_BYTES];
	int alarm_ctl, ret, i;

	put_unaligned_le32(secs, val);

	alarm_ctl = pmic_reg_read(priv->pmic, priv->alarm_base + ALARM_EN_CTL);
	if (alarm_ctl < 0)
		return alarm_ctl;

	if (alarm_ctl & ALARM_EN) {
		ret = pmic_clrsetbits(priv->pmic, priv->alarm_base + ALARM_EN_CTL,
				      ALARM_EN, 0);
		if (ret)
			return ret;
	}

	ret = pmic_clrsetbits(priv->pmic, priv->rtc_base + RTC_EN_CTL, RTC_EN, 0);
	if (ret)
		return ret;

	/*
	 * Write the bytes in ascending order: on PMK8350 a write to WDATA0
	 * after WDATA1 discards WDATA1.
	 */
	for (i = 0; i < RTC_NUM_BYTES; i++) {
		ret = pmic_reg_write(priv->pmic, priv->rtc_base + RTC_WDATA0 + i,
				     val[i]);
		if (ret)
			return ret;
	}

	ret = pmic_clrsetbits(priv->pmic, priv->rtc_base + RTC_EN_CTL, 0, RTC_EN);
	if (ret)
		return ret;

	if (alarm_ctl & ALARM_EN)
		return pmic_clrsetbits(priv->pmic,
				       priv->alarm_base + ALARM_EN_CTL, 0,
				       ALARM_EN);

	return 0;
}

static int qcom_pmic_rtc_write_uefi_offset(u32 offset)
{
	struct qcom_uefi_rtc_info info = {
		.offset_gps = cpu_to_le32(offset - GPS_EPOCH),
	};
	efi_status_t ret;

	ret = efi_set_variable_int(u"RTCInfo", &qcom_uefi_rtc_info_guid,
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   sizeof(info), &info, false);

	return ret == EFI_SUCCESS ? 0 : -EIO;
}

/*
 * The UEFI variable store is loaded with the UEFI subsystem; before that
 * the offset is unknown and -EAGAIN is returned. A missing RTCInfo is
 * created with a zero offset: Linux fails the RTC probe without it.
 */
static int qcom_pmic_rtc_read_uefi_offset(u32 *offset)
{
	struct qcom_uefi_rtc_info info;
	efi_uintn_t size = sizeof(info);
	efi_status_t ret;

	if (!CONFIG_IS_ENABLED(EFI_LOADER) ||
	    efi_obj_list_initialized != EFI_SUCCESS)
		return -EAGAIN;

	ret = efi_get_variable_int(u"RTCInfo", &qcom_uefi_rtc_info_guid, NULL,
				   &size, &info, NULL);
	if (ret == EFI_NOT_FOUND) {
		*offset = 0;
		return qcom_pmic_rtc_write_uefi_offset(0);
	}
	if (ret != EFI_SUCCESS || size != sizeof(info))
		return -EIO;

	*offset = le32_to_cpu(info.offset_gps) + GPS_EPOCH;

	return 0;
}

static int qcom_pmic_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	struct qcom_pmic_rtc_priv *priv = dev_get_priv(dev);
	u32 secs, offset = priv->offset;
	int ret;

	if (priv->use_uefi) {
		ret = qcom_pmic_rtc_read_uefi_offset(&offset);
		if (ret)
			return ret;
	}

	ret = qcom_pmic_rtc_read_raw(priv, &secs);
	if (ret)
		return ret;

	rtc_to_tm((u64)(u32)(secs + offset), tm);

	return 0;
}

static int qcom_pmic_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	struct qcom_pmic_rtc_priv *priv = dev_get_priv(dev);
	time64_t time = rtc_mktime(tm);
	u8 buf[sizeof(u32)];
	u32 raw, offset;
	int ret;

	if (time < 0 || time > U32_MAX)
		return -EINVAL;

	if (priv->allow_set_time)
		return qcom_pmic_rtc_write_raw(priv, time);

	if (!priv->has_nvmem_offset && !priv->use_uefi)
		return -EPERM;

	ret = qcom_pmic_rtc_read_raw(priv, &raw);
	if (ret)
		return ret;

	offset = (u32)time - raw;

	if (priv->use_uefi) {
		if (!CONFIG_IS_ENABLED(EFI_LOADER) ||
		    efi_obj_list_initialized != EFI_SUCCESS)
			return -EAGAIN;
		return qcom_pmic_rtc_write_uefi_offset(offset);
	}

	if (offset == priv->offset)
		return 0;

	put_unaligned_le32(offset, buf);
	ret = nvmem_cell_write(&priv->offset_cell, buf, sizeof(buf));
	if (ret)
		return ret;

	priv->offset = offset;

	return 0;
}

static int qcom_pmic_rtc_probe(struct udevice *dev)
{
	const struct qcom_pmic_rtc_data *data =
		(const struct qcom_pmic_rtc_data *)dev_get_driver_data(dev);
	struct qcom_pmic_rtc_priv *priv = dev_get_priv(dev);
	u8 buf[sizeof(u32)];
	int ret;

	priv->pmic = dev_get_parent(dev);
	priv->rtc_base = data->rtc_base;
	priv->alarm_base = data->alarm_base;

	priv->allow_set_time = dev_read_bool(dev, "allow-set-time");

	if (!priv->allow_set_time &&
	    dev_read_stringlist_search(dev, "nvmem-cell-names", "offset") >= 0) {
		ret = nvmem_cell_get_by_name(dev, "offset", &priv->offset_cell);
		if (ret)
			return ret;

		ret = nvmem_cell_read(&priv->offset_cell, buf, sizeof(buf));
		if (ret)
			return ret;

		priv->offset = get_unaligned_le32(buf);
		priv->has_nvmem_offset = true;
	} else if (!priv->allow_set_time) {
		priv->use_uefi = dev_read_bool(dev, "qcom,uefi-rtc-info");
	}

	ret = pmic_reg_read(priv->pmic, priv->rtc_base + RTC_EN_CTL);
	if (ret < 0)
		return ret;

	if (!(ret & RTC_EN))
		return pmic_clrsetbits(priv->pmic, priv->rtc_base + RTC_EN_CTL,
				       0, RTC_EN);

	return 0;
}

static const struct rtc_ops qcom_pmic_rtc_ops = {
	.get = qcom_pmic_rtc_get,
	.set = qcom_pmic_rtc_set,
};

static const struct qcom_pmic_rtc_data pm8941_data = {
	.rtc_base = 0x6000,
	.alarm_base = 0x6100,
};

static const struct qcom_pmic_rtc_data pmk8350_data = {
	.rtc_base = 0x6100,
	.alarm_base = 0x6200,
};

static const struct udevice_id qcom_pmic_rtc_ids[] = {
	{ .compatible = "qcom,pm8941-rtc", .data = (ulong)&pm8941_data },
	{ .compatible = "qcom,pmk8350-rtc", .data = (ulong)&pmk8350_data },
	{ }
};

U_BOOT_DRIVER(qcom_pmic_rtc) = {
	.name = "qcom_pmic_rtc",
	.id = UCLASS_RTC,
	.of_match = qcom_pmic_rtc_ids,
	.probe = qcom_pmic_rtc_probe,
	.priv_auto = sizeof(struct qcom_pmic_rtc_priv),
	.ops = &qcom_pmic_rtc_ops,
};
