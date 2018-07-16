/*
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0	IBM-pibs
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <watchdog.h>
#include <asm/ppc4xx.h>

DECLARE_GLOBAL_DATA_PTR;


#define CR0_MASK        0x00001fff
#define CR0_EXTCLK_ENA  0x000000c0
#define CR0_UDIV_POS    1
#define UDIV_MAX        32


/*
 * For some SoC's, the cpu clock is on divider chain A, UART on
 * divider chain B ... so cpu clock is irrelevant. Get the
 * "optimized" values that are subject to the 1/2 opb clock
 * constraint.
 */
#if !defined(CONFIG_SYS_EXT_SERIAL_CLOCK)
static u16 serial_bdiv(int baudrate, u32 *udiv)
{
	sys_info_t sysinfo;
	u32 div;		/* total divisor udiv * bdiv */
	u32 umin;		/* minimum udiv	*/
	u16 diff;		/* smallest diff */
	u16 idiff;		/* current diff */
	u16 ibdiv;		/* current bdiv */
	u32 i;
	u32 est;		/* current estimate */
	u32 max;
	get_sys_info(&sysinfo);

	div = sysinfo.freqPLB / (16 * baudrate); /* total divisor */
	umin = sysinfo.pllOpbDiv << 1;	/* 2 x OPB divisor */
	max = 32;			/* highest possible */
	*udiv = diff = max;

	/*
	 * i is the test udiv value -- start with the largest
	 * possible (max) to minimize serial clock and constrain
	 * search to umin.
	 */
	for (i = max; i > umin; i--) {
		ibdiv = div / i;
		est = i * ibdiv;
		idiff = (est > div) ? (est - div) : (div - est);
		if (idiff == 0) {
			*udiv = i;
			break;		/* can't do better */
		} else if (idiff < diff) {
			*udiv = i;	/* best so far */
			diff = idiff;	/* update lowest diff*/
		}
	}

	return div / *udiv;
}
#endif

/*
 * This function returns the UART clock used by the common
 * NS16550 driver. Additionally the SoC internal divisors for
 * optimal UART baudrate are configured.
 */
int get_serial_clock(void)
{
	u32 clk;
	u32 udiv;
	u32 reg = 0 ;
#if !defined(CONFIG_SYS_EXT_SERIAL_CLOCK)
	PPC4xx_SYS_INFO sys_info;
#endif

	/*
	 * Programming of the internal divisors is SoC specific.
	 * Let's handle this in some #ifdef's for the SoC's.
	 */

#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	clk = CONFIG_SYS_EXT_SERIAL_CLOCK;
	udiv = 1;
	reg |= CR0_EXTCLK_ENA;
#else /* CONFIG_SYS_EXT_SERIAL_CLOCK */
	clk = gd->cpu_clk;
	{
		u32 tmp = CONFIG_SYS_BASE_BAUD * 16;

		udiv = (clk + tmp / 2) / tmp;
	}
	if (udiv > UDIV_MAX)                    /* max. n bits for udiv */
		udiv = UDIV_MAX;
#endif /* CONFIG_SYS_EXT_SERIAL_CLOCK */
	reg |= (udiv - 1) << CR0_UDIV_POS;	/* set the UART divisor */
	mtdcr (CPC0_CR0, reg);
#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	clk = CONFIG_SYS_EXT_SERIAL_CLOCK;
#else
	clk = CONFIG_SYS_BASE_BAUD * 16;
#endif

#if defined(CONFIG_SOC_IBM_FSP2)
	/* Skip all this as uart input clock is hardcoded to OPB/4 */
	clk = CONFIG_SYS_EXT_SERIAL_CLOCK;
#else
	MFREG(UART0_SDR, reg);
	reg &= ~CR0_MASK;

#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	reg |= CR0_EXTCLK_ENA;
	udiv = 1;
	clk = CONFIG_SYS_EXT_SERIAL_CLOCK;
#else /* CONFIG_SYS_EXT_SERIAL_CLOCK */
	clk = gd->baudrate * serial_bdiv(gd->baudrate, &udiv) * 16;
#endif /* CONFIG_SYS_EXT_SERIAL_CLOCK */

	reg |= (udiv - UDIV_SUBTRACT) << CR0_UDIV_POS;	/* set the UART divisor */

	/*
	 * Configure input clock to baudrate generator for all
	 * available serial ports here
	 */
	MTREG(UART0_SDR, reg);
#if defined(UART1_SDR)
	MTREG(UART1_SDR, reg);
#endif
#if defined(UART2_SDR)
	MTREG(UART2_SDR, reg);
#endif
#if defined(UART3_SDR)
	MTREG(UART3_SDR, reg);
#endif
#endif /* CONFIG_SOC_IBM_FSP2 */

	/*
	 * Correct UART frequency in bd-info struct now that
	 * the UART divisor is available
	 */
#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	gd->arch.uart_clk = CONFIG_SYS_EXT_SERIAL_CLOCK;
#else
	get_sys_info(&sys_info);
	gd->arch.uart_clk = sys_info.freqUART / udiv;
#endif

	return clk;
}
