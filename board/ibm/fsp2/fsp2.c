/*
 * Copyright (c) 2017 IBM
 * SPDX-License-Identifier:     GPL-2.0+
 */


#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <fsp2.h>
#include <fspboot.h>
#include <watchdog.h>
#include <asm/ppc476fsp2.h>
#include <asm/ppc4xx-emac.h>

#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

extern void touch_l2cache_range(ulong addr, ulong size);
extern void flush_l2cache_range(ulong addr, ulong size);


/* FIXME make each block configurable via spl_config environment key */
static void unit_reset_release(void)
{
	unsigned long tmp;

	/* Misc units */
	mtcmu(URCR0_RW1C, 0x00004000); /* uart */
	mtcmu(URCR1_RW1C, 0xFFE1F1FF); /* PSI/MBX, IIC, FSI/DMA */

	/* change to use "FSP1 mode" for IOU I2C engines */
	tmp = mfcmu(POR_CONF105);
	mtcmu(POR_CONF105, tmp | CONF105_IOU_I2C_ACK);

	/* enable in/outputs for OPB IIC engines 0:4 */
	mtdcrx(PPCU_CFG_01_RS, 0x000F87C0);

	/* Get AHB up and running */
	mtcmu(URCR1_RW1C, 0x00100000); /* release P4XASYS reset */
	mtcmu(URCR1_RW1C, 0x00080000); /* release AHB reset */

	/* USB controller */

	/* Set the uFrame timing to 125 uS */
	tmp = mfcmu( POR_CONF96 );      /* Some of the bit fields in POR_CONF96 map
	                                 * to the USB controller and allow
	                                 * us a degree of control over the uFrame
	                                 * timing.
	                                 */
	tmp = tmp & 0x70000EFF;         /* Attempt to preserve all bits of the
	                                 * register we are not interested in
	                                 * while zeroing out
	                                 * that part of the register dealing
	                                 * with the uFrame timing.
	                                 */
	tmp = tmp | 0x700;              /* Or in the values to set the uFrame
	                                 * timing.
	                                 */
	mtcmu(  POR_CONF96, tmp );      /* Set the USB uFrame timing. */

	mtcmu(POR_CONF102, 0x00fffc00);       /* enable PHY signal signal */
	mtcmu(POR_CONF102, 0x00ef7c00);       /* raise USB_RESET_B */
	udelay(1000);		       /* wait for clock stable */
	mtcmu(URCR0_RW1C, 0x00800000); /* release USB host controller */
	mtcmu(URCR0_RW1C, 0x007E0000); /* release remaining units */
	out32(EHCI_INSNREG5, 0x818a0066); /* enable port1's CPEN */
	udelay(1000);
	out32(EHCI_INSNREG5, 0x828a0066); /* enable port2's CPEN */

	/* ethernet */
	tmp = mfcmu(POR_CONF103);
	tmp &= ~CONF103_ETHX_PHY_RESET_OE;
	tmp |= CONF103_ETHX_EN_MASK;
	mtcmu(POR_CONF103, tmp);
	udelay(20000);
	mtcmu(URCR0_RW1C, 0x00000040); /* release RGMII */
	udelay(1000);
	out32(RGMII_FER, 0x000C0055);	/* set RGMII mode: enable,reduced,gmii*/
	out32(RGMII_SSR, 0x00000404);	/* set speed to 1Gbit */
	udelay(1000);
	mtcmu(URCR0_RW1C, 0x00001C00); /* release eth0 */
	mtcmu(URCR0_RW1C, 0x00000380); /* release eth1 */
	udelay(1000);
	out32(GPIO0_BIDI0_CLR, GPIO_PHY_IRQS); /* clear phy irqs */
	out32(GPIO0_EIR, GPIO_PHY_IRQS); /* enable phy irqs */

	/* eMMC */
	/* FIXME spec says to wait for 20ms, does prior code take this long? */
	tmp = mfcmu(POR_CONF103);
	tmp &= ~CONF103_EMMC_RST_B;
	tmp |= CONF103_EMMC_CLK | CONF103_EMMC_CMD_DATA_IE;
	mtcmu(POR_CONF103, tmp);
	mtcmu(URCR1_RW1C, 0x00040000);
	udelay(1000);
	mtcmu(POR_CONF91, 0x0427D4FE);	/* MMC HC strap values */
	mtcmu(POR_CONF92, 0x480000);	/* MMC xmit tap delay, see
					   emmc_otapdly{en,sel} */
}

int board_early_init_f(void)
{
	unit_reset_release();

	/* Setup the interrupt controllers */
	mtdcr(UIC1SR, 0xffffffff); /* Clear all pending */
	mtdcr(UIC1ER, 0x00000000); /* disable all irqs (eth enabled later) */
	mtdcr(UIC1CR, 0x00000000);
	mtdcr(UIC1PR, 0xffffffff); /* Set active high polarity */
	mtdcr(UIC1TR, 0x00000000);
	mtdcr(UIC1SR, 0xffffffff); /* Clear all pending */

	mtdcr(UIC0SR, 0xffffffff); /* Clear all pending */
	mtdcr(UIC0ER, 0x00000000); /* Disable all irqs */
	mtdcr(UIC0CR, 0x00000000);
	mtdcr(UIC0PR, 0xffffffff); /* Set active high polarity */
	mtdcr(UIC0TR, 0x00000000);
	mtdcr(UIC0SR, 0xffffffff); /* Clear all pending */

	return 0;
}

