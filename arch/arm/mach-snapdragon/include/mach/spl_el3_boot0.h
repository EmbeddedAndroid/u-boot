/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * XBL starts the TZ image, and an SPL flashed in its place, at EL3. Put
 * SCTLR_EL3 in a known state before touching memory, as TF-A BL2 does in
 * that place, rather than depend on the state XBL leaves: MMU, caches,
 * alignment checks and WXN off. The QTI TZ image clears SCTLR_EL3.M first
 * thing too. TF-A BL31, started by SPL, expects the previous EL3 stage to
 * have done this.
 */
#include <asm/system.h>

	mrs	x16, CurrentEL
	cmp	x16, #(3 << 2)
	b.ne	reset

	mrs	x16, sctlr_el3
	mov	x17, #(CR_M | CR_A | CR_C | CR_SA | CR_I)
	orr	x17, x17, #CR_WXN
	bic	x16, x16, x17
	msr	sctlr_el3, x16
	isb
	ic	iallu
	dsb	nsh
	isb
	b	reset
