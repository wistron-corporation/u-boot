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
 * Based on 440gx_enet.c
 *
 * Author:      Kirill Gorelov <kirill.gorelov@auriga.ru>
 * Reworked by: Frank Haverkamp <haver@vnet.ibm.com>
 * Inspiration: Eberhard Amann <esa@de.ibm.com>
 * Fixes:       Kirill Kuvaldin <kirill.kuvaldin@auriga.ru>
 *
 * Please test with:
 *  ping -b -f 192.168.1.0 -l 64 -q -s 1
 */

#include <config.h>
#include <common.h>
#include <net.h>
#include <commproc.h>
#include <miiphy.h>
#include <malloc.h>
#include <command.h>

#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <fsp_enet.h>
#include <asm/ppc4xx-mal.h>

/*
 * Kirill G's first approach to handle rxde errors was to disable
 * receiving in the rxde interrupt and check in the network-poll-loop
 * if the mal channel is still enabled after all pending packets have
 * been received. An alternate approach is to handle the error
 * completely in the ISR, which I prefer because the handling is more
 * local (only in the ISR) doing it that way.
 */
#undef CFG_RX_DISABLED_CHECKING
#undef CFG_EXTENDED_REGISTER_DUMP

/*----------------------------------------------------------------------------+
 * Cache stuff
 *---------------------------------------------------------------------------*/
#define CACHE_MASK_BITS	 (CONFIG_SYS_CACHELINE_SIZE - 1)
#define CACHE_ROUNDUP(A) (((ulong)(A) + CACHE_MASK_BITS) & ~CACHE_MASK_BITS)

#define TX_DESC_SIZE	     CACHE_ROUNDUP((sizeof(mal_desc_t) * NUM_TX_BUFF))
#define RX_DESC_SIZE	     CACHE_ROUNDUP((sizeof(mal_desc_t) * NUM_RX_BUFF))
#define TX_BUFPTR_SIZE       ENET_MAX_MTU_ALIGNED

#define ETH_ALLOC_BUFF_SIZE  (TX_DESC_SIZE + RX_DESC_SIZE +		\
			      NUM_TX_BUFF * ENET_MAX_MTU_ALIGNED +	\
			      2 * CONFIG_SYS_CACHELINE_SIZE)

/* The cacheable/non-cacheable TLB entry trick
 * described below doesn't work on FSP2 since FSP2
 * memory/caches are coherent.  Additionally, it seems
 * to violate the power ISA (6.8.3.1 Book3E).
 * Since FSP2 is coherent, The MAL BDs can live in a
 * cacheable TLB entry. */
#define ctonc(addr) addr
#define nctoc(addr) addr


extern int emac4_mdio_read(struct mii_dev *bus, int port_addr, int dev_addr, int regnum);
extern int emac4_mdio_write(struct mii_dev *bus, int port_addr, int dev_addr, int regnum, u16 value);
extern int phy_setup_aneg(char *devname, unsigned char addr);
extern void miiphy_set_curr_emac_offset(unsigned long emac_offset);
extern void wback_inv_dcache_range(ulong addr, ulong size);
static void enet_wback_inv(void *addr, int size)
{
	wback_inv_dcache_range((ulong)addr, (ulong)addr + size);
}

/*
 * The ethernet descriptor-buffers need to be non-cachable. The
 * original version simply disabled the caching which is resulting in
 * a significant performance loss e.g. when doing ECC or gzip
 * algoritms. To overcome this limitation we use a cachable and a
 * non-cachable TLB entry for the FSP-1 and FSP-0 setups.
 */
#include <asm/ppc476fsp2.h>
#include <asm/ppc4xx-uic.h>

#define mplb2hplb(addr) (addr)

#define FSP_ETH_DEVNAME "ppc_440_eth"

#define EMAC0_OFFSET	0x00000000
#define EMAC1_OFFSET	0x00000100

unsigned long get_dcr(unsigned short);
unsigned long set_dcr(unsigned short, unsigned long);

static inline void rgmii_get_mdio(int input)
{
        u32 fer = in32(RGMII_FER);
        fer |= 0x00080000u >> input;
        out32(RGMII_FER, fer);
}

static inline void rgmii_put_mdio(int input)
{
        u32 fer = in32(RGMII_FER);
        fer &= ~0x00080000u >> input;
        out32(RGMII_FER, fer);
}

unsigned long getmal(uint16_t mal_offs, unsigned dcr_offset)
{
	unsigned long retval;

	retval = get_dcr(mal_offs + dcr_offset);

	return retval;
}

void setmal(uint32_t mal_offs, unsigned dcr_offset, uint32_t malval)
{
	set_dcr(mal_offs + dcr_offset, malval);
}

void flush_dcache_all(void)
{
}

#define EMAC_RESET_TIMEOUT	  1000	/* 1000 ms reset timeout */
#define PHY_AUTONEGOTIATE_TIMEOUT 4000	/* 4000 ms autonegotiate timeout */

/* Ethernet Transmit and Receive Buffers */
#define ENET_MAX_MTU	       PKTSIZE
#define ENET_MAX_MTU_ALIGNED   PKTSIZE_ALIGN
#define MAL_CHANNEL_MASK(chan) (0x80000000 >> (chan))

/*----------------------------------------------------------------------------+
 * Global variables. TX and RX descriptors and buffers.
 *---------------------------------------------------------------------------*/
static struct eth_device eth_dev[EMAC_NUM_DEV];
static struct emac_440gx_hw_st eth_hw[EMAC_NUM_DEV];

static void tx_descr_init(struct eth_device *dev)
{
	int i;
	char *buf;
	EMAC_440GX_HW_PST hw = dev->priv;

	hw->stats.tx_err_index = 0; /* Transmit Error Index for tx_err_log */
	hw->tx_slot = 0;	/* MAL Transmit Slot */
	hw->tx_done = 0;	/* MAL Tramsmit Slot done */
	hw->tx_inq = 0;		/* Packets in TX queue */

	buf = hw->aligned_buf;
	memset(buf, 0, TX_DESC_SIZE);
	enet_wback_inv(buf, TX_DESC_SIZE);

	hw->tx = (mal_desc_t *)ctonc(buf);

	for (i = 0; i < NUM_TX_BUFF; i++) {
		hw->tx[i].ctrl = 0;
		hw->tx[i].data_len = 0;
		hw->txbuf[i] = (hw->aligned_buf + TX_BUFPTR_SIZE * i +
				TX_DESC_SIZE + RX_DESC_SIZE);
		enet_wback_inv((void *)hw->txbuf[i], TX_BUFPTR_SIZE);

		/* HostPLB to MemPLB translation required for FSP-0 */
		hw->tx[i].data_ptr = (char *)mplb2hplb((ulong)hw->txbuf[i]);
		if ((NUM_TX_BUFF - 1) == i)
			hw->tx[i].ctrl |= MAL_TX_CTRL_WRAP;
	}
}

