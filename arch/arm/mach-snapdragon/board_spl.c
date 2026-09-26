// SPDX-License-Identifier: GPL-2.0+
/*
 * Common SPL code for Qualcomm Snapdragon boards.
 *
 * Copyright (c) 2026 Michael Srba <Michael.Srba@seznam.cz>
 */

#include <hang.h>
#include <spl.h>
#include <timestamp.h>
#include <version.h>

void board_init_f(ulong dummy)
{
	/*
	 * An SPL without a driver model serial console prints on the debug
	 * UART, which the boot firmware leaves set up. Announce SPL there, as
	 * preloader_console_init() does on a serial console.
	 */
	if (IS_ENABLED(CONFIG_DEBUG_UART) && CONFIG_IS_ENABLED(BANNER_PRINT))
		puts("\nU-Boot " PHASE_NAME " " PLAIN_VERSION " (" U_BOOT_DATE
		     " - " U_BOOT_TIME " " U_BOOT_TZ ")\n");
}

/* in SPL, we always use internal DT */
int board_fdt_blob_setup(void **fdtp)
{
	return -EEXIST;
}

__weak void reset_cpu(void)
{
	/* This should currently not get called in non-error paths, so just hang */
	printf("reset_cpu called, going to hang()\n");
	hang();
}

u32 spl_boot_device(void)
{
	/* The boot firmware has already loaded the next stages to memory */
	if (CONFIG_IS_ENABLED(RAM_DEVICE))
		return BOOT_DEVICE_RAM;

	/* TODO: check boot reason to support UFS and sdcard */
	return BOOT_DEVICE_DFU;
}
