/*
 * (C) Copyright 2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _PPC476FSP2_H_
#define _PPC476FSP2_H_

#define L2_CACHE_SHIFT	7
#define L2_CACHE_BYTES	(1 << L2_CACHE_SHIFT)

/* FIXME clean up file for FSP2 */

#define CONFIG_SDRAM_PPC4xx_IBM_DDR	/* IBM DDR controller */

/*
 * Some SoC specific registers (not common for all 440 SoC's)
 */

#define MAL_DCR_BASE	0x080
#define MAL1_OFFSET	0x080

/* Memory mapped register */
#define CONFIG_SYS_PERIPHERAL_BASE	0xb0000000 /* Internal Peripherals */

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_PERIPHERAL_BASE + 0x20000)

#define SDR0_PCI0	0x0300

#define CPC0_STRP1_PAE_MASK		(0x80000000 >> 11)
#define CPC0_STRP1_PISE_MASK		(0x80000000 >> 13)

#define CNTRL_DCR_BASE	0x0b0

#define CPC0_SYS0	(CNTRL_DCR_BASE + 0x30)	/* System configuration reg 0 */
#define CPC0_SYS1	(CNTRL_DCR_BASE + 0x31)	/* System configuration reg 1 */

#define CPC0_STRP0	(CNTRL_DCR_BASE + 0x34)	/* Power-on config reg 0 (RO) */
#define CPC0_STRP1	(CNTRL_DCR_BASE + 0x35)	/* Power-on config reg 1 (RO) */

#define CPC0_GPIO	(CNTRL_DCR_BASE + 0x38)	/* GPIO config reg (440GP) */

#define CPC0_CR0	(CNTRL_DCR_BASE + 0x3b)	/* Control 0 register */
#define CPC0_CR1	(CNTRL_DCR_BASE + 0x3a)	/* Control 1 register */

#define PLLSYS0_TUNE_MASK	0xffc00000	/* PLL TUNE bits	    */
#define PLLSYS0_FB_DIV_MASK	0x003c0000	/* Feedback divisor	    */
#define PLLSYS0_FWD_DIV_A_MASK	0x00038000	/* Forward divisor A	    */
#define PLLSYS0_FWD_DIV_B_MASK	0x00007000	/* Forward divisor B	    */
#define PLLSYS0_OPB_DIV_MASK	0x00000c00	/* OPB divisor		    */
#define PLLSYS0_EPB_DIV_MASK	0x00000300	/* EPB divisor		    */
#define PLLSYS0_EXTSL_MASK	0x00000080	/* PerClk feedback path	    */
#define PLLSYS0_RW_MASK		0x00000060	/* ROM width		    */
#define PLLSYS0_RL_MASK		0x00000010	/* ROM location		    */
#define PLLSYS0_ZMII_SEL_MASK	0x0000000c	/* ZMII selection	    */
#define PLLSYS0_BYPASS_MASK	0x00000002	/* Bypass PLL		    */
#define PLLSYS0_NTO1_MASK	0x00000001	/* CPU:PLB N-to-1 ratio	    */

#define PCIL0_BRDGOPT1		(PCIL0_CFGBASE + 0x0040)
#define PCIL0_BRDGOPT2		(PCIL0_CFGBASE + 0x0044)

/*=============================================================================+
|  PLB-Attached DDR3 Core Wrapper
+=============================================================================*/
#define CW_BASE		0x11111800

#define CW_ERR0		(CW_BASE + 0x00)
#define ERR0_MEM_UE	0x00040000
#define ERR0_MEM_CE	0x00020000
#define CW_ERR1		(CW_BASE + 0x01)
#define CW_ERR0_AND	(CW_BASE + 0x02)
#define CW_ERR1_AND	(CW_BASE + 0x03)
#define CW_ERR0_OR	(CW_BASE + 0x04)
#define CW_ERR1_OR	(CW_BASE + 0x05)

#define CW_MC_LFIR	(CW_BASE + 0x10)
#define   MC_LFIR_RE	0x80000000
#define   MC_LFIR_UE	0x40000000
#define CW_MC_LFIR_RST	(CW_BASE + 0x11)


