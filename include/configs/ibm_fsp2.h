/*
 * Copyright (c) 2017 IBM
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/ppc476fsp2.h>

#define CONFIG_SYS_CBSIZE	2048
#define CONFIG_AUTO_COMPLETE

#define CONFIG_WATCHDOG
#define CONFIG_SYS_RAMBOOT
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_SYS_CLK_FREQ		33333333

/* DRAM */
#define CONFIG_SYS_INIT_RAM_ADDR	0x00100000
#define CONFIG_SYS_INIT_RAM_SIZE	0x8000
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xfc000000

/* network */
#define CONFIG_MII
#define CONFIG_PHY_GIGE

#define CONFIG_HAS_ETH1
#define CONFIG_ETHADDR          00:01:73:01:c9:33
#define CONFIG_ETH1ADDR         00:01:73:01:c9:24
#define CONFIG_PHY_ADDR         1       /* PHY address                  */
#define CONFIG_PHY1_ADDR        2       /* EMAC1 PHY address            */

#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#if defined(CONFIG_SYS_RAMBOOT)
#define CONFIG_SYS_TEXT_BASE		0x10000000
#define CONFIG_SYS_LDSCRIPT		"board/ibm/fsp2/u-boot-ram.lds"
#else
#define CONFIG_SYS_TEXT_BASE		0xFFFC0000
#endif

#define CONFIG_SYS_LOAD_ADDR		0x11000000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE	/* Start of U-Boot */
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)		/* Reserve 256kB for monitor */
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)		/* Reserved for malloc */

#define CONFIG_SYS_GBL_DATA_OFFSET	\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_MEM_TOP_HIDE 0x30000000

#define FSP2_RAM_SIZE		0x40000000

#define CONFIG_SYS_BOOTMAPSZ (64 << 20)
#define CONFIG_SYS_BOOTM_LEN (64 << 20)

#define CONFIG_EXTRA_ENV_SETTINGS	\
        "console=ttyS0,115200\0"		\
	"eth0=bcm5461\0" \
	"eth1=bcm5461\0" \
	"ethaddr=00:01:73:01:c9:33\0"	\
	"eth1addr=00:01:73:01:c9:33\0"	\
	"ipaddr=192.168.1.20\0"		\
	"serverip=192.168.1.25\0"		\
	"netmask=255.255.255.0\0"		\
	"ubootaddr=" __stringify(CONFIG_SYS_TEXT_BASE) "\0"

#endif
