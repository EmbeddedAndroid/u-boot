// SPDX-License-Identifier: GPL-2.0+
/*
 * Arm DRTM (DEN0113) dynamic launch from U-Boot
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <bootm.h>
#include <command.h>
#include <lmb.h>
#include <mapmem.h>
#include <vsprintf.h>
#include <asm/system.h>
#include <asm/u-boot-arm.h>
#include <linux/arm-smccc.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/build_bug.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <linux/types.h>

#define DRTM_FN(n)		(0xc4000110 + (n))
#define DRTM_VERSION		DRTM_FN(0)
#define DRTM_FEATURES		DRTM_FN(1)
#define DRTM_DYNAMIC_LAUNCH	DRTM_FN(4)
#define DRTM_GET_ERROR		DRTM_FN(6)

#define DRTM_FEATURE_ID		BIT_ULL(63)
#define DRTM_FEAT_TPM		1
#define DRTM_FEAT_MIN_MEM	2
#define DRTM_FEAT_DMA_PROT	3
#define DRTM_FEAT_BOOT_PE	4
#define DRTM_FEAT_IMG_AUTH	6

#define DRTM_TPM_HASH		BIT_ULL(32)
#define DRTM_DMA_PROT_TYPES	GENMASK(7, 0)
#define DRTM_IMG_AUTH_SUPPORT	BIT(0)

/* DRTM_PARAMETERS features */
#define DRTM_LF_HASH		BIT(0)
#define DRTM_LF_PCR_SCHEMA	GENMASK(2, 1)
#define DRTM_LF_MEM_PROT	GENMASK(5, 3)
#define DRTM_LF_IMG_AUTH	BIT(6)

#define DRTM_PCR_SCHEMA_AUTH	1

#define DRTM_NOT_SUPPORTED	-1

/* DEN0113 DRTM_PARAMETERS, version 1 */
struct drtm_launch_args {
	u16 version;
	u8 res[2];
	u32 features;
	u64 dlme_paddr;
	u64 dlme_size;
	u64 dlme_img_off;
	u64 dlme_img_ep_off;
	u64 dlme_img_size;
	u64 dlme_data_off;
	u64 dce_nwd_paddr;
	u64 dce_nwd_size;
	u64 dma_prot_table_paddr;
	u64 dma_prot_table_size;
} __packed;

static_assert(sizeof(struct drtm_launch_args) == 88);

static const char *const drtm_errors[] = {
	"success", "not supported", "invalid parameters", "denied",
	"not found", "internal error", "invalid memory protection",
	"coprocessor error", "out of resources", "invalid data",
	"secondary PE not off", "already closed", "TPM error",
};

static const char *drtm_strerror(long ret)
{
	if (ret > 0 || -ret >= ARRAY_SIZE(drtm_errors))
		return "unknown";

	return drtm_errors[-ret];
}

static unsigned long drtm_call(unsigned long fid, unsigned long a1,
			       unsigned long *res1)
{
	struct arm_smccc_res res;

	arm_smccc_smc(fid, a1, 0, 0, 0, 0, 0, 0, &res);
	if (res1)
		*res1 = res.a1;

	return res.a0;
}

static int drtm_feature(unsigned int id, unsigned long *val)
{
	return drtm_call(DRTM_FEATURES, DRTM_FEATURE_ID | id, val) == 1 ?
	       0 : -ENOENT;
}

static int drtm_check_el(void)
{
	unsigned long pfr0;

	/* Without EL3 the SMC is undefined */
	asm volatile("mrs %0, id_aa64pfr0_el1" : "=r" (pfr0));
	if (!(pfr0 & ID_AA64PFR0_EL1_EL3)) {
		printf("DRTM: needs EL3 firmware, this CPU has no EL3\n");
		return -ENODEV;
	}
	if (current_el() == 3) {
		printf("DRTM: U-Boot runs at EL3, DRTM needs a normal world caller\n");
		return -EPERM;
	}

	return 0;
}

/* Launch features the DCE would refuse after the quiesce */
static const char *drtm_check_features(unsigned long features)
{
	unsigned long tpm, dma, auth;

	if (!drtm_feature(DRTM_FEAT_TPM, &tpm) &&
	    FIELD_GET(DRTM_LF_HASH, features) != !!(tpm & DRTM_TPM_HASH))
		return "hashing type";
	if (!drtm_feature(DRTM_FEAT_DMA_PROT, &dma) &&
	    (dma & DRTM_DMA_PROT_TYPES) &&
	    FIELD_GET(DRTM_LF_MEM_PROT, features) !=
	    __builtin_ctzl(dma & DRTM_DMA_PROT_TYPES))
		return "DMA protection type";
	if (FIELD_GET(DRTM_LF_PCR_SCHEMA, features) == DRTM_PCR_SCHEMA_AUTH &&
	    !(features & DRTM_LF_IMG_AUTH))
		return "PCR schema without image authentication";
	if ((features & DRTM_LF_IMG_AUTH) &&
	    (drtm_feature(DRTM_FEAT_IMG_AUTH, &auth) ||
	     !(auth & DRTM_IMG_AUTH_SUPPORT)))
		return "image authentication";

	return NULL;
}

