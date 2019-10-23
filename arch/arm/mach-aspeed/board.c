// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 */
#include <common.h>
#include <dm.h>
#include <ram.h>
#include <timer.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <linux/err.h>
#include <dm/uclass.h>

/*
 * Second Watchdog Timer by default is configured
 * to trigger secondary boot source.
 */
#define AST_2ND_BOOT_WDT		1

/*
 * Third Watchdog Timer by default is configured
 * to toggle Flash address mode switch before reset.
 */
#define AST_FLASH_ADDR_DETECT_WDT	2

/*
 * RMII daughtercard workaround
 */
//#define ASPEED_RMII_DAUGHTER_CARD

DECLARE_GLOBAL_DATA_PTR;

#if 0
void lowlevel_init(void)
{
	/*
	 * These two watchdogs need to be stopped as soon as possible,
	 * otherwise the board might hang. By default they are set to
	 * a very short timeout and even simple debug write to serial
	 * console early in the init process might cause them to fire.
	 */
	struct ast_wdt *flash_addr_wdt =
	    (struct ast_wdt *)(WDT_BASE +
			       sizeof(struct ast_wdt) *
			       AST_FLASH_ADDR_DETECT_WDT);

	clrbits_le32(&flash_addr_wdt->ctrl, WDT_CTRL_EN);

#ifndef CONFIG_FIRMWARE_2ND_BOOT
	struct ast_wdt *sec_boot_wdt =
	    (struct ast_wdt *)(WDT_BASE +
			       sizeof(struct ast_wdt) *
			       AST_2ND_BOOT_WDT);

	clrbits_le32(&sec_boot_wdt->ctrl, WDT_CTRL_EN);
#endif
}
#endif

#ifdef ASPEED_RMII_DAUGHTER_CARD
/**
 * @brief	workaround for RMII daughtercard, reset PHY manually
 * 
 * workaround for Aspeed RMII daughtercard, reset Eth PHY by GPO F0 and F2
 * Where GPO F0 controls the reset signal of RMII PHY 1 and 2.
 * Where GPO F2 controls the reset signal of RMII PHY 3 and 4.
*/
void reset_eth_phy(void)
{
#define GRP_F		8
#define PHY_RESET_MASK  (BIT(GRP_F + 0) | BIT(GRP_F + 2))	

	u32 value = readl(0x1e780020);
	u32 direction = readl(0x1e780024);

	debug("RMII workaround: reset PHY manually\n");

	direction |= PHY_RESET_MASK;
	value &= ~PHY_RESET_MASK;
	writel(direction, 0x1e780024);
	writel(value, 0x1e780020);
	while((readl(0x1e780020) & PHY_RESET_MASK) != 0);

	udelay(1000);

	value |= PHY_RESET_MASK;
	writel(value, 0x1e780020);
	while((readl(0x1e780020) & PHY_RESET_MASK) != PHY_RESET_MASK);
}
#endif

int board_init(void)
{
	struct udevice *dev;
	int i;
	int ret;

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef ASPEED_RMII_DAUGHTER_CARD
	reset_eth_phy();
#endif	
	/*
	 * Loop over all MISC uclass drivers to call the comphy code
	 * and init all CP110 devices enabled in the DT
	 */
	i = 0;
	while (1) {
		/* Call the comphy code via the MISC uclass driver */
		ret = uclass_get_device(UCLASS_MISC, i++, &dev);

		/* We're done, once no further CP110 device is found */
		if (ret)
			break;
	}

#if 0
	if (!dev) 
		printf("No MISC found.\n");
#endif
	return 0;
}

int dram_init(void)
{
	struct udevice *dev;
	struct ram_info ram;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM FAIL1\r\n");
		return ret;
	}

	ret = ram_get_info(dev, &ram);
	if (ret) {
		debug("DRAM FAIL2\r\n");
		return ret;
	}

	gd->ram_size = ram.size;
	return 0;
}

int arch_early_init_r(void)
{
#ifdef CONFIG_DM_PCI
	/* Trigger PCIe devices detection */
	pci_init();
#endif

	return 0;
}

