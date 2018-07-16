/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PPC4xx_ISRAM_H_
#define _PPC4xx_ISRAM_H_

/*
 * Internal SRAM
 */
#define ISRAM0_DCR_BASE 0x020
#define ISRAM0_SB0CR	(ISRAM0_DCR_BASE+0x00)	/* SRAM bank config 0*/
#define ISRAM0_SB1CR	(ISRAM0_DCR_BASE+0x01)	/* SRAM bank config 1*/
#define ISRAM0_SB2CR	(ISRAM0_DCR_BASE+0x02)	/* SRAM bank config 2*/
#define ISRAM0_SB3CR	(ISRAM0_DCR_BASE+0x03)	/* SRAM bank config 3*/
#define ISRAM0_BEAR	(ISRAM0_DCR_BASE+0x04)	/* SRAM bus error addr reg */
#define ISRAM0_BESR0	(ISRAM0_DCR_BASE+0x05)	/* SRAM bus error status reg 0 */
#define ISRAM0_BESR1	(ISRAM0_DCR_BASE+0x06)	/* SRAM bus error status reg 1 */
#define ISRAM0_PMEG	(ISRAM0_DCR_BASE+0x07)	/* SRAM power management */
#define ISRAM0_CID	(ISRAM0_DCR_BASE+0x08)	/* SRAM bus core id reg */
#define ISRAM0_REVID	(ISRAM0_DCR_BASE+0x09)	/* SRAM bus revision id reg */
#define ISRAM0_DPC	(ISRAM0_DCR_BASE+0x0a)	/* SRAM data parity check reg */

#endif /* _PPC4xx_ISRAM_H_ */