static void rx_descr_init(struct eth_device *dev)
{
	int i;
	char *buf;
	EMAC_440GX_HW_PST hw = dev->priv;

	hw->rx_slot = 0;	/* MAL Receive Slot */
	hw->rx_i_index = 0;	/* Receive Interrupt Queue Index */
	hw->rx_u_index = 0;	/* Receive User Queue Index */

	buf = hw->aligned_buf + TX_DESC_SIZE;
	memset(buf, 0, RX_DESC_SIZE);
	enet_wback_inv(buf, RX_DESC_SIZE);

	hw->rx = (mal_desc_t *)ctonc(buf);

	for (i = 0; i < NUM_RX_BUFF; i++) {
		uint16_t ctrl = MAL_RX_CTRL_EMPTY | MAL_RX_CTRL_INTR;

		hw->rx[i].data_len = 0;
		hw->rx[i].data_ptr = (char *)mplb2hplb((ulong)net_rx_packets[i]);
		enet_wback_inv((void *)net_rx_packets[i], ENET_MAX_MTU_ALIGNED);
		if ((NUM_RX_BUFF - 1) == i)
			ctrl |= MAL_RX_CTRL_WRAP;

		hw->rx[i].ctrl = ctrl;
		hw->rx_ready[i] = -1;
	}
}

static void rx_recovery(struct eth_device *dev)
{
	EMAC_440GX_HW_PST hw = dev->priv;
	uint32_t emac_m0;

	/* Do recovery: disable, re-initialize, reenable */
	emac_m0 = in32(EMAC_M0 + hw->hw_addr);
	out32(EMAC_M0 + hw->hw_addr, emac_m0 & ~EMAC_M0_RXE); /* Disable RX */
	setmal(hw->mal_offs, MAL0_RXCARR, hw->mal_rx_chan_mask); /* Reset */
	rx_descr_init(dev);
	setmal(hw->mal_offs, MAL0_RXCASR, hw->mal_rx_chan_mask); /* Reenable */
	out32(EMAC_M0 + hw->hw_addr, emac_m0 | EMAC_M0_RXE); /* Reenable RX */
}

/*----------------------------------------------------------------------------+
 *  enet_rcv() handles the ethernet receive data
 *---------------------------------------------------------------------------*/
static void enet_rcv(struct eth_device *dev)
{
	unsigned long data_len;
	EMAC_440GX_HW_PST hw = dev->priv;

	int handled = 0;
	int i;
	int loop_count = 0;

	/* EMAC RX done */
	while (1) {	/* do all */

		/*
		 * Hardware workarround to handle parity errors.
		 * In general this is broken hardware, but the kernel
		 * does not even recognise these errors and is running
		 * fine. And it looks like, these errors are only
		 * temporarily, so uboot also needs to deal with this
		 * ugly hardware.
		 */
		hw->emac_isr = in32(EMAC_ISR + hw->hw_addr);
		if (hw->emac_isr & EMAC_ISR_RXPE) {
			hw->stats.rx_parity_err++;
			rx_recovery(dev);
			if(hw->stats.rx_parity_err >= 999) {
				/* after 999 RXPE interrupts disable this interrupt */
				hw->emac_ier &= ~(EMAC_ISR_RXPE);
				out32(EMAC_IER + hw->hw_addr, hw->emac_ier);
			}
			/* clear exact this pending emac interrupt flag */
			out32(EMAC_ISR + hw->hw_addr, EMAC_ISR_RXPE);
			break;
		}

		i = hw->rx_slot;

		if ((MAL_RX_CTRL_EMPTY & hw->rx[i].ctrl)
		    || (loop_count >= NUM_RX_BUFF))
			break;

		loop_count++;
		handled++;

		data_len = (unsigned long)hw->rx[i].data_len;
		if (data_len) {
			if (data_len > ENET_MAX_MTU) /* Check len */
				data_len = 0;
			else if (EMAC_RX_ERRORS & hw->rx[i].ctrl) {
				/* Check Errors */
				data_len = 0;
				hw->stats.rx_err_log[hw->stats.rx_err_index]
					= hw->rx[i].ctrl;
				hw->stats.rx_err_index++;
				if (hw->stats.rx_err_index == MAX_ERR_LOG)
					hw->stats.rx_err_index = 0;
			}
		}
		if (!data_len) {	/* no data */
			hw->rx[i].ctrl |= MAL_RX_CTRL_EMPTY;
			hw->stats.data_len_err++;
		}

		/* !data_len */
		/* AS.HARNOIS */
		/* Check if user has already eaten buffer */
		/* if not => ERROR */
		else if (hw->rx_ready[hw->rx_i_index] != -1) {
			/* FXIME gather statistics */
			break;
		} else {
			hw->stats.rx += data_len;
			hw->stats.pkts_rx++;

			hw->rx_ready[hw->rx_i_index] = i;
			hw->rx_i_index++;
			if (NUM_RX_BUFF == hw->rx_i_index)
				hw->rx_i_index = 0;

			/*
			 * rx_slot must only be incremented if
			 * rx_ready[hw->rx_i_index] is -1, meaning
			 * that the buffer is empty.
			 *
			 * Otherwise the MAL index and rx_slot get
			 * inconsistent and receiving packets becomes
			 * slow and only possible if e.g. broadcast
			 * traffic happens in parallel.
			 */
			hw->rx_slot++;
			if (NUM_RX_BUFF == hw->rx_slot)
				hw->rx_slot = 0;
		}
	}

	if (hw->stats.rx_high_water < handled)
		hw->stats.rx_high_water = handled;
}

static void mal_reset(struct eth_device *dev)
{
	uint32_t failsafe = 10000;
	EMAC_440GX_HW_PST hw = dev->priv;

	/* MAL RESET - I think it is safer if we do the full reset */
	setmal(hw->mal_offs, MAL0_CFG, MAL_CR_MMSR);

	/* wait for reset */
	while (getmal(hw->mal_offs, MAL0_CFG) & MAL_CR_MMSR) {
		udelay (1000);	/* Delay 1 MS so as not to hammer the reg */
		failsafe--;
		if (failsafe == 0) {
			printf("Could not reset MAL!\n");
			break;
		}
	}
}

static void mal_reset_channels(struct eth_device *dev)
{
	uint32_t failsafe = 10000;
	EMAC_440GX_HW_PST hw = dev->priv;

	/* 1st reset MAL channel */
	/* Note: writing a 0 to a channel has no effect */
	setmal(hw->mal_offs, MAL0_TXCARR, hw->mal_tx_chan_mask);
	setmal(hw->mal_offs, MAL0_RXCARR, hw->mal_rx_chan_mask);

	/* wait for reset */
	while (getmal(hw->mal_offs, MAL0_TXCASR) & hw->mal_tx_chan_mask) {
		udelay (1000);	/* Delay 1 MS so as not to hammer the reg */
		failsafe--;
		if (failsafe == 0) {
			printf("Could not reset MAL channel!\n");
			break;
		}
	}
}

static void emac_reset(struct eth_device *dev)
{
	uint32_t failsafe = 10000;
	EMAC_440GX_HW_PST hw = dev->priv;

	out32(EMAC_M0 + hw->hw_addr, EMAC_M0_SRST);
	while (in32(EMAC_M0 + hw->hw_addr) & EMAC_M0_SRST) {
		udelay (1000);
		failsafe--;
		if (failsafe == 0) {
			printf("Could not reset EMAC!\n");
			break;
		}
	}
}

