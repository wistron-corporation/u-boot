/*
 * Copyright (c) International Business Machines Corp., 2007
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Device driver for the ethernet EMAC4 macro on the FSP-1 and
 *                                EMAC3 macro on the FSP-0.
 *
 * Based on 440gx_enet.h
 *
 * Author: Kirill Gorelov <kirill.gorelov@auriga.ru>
 * Reworked by: Frank Haverkamp <haver@vnet.ibm.com>
 */

#ifndef _FSP_ENET_H_
#define _FSP_ENET_H_

#include <config.h>
#include <net.h>
#include <miiphy.h>
#include <asm/ppc4xx-mal.h>

/*----------------------------------------------------------------------------+
  | General enternet defines.  802 frames are not supported.
  +--------------------------------------------------------------------------*/
#define ETH_MAGIC		0x07111947

#define ENET_ADDR_LENGTH	6
#define ENET_ARPTYPE		0x806
#define ARP_REQUEST		1
#define ARP_REPLY		2
#define ENET_IPTYPE		0x800
#define ARP_CACHE_SIZE		5

#define EMAC_NUM_DEV		2

#define NUM_TX_BUFF		2
#define NUM_RX_BUFF		PKTBUFSRX

struct enet_frame {
	unsigned char dest_addr[ENET_ADDR_LENGTH];
	unsigned char source_addr[ENET_ADDR_LENGTH];
	unsigned short type;
	unsigned char enet_data[1];
};

struct arp_entry {
	unsigned long inet_address;
	unsigned char mac_address[ENET_ADDR_LENGTH];
	unsigned long valid;
	unsigned long sec;
	unsigned long nsec;
};


/* EMAC4 Ethernet MAC Register Addresses */
#define EMAC_BASE		(CONFIG_SYS_PERIPHERAL_BASE)

#define EMAC_M0			(EMAC_BASE)
#define EMAC_M1			(EMAC_BASE + 4)
#define EMAC_TXM0		(EMAC_BASE + 8)
#define EMAC_TXM1		(EMAC_BASE + 12)
#define EMAC_RXM		(EMAC_BASE + 16)
#define EMAC_ISR		(EMAC_BASE + 20)
#define EMAC_IER		(EMAC_BASE + 24)
#define EMAC_IAH		(EMAC_BASE + 28)
#define EMAC_IAL		(EMAC_BASE + 32)
#define EMAC_VLAN_TPID_REG	(EMAC_BASE + 36)
#define EMAC_VLAN_TCI_REG	(EMAC_BASE + 40)
#define EMAC_PAUSE_TIME_REG	(EMAC_BASE + 44)
#define EMAC_IND_HASH_1		(EMAC_BASE + 48)
#define EMAC_IND_HASH_2		(EMAC_BASE + 52)
#define EMAC_IND_HASH_3		(EMAC_BASE + 56)
#define EMAC_IND_HASH_4		(EMAC_BASE + 60)
#define EMAC_GRP_HASH_1		(EMAC_BASE + 64)
#define EMAC_GRP_HASH_2		(EMAC_BASE + 68)
#define EMAC_GRP_HASH_3		(EMAC_BASE + 72)
#define EMAC_GRP_HASH_4		(EMAC_BASE + 76)
#define EMAC_LST_SRC_LOW	(EMAC_BASE + 80)
#define EMAC_LST_SRC_HI		(EMAC_BASE + 84)
#define EMAC_I_FRAME_GAP_REG	(EMAC_BASE + 88)
#define EMAC_STACR		(EMAC_BASE + 92)
#define EMAC_TRTR		(EMAC_BASE + 96)
#define EMAC_RX_HI_LO_WMARK	(EMAC_BASE + 100)

/* bit definitions */
/* MODE REG 0 */
#define EMAC_M0_RXI		0x80000000 /* RXMAC Idle */
#define EMAC_M0_TXI		0x40000000 /* TXMAC Idle */
#define EMAC_M0_SRST		0x20000000 /* Soft Reset */
#define EMAC_M0_TXE		0x10000000 /* Transmit Enable */
#define EMAC_M0_RXE		0x08000000 /* Receive Enable */
#define EMAC_M0_WKE		0x04000000 /* Wakup Enable */

