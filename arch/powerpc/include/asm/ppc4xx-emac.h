/*
 * SPDX-License-Identifier:	GPL-2.0	IBM-pibs
 */
/*----------------------------------------------------------------------------+
|
|  File Name:	enetemac.h
|
|  Function:	Header file for the EMAC3 macro on the 405GP.
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

#ifndef _PPC4XX_ENET_H_
#define _PPC4XX_ENET_H_

#include <net.h>
#include "asm/ppc4xx-mal.h"


/*-----------------------------------------------------------------------------+
| General enternet defines.  802 frames are not supported.
+-----------------------------------------------------------------------------*/
#define ENET_ADDR_LENGTH		6
#define ENET_ARPTYPE			0x806
#define ARP_REQUEST			1
#define ARP_REPLY			2
#define ENET_IPTYPE			0x800
#define ARP_CACHE_SIZE			5

#define NUM_TX_BUFF 1
#define NUM_RX_BUFF PKTBUFSRX

struct enet_frame {
   unsigned char	dest_addr[ENET_ADDR_LENGTH];
   unsigned char	source_addr[ENET_ADDR_LENGTH];
   unsigned short	type;
   unsigned char	enet_data[1];
};

struct arp_entry {
   unsigned long	inet_address;
   unsigned char	mac_address[ENET_ADDR_LENGTH];
   unsigned long	valid;
   unsigned long	sec;
   unsigned long	nsec;
};


/* Statistic Areas */
#define MAX_ERR_LOG 10

typedef struct emac_stats_st{	/* Statistic Block */
	int data_len_err;
	int rx_frames;
	int rx;
	int rx_prot_err;
	int int_err;
	int pkts_tx;
	int pkts_rx;
	int pkts_handled;
	short tx_err_log[MAX_ERR_LOG];
	short rx_err_log[MAX_ERR_LOG];
} EMAC_STATS_ST, *EMAC_STATS_PST;

/* Structure containing variables used by the shared code (4xx_enet.c) */
typedef struct emac_4xx_hw_st {
    uint32_t		hw_addr;		/* EMAC offset */
    uint32_t		tah_addr;		/* TAH offset */
    uint32_t		phy_id;
    uint32_t		phy_addr;
    uint32_t		original_fc;
    uint32_t		txcw;
    uint32_t		autoneg_failed;
    uint32_t		emac_ier;
    volatile mal_desc_t *tx;
    volatile mal_desc_t *rx;
    u32			tx_phys;
    u32			rx_phys;
    bd_t		*bis;	/* for eth_init upon mal error */
    mal_desc_t		*alloc_tx_buf;
    mal_desc_t		*alloc_rx_buf;
    char		*txbuf_ptr;
    uint16_t		devnum;
    int			get_link_status;
    int			tbi_compatibility_en;
    int			tbi_compatibility_on;
    int			fc_send_xon;
    int			report_tx_early;
    int			first_init;
    int			tx_err_index;
    int			rx_err_index;
    int			rx_slot;	/* MAL Receive Slot */
    int			rx_i_index;	/* Receive Interrupt Queue Index */
    int			rx_u_index;	/* Receive User Queue Index */
    int			tx_slot;	/* MAL Transmit Slot */
    int			tx_i_index;	/* Transmit Interrupt Queue Index */
    int			tx_u_index;		/* Transmit User Queue Index */
    int			rx_ready[NUM_RX_BUFF];	/* Receive Ready Queue */
    int			tx_run[NUM_TX_BUFF];	/* Transmit Running Queue */
    int			is_receiving;	/* sync with eth interrupt */
    int			print_speed;	/* print speed message upon start */
    EMAC_STATS_ST	stats;
} EMAC_4XX_HW_ST, *EMAC_4XX_HW_PST;


#define EMAC_NUM_DEV		2
#ifdef CONFIG_IBM_EMAC4_V4	/* EMAC4 V4 changed bit setting */
#define EMAC_STACR_OC_MASK	(0x00008000)
#else
#define EMAC_STACR_OC_MASK	(0x00000000)
#endif

/*
 * XMII bridge configurations for those systems (e.g. 405EX(r)) that do
 * not have a pin function control (PFC) register to otherwise determine
 * the bridge configuration.
 */