static void setup_macaddr(struct eth_device *dev)
{
	EMAC_440GX_HW_PST hw = dev->priv;
	uint32_t reg = 0x00000000;

	reg |= dev->enetaddr[0];	/* set high address */
	reg = reg << 8;
	reg |= dev->enetaddr[1];
	out32(EMAC_IAH + hw->hw_addr, reg);
	reg = 0x00000000;
	reg |= dev->enetaddr[2];	/* set low address  */
	reg = reg << 8;
	reg |= dev->enetaddr[3];
	reg = reg << 8;
	reg |= dev->enetaddr[4];
	reg = reg << 8;
	reg |= dev->enetaddr[5];
	out32(EMAC_IAL + hw->hw_addr, reg);
}

static void setup_phy_speed(struct eth_device *dev, bd_t *bis)
{
	int i, phy_owner, speed, duplex;
	EMAC_440GX_HW_PST hw = dev->priv;
	uint32_t mode_reg, reg;
	unsigned short reg_short;
	sys_info_t sysinfo;

	mode_reg = 0x0;		/* Whack the M1 register */
	reg_short = 0;		/* wait for PHY to complete auto negotiation */
	reg = hw->phyp.addr;	/* use generic support */
	bis->bi_phynum[hw->devnum] = reg;

	/*
	 * Reset the phy, only if its the first time through
	 * otherwise, just check the speeds & feeds
	 */
	phy_owner = hw->phyp.owner;

	/* To avoid potential vulnerability it's necessary to check if
	 * the obtained emac number owning the phy is less than EMAC_NUM_DEV */
	phy_owner = ((phy_owner < EMAC_NUM_DEV) ? phy_owner : hw->devnum);

	miiphy_set_curr_emac_offset(hw->emac_offsets[phy_owner]);
	if (hw->first_init == 0) {
		miiphy_reset(dev->name, reg);

		/* Start/Restart autonegotiation */
		if (hw->phyp.type == PHY_TYPE_NOPHY) {
			phy_setup_aneg(dev->name, reg);
			udelay (1000);
		}
	}
	/* FIXME interface deprecated, use new phy_read on a phy_device
	 * found via phy_connect
	 */
	miiphy_read (dev->name, reg, MII_BMSR, &reg_short);

	/*
	 * Wait if PHY is capable of autonegotiation and
	 * autonegotiation is not complete
	 */
	if ((reg_short & BMSR_ANEGCAPABLE)
	    && !(reg_short & BMSR_ANEGCOMPLETE)) {
		puts ("Waiting for PHY auto negotiation to complete");
		i = 0;
		while (!(reg_short & BMSR_ANEGCOMPLETE)) {
			/*
			 * Timeout reached ?
			 */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts (" TIMEOUT !\n");
				break;
			}

			if ((i++ % 1000) == 0) {
				putc ('.');
			}
			udelay (1000);	/* 1 ms */
			miiphy_read (dev->name, reg, MII_BMSR, &reg_short);

		}
		puts (" done\n");
	}

	speed = miiphy_speed(dev->name, reg);
	duplex = miiphy_duplex(dev->name, reg);

	if (hw->phyp.speed)
		speed = hw->phyp.speed;
	if (hw->phyp.duplex)
		duplex = hw->phyp.duplex;

	if (hw->print_speed) {
		hw->print_speed = 0;
		printf ("ENET Speed is %d Mbps - %s duplex connection\n",
			(int) speed, (duplex == HALF) ? "HALF" : "FULL");
	}

	/* Setup RGMII speed */
	if (speed == 1000)
		reg = (RGMII_SSR_SP_1000MBPS << RGMII_SSR_V(hw->devnum));
	else if (speed == 100)
		reg = (RGMII_SSR_SP_100MBPS << RGMII_SSR_V(hw->devnum));
	else if (speed == 10)
		reg = (RGMII_SSR_SP_10MBPS << RGMII_SSR_V(hw->devnum));
	else {
		printf("Invalid speed for RGMII: %d\n", speed);
		return;
	}

	out32(RGMII_SSR, reg);

	/* Set receive fifo and tx fifo */
	mode_reg = in32(EMAC_M1 + hw->hw_addr);
	mode_reg |= EMAC_M1_RFS_DEFAULT | EMAC_M1_TX_FIFO_DEFAULT;

	/* Setup multi packet TX mode */
	mode_reg |= EMAC_M1_TR0_MULTI;

	get_sys_info(&sysinfo);

	/* Setup OPB speed */
	mode_reg &= ~0x00000038;
	if (sysinfo.freqOPB <= 50000000);
	else if (sysinfo.freqOPB <= 66666667)
		mode_reg |= EMAC_M1_OBCI_66;
	else if (sysinfo.freqOPB <= 83333333)
		mode_reg |= EMAC_M1_OBCI_83;
	else if (sysinfo.freqOPB <= 100000000)
		mode_reg |= EMAC_M1_OBCI_100;
	else
		mode_reg |= EMAC_M1_OBCI_GT100;

	/* Set speed */
	if (speed == _1000BASET)
		mode_reg = mode_reg | EMAC_M1_MF_1000MBPS | EMAC_M1_IST;
	else if (speed == _100BASET)
		mode_reg = mode_reg | EMAC_M1_MF_100MBPS | EMAC_M1_IST;
	else
		mode_reg = mode_reg & ~EMAC_M1_MF_MASK;	/* 10 MBPS */
	if (duplex == FULL)
		mode_reg = mode_reg | EMAC_M1_FDE | EMAC_M1_IST;

	out32(EMAC_M1 + hw->hw_addr, mode_reg);

	/*
	 * Set EMAC IER - store in hw->emac_ier for later setup. Here
	 * BFCS bad frame sequence checking is enabled.
	 */
	hw->emac_ier = (EMAC_ISR_TXPE | /* Tx Parity Err */
			EMAC_ISR_RXPE |	/* Rx Parity Err */
			EMAC_ISR_OVR  | /* Overrun */
			EMAC_ISR_BP   | /* Bad Frame */
			EMAC_ISR_PTLE | /* Frame Is Too Long */
			EMAC_ISR_BFCS |	/* Bad FCS */
			EMAC_ISR_PTLE | /* Frame Is Too Long */
			EMAC_ISR_ORE  | /* Out of Range Err */
			EMAC_ISR_IRE  | /* In Range Err */
			EMAC_ISR_DB0  |	/* Dead Bit */
			EMAC_ISR_SE0  | /* SQE Error */
			EMAC_ISR_TE0);	/* Transmit Err */

	if (speed == _100BASET)
		hw->emac_ier |= EMAC_ISR_SYE; /* Alignment Err */

	/* clear all pending emac interrupts */
	out32(EMAC_ISR + hw->hw_addr, 0xffffffff);
	/* set emac interrupt mask */
	out32(EMAC_IER + hw->hw_addr, hw->emac_ier);
}

#define PHY_LED_SELECTOR1	0xB43C /* ENERGY/LINK and ACTIVITY */