int checkboard(void)
{
	unsigned long pvr = mfspr(SPRN_PVR);
	unsigned long dbsr = mfspr(SPRN_DBSR);
	unsigned long tsr = mfspr(SPRN_TSR);
	unsigned long crcs = mfcmu(CMU_CRCS);
	char *buf = env_get("serial#");

	printf("Board: IBM Sunray2");
	puts(", serial# ");
	puts(buf);
	putc('\n');

	printf(" Early Debug Info:\n");
	printf("\tPVR  %08lx  DBSR  %08lx  CRCS   %08lx\n", pvr, dbsr, crcs);
	printf("\tTSR  %08lx  BWTC  %08x   \n", tsr, mfcmu(CMU_BWTC));

	return (0);
}

/*
 * Check for UEs
 * @return non-zero if UE found, zero otherwise
 */
static int check_for_ue(void)
{
	if (mfdcrx(CW_ERR0) & ERR0_MEM_UE)
		return 1;

	return 0;
}

/* read/write all specified memory, using L2, watching for UEs.
 * If a UE is detected then fill memory with zeros, again, using the L2.
 *
 * @start: Address at which to commence memory scrub/fill
 * @end: Address at which to end memory scrub/fill
 * @ce: Indication of whether any CEs were detected
 *
 * @return MEM_SCRUBBED or MEM_FILLED, as appropriate
 */
int mem_init(unsigned long start, unsigned long end, int *ce)
{
	/* Clear CE counts */
	mtdcrx(ECCERR_COUNT_PORT0, 0x0);

	unsigned long addr = start, len = end - start;
	do {
		unsigned long step = (len > MEM_STEP) ? MEM_STEP : len;
		touch_l2cache_range(addr, addr + step);
		flush_l2cache_range(addr, addr + step);

		if (check_for_ue())
			break;

		addr += step, len -= step;
	} while (addr < end);

	/* clear any CEs */
	if (mfdcrx(CW_ERR0) & ERR0_MEM_CE) {
		unsigned long mccr = mfcmu(CMU_MCCR);
		mtcmu(CMU_MCCR, mccr | MCCR_LFIRCE);
		mtcmu(CMU_MCCR, mccr & ~MCCR_LFIRCE);
		mtdcrx(MCIF_ECCERR_STATUS_CLR, 0xffffffff);
		mtdcrx(CW_ERR0_AND, ~ERR0_MEM_CE);
		if (ce)
			*ce = 1;
	}

	WATCHDOG_RESET();

	if (addr < end) {
		addr = start, len = end - start;

		do {
			unsigned long step = (len > MEM_STEP) ? MEM_STEP : len;

			/* FIXME use dcbz? */
			int i;
			for (i = addr; i < addr + step; i++) {
				*(uint32_t *)i = 0;
			}

			flush_l2cache_range(addr, addr + step);

			addr += step, len -= step;
		} while (addr < end);

		/* clear out CE/UE */
		unsigned long mccr = mfcmu(CMU_MCCR);
		mtcmu(CMU_MCCR, mccr | (MCCR_LFIRUE | MCCR_LFIRCE));
		mtcmu(CMU_MCCR, mccr & ~(MCCR_LFIRUE | MCCR_LFIRCE));

		mtdcrx(MCIF_ECCERR_STATUS_CLR, 0xffffffff);
		mtdcrx(MCIF_ECCERR_ADDR, 0x0);
		mtdcrx(CW_ERR0_AND, ~ERR0_MEM_UE);
		mtdcrx(CW_ERR0_AND, ~ERR0_MEM_CE);

		return MEM_FILLED;
	}

	return MEM_SCRUBBED;
}

int dram_init_banksize(void) {
	gd->bd->bi_memstart = 0x0;
	gd->bd->bi_memsize = FSP2_RAM_SIZE;
	return 0;
}

/* perform memory initialization here.  scrub/fill as required */
int dram_init(void)
{
	long dram_size = FSP2_RAM_SIZE;
	unsigned long start = 64*1024; /* skip exception vectors */
	unsigned long end = dram_size - 2*1024*1024; /* FIXME size of u-boot? */

	mem_init(start, end, NULL);
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, FSP2_RAM_SIZE);
	return 0;
}

void board_add_ram_info(int use_default)
{
	unsigned long val;

	val = mfcmu(CMU_MCCR) & MCCR_800MHZ;
	val ? puts(" DDR1600") : puts(" DDR1333");

	puts(", (ECC ");
	val = mfdcrx(MCIF_MCOPT1);
	if (!(val & MCOPT1_ECC_EN))
		puts("not ");

	puts("enabled)\n");
}