#define EMAC_PHY_MODE_NONE		0
#define EMAC_PHY_MODE_NONE_RGMII	1
#define EMAC_PHY_MODE_RGMII_NONE	2
#define EMAC_PHY_MODE_RGMII_RGMII	3
#define EMAC_PHY_MODE_NONE_GMII		4
#define EMAC_PHY_MODE_GMII_NONE		5
#define EMAC_PHY_MODE_NONE_MII		6
#define EMAC_PHY_MODE_MII_NONE		7

/* ZMII Bridge Register addresses */
#define ZMII0_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0780)
#define ZMII0_FER		(ZMII0_BASE)
#define ZMII0_SSR		(ZMII0_BASE + 4)
#define ZMII0_SMIISR		(ZMII0_BASE + 8)

/* ZMII FER Register Bit Definitions */
#define ZMII_FER_DIS		(0x0)
#define ZMII_FER_MDI		(0x8)
#define ZMII_FER_SMII		(0x4)
#define ZMII_FER_RMII		(0x2)
#define ZMII_FER_MII		(0x1)
#define ZMII_FER_RSVD11		(0x00200000)
#define ZMII_FER_RSVD10		(0x00100000)
#define ZMII_FER_RSVD14_31	(0x0003FFFF)
#define ZMII_FER_V(__x)		(((3 - __x) * 4) + 16)


/* ZMII Speed Selection Register Bit Definitions */
#define ZMII0_SSR_SCI		(0x4)
#define ZMII0_SSR_FSS		(0x2)
#define ZMII0_SSR_SP		(0x1)
#define ZMII0_SSR_RSVD16_31	(0x0000FFFF)
#define ZMII0_SSR_V(__x)		(((3 - __x) * 4) + 16)


/* ZMII SMII Status Register Bit Definitions */
#define ZMII0_SMIISR_E1		(0x80)
#define ZMII0_SMIISR_EC		(0x40)
#define ZMII0_SMIISR_EN		(0x20)
#define ZMII0_SMIISR_EJ		(0x10)
#define ZMII0_SMIISR_EL		(0x08)
#define ZMII0_SMIISR_ED		(0x04)
#define ZMII0_SMIISR_ES		(0x02)
#define ZMII0_SMIISR_EF		(0x01)
#define ZMII0_SMIISR_V(__x)	((3 - __x) * 8)

/* RGMII Register Addresses */
#define RGMII_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x600)
#define RGMII_FER		(RGMII_BASE + 0x00)
#define RGMII_SSR		(RGMII_BASE + 0x04)

/* RGMII Function Enable (FER) Register Bit Definitions */
#define RGMII_FER_DIS		(0x00)
#define RGMII_FER_RTBI		(0x04)
#define RGMII_FER_RGMII		(0x05)
#define RGMII_FER_TBI		(0x06)
#define RGMII_FER_GMII		(0x07)
#define RGMII_FER_MII		(RGMII_FER_GMII)
#define RGMII_FER_V(__x)	((__x - 2) * 4)
#define RGMII_FER_MDIO(__x)	(1 << (19 - (__x)))

/* RGMII Speed Selection Register Bit Definitions */
#define RGMII_SSR_SP_10MBPS	(0x01)
#define RGMII_SSR_SP_100MBPS	(0x02)
#define RGMII_SSR_SP_1000MBPS	(0x04)
#define RGMII_SSR_V(__x)	((__x -2) * 8)

/* Ethernet MAC Regsiter Addresses */
#define EMAC0_BASE		(CONFIG_SYS_PERIPHERAL_BASE)
#define EMAC0_MR0		(EMAC0_BASE)
#define EMAC0_MR1		(EMAC0_BASE + 0x04)
#define EMAC0_TMR0		(EMAC0_BASE + 0x08)
#define EMAC0_TMR1		(EMAC0_BASE + 0x0c)
#define EMAC0_RXM		(EMAC0_BASE + 0x10)
#define EMAC0_ISR		(EMAC0_BASE + 0x14)
#define EMAC0_IER		(EMAC0_BASE + 0x18)
#define EMAC0_IAH		(EMAC0_BASE + 0x1c)
#define EMAC0_IAL		(EMAC0_BASE + 0x20)
#define EMAC0_PTR		(EMAC0_BASE + 0x2c)
#define EMAC0_PAUSE_TIME_REG	EMAC0_PTR
#define EMAC0_IPGVR		(EMAC0_BASE + 0x58)
#define EMAC0_I_FRAME_GAP_REG	EMAC0_IPGVR
#define EMAC0_STACR		(EMAC0_BASE + 0x5c)
#define EMAC0_TRTR		(EMAC0_BASE + 0x60)
#define EMAC0_RWMR		(EMAC0_BASE + 0x64)
#define EMAC0_RX_HI_LO_WMARK	EMAC0_RWMR