static void setup_phy_leds(struct eth_device *dev)
{
	EMAC_440GX_HW_PST hw = dev->priv;
	uint32_t reg = hw->phyp.addr;
	miiphy_write(dev->name, reg, MII_SHADOW, PHY_LED_SELECTOR1);
}

static void eth_calc_result(struct eth_device *dev)
{
	char sret[256];
	EMAC_440GX_HW_PST hw = dev->priv;
	uint32_t elapsed = hw->stats.time_end - hw->stats.time_start;

	sprintf(sret, "time=%d@%dHz,tx=%d,txp=%d,tx_err=%d,tx_discard=%d,"
		"tx_parity=%d,rx=%d,rxp=%d,rx_err=%d,dlen_err=%d,rx_parity=%d,"
		"emac_isr=%08x,mal_esr=%08x", elapsed, CONFIG_SYS_HZ,
		hw->stats.tx, hw->stats.pkts_tx, hw->stats.tx_err,
		hw->stats.tx_discard, hw->stats.tx_parity_err,
		hw->stats.rx, hw->stats.pkts_rx, hw->stats.rx_err,
		hw->stats.data_len_err, hw->stats.rx_parity_err,
		hw->emac_isr, hw->mal_esr);
	env_set("eth_result", sret);
}

/*
 * Disable MAL channel, and EMACn
 *
 * FIXME Reset EMAC before resetting MAL. There was a MAL TX and RX
 * hang after Core reset.
 */
static void fsp_eth_halt(struct eth_device *dev)
{
	uint32_t msr, mal_txcasr, mal_rxcasr;
	EMAC_440GX_HW_PST hw = dev->priv;

	hw->stats.time_end = get_timer(0);
	eth_calc_result(dev);

	msr = mfmsr();
	mtmsr(msr & ~MSR_EE);
        rgmii_put_mdio(hw->devnum);
	emac_reset(dev);
	out32(EMAC_IER + hw->hw_addr, 0x00000000); /* clear emac intr mask */

	/* Disable MAL TX and RX Channel */
	setmal(hw->mal_offs, MAL0_TXCARR, hw->mal_tx_chan_mask);
	setmal(hw->mal_offs, MAL0_RXCARR, hw->mal_rx_chan_mask);

	/* Clear Active TX and RX Channel Mask */
	mal_txcasr = getmal(hw->mal_offs, MAL0_TXCASR);
	setmal(hw->mal_offs, MAL0_TXCASR, mal_txcasr & ~hw->mal_tx_chan_mask);

	mal_rxcasr = getmal(hw->mal_offs, MAL0_RXCASR);
	setmal(hw->mal_offs, MAL0_RXCASR, mal_rxcasr & ~hw->mal_rx_chan_mask);

	mal_reset_channels(dev);
	mal_reset(dev);
	mtmsr(msr);

	hw->print_speed = 1;	/* print speed message again next time */
}


/*
 * This function is called whenever a network command is started. The
 * core data structures are already setup in fsp_eth_initialize().
 *
 * Malloc MAL buffer desciptors, make sure they are aligned on cache
 * line boundary size (401/403/IOP480 = 16, 405 = 32) and doesn't
 * cross cache block boundaries.
 */
static int fsp_eth_init(struct eth_device *dev, bd_t * bis)
{
	unsigned long msr;
	sys_info_t sysinfo;
	EMAC_440GX_HW_PST hw = dev->priv;

	env_set("eth_result", NULL);
	flush_dcache_all();

	/* Clear statistics of this run - discuss: do we want this? */
	memset(&hw->stats, 0, sizeof(hw->stats));

	hw->emac_isr = 0;
	hw->mal_esr = 0;

	if (!hw->alloc_buf)
		hw->alloc_buf = malloc(ETH_ALLOC_BUFF_SIZE);
	if (!hw->alloc_buf) {
		puts("ERR: No mem alailable!\n");
		return -1;
	}
	hw->aligned_buf = (char *)CACHE_ROUNDUP(hw->alloc_buf);
	hw->print_speed = 1;
	hw->stats.time_start = get_timer(0);

	/* before doing anything, figure out if we have a MAC address */
	/* if not, bail */
	if (memcmp(dev->enetaddr, "\0\0\0\0\0\0", 6) == 0)
		return -1;

	/* Need to get the OPB frequency so we can access the PHY */
	get_sys_info(&sysinfo);

	/* Initialize PHY parameters */
	/* Fill in EMAC hwaddr offsets */
	miiphy_init_phy_params(&hw->phyp, hw->devnum);

	msr = mfmsr();
	mtmsr(msr & ~MSR_EE);	/* disable interrupts */

	emac_reset(dev);
        rgmii_get_mdio(hw->devnum);
	mal_reset(dev);
	mal_reset_channels(dev);

	setup_phy_speed(dev, bis);
	setup_phy_leds(dev);

	/* set the Mal configuration reg */
	setmal(hw->mal_offs, MAL0_CFG, MAL_CR_PLBB | MAL_CR_OPBBL | MAL_CR_LEA |
	       MAL_CR_PLBLT_DEFAULT | MAL_CR_EOPIE | 0x00330000);

	/* setup MAL tx & rx channel pointers */
	tx_descr_init(dev);
	setmal(hw->mal_offs, MAL0_TXCTP0R + hw->mal_tx_chan,
	       (uint32_t)mplb2hplb((uint32_t)nctoc((ulong)hw->tx)));
	setmal(hw->mal_offs, MAL0_TXCASR, hw->mal_tx_chan_mask);

	/* set transmit request threshold register */
	out32(EMAC_TRTR + hw->hw_addr, 0x18000000); /* 256 byte threshold */

	rx_descr_init(dev);

	setmal(hw->mal_offs, MAL0_RXCTP0R + hw->mal_rx_chan,
	       (uint32_t)mplb2hplb((uint32_t)nctoc((ulong)hw->rx)));
	setmal(hw->mal_offs, MAL0_RCBS0 + hw->mal_rx_chan,
	       ENET_MAX_MTU_ALIGNED / 16);
	setmal(hw->mal_offs, MAL0_RXCASR, hw->mal_rx_chan_mask);

	/* set transmit enable & receive enable */
	out32(EMAC_M0 + hw->hw_addr, EMAC_M0_TXE | EMAC_M0_RXE);
	setup_macaddr(dev);

	/* Enable broadcast and indvidual address */
	/* TBS: enabling runts as some misbehaved nics will send runts */
	out32(EMAC_RXM + hw->hw_addr, EMAC_RMR_BAE | EMAC_RMR_IAE);

	/* set receive	low/high water mark register */
	/* 440GP has a 64 byte burst length */
	out32(EMAC_RX_HI_LO_WMARK + hw->hw_addr, 0x80009000);
	out32(EMAC_TXM1 + hw->hw_addr, 0xf8640000);

	/* Set fifo limit entry in tx mode 0 */
	out32(EMAC_TXM0 + hw->hw_addr, 0x00000003);

	/* Frame gap set */
	out32(EMAC_I_FRAME_GAP_REG + hw->hw_addr, 0x00000008);
	mtmsr(msr);		/* enable interrupts again */

	hw->bis = bis;
	hw->first_init = 1;

	return 1;
}