/* MODE Reg 1 EMAC4 */
#define EMAC_M1_FDE		0x80000000
#define EMAC_M1_ILE		0x40000000
#define EMAC_M1_VLE		0x20000000
#define EMAC_M1_EIFC		0x10000000
#define EMAC_M1_APP		0x08000000
#define EMAC_M1_RSVD		0x06000000
#define EMAC_M1_IST		0x01000000
#define EMAC_M1_MF_MASK		0x00C00000 /* 8:9 Medium Frequency */
#define EMAC_M1_MF_1000MBPS	0x00800000 /* 0's for 10MBPS */
#define EMAC_M1_MF_100MBPS	0x00400000
#define EMAC_M1_MF_10MBPS	0x00000000 /*	  00 - 10MBPS */
#define EMAC_M1_RFS_16K		0x00280000 /* ~4k for 512 byte */
#define EMAC_M1_RFS_8K		0x00200000 /* ~4k for 512 byte */
#define EMAC_M1_RFS_4K		0x00180000 /* ~4k for 512 byte */
#define EMAC_M1_RFS_2K		0x00100000
#define EMAC_M1_RFS_1K		0x00080000
#define EMAC_M1_RFS_DEFAULT     EMAC_M1_RFS_2K
#define EMAC_M1_TX_FIFO_16K	0x00050000 /* 0's for 512 byte */
#define EMAC_M1_TX_FIFO_8K	0x00040000
#define EMAC_M1_TX_FIFO_4K	0x00030000
#define EMAC_M1_TX_FIFO_2K	0x00020000
#define EMAC_M1_TX_FIFO_1K	0x00010000
#define EMAC_M1_TX_FIFO_DEFAULT EMAC_M1_TX_FIFO_2K
#define EMAC_M1_TR0_MULTI	0x00008000 /* Transmit Request:
					      0: Single packet mode
					      1: Multi packet mode */
#define EMAC_M1_MWSW		0x00007000 /* Max Waiting Status Words */
#define EMAC_M1_JUMBO_ENABLE	0x00000800 /* Jumbo Enable */
#define EMAC_M1_IPPA		0x000007c0 /* Internal PCS PHY Addr */
#define EMAC_M1_OBCI_GT100	0x00000020 /* OPB bus clock identification */
#define EMAC_M1_OBCI_100	0x00000018
#define EMAC_M1_OBCI_83		0x00000010
#define EMAC_M1_OBCI_66		0x00000008
#define EMAC_M1_RSVD1		0x00000007

/*
 * STA CONTROL REG
 *
 * EMAC4 - OC bit has different meaning than for EMAC3
 */
#define EMAC_STACR_PHYDATA	0xffff0000 /* PHY data */
#define EMAC_STACR_OC		0x00008000 /* MMA GoGit/Op Complete */
#define EMAC_STACR_PHYE		0x00004000 /* PHY_ERR */
#define EMAC_STACR_INDIRECT_MODE 0x00002000 /* Indirect Mode Selection */
#define EMAC_STACR_OPCODE	0x00001800 /* STA Opcode */
#define EMAC_STACR_READ		0x00001000 /*	 10 read  */
#define EMAC_STACR_WRITE	0x00000800 /*	 01 write */
#define EMAC_STACR_PORTADDR	0x000003E0 /* Port Address */
#define	  STACR_PORTADDR_SHIFT	5
#define EMAC_STACR_DEVICEADDR	0x0000001F /* Device Address */

/* Transmit Request Threshold Register */
#define EMAC_TRTR_256		0x18000000 /* 0's for 64 Bytes */
#define EMAC_TRTR_192		0x10000000
#define EMAC_TRTR_128		0x01000000

/* Transmit Mode Register 0 */
#define EMAC_TXM0_GNP0		0x80000000
#define EMAC_TXM0_GNP1		0x40000000
#define EMAC_TXM0_GNPD		0x20000000
#define EMAC_TXM0_FC		0x10000000

/* Receive Mode Register */
#define EMAC_RMR_SP		0x80000000
#define EMAC_RMR_SFCS		0x40000000
#define EMAC_RMR_ARRP		0x20000000
#define EMAC_RMR_ARP		0x10000000
#define EMAC_RMR_AROP		0x08000000
#define EMAC_RMR_ARPI		0x04000000
#define EMAC_RMR_PPP		0x02000000
#define EMAC_RMR_PME		0x01000000
#define EMAC_RMR_PMME		0x00800000
#define EMAC_RMR_IAE		0x00400000
#define EMAC_RMR_MIAE		0x00200000
#define EMAC_RMR_BAE		0x00100000
#define EMAC_RMR_MAE		0x00080000

/* Interrupt Status & enable Regs */

/*
 *      MSB ------------------------------- LSB
 *		    11.1111.1111.2222.2222.2233
 *      0123.4567.8901.2345.6789.0123.4567.8901
 */