static int do_drtm_info(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	static const struct {
		unsigned int id;
		const char *name;
	} feats[] = {
		{ DRTM_FEAT_TPM, "TPM" },
		{ DRTM_FEAT_MIN_MEM, "minimum memory" },
		{ DRTM_FEAT_DMA_PROT, "DMA protection" },
		{ DRTM_FEAT_BOOT_PE, "boot PE" },
		{ DRTM_FEAT_IMG_AUTH, "image auth" },
	};
	unsigned long ver, val;
	unsigned long err;
	int i;

	if (drtm_check_el())
		return CMD_RET_FAILURE;

	ver = drtm_call(DRTM_VERSION, 0, NULL);
	if ((long)ver == DRTM_NOT_SUPPORTED) {
		printf("DRTM: not supported by the firmware\n");
		return CMD_RET_FAILURE;
	}
	printf("DRTM version %lu.%lu\n", (ver >> 16) & 0x7fff, ver & 0xffff);

	for (i = 0; i < ARRAY_SIZE(feats); i++) {
		if (drtm_feature(feats[i].id, &val))
			printf("  %-16s not supported\n", feats[i].name);
		else
			printf("  %-16s 0x%lx\n", feats[i].name, val);
	}

	if (!drtm_call(DRTM_GET_ERROR, 0, &err))
		printf("  %-16s 0x%lx\n", "error code", err);

	return CMD_RET_SUCCESS;
}

static int do_drtm_launch(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	struct drtm_launch_args *args;
	unsigned long dlme, size, img_size, data_off, ep_off = 0;
	unsigned long features = 0, minmem;
	const char *bad;
	phys_addr_t params;
	long ret;

	if (argc < 5 || argc > 7)
		return CMD_RET_USAGE;

	dlme = hextoul(argv[1], NULL);
	size = hextoul(argv[2], NULL);
	img_size = hextoul(argv[3], NULL);
	data_off = hextoul(argv[4], NULL);
	if (argc > 5)
		ep_off = hextoul(argv[5], NULL);
	if (argc > 6)
		features = hextoul(argv[6], NULL);

	/* Refuse here what the firmware would refuse after the quiesce */
	if (!IS_ALIGNED(dlme | size | data_off, SZ_4K) || dlme < SZ_4K ||
	    dlme + size < dlme || !img_size || img_size > data_off ||
	    data_off >= size || ep_off >= img_size ||
	    !IS_ALIGNED(ep_off, 4) || features > U32_MAX) {
		printf("DRTM: invalid DLME region, image, entry point or features\n");
		return CMD_RET_FAILURE;
	}

	/* The parameters go in the page below the DLME region */
	params = dlme - SZ_4K;
	if (lmb_alloc_mem(LMB_MEM_ALLOC_ADDR, 0, &params, size + SZ_4K,
			  LMB_NONE)) {
		printf("DRTM: the DLME region or the page below it is not free memory\n");
		return CMD_RET_FAILURE;
	}

	if (drtm_check_el())
		return CMD_RET_FAILURE;
	if (drtm_call(DRTM_FEATURES, DRTM_DYNAMIC_LAUNCH, NULL)) {
		printf("DRTM: dynamic launch not supported by the firmware\n");
		return CMD_RET_FAILURE;
	}
	if (!drtm_feature(DRTM_FEAT_MIN_MEM, &minmem) &&
	    (size - data_off) / SZ_4K < (minmem & U32_MAX)) {
		printf("DRTM: DLME data needs %lu pages\n", minmem & U32_MAX);
		return CMD_RET_FAILURE;
	}
	bad = drtm_check_features(features);
	if (bad) {
		printf("DRTM: features 0x%lx refused: %s\n", features, bad);
		return CMD_RET_FAILURE;
	}

	args = map_sysmem(params, SZ_4K);
	memset(args, 0, sizeof(*args));
	args->version = 1;
	args->features = features;
	args->dlme_paddr = dlme;
	args->dlme_size = size;
	args->dlme_img_ep_off = ep_off;
	args->dlme_img_size = img_size;
	args->dlme_data_off = data_off;
	unmap_sysmem(args);

	printf("DRTM: launching DLME at 0x%lx\n", dlme + ep_off);
	bootm_final(0);
	cleanup_before_linux();

	ret = drtm_call(DRTM_DYNAMIC_LAUNCH, params, NULL);

	/* Only a refused launch returns, with U-Boot quiesced */
	printf("DRTM: launch failed: %ld (%s)\n", ret, drtm_strerror(ret));
	puts("### ERROR ### Please RESET the board ###\n");
	do_reset(NULL, 0, 0, NULL);

	return CMD_RET_FAILURE;
}

U_BOOT_LONGHELP(drtm,
	"info - show the DRTM version, features and error code\n"
	"drtm launch <dlme> <size> <image size> <data offset> [<entry offset> [<features>]]\n"
	"    - dynamic launch of the DLME image at <dlme>, in a DLME region of\n"
	"      <size> bytes whose DLME data starts at <data offset> (hex); the\n"
	"      launch parameters are written to the free 4 KiB below <dlme>");

U_BOOT_CMD_WITH_SUBCMDS(drtm, "Arm DRTM dynamic launch", drtm_help_text,
	U_BOOT_SUBCMD_MKENT(info, 1, 1, do_drtm_info),
	U_BOOT_SUBCMD_MKENT(launch, 7, 0, do_drtm_launch));