static void handle_xmit(struct eth_device *dev)
{
	EMAC_440GX_HW_PST hw = dev->priv;

	while (hw->tx_inq) {
		uint16_t error;

		/* TX is still ongoing */
		if (hw->tx[hw->tx_done].ctrl & MAL_TX_CTRL_READY)
			break;

		/* Error handling */
		error = hw->tx[hw->tx_done].ctrl & 0x03ff;
		if (error) {
			hw->stats.tx_err++;
			hw->stats.tx_err_log[hw->stats.tx_err_index] = error;
			hw->stats.tx_err_index++;
			if (hw->stats.tx_err_index == MAX_ERR_LOG)
				hw->stats.tx_err_index = 0;
		}
		if (hw->tx_inq > hw->stats.tx_high_water)
			hw->stats.tx_high_water = hw->tx_inq;
		hw->tx_done++;
		if (hw->tx_done == NUM_TX_BUFF)  /* Warp MAL TX Buffer */
			hw->tx_done = 0;
		hw->tx_inq--;
	}
}

static int fsp_eth_send(struct eth_device *dev, void *ptr, int len)
{
	struct enet_frame *ef_ptr;
	ulong time_start, time_now;
	EMAC_440GX_HW_PST hw = dev->priv;
	uint32_t msr;

	msr = mfmsr();
	mtmsr(msr & ~MSR_EE);

	/* make sure that there is a free transmit buffer */
	time_start = get_timer(0);
	while (1) {
		if ((hw->tx[hw->tx_slot].ctrl & MAL_TX_CTRL_READY) == 0)
			break;	/* TX slot is free */

		/*
		 * Timeout after 2 ms. After this time one frame must
		 * be sent! (10Mbit and 1518 Bytes takes 1.21 msec)
		 *
		 * Timer tick is at 1/CFG_HZ which is 1/1000 = 1msec
		 */
		time_now = get_timer(0);
		if ((time_now - time_start) > 50) {
			hw->stats.tx_discard++;	/* Lost frame */
			mtmsr(msr);
			return -1;
		}
	}

	handle_xmit(dev);

	ef_ptr = (struct enet_frame *)ptr;

	/* Copy in our address into the frame. */
	memcpy(ef_ptr->source_addr, dev->enetaddr, ENET_ADDR_LENGTH);

	/* If frame is too long or too short, modify length. */
	/* TBS: where does the fragment go???? */
	if (len > ENET_MAX_MTU)
		len = ENET_MAX_MTU;

	memcpy((void *) hw->txbuf[hw->tx_slot], (const void *)ptr, len);
	enet_wback_inv((void *)hw->txbuf[hw->tx_slot], ENET_MAX_MTU);

	/* set TX Buffer busy, and send it */
	hw->tx[hw->tx_slot].data_len = (short) len;
	hw->tx[hw->tx_slot].ctrl =
		(MAL_TX_CTRL_LAST  | MAL_TX_CTRL_INTR |
		 EMAC_TX_CTRL_GFCS | EMAC_TX_CTRL_GP) &
		~(EMAC_TX_CTRL_ISA | EMAC_TX_CTRL_RSA);
	if ((NUM_TX_BUFF - 1) == hw->tx_slot)
		hw->tx[hw->tx_slot].ctrl |= MAL_TX_CTRL_WRAP;

	hw->tx[hw->tx_slot].ctrl |= MAL_TX_CTRL_READY;

	hw->stats.pkts_tx++;
	hw->stats.tx += len;
	hw->tx_inq++;
	hw->tx_slot++;
	if (hw->tx_slot == NUM_TX_BUFF)
		hw->tx_slot = 0;

	/* Trigger packet transmission */
	out32(EMAC_TXM0 + hw->hw_addr, EMAC_TXM0_GNP0);

	mtmsr(msr);
	return len;
}

/* FIXME This interrupt never occured. Code is untested and a best guess. */
static void int_mal_txeob(void *d)
{
	struct eth_device *dev = d;
	EMAC_440GX_HW_PST hw = dev->priv;

	hw->mal_txeob_count++;
	hw->mal_txeobisr = getmal(hw->mal_offs, MAL0_TXEOBISR);
	handle_xmit(dev);
	setmal(hw->mal_offs, MAL0_TXEOBISR, hw->mal_txeobisr);
}

/* FIXME This interrupt never occured. Code is untested and a best guess. */
static void int_mal_serr(void *d)
{
	struct eth_device *dev = d;
	EMAC_440GX_HW_PST hw = dev->priv;

	hw->mal_serr_count++;
	hw->stats.rx_err++;

	hw->emac_isr = in32(EMAC_ISR + hw->hw_addr);
	out32(EMAC_ISR + hw->hw_addr, hw->emac_isr); /* Clear EMAC ISR */
	hw->mal_esr = getmal(hw->mal_offs, MAL0_ESR);
	setmal(hw->mal_offs, MAL0_ESR, hw->mal_esr); /* Clear MAL ESR */

	rx_recovery(dev);
}

static void int_mal_rxeob(void *d)
{
	struct eth_device *dev = d;
	EMAC_440GX_HW_PST hw = dev->priv;

	hw->mal_rxeob_count++;
	hw->mal_rxeobisr = getmal(hw->mal_offs, MAL0_RXEOBISR);
	enet_rcv(dev);
	/* irq_disable(hw->mal_irq_rxeob); */	/* reenable in net-loop */
	setmal(hw->mal_offs, MAL0_RXEOBISR, hw->mal_rxeobisr);
}

#if defined(CFG_RX_DISABLED_CHECKING)
/*
 * RX Descriptor Error Interrupt happens if no free rx descriptor is
 * available for the MAL (e.g. overflow).
 */
static int mal_rx_channel_enabled(struct eth_device *dev)
{
	EMAC_440GX_HW_PST hw = dev->priv;
	return (getmal(hw->mal_offs, MAL0_RXCASR) & hw->mal_rx_chan_mask);
}

static void reenable_receive(struct eth_device *dev)
{
	EMAC_440GX_HW_PST hw = dev->priv;

	hw->stats.reenable_rx_count++;
	rx_recovery(dev);
}
#endif

/*
 * In contrast to the previous versions, which disabled receiving and
 * recovered in the poll loop, I am trying here a different more local
 * error handling similar to the one done by Eberhad Amanns zBIOS.
 */
static void int_mal_rxde(void *d)
{
	struct eth_device *dev = d;
	EMAC_440GX_HW_PST hw = dev->priv;

	hw->mal_rxde_count++;
	hw->stats.rx_err++;
	hw->mal_rxdeir = getmal(hw->mal_offs, MAL0_RXDEIR);
	hw->mal_esr = getmal(hw->mal_offs, MAL0_ESR);

	/* Clear status bits */
	setmal(hw->mal_offs, MAL0_ESR, hw->mal_esr);
	setmal(hw->mal_offs, MAL0_RXDEIR, hw->mal_rxdeir);

	rx_recovery(dev);
}

