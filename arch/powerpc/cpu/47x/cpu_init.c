/*
 * (C) Copyright 2000-2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <asm/ppc4xx-emac.h>
#include <asm/processor.h>
#include <asm/ppc4xx.h>

DECLARE_GLOBAL_DATA_PTR;

void reset_4xx_watchdog(void);

#ifndef CONFIG_SYS_PLL_RECONFIG
#define CONFIG_SYS_PLL_RECONFIG	0
#endif

void reconfigure_pll(u32 new_cpu_freq)
{
}

#ifdef CONFIG_SYS_4xx_CHIP_21_ERRATA
void
chip_21_errata(void)
{
	/*
	 * See rev 1.09 of the 405EX/405EXr errata.  CHIP_21 says that
	 * sometimes reading the PVR and/or SDR0_ECID results in incorrect
	 * values.  Since the rev-D chip uses the SDR0_ECID bits to control
	 * internal features, that means the second PCIe or ethernet of an EX
	 * variant could fail to work.  Also, security features of both EX and
	 * EXr might be incorrectly disabled.
	 *
	 * The suggested workaround is as follows (covering rev-C and rev-D):
	 *
	 * 1.Read the PVR and SDR0_ECID3.
	 *
	 * 2.If the PVR matches an expected Revision C PVR value AND if
	 * SDR0_ECID3[12:15] is different from PVR[28:31], then processor is
	 * Revision C: continue executing the initialization code (no reset
	 * required).  else go to step 3.
	 *
	 * 3.If the PVR matches an expected Revision D PVR value AND if
	 * SDR0_ECID3[10:11] matches its expected value, then continue
	 * executing initialization code, no reset required.  else write
	 * DBCR0[RST] = 0b11 to generate a SysReset.
	 */

	u32 pvr;
	u32 pvr_28_31;
	u32 ecid3;
	u32 ecid3_10_11;
	u32 ecid3_12_15;

	/* Step 1: */
	pvr = get_pvr();
	mfsdr(SDR0_ECID3, ecid3);

	/* Step 2: */
	pvr_28_31 = pvr & 0xf;
	ecid3_10_11 = (ecid3 >> 20) & 0x3;
	ecid3_12_15 = (ecid3 >> 16) & 0xf;
	if ((pvr == CONFIG_405EX_CHIP21_PVR_REV_C) &&
			(pvr_28_31 != ecid3_12_15)) {
		/* No reset required. */
		return;
	}

	/* Step 3: */
	if ((pvr == CONFIG_405EX_CHIP21_PVR_REV_D) &&
			(ecid3_10_11 == CONFIG_405EX_CHIP21_ECID3_REV_D)) {
		/* No reset required. */
		return;
	}

	/* Reset required. */
	__asm__ __volatile__ ("sync; isync");
	mtspr(SPRN_DBCR0, 0x30000000);
}
#endif

/*
 * Breath some life into the CPU...
 *
 * Reconfigure PLL if necessary,
 * set up the memory map,
 * initialize a bunch of registers
 */
void
cpu_init_f (void)
{
#if defined(CONFIG_WATCHDOG)
	u32 val;
#endif

#ifdef CONFIG_SYS_4xx_CHIP_21_ERRATA
	chip_21_errata();
#endif

	reconfigure_pll(CONFIG_SYS_PLL_RECONFIG);

#if defined(CONFIG_SYS_4xx_GPIO_TABLE)
	gpio_set_chip_configuration();
#endif /* CONFIG_SYS_4xx_GPIO_TABLE */

	/*
	 * External Bus Controller (EBC) Setup
	 */
#if (defined(CONFIG_SYS_EBC_PB0AP) && defined(CONFIG_SYS_EBC_PB0CR))
	mtebc(PB0AP, CONFIG_SYS_EBC_PB0AP);
	mtebc(PB0CR, CONFIG_SYS_EBC_PB0CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB1AP) && defined(CONFIG_SYS_EBC_PB1CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 1))
	mtebc(PB1AP, CONFIG_SYS_EBC_PB1AP);
	mtebc(PB1CR, CONFIG_SYS_EBC_PB1CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB2AP) && defined(CONFIG_SYS_EBC_PB2CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 2))
	mtebc(PB2AP, CONFIG_SYS_EBC_PB2AP);
	mtebc(PB2CR, CONFIG_SYS_EBC_PB2CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB3AP) && defined(CONFIG_SYS_EBC_PB3CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 3))
	mtebc(PB3AP, CONFIG_SYS_EBC_PB3AP);
	mtebc(PB3CR, CONFIG_SYS_EBC_PB3CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB4AP) && defined(CONFIG_SYS_EBC_PB4CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 4))
	mtebc(PB4AP, CONFIG_SYS_EBC_PB4AP);
	mtebc(PB4CR, CONFIG_SYS_EBC_PB4CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB5AP) && defined(CONFIG_SYS_EBC_PB5CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 5))
	mtebc(PB5AP, CONFIG_SYS_EBC_PB5AP);
	mtebc(PB5CR, CONFIG_SYS_EBC_PB5CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB6AP) && defined(CONFIG_SYS_EBC_PB6CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 6))
	mtebc(PB6AP, CONFIG_SYS_EBC_PB6AP);
	mtebc(PB6CR, CONFIG_SYS_EBC_PB6CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB7AP) && defined(CONFIG_SYS_EBC_PB7CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 7))
	mtebc(PB7AP, CONFIG_SYS_EBC_PB7AP);
	mtebc(PB7CR, CONFIG_SYS_EBC_PB7CR);
#endif

#if defined (CONFIG_SYS_EBC_CFG)
	mtebc(EBC0_CFG, CONFIG_SYS_EBC_CFG);
#endif

#if defined(CONFIG_WATCHDOG)
	val = mfspr(SPRN_TCR);
	val |= 0xf0000000;      /* generate system reset after 2.684 seconds */
#if defined(CONFIG_SYS_4xx_RESET_TYPE)
	val &= ~0x30000000;			/* clear WRC bits */
	val |= CONFIG_SYS_4xx_RESET_TYPE << 28;	/* set board specific WRC type */
#endif
	mtspr(SPRN_TCR, val);

	val = mfspr(SPRN_TSR);
	val |= 0x80000000;      /* enable watchdog timer */
	mtspr(SPRN_TSR, val);

	reset_4xx_watchdog();
#endif /* CONFIG_WATCHDOG */

}

/*
 * initialize higher level parts of CPU like time base and timers
 */
int cpu_init_r (void)
{
	return 0;
}


