/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Microsoft Corporation
 */

#ifndef __TPM2_TEE_H__
#define __TPM2_TEE_H__

#include <tee.h>

/* TA function that submits a TPM command (fTPM TA and dTPM PTA) */
#define TPM2_TEE_SUBMIT_COMMAND		0

/* Max. buffer sizes of the TA */
#define TPM2_TEE_MAX_COMMAND_SIZE	4096
#define TPM2_TEE_MAX_RESPONSE_SIZE	4096

/**
 * struct tpm2_tee_priv - private context of a TPM reached through a TA
 * @tee_dev:  struct udevice for TEE.
 * @session:  TA session identifier.
 * @is_open:  Indicates whether the driver is already opened by client or not.
 * @shm:      Memory pool shared with the TA.
 */
struct tpm2_tee_priv {
	struct udevice *tee_dev;
	u32 session;
	int is_open;
	struct tee_shm *shm;
};

int tpm2_tee_xfer(struct udevice *dev, const u8 *sendbuf, size_t send_size,
		  u8 *recvbuf, size_t *recv_len);
int tpm2_tee_open(struct udevice *dev);
int tpm2_tee_close(struct udevice *dev);
int tpm2_tee_probe(struct udevice *dev, const struct tee_optee_ta_uuid *uuid);
int tpm2_tee_remove(struct udevice *dev);

#endif /* __TPM2_TEE_H__ */