/* FIXME do recovery */
static void int_emac(void *d)
{
	struct eth_device *dev = d;
	EMAC_440GX_HW_PST hw = dev->priv;

	hw->emac_eth_count++;
	hw->emac_isr = in32(EMAC_ISR + hw->hw_addr);
	hw->mal_esr = getmal(hw->mal_offs, MAL0_ESR);

	hw->stats.emac_isr[hw->stats.emac_isr_index++] = hw->emac_isr;
	if (hw->stats.emac_isr_index == MAX_ERR_LOG)
		hw->stats.emac_isr_index = 0;

	hw->stats.mal_esr[hw->stats.mal_esr_index++] = hw->mal_esr;
	if (hw->stats.mal_esr_index == MAX_ERR_LOG)
		hw->stats.mal_esr_index = 0;

	if (hw->emac_isr & EMAC_ISR_TXPE) {
		hw->stats.tx_parity_err++;
		/* hw->stats.tx_err++; */ /* Cannot determine affected pkt! */
		if(hw->stats.tx_parity_err >= 999) {
			/* after 999 TXPE interrupts disable this interrupt */
			hw->emac_ier &= ~(EMAC_ISR_TXPE);
			out32(EMAC_IER + hw->hw_addr, hw->emac_ier);
		}
	}

	/*
	 * EMAC4_Cu08_ER.pdf #3 Parity Error Indication
	 * EMAC4 has two mechanisms used to signal parity error detection:
	 *  1. Through the receive/transmit status port, delivered to MAL.
	 *     Bit 5 in the status word supplied by EMAC to
	 *     MAL (see "MCMAL TX Control/Status Port" and "MCMAL RX Status
	 *     Port" in the EMAC4 databook (IBM document #SA14-2480-0x))
	 *     indicates parity errors detected on the SRAM array.
	 *  2. Through the interrupt mechanism, asserting the EMC_INT signal
	 *     which can be connected to a UIC. The first parity error
	 *     detection method does not work, meaning, EMAC will never
	 *     indicate parity error through the transmit/receive status port
	 *     (conveyed by the OPB bus to MAL).
	 * Impact
	 * MAL will not know that EMAC has encountered a parity error (since
	 * the conveyed status will never indicate such an error).
	 * Workaround(s)
	 * Use the second mechanism to detect parity errors on EMAC4's SRAMs.
	 *
	 * FIMXE We need to think about a situation when beeing in an
	 * rxeob interrupt processing packets, in this case this irq
	 * will not trigger and we might pass a packet with parity
	 * problems to the upper layer. We need to check the
	 * EMAC_ISR_RXPE bit in the rxeob ISR on every packet we
	 * touch.
	 */
	if (hw->emac_isr & EMAC_ISR_RXPE) {
		hw->stats.rx_parity_err++;
		/* hw->stats.rx_err++; */ /* Cannot determine affected pkt! */
		rx_recovery(dev);
		if(hw->stats.rx_parity_err >= 999) {
			/* after 999 RXPE interrupts disable this interrupt */
			hw->emac_ier &= ~(EMAC_ISR_RXPE);
			out32(EMAC_IER + hw->hw_addr, hw->emac_ier);
		}
	}

	/* clear the pending emac interrupt flags */
	out32(EMAC_ISR + hw->hw_addr, hw->emac_isr);
}

/*
 * FIXME This function is untested. I have never seen this interrupt
 * to trigger yet.
 */
static void int_mal_txde(void *d)
{
	struct eth_device *dev = d;
	EMAC_440GX_HW_PST hw = dev->priv;

	hw->mal_txde_count++;
	hw->mal_txdeir = getmal(hw->mal_offs, MAL0_TXDEIR);
	/* FIXME Do something more usefull */

	setmal(hw->mal_offs, MAL0_TXCARR, hw->mal_tx_chan_mask); /* Reset */
	tx_descr_init(dev);

	/* Reenable the transmit channel */
	setmal(hw->mal_offs, MAL0_TXCASR, hw->mal_tx_chan_mask); /* Reenable */
	hw->stats.tx_err++;

	setmal(hw->mal_offs, MAL0_TXDEIR, hw->mal_txdeir);
}

static int fsp_eth_rx(struct eth_device *dev)
{
	int length;
	int user_index;
	unsigned long msr;
	EMAC_440GX_HW_PST hw = dev->priv;

	for (;;) {
		msr = mfmsr();
		mtmsr(msr & ~MSR_EE);

		user_index = hw->rx_ready[hw->rx_u_index];
		if (user_index == -1) {
			length = -1;
#if defined(CFG_RX_DISABLED_CHECKING)
			/* Check if we have to reenable MAL receive channel*/
			if (!mal_rx_channel_enabled(dev))
				reenable_receive(dev);
#endif
			mtmsr (msr); /* Enable IRQ's */
			break;	/* nothing received - leave for() loop */
		}

		length = hw->rx[user_index].data_len;
		/* Pass the packet up to the protocol layers. */
		net_process_received_packet(net_rx_packets[user_index], length - 4);
		enet_wback_inv((void *)net_rx_packets[user_index],
			       ENET_MAX_MTU_ALIGNED);

		/* Free Recv Buffer */
		hw->rx[user_index].ctrl |= MAL_RX_CTRL_EMPTY;

		/* Free rx buffer descriptor queue */
		hw->rx_ready[hw->rx_u_index] = -1;
		hw->rx_u_index++;
		if (NUM_RX_BUFF == hw->rx_u_index)
			hw->rx_u_index = 0;

		hw->stats.pkts_handled++;
		mtmsr(msr);	/* Enable IRQ's */
	}

	/* irq_enable(hw->mal_irq_rxeob); */
	return length;
}

/*
 * FIXME Figure out how to remove the ugly ifdefs.
 */