/* bit definitions */
/* MODE REG 0 */
#define EMAC_MR0_RXI		(0x80000000)
#define EMAC_MR0_TXI		(0x40000000)
#define EMAC_MR0_SRST		(0x20000000)
#define EMAC_MR0_TXE		(0x10000000)
#define EMAC_MR0_RXE		(0x08000000)
#define EMAC_MR0_WKE		(0x04000000)
#define EMAC_MR1_FDE		0x80000000
#define EMAC_MR1_ILE		0x40000000
#define EMAC_MR1_VLE		0x20000000
#define EMAC_MR1_EIFC		0x10000000
#define EMAC_MR1_APP		0x08000000
#define EMAC_MR1_AEMI		0x02000000
#define EMAC_MR1_IST		0x01000000
#define EMAC_MR1_MF_1000MBPS	0x00800000	/* 0's for 10MBPS */
#define EMAC_MR1_MF_100MBPS	0x00400000
#define EMAC_MR1_RFS_MASK	0x00300000
#define EMAC_MR1_RFS_4K		0x00300000
#define EMAC_MR1_RFS_2K		0x00200000
#define EMAC_MR1_RFS_1K		0x00100000
#define EMAC_MR1_RFS_512		0x00000000
#define EMAC_MR1_TX_FIFO_MASK	0x000c0000
#define EMAC_MR1_TX_FIFO_2K	0x00080000
#define EMAC_MR1_TX_FIFO_1K	0x00040000
#define EMAC_MR1_TX_FIFO_512	0x00000000
#define EMAC_MR1_TR0_DEPEND	0x00010000	/* 0'x for single packet */
#define EMAC_MR1_TR0_MULTI	0x00008000
#define EMAC_MR1_TR1_DEPEND	0x00004000
#define EMAC_MR1_TR1_MULTI	0x00002000

#define EMAC_MR1_FIFO_MASK	(EMAC_MR1_RFS_MASK | EMAC_MR1_TX_FIFO_MASK)
/* Set receive fifo to 4k and tx fifo to 2k */
#define EMAC_MR1_FIFO_SIZE	(EMAC_MR1_RFS_4K | EMAC_MR1_TX_FIFO_2K)

/* Transmit Mode Register 0 */
#define EMAC_TMR0_GNP0		(0x80000000)
#define EMAC_TMR0_GNP1		(0x40000000)
#define EMAC_TMR0_GNPD		(0x20000000)
#define EMAC_TMR0_FC		(0x10000000)

/* Receive Mode Register */
#define EMAC_RMR_SP		(0x80000000)
#define EMAC_RMR_SFCS		(0x40000000)
#define EMAC_RMR_ARRP		(0x20000000)
#define EMAC_RMR_ARP		(0x10000000)
#define EMAC_RMR_AROP		(0x08000000)
#define EMAC_RMR_ARPI		(0x04000000)
#define EMAC_RMR_PPP		(0x02000000)
#define EMAC_RMR_PME		(0x01000000)
#define EMAC_RMR_PMME		(0x00800000)
#define EMAC_RMR_IAE		(0x00400000)
#define EMAC_RMR_MIAE		(0x00200000)
#define EMAC_RMR_BAE		(0x00100000)
#define EMAC_RMR_MAE		(0x00080000)