#define EMAC_ISR_TXPE		0x20000000 /* Tx Parity Err */
#define EMAC_ISR_RXPE		0x10000000 /* Rx Parity Err */
#define EMAC_ISR_TX_TFAE	0x08000000 /* Tx Fifo Almost Empty */
#define EMAC_ISR_RX_RFAF	0x08000000 /* Rx Fifo Almost Full */
#define EMAC_ISR_OVR		0x02000000 /* Overrun */
#define EMAC_ISR_PP		0x01000000 /* Pause Frame */
#define EMAC_ISR_BP		0x00800000 /* Bad Frame */
#define EMAC_ISR_RP		0x00400000 /* Runt Frame */
#define EMAC_ISR_SE		0x00200000 /* Short Event */
#define EMAC_ISR_SYE		0x00100000 /* Alignment Err */
#define EMAC_ISR_BFCS		0x00080000 /* Bad FCS */
#define EMAC_ISR_PTLE		0x00040000 /* Frame Is Too Long */
#define EMAC_ISR_ORE		0x00020000 /* Out of Range Err */
#define EMAC_ISR_IRE		0x00010000 /* In Range Err */
#define EMAC_ISR_DBDM		0x00000200
#define EMAC_ISR_DB0		0x00000100 /* Dead Bit */
#define EMAC_ISR_SE0		0x00000080 /* SQE Error */
#define EMAC_ISR_TE0		0x00000040 /* Transmit Err */
#define EMAC_ISR_DB1		0x00000020
#define EMAC_ISR_SE1		0x00000010
#define EMAC_ISR_TE1		0x00000008
#define EMAC_ISR_MOS		0x00000002 /* MMA Op Succeeded */
#define EMAC_ISR_MOF		0x00000001 /* MMA Op Failed */

/*
 * The follwing defines are for the MadMAL status and control
 * registers. For bits 0..5 look at the mal.h file
 */
/* TX Control bits */
#define EMAC_TX_CTRL_GFCS	0x0200 /* Generate FCS */
#define EMAC_TX_CTRL_GP		0x0100 /* Generate padding */
#define EMAC_TX_CTRL_ISA	0x0080 /* Insert source address */
#define EMAC_TX_CTRL_RSA	0x0040 /* Replace source address */
#define EMAC_TX_CTRL_IVT	0x0020 /* Insert VLAN Tag */
#define EMAC_TX_CTRL_RVT	0x0010 /* Replace VLAN Tag */
#define EMAC_TX_CTRL_DEFAULT	(EMAC_TX_CTRL_GFCS |EMAC_TX_CTRL_GP)

/* TX Status bits */
#define EMAC_TX_ST_PARITY	0x0400 /* Parity Error */
#define EMAC_TX_ST_BFCS		0x0200 /* Bad FCS */
#define EMAC_TX_ST_BPP		0x0100 /* Bad previous packet */
#define EMAC_TX_ST_LCS		0x0080 /* Loss of carrier sense */
#define EMAC_TX_ST_ED		0x0040 /* Excessive deferral */
#define EMAC_TX_ST_EC		0x0020 /* Excessive collisions */
#define EMAC_TX_ST_LC		0x0010 /* Late collision */
#define EMAC_TX_ST_MC		0x0008 /* Multiple collision */
#define EMAC_TX_ST_SC		0x0004 /* Single collision */
#define EMAC_TX_ST_UR		0x0002 /* Underrun */
#define EMAC_TX_ST_SQE		0x0001 /* Signal quality error (SQE)  */
#define EMAC_TX_ST_DEFAULT	0x03F3

/* RX status bits */
#define EMAC_RX_ST_PARITY       0x0400 /* Parity Error */
#define EMAC_RX_ST_OE		0x0200 /* Overrun */
#define EMAC_RX_ST_PP		0x0100 /* Pause Frame */
#define EMAC_RX_ST_BP		0x0080 /* Bad Frame */
#define EMAC_RX_ST_RP		0x0040 /* Runt Frame */
#define EMAC_RX_ST_SE		0x0020 /* Short Event */
#define EMAC_RX_ST_AE		0x0010 /* Alignement Error */
#define EMAC_RX_ST_BFCS		0x0008 /* Bad Frame Check Sequence */
#define EMAC_RX_ST_PTL		0x0004 /* Frame is too long */
#define EMAC_RX_ST_ORE		0x0002 /* Out or range error */
#define EMAC_RX_ST_IRE		0x0001 /* In range error */

/* all the errors we care about */
#define EMAC_RX_ERRORS		0x03FF /* Errors Above */

/* Statistic Areas */
#define MAX_ERR_LOG 10

/* RGMII */
#define RGMII_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x600)

#define RGMII_FER		(RGMII_BASE + 0x00)
#define RGMII_SSR		(RGMII_BASE + 0x04)

#define RGMII_SSR_SP_10MBPS	(0x00)
#define RGMII_SSR_SP_100MBPS	(0x02)
#define RGMII_SSR_SP_1000MBPS	(0x04)

#define RGMII_SSR_V(__x)	((__x) * 8)