int fsp_eth_initialize(bd_t *bis)
{
	struct eth_device *dev;
	int eth_num = 0;
	EMAC_440GX_HW_PST hw = NULL;

	/* set phy num and mode */
	bis->bi_phynum[0] = CONFIG_PHY_ADDR;
	bis->bi_phynum[1] = CONFIG_PHY1_ADDR;

	for (eth_num = 0; eth_num < EMAC_NUM_DEV; eth_num++) {

		dev = &eth_dev[eth_num];
		memset(dev, 0, sizeof(*dev));

		hw = &eth_hw[eth_num];
		memset(hw, 0, sizeof(*hw));

		dev->priv = (void *)hw;
		dev->init = fsp_eth_init;
		dev->halt = fsp_eth_halt;
		dev->send = fsp_eth_send;
		dev->recv = fsp_eth_rx;

		sprintf(dev->name, "%s%d", FSP_ETH_DEVNAME, eth_num);

		hw->magic = ETH_MAGIC;
		hw->devnum = eth_num;
		hw->mal_tx_chan = 0;
		hw->mal_tx_chan_mask = MAL_CHANNEL_MASK(hw->mal_tx_chan);
		hw->mal_rx_chan = 0;
		hw->mal_rx_chan_mask = MAL_CHANNEL_MASK(hw->mal_rx_chan);

		hw->emac_offsets[0] = EMAC0_OFFSET;
		hw->emac_offsets[1] = EMAC1_OFFSET;
		hw->hw_addr = hw->emac_offsets[eth_num];

		switch (eth_num) {
		case 0:
			hw->mal_offs = 0;
			memcpy(dev->enetaddr, bis->bi_enetaddr, 6);

			hw->mal_irq_rxeob = VECNUM_MAL_RXEOB; /* RX EOB */
			hw->mal_irq_txeob = VECNUM_MAL_TXEOB; /* TX EOB */
			hw->mal_irq_rxde  = VECNUM_MAL_RXDE; /* RX DE */
			hw->mal_irq_txde  = VECNUM_MAL_TXDE; /* TX DE */
			hw->emac_irq_eth  = VECNUM_ETH0; /* EMAC */
			hw->mal_irq_serr  = VECNUM_MAL_SERR; /* SERR */
			break;
		case 1:
			hw->mal_offs = MAL1_OFFSET;
			memcpy(dev->enetaddr, bis->bi_enet1addr, 6);
			hw->mal_irq_rxeob = VECNUM_MAL1_RXEOB;
			hw->mal_irq_txeob = VECNUM_MAL1_TXEOB;
			hw->mal_irq_rxde  = VECNUM_MAL1_RXDE;
			hw->mal_irq_txde  = VECNUM_MAL1_TXDE;
			hw->emac_irq_eth  = VECNUM_ETH1; /* EMAC */
			hw->mal_irq_serr  = VECNUM_MAL1_SERR;
			break;
		}

		setmal(hw->mal_offs, MAL0_ESR,    0xffffffff); /* Clear irqs */
		setmal(hw->mal_offs, MAL0_TXDEIR, 0xffffffff);
		setmal(hw->mal_offs, MAL0_RXDEIR, 0xffffffff);
		setmal(hw->mal_offs, MAL0_IER,
		       (MAL_IER_PT  | /* PLB Timeout */
			MAL_IER_PRE | /* PLB Read Err */
			MAL_IER_PWE | /* PLB Write Error Interrupt */
			MAL_IER_DE  | /* Descriptor Error Interrupt */
			MAL_IER_OTE | /* OPB Timeout Error Interrupt */
			MAL_IER_OE  | /* OPB Slave Error Interrupt */
			MAL_IER_PE)); /* OPB MRQ Interrupt */
		irq_install_handler(hw->mal_irq_txeob, int_mal_txeob, dev);
		irq_install_handler(hw->mal_irq_rxeob, int_mal_rxeob, dev);
		irq_install_handler(hw->mal_irq_txde, int_mal_txde, dev);
		irq_install_handler(hw->mal_irq_rxde, int_mal_rxde, dev);
		irq_install_handler(hw->emac_irq_eth, int_emac, dev);
		irq_install_handler(hw->mal_irq_serr, int_mal_serr, dev);
		miiphy_init_phy_params(&hw->phyp, hw->devnum);
                miiphy_set_curr_emac_offset(hw->emac_offsets[hw->phyp.owner]);
		rgmii_get_mdio(hw->devnum);
		setup_phy_leds(dev);
		rgmii_put_mdio(hw->devnum);
		eth_register (dev);

#if  defined(CONFIG_PHYLIB) || defined(CONFIG_CMD_MII)
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();

	if (!mdiodev)
		return -ENOMEM;
	strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
	mdiodev->read = emac4_mdio_read;
	mdiodev->write = emac4_mdio_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;
#endif

		//fsp_eth_init(dev, bis);
	}
	return 1;
}

int ppc_4xx_eth_initialize(bd_t *bis)
{
	return fsp_eth_initialize(bis);
}

static void eth_print_buffers(struct eth_device *dev)
{
	int i;
	EMAC_440GX_HW_PST hw = dev->priv;

	if ((!hw) || (!hw->tx) || (!hw->rx))
		return;

	for (i = 0; i < NUM_TX_BUFF; i++) {
		printf(" T%02d %08lx c=%04x l=%04x ", i,
		       (ulong)hw->tx[i].data_ptr, (ushort)hw->tx[i].ctrl,
		       hw->tx[i].data_len);
		if (MAL_TX_CTRL_READY & hw->tx[i].ctrl)
			puts("r ");
		else
			puts("e ");
		if (hw->rx_slot == i) putc('*'); else putc(' ');
		if (i & 1)
			putc('\n');

	}
	if (NUM_TX_BUFF & 0x1) putc('\n');
	for (i = 0; i < NUM_RX_BUFF; i++) {
		printf(" R%02d %08lx c=%04x l=%04x ", i,
		       (ulong)hw->rx[i].data_ptr, (ushort)hw->rx[i].ctrl,
		       hw->rx[i].data_len);
		if (MAL_RX_CTRL_EMPTY & hw->rx[i].ctrl)
			puts("e ");
		else
			puts("f ");
		if (hw->rx_slot == i) putc('*'); else putc(' ');
		if (i & 1)
			putc('\n');
	}
	/* printf(" rx_ready: ");
	 * for (i = 0; i < NUM_RX_BUFF; i++)
	 *	printf("%d ", hw->rx_ready[i]);
	 * printf("\n");
	 */
}

static void print_emac_isr(uint32_t emac_isr)
{
	printf("EMACISR: %08x\n", emac_isr);
	if (emac_isr & EMAC_ISR_TXPE)
		printf("  Tx Parity Err\n");
	if (emac_isr & EMAC_ISR_RXPE)
		printf("  Rx Parity Err\n");
	if (emac_isr & EMAC_ISR_TX_TFAE)
		printf("  Tx Fifo Almost Empty\n");
	if (emac_isr & EMAC_ISR_RX_RFAF)
		printf("  Rx Fifo Almost Full\n");
	if (emac_isr & EMAC_ISR_OVR)
		printf("  Overrun\n");
	if (emac_isr & EMAC_ISR_PP)
		printf("  Pause Frame\n");
	if (emac_isr & EMAC_ISR_BP)
		printf("  Bad Frame\n");
	if (emac_isr & EMAC_ISR_RP)
		printf("  Runt Frame\n");
	if (emac_isr & EMAC_ISR_SE)
		printf("  Short Event\n");
	if (emac_isr & EMAC_ISR_SYE)
		printf("  Alignment Err\n");
	if (emac_isr & EMAC_ISR_BFCS)
		printf("  Bad FCS\n");
	if (emac_isr & EMAC_ISR_PTLE)
		printf("  Frame Is Too Long\n");
	if (emac_isr & EMAC_ISR_ORE)
		printf("  Out of Range Err\n");
	if (emac_isr & EMAC_ISR_IRE)
		printf("  In Range Err\n");
	if (emac_isr & EMAC_ISR_DB0)
		printf("  Dead Bit\n");
	if (emac_isr & EMAC_ISR_SE0)
		printf("  SQE Error\n");
	if (emac_isr & EMAC_ISR_TE0)
		printf("  Transmit Err\n");
	if (emac_isr & EMAC_ISR_MOS)
		printf("  MMA Op Succeeded\n");
	if (emac_isr & EMAC_ISR_MOF)
		printf("  MMA Op Failed\n");
}

