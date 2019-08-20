// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/aspeed_scu_info.h>

extern int
aspeed_get_mac_phy_interface(u8 num)
{
	u32 strap1 = readl(ASPEED_HW_STRAP1);
#ifdef ASPEED_HW_STRAP2
	u32 strap2 = readl(ASPEED_HW_STRAP2);
#endif
	switch(num) {
		case 0:
			if(strap1 & BIT(6)) {
				return 1;
			} else {
				return 0;
			}
			break;
		case 1:
			if(strap1 & BIT(7)) {
				return 1;
			} else {
				return 0;
			}
			break;
#ifdef ASPEED_HW_STRAP2			
		case 2:
			if(strap2 & BIT(0)) {
				return 1;
			} else {
				return 0;
			}
			break;
		case 3:
			if(strap2 & BIT(1)) {
				return 1;
			} else {
				return 0;
			}
			break;
#endif			
	}
	return -1;
}

extern void
aspeed_security_info(void)
{
	if(readl(ASPEED_HW_STRAP1) & BIT(1))
		printf("Security Boot \n");
}	

/*	ASPEED_SYS_RESET_CTRL	: System reset contrl/status register*/
#define SYS_WDT4_SW_RESET		BIT(31)
#define SYS_WDT4_ARM_RESET		BIT(30)
#define SYS_WDT4_FULL_RESET		BIT(29)
#define SYS_WDT4_SOC_RESET		BIT(28)
#define SYS_WDT3_SW_RESET		BIT(27)
#define SYS_WDT3_ARM_RESET		BIT(26)
#define SYS_WDT3_FULL_RESET		BIT(25)
#define SYS_WDT3_SOC_RESET		BIT(24)
#define SYS_WDT2_SW_RESET		BIT(23)
#define SYS_WDT2_ARM_RESET		BIT(22)
#define SYS_WDT2_FULL_RESET		BIT(21)
#define SYS_WDT2_SOC_RESET		BIT(20)
#define SYS_WDT1_SW_RESET		BIT(19)
#define SYS_WDT1_ARM_RESET		BIT(18)
#define SYS_WDT1_FULL_RESET		BIT(17)
#define SYS_WDT1_SOC_RESET		BIT(16)

#define SYS_CM3_EXT_RESET		BIT(6)
#define SYS_PCI2_RESET			BIT(5)
#define SYS_PCI1_RESET			BIT(4)
#define SYS_DRAM_ECC_RESET		BIT(3)
#define SYS_FLASH_ABR_RESET		BIT(2)
#define SYS_EXT_RESET			BIT(1)
#define SYS_PWR_RESET_FLAG		BIT(0)