/* phy types */
#define PHY_TYPE_NOPHY		1
#define PHY_TYPE_BCM5221	2 /* single phy used for FSP-0 */
#define PHY_TYPE_BCM5241	3 /* single phy used for FSP-1 */
#define PHY_TYPE_BCM5222	4 /* dual phy only for FSP-0 */
#define PHY_TYPE_BCM5461	5 /* single phy used for FSP-2 */
#define PHY_TYPE_UNKNOWN	-1

struct phy_props {
	int type;		/* single, double, nophy, ... */
	int owner;		/* controlling EMAC */
	int addr;		/* phy id */
	int speed;		/* 0: probing */
	int duplex;		/* 0: probe */
};

/* Initiates PHY parameters using environment variables */
void miiphy_init_phy_params(struct phy_props *props, int devnum);

/* We need silently gather a lot of information to be able to
   understand our network problems. */
struct emac_stats_st {
	uint32_t time_start;
	uint32_t time_end;
	int data_len_err;
	int rx;			/* received bytes */
	int tx;			/* transmitted bytes */
	int pkts_tx;
	int pkts_rx;
	int tx_err;
	int rx_err;
	int pkts_handled;
	int reenable_rx_count;
	int tx_discard;
	int tx_high_water;
	int rx_high_water;
	int rx_parity_err;
	int tx_parity_err;
	int tx_err_index;
	short tx_err_log[MAX_ERR_LOG];
	int rx_err_index;
	short rx_err_log[MAX_ERR_LOG];
	int emac_isr_index;
	uint32_t emac_isr[MAX_ERR_LOG];
	int mal_esr_index;
	uint32_t mal_esr[MAX_ERR_LOG];
};

/* Structure containing variables used by the shared code (440gx_enet.c) */
typedef struct emac_440gx_hw_st {
	uint32_t magic;		/* Magic number for hopeless e.g. JTAG cases */
	uint32_t hw_addr;	/* EMAC offset */
	uint32_t phy_id;
	uint32_t phy_addr;
	uint32_t mal_offs;	/* MAL offset */

	/* Buffer layout (cache-line aligned):
	 *   NUM_TX_BUFF TX Descriptors
	 *   NUM_RX_BUFF RX Descriptors
	 *   NUM_TX_BUFF TX Buffers
	 *   ... RX Buffers are defined in net.c
	 */
	char *alloc_buf;
	char *aligned_buf;

	volatile mal_desc_t *tx; /* Uncached and aligned */
	volatile mal_desc_t *rx; /* Uncached and aligned */
	bd_t *bis;		/* for eth_init upon mal error */
	char *txbuf[NUM_TX_BUFF];
	uint16_t devnum;
	int first_init;

	int rx_slot;		/* MAL Receive Slot */
	int rx_i_index;		/* Receive Interrupt Queue Index */
	int rx_u_index;		/* Receive User Queue Index */
	int rx_ready[NUM_RX_BUFF]; /* Receive Ready Queue */
	unsigned short mal_rx_chan; /* Receive Mal Channel */
	uint32_t mal_rx_chan_mask;

	int tx_slot;		/* MAL Transmit Slot */
	int tx_done;		/* MAL Transmit Slot done */
	int tx_inq;		/* Number of packets in TX queue */
	unsigned short mal_tx_chan; /* Transmit Mal Channel */
	uint32_t mal_tx_chan_mask;

	/* Irq numbers */
	int mal_irq_rxeob;	/* packet received */
	int mal_irq_txeob;	/* packet sent */
	int mal_irq_rxde;	/* rx descriptor error */
	int mal_irq_txde;	/* tx descriptor error */
	int mal_irq_serr;	/* SERR */
	int emac_irq_ewu;	/* Wakeup - normally unused */
	int emac_irq_eth;	/* EMAC interrupt */

	int print_speed;	/* print speed message upon start */
	uint32_t emac_offsets[EMAC_NUM_DEV]; /* first offset is always zero */
	struct phy_props phyp;	/* PHY properties */
	struct emac_stats_st stats; /* Interface statistics */

	/* Buffer some registers which to learn how the hardware operates */
	uint32_t emac_ier;	/* Enabled interrupts */
	uint32_t emac_isr;	/* Last value on error */
	uint32_t mal_esr;	/* Last value on error */
	uint32_t mal_txeobisr;	/* TX End of Buffer Interrupt Status Reg */
	uint32_t mal_rxeobisr;	/* RX End of Buffer Interrupt Status Reg */
	uint32_t mal_txdeir;	/* TX Descriptor Error Interrupt Register */
	uint32_t mal_rxdeir;	/* RX Descriptor Error Interrupt Register */

	int mal_rxeob_count;
	int mal_txeob_count;
	int mal_rxde_count;
	int mal_txde_count;
	int mal_serr_count;
	int emac_eth_count;
} EMAC_440GX_HW_ST, *EMAC_440GX_HW_PST;

#endif