/*=============================================================================+
|  Memory controller
+=============================================================================*/
#define MCIF_BASE	0x11120000

#define MCIF_MCSTAT		(MCIF_BASE + 0x0010)
#define PREFILL_COMP		0x08000000
#define MCIF_MCOPT2		(MCIF_BASE + 0x0021)
#define MCIF_MCOPT2_MC_ENABLE	0x10000000
#define MCIF_SCRUB_CNTL		(MCIF_BASE + 0x00AA)
#define SCRUB_CNTL_MODE_DIS	0x00000000
#define SCRUB_CNTL_MODE_FILL	0x40000000
#define SCRUB_CNTL_MODE_EN	0x80000000
#define SCRUB_CNTL_UE_RESP	0x20000000
#define SCRUB_CNTL_CE_RESP	0x10000000
#define SCRUB_RANK0_PORT0	0x00008000
#define MCIF_SCRUB_INT		(MCIF_BASE + 0x00AB)
#define MCIF_SCRUB_CUR		(MCIF_BASE + 0x00AC)
#define MCIF_PREFILL_DATA	(MCIF_BASE + 0x00AE)
#define MCIF_SCRUB_START	(MCIF_BASE + 0x00B0)
#define MCIF_SCRUB_STOP		(MCIF_BASE + 0x00D0)
#define MCIF_ECCERR_ADDR	(MCIF_BASE + 0x00E0)
#define MCIF_ECCERR_STATUS_CLR	(MCIF_BASE + 0x00F0)
#define CE_BIT_ID_MASK		0xFF000000

#define SCRUB_CNTL		(MCIF_BASE + 0x00AA)
#define   CNTL_MODE		0xC0000000
#define   CNTL_MODE_DIS		0x00000000
#define   CNTL_MODE_FILL	0x40000000
#define   CNTL_MODE_SCRUB	0x80000000
#define   CNTL_UE_RESP		0x20000000
#define   CNTL_CE_RESP		0x10000000
#define   CNTL_SCRUB_RANGE_EN	0x0000FFFF
#define SCRUB_INT		(MCIF_BASE + 0x00AB)
#define SCRUB_CUR		(MCIF_BASE + 0x00AC)
#define SCRUB_CUR_EXT		(MCIF_BASE + 0x00AD)
#define SCRUB_PREFILL0		(MCIF_BASE + 0x00AE)
#define SCRUB_PREFILL1		(MCIF_BASE + 0x00AF)
#define SCRUB_ST_RANK0_PORT0	(MCIF_BASE + 0x00B0)
#define SCRUB_ST_RANK0_PORT1	(MCIF_BASE + 0x00B1)
#define SCRUB_ST_RANK0_PORT2	(MCIF_BASE + 0x00B2)
#define SCRUB_ST_RANK0_PORT3	(MCIF_BASE + 0x00B3)
#define SCRUB_ST_RANK1_PORT0	(MCIF_BASE + 0x00B4)
#define SCRUB_ST_RANK1_PORT1	(MCIF_BASE + 0x00B5)
#define SCRUB_ST_RANK1_PORT2	(MCIF_BASE + 0x00B6)
#define SCRUB_ST_RANK1_PORT3	(MCIF_BASE + 0x00B7)
#define SCRUB_ST_RANK2_PORT0	(MCIF_BASE + 0x00B8)
#define SCRUB_ST_RANK2_PORT1	(MCIF_BASE + 0x00B9)
#define SCRUB_ST_RANK2_PORT2	(MCIF_BASE + 0x00BA)
#define SCRUB_ST_RANK2_PORT3	(MCIF_BASE + 0x00BB)
#define SCRUB_ST_RANK3_PORT0	(MCIF_BASE + 0x00BC)
#define SCRUB_ST_RANK3_PORT1	(MCIF_BASE + 0x00BD)
#define SCRUB_ST_RANK3_PORT2	(MCIF_BASE + 0x00BE)
#define SCRUB_ST_RANK3_PORT3	(MCIF_BASE + 0x00BF)
#define SCRUB_END_RANK0_PORT0	(MCIF_BASE + 0x00D0)
#define SCRUB_END_RANK0_PORT1	(MCIF_BASE + 0x00D1)
#define SCRUB_END_RANK0_PORT2	(MCIF_BASE + 0x00D2)
#define SCRUB_END_RANK0_PORT3	(MCIF_BASE + 0x00D3)
#define SCRUB_END_RANK1_PORT0	(MCIF_BASE + 0x00D4)
#define SCRUB_END_RANK1_PORT1	(MCIF_BASE + 0x00D5)
#define SCRUB_END_RANK1_PORT2	(MCIF_BASE + 0x00D6)
#define SCRUB_END_RANK1_PORT3	(MCIF_BASE + 0x00D7)
#define SCRUB_END_RANK2_PORT0	(MCIF_BASE + 0x00D8)
#define SCRUB_END_RANK2_PORT1	(MCIF_BASE + 0x00D9)
#define SCRUB_END_RANK2_PORT2	(MCIF_BASE + 0x00DA)
#define SCRUB_END_RANK2_PORT3	(MCIF_BASE + 0x00DB)
#define SCRUB_END_RANK3_PORT0	(MCIF_BASE + 0x00DC)
#define SCRUB_END_RANK3_PORT1	(MCIF_BASE + 0x00DD)
#define SCRUB_END_RANK3_PORT2	(MCIF_BASE + 0x00DE)
#define SCRUB_END_RANK3_PORT3	(MCIF_BASE + 0x00DF)
#define ECCERR_COUNT_PORT0	(MCIF_BASE + 0x00E4)
#define ECCERR_COUNT_PORT1	(MCIF_BASE + 0x00E5)
#define ECCERR_COUNT_PORT2	(MCIF_BASE + 0x00E6)
#define ECCERR_COUNT_PORT3	(MCIF_BASE + 0x00E7)
#define   ECCERR_COUNT_CE_TCNT	0xff000000
#define   ECCERR_COUNT_CE_COUNT	0x000fffff

