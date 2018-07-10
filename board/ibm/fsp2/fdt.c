/*
 * Copyright (c) 2017 IBM
 * SPDX-License-Identifier:     GPL-2.0+
 */


#include <common.h>
#include <asm/processor.h>

#include <libfdt.h>
#include <libfdt_env.h>
#include <fdt_support.h>

static unsigned long get_tbfreq(unsigned long cpufreq)
{
	unsigned long tcs, divisor;

	/*
	 * NOTE: This code assumes the timer source is the CPU clock.  If we
	 * ever want to use an external clock instead of the CPU clock, we'll
	 * need to also check CCR1[TSS] to determine which timer source is
	 * being divided.  But currently we don't have an external clock
	 * connected.
	 */

	tcs = (mfspr(SPRN_CCR1) & 0x300) >> 8;
	divisor = tcs ? 0x2 << tcs : 1;

	/* div with mathematical rounding up */
	return (cpufreq + divisor/2) / divisor;
}

int ft_board_setup(void* blob, bd_t *bd)
{
	fdt_fixup_memory(blob, (u64)env_get_bootm_low(), (u64)FSP2_RAM_SIZE);
	fdt_fixup_ethernet(blob);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4, "clock-frequency",
			     bd->bi_intfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "timebase-frequency", get_tbfreq(bd->bi_intfreq),
			     1);
	return 0;
}