static void print_mal_esr(uint32_t mal_esr)
{
	printf("MALESR: %08x\n", mal_esr);
	printf("  ChannelMask: 0x%02x\n", (mal_esr & MAL_ESR_CID) >> 25);
	if (mal_esr & MAL_ESR_PLBTE)
		printf("  PLB Timeout Err\n");
	if (mal_esr & MAL_ESR_PLBRE)
		printf("  PLB Read Err\n");
	if (mal_esr & MAL_ESR_PLBWE)
		printf("  PLB Write Err\n");
	if (mal_esr &  MAL_ESR_DE)
		printf("  Descriptor Err\n");
	if (mal_esr & MAL_ESR_OTE)
		printf("  OPB Timeout Err\n");
	if (mal_esr & MAL_ESR_OSE)
		printf("  OPB Slave Err\n");
	if (mal_esr & MAL_ESR_PBEI)
		printf("  PLB MIRQ has occured\n");
}

static void eth_print_stats(struct eth_device *dev)
{
	int i;
	EMAC_440GX_HW_PST hw = dev->priv;
	uint32_t elapsed = hw->stats.time_end - hw->stats.time_start;

	printf(" pkts_rx ..........: %d\n"
	       " rx ...............: %d bytes\n"
	       " rx_err ...........: %d\n"
	       " data_len_err .....: %d\n"
	       " reenable_rx_count : %d\n"
	       " rx_high_water ....: %d\n"
	       " rx_parity_err ....: %d\n"
	       " pkts_tx ..........: %d\n"
	       " tx ...............: %d bytes\n"
	       " tx_err ...........: %d\n"
	       " tx_discard .......: %d\n"
	       " tx_high_water ....: %d\n"
	       " tx_parity_err ....: %d\n"
	       " pkts_handled .....: %d\n"
	       " emac_isr .........: %08x "
	       " mal_esr ..........: %08x\n"
	       " mal_txeobisr .....: %08x "
	       " mal_rxeobisr .....: %08x\n"
	       " mal_txdeir .......: %08x "
	       " mal_rxdeir .......: %08x\n"
	       " elapsed ..........: %d\n"
	       " irqs: rxeob=%d txeob=%d rxde=%d txde=%d serr=%d emac=%d\n",
	       hw->stats.pkts_rx, hw->stats.rx, hw->stats.rx_err,
	       hw->stats.data_len_err, hw->stats.reenable_rx_count,
	       hw->stats.rx_high_water, hw->stats.rx_parity_err,
	       hw->stats.pkts_tx, hw->stats.tx, hw->stats.tx_err,
	       hw->stats.tx_discard, hw->stats.tx_high_water,
	       hw->stats.tx_parity_err, hw->stats.pkts_handled,
	       hw->emac_isr, hw->mal_esr, hw->mal_txeobisr, hw->mal_rxeobisr,
	       hw->mal_txdeir, hw->mal_rxdeir, elapsed, hw->mal_rxeob_count,
	       hw->mal_txeob_count, hw->mal_rxde_count,  hw->mal_txde_count,
	       hw->mal_serr_count, hw->emac_eth_count);

	printf(" tx_err_log:  ");
	for (i = 0; i < MAX_ERR_LOG; i++)
		printf(" %04x", hw->stats.tx_err_log[i]);
	printf("\n rx_err_log:  ");
	for (i = 0; i < MAX_ERR_LOG; i++)
		printf(" %04x", hw->stats.rx_err_log[i]);
	printf("\n emac_isr:  ");
	for (i = 0; i < MAX_ERR_LOG; i++)
		printf(" %x", hw->stats.emac_isr[i]);
	printf("\n mal_esr:  ");
	for (i = 0; i < MAX_ERR_LOG; i++)
		printf(" %x", hw->stats.mal_esr[i]);

	printf("\n");
	if (hw->mal_esr)
		print_mal_esr(hw->mal_esr);
	if (hw->emac_isr)
		print_emac_isr(hw->emac_isr);
}

#if defined(CFG_EXTENDED_REGISTER_DUMP)
static void eth_dump_regs(struct eth_device *dev)
{
	EMAC_440GX_HW_PST hw = dev->priv;

	printf("Eth Register Dump:\n");
	print_mal_esr(getmal(hw->mal_offs, MAL0_ESR));
	print_emac_isr(in32(EMAC_ISR + hw->hw_addr));
	printf("MAL_RXCASR:  %08x ", getmal(hw->mal_offs, MAL0_RXCASR));
	if (getmal(hw->mal_offs, MAL0_RXCASR) & hw->mal_rx_chan_mask)
		puts("enabled\n"); else puts("disabled\n");
	printf("MAL_TXCASR:  %08x ", getmal(hw->mal_offs, MAL0_TXCASR));
	if (getmal(hw->mal_offs, MAL0_TXCASR) & hw->mal_tx_chan_mask)
		puts("enabled\n"); else puts("disabled\n");
	printf("MAL_RXDEIR:  %08x ", getmal(hw->mal_offs, MAL0_RXDEIR));
	printf("MAL_TXDEIR:  %08x ", getmal(hw->mal_offs, MAL0_TXDEIR));
	printf("MAL_TXCTP0R: %08x\n", getmal(hw->mal_offs, MAL0_TXCTP0R));
	printf("MAL_RXCTP0R: %08x ", getmal(hw->mal_offs, MAL0_RXCTP0R));
	printf("EMAC_M0:     %08x ", in32(EMAC_M0 + hw->hw_addr));
	printf("EMAC_M1:     %08x\n", in32(EMAC_M1 + hw->hw_addr));
	printf("EMAC_RXM:    %08x ", in32(EMAC_RXM + hw->hw_addr));
	printf("EMAC_TXM0:   %08x ", in32(EMAC_TXM0 + hw->hw_addr));
	printf("EMAC_ISR:    %08x\n", in32(EMAC_ISR + hw->hw_addr));
	printf("EMAC_IER:    %08x\n", in32(EMAC_IER + hw->hw_addr));
}
#endif /* defined(CFG_EXTENDED_REGISTER_DUMP) */

static void eth_print_errors(struct eth_device *dev)
{
	printf("Interface: %s\n", dev->name);
	eth_print_buffers(dev);
	eth_print_stats(dev);
#if defined(CFG_EXTENDED_REGISTER_DUMP)
	eth_dump_regs(dev);
#endif
}

static int do_ethinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct eth_device *dev = eth_get_dev();

	if (!dev)
		return -1;
	eth_print_errors(dev);
	return 0;
}

U_BOOT_CMD(ethinfo, CONFIG_SYS_MAXARGS, 1, do_ethinfo,
	   "Print eth driver debug info\n",
	   ""
);

/*
static const struct eth_ops emac4_ops = {
	.start		= emac4_start,
	.send		= emac4_send,
	.recv		= emac4_recv,
	.free_pkt	= emac4_free_pkt,
	.stop		= emac4_stop,
	.write_hwaddr	= emac4_write_hwaddr,
};

static const struct udevice_id emac4_ids[] = {
	{ .compatible = "ibm,emac4" },
	{}
};

U_BOOT_DRIVER(emac4) = {
	.name	= "emac4",
	.id	= UCLASS_ETH,
	.of_match = emac4_ids,
	.ops	= &emac4_ops,
	.ofdata_to_platdata = emac4_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.priv_auto_alloc_size = sizeof(struct emac4_priv),
	.probe	= emac4_probe,
}
*/
