/*
 * (C) Copyright 2000-2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * CPU specific code
 *
 * written or collected and sometimes rewritten by
 * Magnus Damm <damm@bitsmart.com>
 *
 * minor modifications by
 * Wolfgang Denk <wd@denx.de>
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/cache.h>
#include <asm/ppc4xx.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

void board_reset(void);

/*
 * To provide an interface to detect CPU number for boards that support
 * more then one CPU, we implement the "weak" default functions here.
 *
 * Returns CPU number
 */
int __get_cpu_num(void)
{
	return NA_OR_UNKNOWN_CPU;
}
int get_cpu_num(void) __attribute__((weak, alias("__get_cpu_num")));

#if defined(SDR0_PINSTP_SHIFT)
static int bootstrap_option(void)
{
	unsigned long val;

	mfsdr(SDR0_PINSTP, val);
	return ((val & 0xf0000000) >> SDR0_PINSTP_SHIFT);
}
#endif /* SDR0_PINSTP_SHIFT */

int checkcpu (void)
{
	uint pvr = get_pvr();
	ulong clock = gd->cpu_clk;
	char buf[32];
	char addstr[64] = "";
	sys_info_t sys_info;
	int cpu_num;

	cpu_num = get_cpu_num();
	if (cpu_num >= 0)
		printf("CPU%d:  ", cpu_num);
	else
		puts("CPU:   ");

	get_sys_info(&sys_info);
	puts("IBM PowerPC ");

	switch (pvr) {
	case PVR_476FSP2:
		puts("476 FSP2 DD1.0");
		break;
	default:
		printf (" UNKNOWN (PVR=%08x)", pvr);
		break;
	}

	printf (" at %s MHz (PLB=%lu OPB=%lu EBC=%lu",
		strmhz(buf, clock),
		sys_info.freqPLB / 1000000,
		get_OPB_freq() / 1000000,
		sys_info.freqEBC / 1000000);
	printf(")\n");

	if (addstr[0] != 0)
		printf("       %s\n", addstr);

#if defined(I2C_BOOTROM)
	printf ("       I2C boot EEPROM %sabled\n", i2c_bootrom_enabled() ? "en" : "dis");
#endif	/* I2C_BOOTROM */
#if defined(SDR0_PINSTP_SHIFT)
	printf ("       Bootstrap Option %c - ", bootstrap_char[bootstrap_option()]);
	printf ("Boot ROM Location %s", bootstrap_str[bootstrap_option()]);
	putc('\n');
#endif	/* SDR0_PINSTP_SHIFT */

#if defined(CONFIG_PCI) && defined(PCI_ASYNC)
	if (pci_async_enabled()) {
		printf (", PCI async ext clock used");
	} else {
		printf (", PCI sync clock at %lu MHz",
		       sys_info.freqPLB / sys_info.pllPciDiv / 1000000);
	}
#endif

	printf("       32 KiB I-Cache 32 KiB D-Cache");
	putc ('\n');

	return 0;
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined(CONFIG_BOARD_RESET)
	board_reset();
#else
#if defined(CONFIG_SYS_4xx_RESET_TYPE)
	mtspr(SPRN_DBCR0, CONFIG_SYS_4xx_RESET_TYPE << 28);
#else
	/*
	 * Initiate system reset in debug control register DBCR
	 */
	mtspr(SPRN_DBCR0, 0x30000000);
#endif /* defined(CONFIG_SYS_4xx_RESET_TYPE) */
#endif /* defined(CONFIG_BOARD_RESET) */

	return 1;
}


/*
 * Get timebase clock frequency
 */
unsigned long get_tbclk (void)
{
	sys_info_t  sys_info;

	get_sys_info(&sys_info);
#if defined(CONFIG_SOC_IBM_FSP2)
	return (sys_info.freqTmrClk);
#else
	return (sys_info.freqProcessor);
#endif
}


#if defined(CONFIG_WATCHDOG)
void reset_4xx_watchdog(void)
{
	/*
	 * Clear TSR(WIS) bit
	 */
	mtspr(SPRN_TSR, 0x40000000);
}

void watchdog_reset(void)
{
	int re_enable = disable_interrupts();
	reset_4xx_watchdog();
	if (re_enable) enable_interrupts();
}
#endif	/* CONFIG_WATCHDOG */

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_IBM_EMAC4)
	printf("eth init\n");
	ppc_4xx_eth_initialize(bis);
#endif
	return 0;
}
