// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */
#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

#define AST_GPIO_BASE		(0x1E780000)
#define AST_GPIOABCD_DRCTN	(AST_GPIO_BASE + 0x004)
#define AST_GPIOEFGH_DRCTN	(AST_GPIO_BASE + 0x024)
#define AST_GPIOMNOP_DRCTN	(AST_GPIO_BASE + 0x07C)
#define AST_GPIOUVWX_DRCTN	(AST_GPIO_BASE + 0x08C)
#define AST_GPIOYZ_DRCTN	(AST_GPIO_BASE + 0x1E4)

int board_init(void)
{
	u32 direction;

	struct udevice *dev;
	int i;
	int ret;

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

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

	/* 
	 * set 32 GPIO ouput pins for ATE report 
	 *   GPIOV[7:0] -> ATE[7:0]
	 *   GPIOY[3:0] -> ATE[11:8]
	 *   GPIOM[3:0] -> ATE[15:12]
	 *   GPIOH[3:0] -> ATE[19:16]
	 *   GPIOB[3:0] -> ATE[23:20]
	 *   GPIOM[5:4] -> ATE[25:24]
	 *   GPION[5:0] -> ATE[31:26]
	 */
	/* GPIOB[3:0] */
	direction = readl(AST_GPIOABCD_DRCTN);
	direction |= 0xF00;
	writel(direction, AST_GPIOABCD_DRCTN);

	/* GPIOH[3:0] */
	direction = readl(AST_GPIOEFGH_DRCTN);
	direction |= 0xF000000;
	writel(direction, AST_GPIOEFGH_DRCTN);

	/* GPIOM[3:0], GPIOM[5:4], GPION[5:0] */
	direction = readl(AST_GPIOMNOP_DRCTN);
	direction |= 0x3F3F;
	writel(direction, AST_GPIOMNOP_DRCTN);

	/* GPIOV[7:0] */
	direction = readl(AST_GPIOUVWX_DRCTN);
	direction |= 0xFF00;
	writel(direction, AST_GPIOUVWX_DRCTN);

	/* GPIOY[3:0] */
	direction = readl(AST_GPIOYZ_DRCTN);
	direction |= 0xF;
	writel(direction, AST_GPIOYZ_DRCTN);

	return 0;
}
