/* include/mal.h, openbios_walnut, walnut_bios 8/6/99 08:48:40 */
/*
 * SPDX-License-Identifier:	GPL-2.0	IBM-pibs
 */
/*----------------------------------------------------------------------------+
|
|  File Name:	mal.h
|
|  Function:	Header file for the MAL (MADMAL) macro on the 405GP.
|
|  Author:	Mark Wisner
|
|  Change Activity-
|
|  Date	       Description of Change					   BY
|  ---------   ---------------------					   ---
|  29-Apr-99   Created							   MKW
|
+----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
|  17-Nov-03  Travis B. Sawyer, Sandburst Corporation, tsawyer@sandburst.com
|	      Added register bit definitions to support multiple channels
+----------------------------------------------------------------------------*/
#ifndef _mal_h_
#define _mal_h_

#if !defined(MAL_DCR_BASE)
#define MAL_DCR_BASE	0x180
#endif
#define MAL0_CFG	(MAL_DCR_BASE + 0x00)	/* MAL Config reg	*/
#define MAL0_ESR	(MAL_DCR_BASE + 0x01)	/* Error Status (Read/Clear) */
#define MAL0_IER	(MAL_DCR_BASE + 0x02)	/* Interrupt enable */
#define MAL0_TXCASR	(MAL_DCR_BASE + 0x04)	/* TX Channel active (set) */
#define MAL0_TXCARR	(MAL_DCR_BASE + 0x05)	/* TX Channel active (reset) */
#define MAL0_TXEOBISR	(MAL_DCR_BASE + 0x06)	/* TX End of buffer int status*/
#define MAL0_TXDEIR	(MAL_DCR_BASE + 0x07)	/* TX Descr. Error Int */
#define MAL0_TXBADDR	(MAL_DCR_BASE + 0x09)	/* TX descriptor base addr*/
#define MAL0_RXCASR	(MAL_DCR_BASE + 0x10)	/* RX Channel active (set) */
#define MAL0_RXCARR	(MAL_DCR_BASE + 0x11)	/* RX Channel active (reset) */
#define MAL0_RXEOBISR	(MAL_DCR_BASE + 0x12)	/* RX End of buffer int status*/
#define MAL0_RXDEIR	(MAL_DCR_BASE + 0x13)	/* RX Descr. Error Int */
#define MAL0_RXBADDR	(MAL_DCR_BASE + 0x15)	/* RX descriptor base addr */
#define MAL0_TXCTP0R	(MAL_DCR_BASE + 0x20)	/* TX 0 Channel table pointer */
#define MAL0_TXCTP1R	(MAL_DCR_BASE + 0x21)	/* TX 1 Channel table pointer */
#define MAL0_TXCTP2R	(MAL_DCR_BASE + 0x22)	/* TX 2 Channel table pointer */
#define MAL0_TXCTP3R	(MAL_DCR_BASE + 0x23)	/* TX 3 Channel table pointer */
#define MAL0_RXCTP0R	(MAL_DCR_BASE + 0x40)	/* RX 0 Channel table pointer */
#define MAL0_RXCTP1R	(MAL_DCR_BASE + 0x41)	/* RX 1 Channel table pointer */
#define MAL0_RCBS0	(MAL_DCR_BASE + 0x60)	/* RX 0 Channel buffer size */
#define MAL0_RCBS1	(MAL_DCR_BASE + 0x61)	/* RX 1 Channel buffer size */
/* MADMAL transmit and receive status/control bits  */
/* for COMMAC bits, refer to the COMMAC header file */

#define MAL_TX_CTRL_READY 0x8000
#define MAL_TX_CTRL_WRAP  0x4000
#define MAL_TX_CTRL_CM	  0x2000
#define MAL_TX_CTRL_LAST  0x1000
#define MAL_TX_CTRL_INTR  0x0400

#define MAL_RX_CTRL_EMPTY 0x8000
#define MAL_RX_CTRL_WRAP  0x4000
#define MAL_RX_CTRL_CM	  0x2000
#define MAL_RX_CTRL_LAST  0x1000
#define MAL_RX_CTRL_FIRST 0x0800
#define MAL_RX_CTRL_INTR  0x0400

      /* Configuration Reg  */
#define MAL_CR_MMSR	  0x80000000
#define MAL_CR_PLBP_1	  0x00400000   /* lowsest is 00 */
#define MAL_CR_PLBP_2	  0x00800000
#define MAL_CR_PLBP_3	  0x00C00000   /* highest	*/
#define MAL_CR_GA	  0x00200000
#define MAL_CR_OA	  0x00100000
#define MAL_CR_PLBLE	  0x00080000
#define MAL_CR_PLBLT_1	0x00040000
#define MAL_CR_PLBLT_2	0x00020000
#define MAL_CR_PLBLT_3	0x00010000
#define MAL_CR_PLBLT_4	0x00008000
#define MAL_CR_PLBLT_DEFAULT 0x00078000 /* ????? */
#define MAL_CR_PLBB	  0x00004000
#define MAL_CR_OPBBL	  0x00000080
#define MAL_CR_EOPIE	  0x00000004
#define MAL_CR_LEA	  0x00000002
#define MAL_CR_MSD	  0x00000001

    /* Error Status Reg	   */
#define MAL_ESR_EVB	  0x80000000
#define MAL_ESR_CID	  0x40000000
#define MAL_ESR_PLBTE     0x00800000
#define MAL_ESR_PLBRE     0x00400000
#define MAL_ESR_PLBWE     0x00200000
#define MAL_ESR_DE	  0x00100000
#define MAL_ESR_ONE	  0x00080000
#define MAL_ESR_OTE	  0x00040000
#define MAL_ESR_OSE	  0x00020000
#define MAL_ESR_PEIN	  0x00010000
      /* same bit position as the IER */
      /* VV			 VV   */
#define MAL_ESR_DEI	  0x00000010
#define MAL_ESR_ONEI	  0x00000008
#define MAL_ESR_OTEI	  0x00000004
#define MAL_ESR_OSEI	  0x00000002
#define MAL_ESR_PBEI	  0x00000001
      /* ^^			 ^^   */
      /* Mal IER		      */
#if defined(CONFIG_47x)
#define MAL_IER_PT	  0x00000080
#define MAL_IER_PRE	  0x00000040
#define MAL_IER_PWE	  0x00000020
#define MAL_IER_DE	  0x00000010
#define MAL_IER_OTE	  0x00000004
#define MAL_IER_OE	  0x00000002
#define MAL_IER_PE	  0x00000001
#else
#define MAL_IER_DE	  0x00000010
#define MAL_IER_NE	  0x00000008
#define MAL_IER_TE	  0x00000004
#define MAL_IER_OPBE	  0x00000002
#define MAL_IER_PLBE	  0x00000001
#endif

/* MAL Channel Active Set and Reset Registers */
#define MAL_TXRX_CASR	(0x80000000)

#define MAL_TXRX_CASR_V(__x)  (__x)  /* Channel 0 shifts 0, channel 1 shifts 1, etc */


/* MAL Buffer Descriptor structure */
typedef struct {
  short	 ctrl;		    /* MAL / Commac status control bits */
  short	 data_len;	    /* Max length is 4K-1 (12 bits)	*/
  char	*data_ptr;	    /* pointer to actual data buffer	*/
} mal_desc_t;

#endif