/*
 * Register defn's for custom logic
 */
#define DCRN_CMU_ADDR	0x0C
#define DCRN_CMU_DATA	0x0D

#define CMU_CRCS	0x00
#define CRCS_STAT_MASK		0xF0000000
#define CRCS_STAT_POR		0x10000000
#define CRCS_STAT_ALTERNATE_POR 0x00000000 /* Alternate POR */
#define CRCS_STAT_PHR		0x20000000 /* Pinhole reset */
#define CRCS_STAT_PERST		0x30000000 /* PERST_N pin caused PCIe reset*/
#define CRCS_STAT_CRCS_SYS	0x40000000 /* Write to bit 24 of this reg */
#define CRCS_STAT_DBCR_SYS	0x50000000 /* Check TSR */
#define CRCS_STAT_HOST_SYS	0x60000000
#define CRCS_STAT_CHIP_RST_B	0x70000000
#define CRCS_STAT_CRCS_CHIP	0x80000000 /* Write to bit 25 of this reg */
#define CRCS_STAT_DBCR_CHIP	0x90000000 /* Check TSR */
#define CRCS_STAT_HOST_CHIP	0xA0000000
#define CRCS_STAT_PSI_CHIP	0xB0000000
#define CRCS_STAT_CRCS_CORE	0xC0000000 /* Write to bit 26 of this reg */
#define CRCS_STAT_DBCR_CORE	0xD0000000 /* Check TSR */
#define CRCS_STAT_HOST_CORE	0xE0000000
#define CRCS_STAT_PCIE_HOT	0xF0000000
#define CRCS_WATCHE		0x08000000
#define CRCS_CORE		0x04000000
#define CRCS_CHIP		0x02000000
#define CRCS_SYS		0x01000000
#define CRCS_WRCR		0x00800000 /* Watchdog reset on core reset */
#define CRCS_CONF		0x00100000 /* Reset CMU config on reset */
#define CRCS_EXTCR		0x00100000 /* CHIP_RST_B triggers reset */
#define CRCS_SCM		0x00080000 /* SideCar mode pin at POR */

