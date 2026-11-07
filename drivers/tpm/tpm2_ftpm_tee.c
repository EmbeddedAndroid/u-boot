// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Microsoft Corporation
 *
 * Authors:
 * Thirupathaiah Annapureddy <thiruan@microsoft.com>
 *
 * Description:
 * Device Driver for a firmware TPM as described here:
 * https://www.microsoft.com/en-us/research/publication/ftpm-software-implementation-tpm-chip/
 *
 * A reference implementation is available here:
 * https://github.com/microsoft/ms-tpm-20-ref/tree/master/Samples/ARM32-FirmwareTPM/optee_ta/fTPM
 */

#include <dm.h>
#include <tpm-v2.h>
#include <tee.h>
#include <tee/optee_service.h>

#include "tpm2_ftpm_tee.h"
#include "tpm2_tee.h"

OPTEE_SERVICE_DRIVER(optee_ftpm, TA_FTPM_UUID, "ftpm_tee");

static int ftpm_tee_desc(struct udevice *dev, char *buf, int size)
{
	if (size < 32)
		return -ENOSPC;

	return snprintf(buf, size, "Microsoft OP-TEE fTPM");
}

static int ftpm_tee_probe(struct udevice *dev)
{
	const struct tee_optee_ta_uuid uuid = TA_FTPM_UUID;

	return tpm2_tee_probe(dev, &uuid);
}

static const struct tpm_ops ftpm_tee_ops = {
	.open		= tpm2_tee_open,
	.close		= tpm2_tee_close,
	.get_desc	= ftpm_tee_desc,
	.xfer		= tpm2_tee_xfer,
};

static const struct udevice_id ftpm_tee_ids[] = {
	{ .compatible = "microsoft,ftpm" },
	{ }
};

U_BOOT_DRIVER(ftpm_tee) = {
	.name   = "ftpm_tee",
	.id     = UCLASS_TPM,
	.of_match = ftpm_tee_ids,
	.ops    = &ftpm_tee_ops,
	.probe	= ftpm_tee_probe,
	.remove	= tpm2_tee_remove,
	.flags	= DM_FLAG_OS_PREPARE,
	.priv_auto	= sizeof(struct tpm2_tee_priv),
};