/* Interrupt Status & enable Regs */
#define EMAC_ISR_OVR		(0x02000000)
#define EMAC_ISR_PP		(0x01000000)
#define EMAC_ISR_BP		(0x00800000)
#define EMAC_ISR_RP		(0x00400000)
#define EMAC_ISR_SE		(0x00200000)
#define EMAC_ISR_SYE		(0x00100000)
#define EMAC_ISR_BFCS		(0x00080000)
#define EMAC_ISR_PTLE		(0x00040000)
#define EMAC_ISR_ORE		(0x00020000)
#define EMAC_ISR_IRE		(0x00010000)
#define EMAC_ISR_DBDM		(0x00000200)
#define EMAC_ISR_DB0		(0x00000100)
#define EMAC_ISR_SE0		(0x00000080)
#define EMAC_ISR_TE0		(0x00000040)
#define EMAC_ISR_DB1		(0x00000020)
#define EMAC_ISR_SE1		(0x00000010)
#define EMAC_ISR_TE1		(0x00000008)
#define EMAC_ISR_MOS		(0x00000002)
#define EMAC_ISR_MOF		(0x00000001)

/* STA CONTROL REG */
#define EMAC_STACR_OC		(0x00008000)
#define EMAC_STACR_PHYE		(0x00004000)

#ifdef CONFIG_IBM_EMAC4_V4	/* EMAC4 V4 changed bit setting */
#define EMAC_STACR_INDIRECT_MODE (0x00002000)
#define EMAC_STACR_WRITE	(0x00000800) /* $BUC */
#define EMAC_STACR_READ		(0x00001000) /* $BUC */
#define EMAC_STACR_OP_MASK	(0x00001800)
#define EMAC_STACR_MDIO_ADDR	(0x00000000)
#define EMAC_STACR_MDIO_WRITE	(0x00000800)
#define EMAC_STACR_MDIO_READ	(0x00001800)
#define EMAC_STACR_MDIO_READ_INC (0x00001000)
#else
#define EMAC_STACR_WRITE	(0x00002000)
#define EMAC_STACR_READ		(0x00001000)
#endif

#define EMAC_STACR_CLK_83MHZ	(0x00000800)  /* 0's for 50Mhz */
#define EMAC_STACR_CLK_66MHZ	(0x00000400)
#define EMAC_STACR_CLK_100MHZ	(0x00000C00)

/* Transmit Request Threshold Register */
#define EMAC_TRTR_256		(0x18000000)   /* 0's for 64 Bytes */
#define EMAC_TRTR_192		(0x10000000)
#define EMAC_TRTR_128		(0x01000000)

/* the follwing defines are for the MadMAL status and control registers. */
/* For bits 0..5 look at the mal.h file					 */
#define EMAC_TX_CTRL_GFCS	(0x0200)
#define EMAC_TX_CTRL_GP		(0x0100)
#define EMAC_TX_CTRL_ISA	(0x0080)
#define EMAC_TX_CTRL_RSA	(0x0040)
#define EMAC_TX_CTRL_IVT	(0x0020)
#define EMAC_TX_CTRL_RVT	(0x0010)

#define EMAC_TX_CTRL_DEFAULT (EMAC_TX_CTRL_GFCS |EMAC_TX_CTRL_GP)

#define EMAC_TX_ST_BFCS		(0x0200)
#define EMAC_TX_ST_BPP		(0x0100)
#define EMAC_TX_ST_LCS		(0x0080)
#define EMAC_TX_ST_ED		(0x0040)
#define EMAC_TX_ST_EC		(0x0020)
#define EMAC_TX_ST_LC		(0x0010)
#define EMAC_TX_ST_MC		(0x0008)
#define EMAC_TX_ST_SC		(0x0004)
#define EMAC_TX_ST_UR		(0x0002)
#define EMAC_TX_ST_SQE		(0x0001)

#define EMAC_TX_ST_DEFAULT	(0x03F3)


/* madmal receive status / Control bits */

#define EMAC_RX_ST_OE		(0x0200)
#define EMAC_RX_ST_PP		(0x0100)
#define EMAC_RX_ST_BP		(0x0080)
#define EMAC_RX_ST_RP		(0x0040)
#define EMAC_RX_ST_SE		(0x0020)
#define EMAC_RX_ST_AE		(0x0010)
#define EMAC_RX_ST_BFCS		(0x0008)
#define EMAC_RX_ST_PTL		(0x0004)
#define EMAC_RX_ST_ORE		(0x0002)
#define EMAC_RX_ST_IRE		(0x0001)
/* all the errors we care about */
#define EMAC_RX_ERRORS		(0x03FF)

#endif /* _PPC4XX_ENET_H_ */