#define CMU_BWTC	0x05
#define BWTC_CFG01_MASK     0xC0000000
#define BWTC_CFG01_DIS      0x00000000
#define BWTC_CFG01_2MB      0x40000000
#define BWTC_CFG01_4MB      0x80000000
#define BWTC_CFG01_8MB      0xC0000000
#define BWTC_CFG2           0x00000800
#define BWTC_WDT_MASK       0x00000700
#define BWTC_WDT_101MS      0x00000000
#define BWTC_WDT_201MS      0x00000100
#define BWTC_WDT_403MS      0x00000200
#define BWTC_WDT_805MS      0x00000300
#define BWTC_WDT_3SEC       0x00000400
#define BWTC_WDT_13SEC      0x00000500
#define BWTC_WDT_26SEC      0x00000600
#define BWTC_WDT_52SEC      0x00000700
#define BWTC_BANK           0x00000002
#define BWTC_WDTE           0x00000001

#define CMU_MCCR	0x3C
#define MCCR_800MHZ	0x00000001
#define MCCR_LFIRUE	0x00000100
#define MCCR_LFIRCE	0x00000080

#define CMU_FIR0	0x3D
#define FIR0_PCIEHR	0x08000000

#define POR_CONF0	0x40
#define CONF0_SPI_PHASE_MASK		0x000001FE
#define CONF0_SPI_PHASE_SHIFT		1
#define CONF0_SPI_PHASE			0x8 /* 41.5Mhz */
#define POR_CONF91	0x9B
#define POR_CONF92	0x9C
#define POR_CONF103	0xA7
#define CONF103_ETHX_PHY_RESET_OE	0x02001000
#define CONF103_ETHX_EN_MASK		0xFDFFEFF1
#define CONF103_EMMC_CLK		0x00000008
#define CONF103_EMMC_RST_B		0x00000004
#define CONF103_EMMC_CMD_DATA_IE	0x00000002

#define URCR0_RWOR	0x01
#define URCR0_RW1C	0x02
#define URCR0_RWORP	0x2D

#define URCR1_RWOR	0x03
#define URCR1_RW1C	0x04
#define URCR1_RWORP	0x2E
#define URCR1_PCIEPLB	0x000008000
#define URCR1_PCIESPE	0x000004000

#define URCR2_RWOR	0x33
#define URCR2_RW1C	0x34
#define URCR2_RWORP	0x2F
#define URCR2_FFU	0x80000000
#define URCR2_PCIERC	0x00000004
#define URCR2_PCIESB	0x00000002
#define URCR2_PCIEPCS	0x00000001

#define POR_CONF96  0xA0
#define POR_CONF102	0xA6
#define POR_CONF105	0xA9
#define CONF105_IOU_I2C_ACK	0x20000000

#define PPCU_CFG_BASE	0x11111400
#define PPCU_CFG_01_RS	(PPCU_CFG_BASE + 0x021)

#define GPIO0_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x27F80)
#define GPIO0_BIDI0	(GPIO0_BASE + 0x00)
#define GPIO0_BIDI0_CLR	(GPIO0_BASE + 0x04)
#define GPIO0_BIDI0_SET	(GPIO0_BASE + 0x08)
#define GPIO0_BIDI1	(GPIO0_BASE + 0x0c)
#define GPIO0_BIDI1_CLR	(GPIO0_BASE + 0x10)
#define GPIO0_BIDI1_SET	(GPIO0_BASE + 0x14)
#define GPIO0_EIR	(GPIO0_BASE + 0x24)

#define MCIF_BASE	0x11120000
#define MCIF_MCOPT1	(MCIF_BASE + 0x20)
#define MCOPT1_ECC_EN	0x10000000

#define EHCI_BASE       0xa2000000
#define EHCI_HC_LENGTH  (in32(EHCI_BASE) & 0x000000ff)
#define EHCI_INSNREG5   (EHCI_BASE + EHCI_HC_LENGTH + 0x94)

