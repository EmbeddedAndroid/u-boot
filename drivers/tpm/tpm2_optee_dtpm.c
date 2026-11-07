// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Driver for a discrete TPM 2.0 that OP-TEE owns: the chip sits on a bus
 * that only the secure world can reach (for instance an SPI TPM on a serial
 * engine assigned to OP-TEE), and OP-TEE relays raw TPM commands to it
 * through its TPM passthrough pseudo TA.
 */

#include <dm.h>
#include <tpm-v2.h>
#include <tee.h>
#include <tee/optee_service.h>

#include "tpm2_tee.h"

/* OP-TEE TPM passthrough PTA */
#define TA_DTPM_UUID { 0x50568cd9, 0x370a, 0x46b8, \
	{ 0x9f, 0x2c, 0x6f, 0xd9, 0x06, 0x64, 0xd2, 0xee } }

/*
 * The TPM has no node of its own in the device tree: the OP-TEE driver
 * binds this driver when OP-TEE enumerates the passthrough PTA.
 */
OPTEE_SERVICE_DRIVER(optee_dtpm, TA_DTPM_UUID, "optee_dtpm");

static int optee_dtpm_desc(struct udevice *dev, char *buf, int size)
{
	if (size < 32)
		return -ENOSPC;

	return snprintf(buf, size, "OP-TEE discrete TPM");
}

static int optee_dtpm_probe(struct udevice *dev)
{
	const struct tee_optee_ta_uuid uuid = TA_DTPM_UUID;

	return tpm2_tee_probe(dev, &uuid);
}

static const struct tpm_ops optee_dtpm_ops = {
	.open		= tpm2_tee_open,
	.close		= tpm2_tee_close,
	.get_desc	= optee_dtpm_desc,
	.xfer		= tpm2_tee_xfer,
};

U_BOOT_DRIVER(optee_dtpm) = {
	.name	= "optee_dtpm",
	.id	= UCLASS_TPM,
	.ops	= &optee_dtpm_ops,
	.probe	= optee_dtpm_probe,
	.remove	= tpm2_tee_remove,
	.flags	= DM_FLAG_OS_PREPARE,
	.priv_auto	= sizeof(struct tpm2_tee_priv),
};
