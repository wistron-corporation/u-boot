/*
 * (C) Copyright 2000-2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <ppc_asm.tmpl>
#include <asm/ppc4xx.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef DEBUG
#define DEBUGF(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGF(fmt,args...)
#endif

#if defined(CONFIG_SOC_IBM_FSP2)
void get_sys_info(sys_info_t *sysInfo)
{
	unsigned long tcs, divisor;

	/* FIXME should be taken from hardware */
	sysInfo->freqProcessor = 1333333333;
	sysInfo->freqPLB = 666666666;
	sysInfo->freqOPB = 83333333;
	sysInfo->freqUART = sysInfo->freqOPB/4;

	/* This logic only supports using the CPU clock.  If an external
	 * clock is used (CCR1[TSS] = 0b1), then this logic will need to be
	 * updated. */
	tcs = (mfspr(SPRN_CCR1) & CCR1_TCS_MASK ) >> 8;
	divisor = tcs ? 2 << tcs : 1;
	sysInfo->freqTmrClk = sysInfo->freqProcessor / divisor;
}
#endif

int get_clocks (void)
{
	sys_info_t sys_info;

	get_sys_info (&sys_info);
	gd->cpu_clk = sys_info.freqProcessor;
	gd->bus_clk = sys_info.freqPLB;

	return (0);
}


/********************************************
 * get_bus_freq
 * return PLB bus freq in Hz
 *********************************************/
ulong get_bus_freq (ulong dummy)
{
	ulong val;
	sys_info_t sys_info;

	get_sys_info (&sys_info);
	val = sys_info.freqPLB;

	return val;
}

ulong get_OPB_freq (void)
{
	PPC4xx_SYS_INFO sys_info;

	get_sys_info (&sys_info);

	return sys_info.freqOPB;
}
