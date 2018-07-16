/*
 *  Copyright (C) 2002 Scott McNutt <smcnutt@artesyncp.com>
 *
 * (C) Copyright 2008-2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PPC4xx_UIC_H_
#define _PPC4xx_UIC_H_

/*
 * Define the number of UIC's
 */
#if defined(CONFIG_47x)
#define UIC_MAX		2
#else
#define UIC_MAX		1
#endif

#define IRQ_MAX		(UIC_MAX * 32)

/*
 * UIC register
 */
#define UIC_SR	0x0			/* UIC status			*/
#define UIC_ER	0x2			/* UIC enable			*/
#define UIC_CR	0x3			/* UIC critical			*/
#define UIC_PR	0x4			/* UIC polarity			*/
#define UIC_TR	0x5			/* UIC triggering		*/
#define UIC_MSR 0x6			/* UIC masked status		*/
#define UIC_VR	0x7			/* UIC vector			*/
#define UIC_VCR 0x8			/* UIC vector configuration	*/

/*
 * On 440GX we use the UICB0 as UIC0. Its the root UIC where all other UIC's
 * are cascaded on. With this trick we can use the common UIC code for 440GX
 * too.
 */
#if defined(CONFIG_47x)
#define UIC0_DCR_BASE 0x2c0
#define UIC1_DCR_BASE 0x358
#define UIC2_DCR_BASE 0x000
#define UIC3_DCR_BASE 0x000
#else
#define UIC0_DCR_BASE 0xc0
#define UIC1_DCR_BASE 0xd0
#define UIC2_DCR_BASE 0xe0
#define UIC3_DCR_BASE 0xf0
#endif

#define UIC0SR	(UIC0_DCR_BASE+0x0)	/* UIC0 status			*/
#define UIC0ER	(UIC0_DCR_BASE+0x2)	/* UIC0 enable			*/
#define UIC0CR	(UIC0_DCR_BASE+0x3)	/* UIC0 critical		*/
#define UIC0PR	(UIC0_DCR_BASE+0x4)	/* UIC0 polarity		*/
#define UIC0TR	(UIC0_DCR_BASE+0x5)	/* UIC0 triggering		*/
#define UIC0MSR (UIC0_DCR_BASE+0x6)	/* UIC0 masked status		*/
#define UIC0VR	(UIC0_DCR_BASE+0x7)	/* UIC0 vector			*/
#define UIC0VCR (UIC0_DCR_BASE+0x8)	/* UIC0 vector configuration	*/

#define UIC1SR	(UIC1_DCR_BASE+0x0)	/* UIC1 status			*/
#define UIC1ER	(UIC1_DCR_BASE+0x2)	/* UIC1 enable			*/
#define UIC1CR	(UIC1_DCR_BASE+0x3)	/* UIC1 critical		*/
#define UIC1PR	(UIC1_DCR_BASE+0x4)	/* UIC1 polarity		*/
#define UIC1TR	(UIC1_DCR_BASE+0x5)	/* UIC1 triggering		*/
#define UIC1MSR (UIC1_DCR_BASE+0x6)	/* UIC1 masked status		*/
#define UIC1VR	(UIC1_DCR_BASE+0x7)	/* UIC1 vector			*/
#define UIC1VCR (UIC1_DCR_BASE+0x8)	/* UIC1 vector configuration	*/

#define UIC2SR	(UIC2_DCR_BASE+0x0)	/* UIC2 status-Read Clear	*/
#define UIC2ER	(UIC2_DCR_BASE+0x2)	/* UIC2 enable			*/
#define UIC2CR	(UIC2_DCR_BASE+0x3)	/* UIC2 critical		*/
#define UIC2PR	(UIC2_DCR_BASE+0x4)	/* UIC2 polarity		*/
#define UIC2TR	(UIC2_DCR_BASE+0x5)	/* UIC2 triggering		*/
#define UIC2MSR (UIC2_DCR_BASE+0x6)	/* UIC2 masked status		*/
#define UIC2VR	(UIC2_DCR_BASE+0x7)	/* UIC2 vector			*/
#define UIC2VCR (UIC2_DCR_BASE+0x8)	/* UIC2 vector configuration	*/

#define UIC3SR	(UIC3_DCR_BASE+0x0)	/* UIC3 status-Read Clear	*/
#define UIC3ER	(UIC3_DCR_BASE+0x2)	/* UIC3 enable			*/
#define UIC3CR	(UIC3_DCR_BASE+0x3)	/* UIC3 critical		*/
#define UIC3PR	(UIC3_DCR_BASE+0x4)	/* UIC3 polarity		*/
#define UIC3TR	(UIC3_DCR_BASE+0x5)	/* UIC3 triggering		*/
#define UIC3MSR (UIC3_DCR_BASE+0x6)	/* UIC3 masked status		*/
#define UIC3VR	(UIC3_DCR_BASE+0x7)	/* UIC3 vector			*/
#define UIC3VCR (UIC3_DCR_BASE+0x8)	/* UIC3 vector configuration	*/

/*
 * Now the interrupt vector definitions. They are different for most of
 * the 4xx variants, so we need some more #ifdef's here. No mask
 * definitions anymore here. For this please use the UIC_MASK macro below.
 *
 * Note: Please only define the interrupts really used in U-Boot here.
 * Those are the cascading and EMAC/MAL related interrupt.
 */

#if !defined(VECNUM_ETH1_OFFS)
#define VECNUM_ETH1_OFFS	1
#endif

#if defined(CONFIG_47x)
#define VECNUM_UIC1_2C 6
#define VECNUM_UIC1_2NC 23
#define VECNUM_UIC1CI VECNUM_UIC1_2C
#define VECNUM_UIC1NCI VECNUM_UIC1_2NC
#define VECNUM_ETH0_WAKE (32 + 0)
#define VECNUM_ETH0            (32 + 1)
#define VECNUM_E0MIB           (32 + 2)
#define VECNUM_MAL_RXEOB       (32 + 3)
#define VECNUM_MAL_TXEOB       (32 + 4)
#define VECNUM_MAL_RXDE                (32 + 5)
#define VECNUM_MAL_TXDE                (32 + 6)
#define VECNUM_MAL_SERR                (32 + 7)
#define VECNUM_ETH1_WAKE       (32 + 8)
#define VECNUM_ETH1            (32 + 9)
#define VECNUM_E1MIB           (32 + 10)
#define VECNUM_MAL1_RXEOB      (32 + 11)
#define VECNUM_MAL1_TXEOB      (32 + 12)
#define VECNUM_MAL1_RXDE       (32 + 13)
#define VECNUM_MAL1_TXDE       (32 + 14)
#define VECNUM_MAL1_SERR       (32 + 15)
#endif

/*
 * Mask definitions (used for example in 4xx_enet.c)
 */
#define UIC_MASK(vec)		(0x80000000 >> ((vec) & 0x1f))
/* UIC_NR won't work for 440GX because of its specific UIC DCR addresses */
#define UIC_NR(vec)		((vec) >> 5)

#endif /* _PPC4xx_UIC_H_ */