extern void 
aspeed_sys_reset_info(void)
{
	u32 rest = readl(ASPEED_SYS_RESET_CTRL);

	if (rest & SYS_PWR_RESET_FLAG) {
		printf("RST : Power On \n");
		writel(rest, ASPEED_SYS_RESET_CTRL);
	} else {

		if (rest & (SYS_WDT4_SOC_RESET | SYS_WDT4_FULL_RESET | SYS_WDT4_ARM_RESET | SYS_WDT4_SW_RESET)) {
			printf("RST : WDT4 ");
			if(rest & SYS_WDT4_FULL_RESET) {
				printf("FULL ");
				writel(SYS_WDT4_FULL_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT4_SOC_RESET) {
				printf("SOC ");
				writel(SYS_WDT4_SOC_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT4_ARM_RESET) {
				printf("ARM ");
				writel(SYS_WDT4_ARM_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT4_SW_RESET) {
				printf("SW ");
				writel(SYS_WDT4_SW_RESET, ASPEED_SYS_RESET_CTRL);
			}
			printf("\n");
		}

		if (rest & (SYS_WDT3_SOC_RESET | SYS_WDT3_FULL_RESET | SYS_WDT3_ARM_RESET | SYS_WDT3_SW_RESET)) {
			printf("RST : WDT3 ");
			if(rest & SYS_WDT3_FULL_RESET) {
				printf("FULL ");
				writel(SYS_WDT3_FULL_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT3_SOC_RESET) {
				printf("SOC ");
				writel(SYS_WDT3_SOC_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT3_ARM_RESET) {
				printf("ARM ");
				writel(SYS_WDT3_ARM_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT3_SW_RESET) {
				printf("SW ");
				writel(SYS_WDT3_SW_RESET, ASPEED_SYS_RESET_CTRL);
			}
			printf("\n");
		}

		if (rest & (SYS_WDT2_SOC_RESET | SYS_WDT2_FULL_RESET | SYS_WDT2_ARM_RESET | SYS_WDT2_SW_RESET)) {
			printf("RST : WDT2 ");
			if(rest & SYS_WDT2_FULL_RESET) {
				printf("FULL ");
				writel(SYS_WDT2_FULL_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT2_SOC_RESET) {
				printf("SOC ");
				writel(SYS_WDT2_SOC_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT2_ARM_RESET) {
				printf("ARM ");
				writel(SYS_WDT2_ARM_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT2_SW_RESET) {
				printf("SW ");
				writel(SYS_WDT2_SW_RESET, ASPEED_SYS_RESET_CTRL);
			}
			printf("\n");
		}

		if (rest & (SYS_WDT1_SOC_RESET | SYS_WDT1_FULL_RESET | SYS_WDT1_ARM_RESET | SYS_WDT1_SW_RESET)) {
			printf("RST : WDT1 ");
			if(rest & SYS_WDT1_FULL_RESET) {
				printf("FULL ");
				writel(SYS_WDT1_FULL_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT1_SOC_RESET) {
				printf("SOC ");
				writel(SYS_WDT1_SOC_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT1_ARM_RESET) {
				printf("ARM ");
				writel(SYS_WDT1_ARM_RESET, ASPEED_SYS_RESET_CTRL);
			}
			if(rest & SYS_WDT1_SW_RESET) {
				printf("SW ");
				writel(SYS_WDT1_SW_RESET, ASPEED_SYS_RESET_CTRL);
			}
			printf("\n");
		}

		if (rest & SYS_CM3_EXT_RESET) {
			printf("RST : SYS_CM3_EXT_RESET \n");
			writel(SYS_CM3_EXT_RESET, ASPEED_SYS_RESET_CTRL);		
		}
		
		if (rest & (SYS_PCI1_RESET | SYS_PCI2_RESET)) {
			printf("PCI RST : ");
			if (rest & SYS_PCI1_RESET) {
				printf("#1 ");
				writel(SYS_PCI1_RESET, ASPEED_SYS_RESET_CTRL);		
			}
			
			if (rest & SYS_PCI2_RESET) {
				printf("#2 ");
				writel(SYS_PCI2_RESET, ASPEED_SYS_RESET_CTRL);		
			}
			printf("\n");
		}

		if (rest & SYS_DRAM_ECC_RESET) {
			printf("RST : DRAM_ECC_RESET \n");
			writel(SYS_FLASH_ABR_RESET, ASPEED_SYS_RESET_CTRL);		
		}

		if (rest & SYS_FLASH_ABR_RESET) {
			printf("RST : SYS_FLASH_ABR_RESET \n");
			writel(SYS_FLASH_ABR_RESET, ASPEED_SYS_RESET_CTRL);		
		}
		if (rest & SYS_EXT_RESET) {
			printf("RST : External \n");
			writel(SYS_EXT_RESET, ASPEED_SYS_RESET_CTRL);
		}	
	}
}

#define SOC_FW_INIT_DRAM		BIT(7)

extern void
aspeed_who_init_dram(void)
{
	if(readl(ASPEED_VGA_HANDSHAKE0) & SOC_FW_INIT_DRAM)
		printf("[init by SOC]\n");
	else
		printf("[init by VBIOS]\n");
}

extern void
aspeed_2nd_wdt_mode(void)
{
	if(readl(ASPEED_HW_STRAP2) & BIT(11)) {
		printf("2nd Boot : Enable, ");
		if(readl(ASPEED_HW_STRAP2) & BIT(12))
			printf("Single SPI ");
		else
			printf("Dual SPI ");
		printf("= %s \n", readl(0x1e620064) & BIT(4) ? "Alternate":"Primary");
	}
}

extern void
aspeed_spi_strap_mode(void)
{
	if(readl(ASPEED_HW_STRAP2) & BIT(10))
		printf("SPI : 3/4 byte mode auto detection \n");
}

extern void
aspeed_espi_mode(void)
{
	int espi_mode = 0;
	int sio_disable = 0;
	u32 sio_addr = 0x2e;

	if(readl(ASPEED_HW_STRAP2) & BIT(6))
		espi_mode = 0;
	else
		espi_mode = 1;

	if(readl(ASPEED_HW_STRAP2) & BIT(2))
		sio_addr = 0x4e;

	if(readl(ASPEED_HW_STRAP2) & BIT(3))
		sio_disable = 1;

	if(espi_mode)
		printf("eSPI Mode : SIO:%s ",  sio_disable ? "Disable" : "Enable");
	else
		printf("LPC Mode : SIO:%s ",  sio_disable ? "Disable" : "Enable");

	if(!sio_disable)
		printf(": SuperIO-%02x\n",  sio_addr);
	else
		printf("\n");
}