/* SFC Registers */
#define SFC_BASE_ADDR	(CONFIG_SYS_PERIPHERAL_BASE+0x10400)
#define SFC_ERRORS	(SFC_BASE_ADDR + 0x00)
#define SFC_INTMSK	(SFC_BASE_ADDR + 0x04)
#define SFC_CLRIMSK	(SFC_BASE_ADDR + 0x08)
#define SFC_STATUS	(SFC_BASE_ADDR + 0x0C)
#define SFC_CONF	(SFC_BASE_ADDR + 0x10)
#define SFC_INTENM	(SFC_BASE_ADDR + 0x14)
#define SFC_CONF2	(SFC_BASE_ADDR + 0x18)
#define SFC_ERRTAG	(SFC_BASE_ADDR + 0x1C)
#define SFC_ERROFF	(SFC_BASE_ADDR + 0x20)
#define SFC_ERRSYN	(SFC_BASE_ADDR + 0x24)
#define SFC_ERRDATH	(SFC_BASE_ADDR + 0x28)
#define SFC_ERRDATL	(SFC_BASE_ADDR + 0x2C)
#define SFC_ERRCNT	(SFC_BASE_ADDR + 0x30)
#define SFC_CLRCNT	(SFC_BASE_ADDR + 0x34)
#define SFC_ERRINJ	(SFC_BASE_ADDR + 0x38)
#define SFC_SPICLK	(SFC_BASE_ADDR + 0x3C)
#define SFC_CMD		(SFC_BASE_ADDR + 0x40)
#define SFC_ADR		(SFC_BASE_ADDR + 0x44)
#define SFC_ERASMS	(SFC_BASE_ADDR + 0x48)
#define SFC_ERALGS	(SFC_BASE_ADDR + 0x4C)
#define SFC_CONF3	(SFC_BASE_ADDR + 0x50)
#define SFC_CONF4	(SFC_BASE_ADDR + 0x54)
#define SFC_CONF5	(SFC_BASE_ADDR + 0x58)
#define SFC_CONF6	(SFC_BASE_ADDR + 0x5C)
#define SFC_CONF7	(SFC_BASE_ADDR + 0x60)
#define SFC_CONF8	(SFC_BASE_ADDR + 0x64)
#define SFC_CONF9	(SFC_BASE_ADDR + 0x68)
#define SFC_ERRCONF	(SFC_BASE_ADDR + 0x6C)
#define SFC_PROTA	(SFC_BASE_ADDR + 0x70)
#define SFC_PROTM	(SFC_BASE_ADDR + 0x74)
#define SFC_ECCADR	(SFC_BASE_ADDR + 0x78)
#define SFC_ECCRNG	(SFC_BASE_ADDR + 0x7C)
#define SFC_ADRCBF	(SFC_BASE_ADDR + 0x80)
#define SFC_ADRCMF	(SFC_BASE_ADDR + 0x84)
#define SFC_ADRCBS	(SFC_BASE_ADDR + 0x88)
#define SFC_ADRCMS	(SFC_BASE_ADDR + 0x8C)
#define SFC_OADRNB	(SFC_BASE_ADDR + 0x90)
#define SFC_OADRNS	(SFC_BASE_ADDR + 0x94)
#define SFC_OADRRB	(SFC_BASE_ADDR + 0x98)
#define SFC_CHIPIDCONF	(SFC_BASE_ADDR + 0x9C)

#define SFC_DATA	(SFC_BASE_ADDR + 0x100)
#define SFC_DATA_SIZE	256

#define SFC_STATUS_DONE	0x00000001

#define SFC_CMD_SHIFT	9
#define SFC_READ_RAW	(0x03 << SFC_CMD_SHIFT)
#define SFC_CMD_4BYTE	(0x37 << SFC_CMD_SHIFT)

#define SPICLK_CYCLE_MASK	0x0000FFFF

/* Bits where the Eth PHY IRQs are connected.  Discovered via email from
 * Barbara/Shaun.  If they can change per card, then make configurable
 * via bootenv key. */
#define GPIO_PHY_IRQS	0x60000000

/*
 * Macros for indirect CMU access
 */
#define mtcmu(reg,d)	\
  ({ mtdcr(DCRN_CMU_ADDR, (reg)); mtdcr(DCRN_CMU_DATA, (d)); })

#define mfcmu(reg)	\
  ({ unsigned int val;\
	  mtdcr(DCRN_CMU_ADDR, (reg)); val = mfdcr(DCRN_CMU_DATA); val; })

#endif /* _PPC476FSP2_H_ */
