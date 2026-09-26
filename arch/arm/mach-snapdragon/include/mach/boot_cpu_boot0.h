/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Move to the CPU selected by CONFIG_QCOM_BOOT_CPU_MPIDR: start it at _start
 * with PSCI CPU_ON, passing x0 as the context ID, then turn this CPU off.
 */
#include <asm/psci.h>

	mrs	x9, mpidr_el1
	ldr	x10, =0xff00ffffff		/* Aff3..Aff0 */
	and	x9, x9, x10
	ldr	x1, =CONFIG_QCOM_BOOT_CPU_MPIDR
	cmp	x9, x1
	b.eq	reset

	mov	x19, x0				/* preserved across SMC */
	adr	x2, _start
	mov	x3, x19
	ldr	x0, =ARM_PSCI_0_2_FN64_CPU_ON
	smc	#0
	cbnz	x0, 2f

	ldr	x0, =ARM_PSCI_0_2_FN_CPU_OFF
	smc	#0
1:	wfi
	b	1b

	/* CPU_ON failed: stay on this CPU. */
2:	mov	x0, x19
	b	reset
	.ltorg
