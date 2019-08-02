/*
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define MACTEST_C

#include "swfunc.h"
#include "comminf.h"
//#include "io.h"
#include "stduboot.h"
#include <command.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <post.h>
#include "mem_io.h"

#include "phy_api.h"
#include "mac_api.h"

#define ARGV_MAC_IDX		1
#define ARGV_MDIO_IDX		2
#define ARGV_SPEED		3
#define ARGV_CTRL		4
#define ARGV_LOOP		5
#define ARGV_TEST_MODE		6
#define ARGV_PHY_ADDR		7
#define ARGV_TIMING_MARGIN	8


uint8_t __attribute__ ((aligned (1024*1024))) tdes_buf[TDES_SIZE];
uint8_t __attribute__ ((aligned (1024*1024))) rdes_buf[RDES_SIZE];
uint8_t __attribute__ ((aligned (1024*1024))) dma_buf[DMA_BUF_SIZE];

struct mac_ctrl_desc {
	uint32_t base_reset_assert;
	uint32_t bit_reset_assert;
	uint32_t base_reset_deassert;
	uint32_t bit_reset_deassert;

	uint32_t base_clk_stop;
	uint32_t bit_clk_stop;
	uint32_t base_clk_start;
	uint32_t bit_clk_start;
};

#if defined(CONFIG_ASPEED_AST2600)
const uint32_t mac_base_lookup_tbl[4] = {MAC1_BASE, MAC2_BASE, MAC3_BASE,
					 MAC4_BASE};
const uint32_t mdio_base_lookup_tbl[4] = {MDIO0_BASE, MDIO1_BASE, MDIO2_BASE,
					 MDIO3_BASE};
const struct mac_ctrl_desc mac_ctrl_lookup_tbl[4] = {
	{
		.base_reset_assert = 0x40, .bit_reset_assert = BIT(11),
		.base_reset_deassert = 0x44,.bit_reset_deassert = BIT(11),
		.base_clk_stop = 0x80, .bit_clk_stop = BIT(20),
		.base_clk_start = 0x84, .bit_clk_start = BIT(20),
	},
	{
		.base_reset_assert = 0x40, .bit_reset_assert = BIT(12),
		.base_reset_deassert = 0x44,.bit_reset_deassert = BIT(12),
		.base_clk_stop = 0x80, .bit_clk_stop = BIT(21),
		.base_clk_start = 0x84,.bit_clk_start = BIT(21),
	},
	{
		.base_reset_assert = 0x50, .bit_reset_assert = BIT(20),
		.base_reset_deassert = 0x54,.bit_reset_deassert = BIT(20),
		.base_clk_stop = 0x90, .bit_clk_stop = BIT(20),
		.base_clk_start = 0x94, .bit_clk_start = BIT(20),
	},
	{
		.base_reset_assert = 0x50, .bit_reset_assert = BIT(21),
		.base_reset_deassert = 0x54,.bit_reset_deassert = BIT(21),
		.base_clk_stop = 0x90, .bit_clk_stop = BIT(21),
		.base_clk_start = 0x94,.bit_clk_start = BIT(21),
	}
};
#else
const uint32_t mac_base_lookup_tbl[2] = {MAC1_BASE, MAC2_BASE};
const uint32_t mdio_base_lookup_tbl[2] = {MDIO0_BASE, MDIO1_BASE};
const struct mac_ctrl_desc mac_ctrl_lookup_tbl[2] = {
	{
		.base_reset_assert = 0x04, .bit_reset_assert = 11,
		.base_reset_deassert = 0x04,.bit_reset_deassert = 11,
		.base_clk_stop = 0x0c, .bit_clk_stop = 20,
		.base_clk_start = 0x0c, .bit_clk_start = 20,
	},
	{
		.base_reset_assert = 0x04, .bit_reset_assert = 12,
		.base_reset_deassert = 0x04,.bit_reset_deassert = 12,
		.base_clk_stop = 0x0c, .bit_clk_stop = 21,
		.base_clk_start = 0x0c,.bit_clk_start = 21,
	}
};
#endif

#if 1
void Print_Header (MAC_ENGINE *eng, BYTE option) 
{
	option = STD_OUT;

	if (eng->run.speed_sel[0]) {
		PRINTF(option, " 1G   ");
	} else if (eng->run.speed_sel[1]) {
		PRINTF(option, " 100M ");
	} else {
		PRINTF(option, " 10M  ");
	}

	switch ( eng->arg.test_mode ) {
		case 0 : { PRINTF( option, "Tx/Rx frame checking       \n" ); break;                     }
		case 1 : { PRINTF( option, "Tx output 0xff frame       \n" ); break;                     }
		case 2 : { PRINTF( option, "Tx output 0x55 frame       \n" ); break;                     }
		case 3 : { PRINTF( option, "Tx output random frame     \n" ); break;                     }
		case 4 : { PRINTF( option, "Tx output ARP frame        \n" ); break;                     }
		case 5 : { PRINTF( option, "Tx output 0x%08x frame    \n", eng->arg.GUserDVal ); break; }
		case 6 : { PRINTF( option, "IO delay testing           \n" ); break;                     }
		case 7 : { PRINTF( option, "IO delay testing(Strength) \n" ); break;                     }
		case 8 : { PRINTF( option, "Tx frame                   \n" ); break;                     }
		case 9 : { PRINTF( option, "Rx frame checking          \n" ); break;                     }
	}
}
#endif
static void print_arg_test_mode(MAC_ENGINE *p_eng) 
{
	uint8_t item[32] = "test_mode[dec]";

	if (p_eng->arg.run_mode == MODE_NCSI) {
		printf("%20s| 0: NCSI configuration with    "
		       "Disable_Channel request\n", item);
		printf("%20s| (default:%3d)\n", "", DEF_GTESTMODE);
		printf("%20s| 1: NCSI configuration without "
		       "Disable_Channel request\n", "");
	} else {
		printf("%20s| (default:%3d)\n", "", DEF_GTESTMODE);
		printf("%20s| 0: delay-scanning by frame-loopback\n", item);
		printf("%20s| 1: Tx output 0xff frame\n","");
		printf("%20s| 2: Tx output 0x55 frame\n", "");
		printf("%20s| 3: Tx output random frame\n", "");
		printf("%20s| 4: Tx output ARP frame\n", "");
		printf("%20s| 5: Tx output user defined value "
		       "frame (default:0x%8x)\n", "",
		       DEF_GUSER_DEF_PACKET_VAL);
	}

	printf("%20s| 6: IO timing testing\n", "");
	printf("%20s| 7: IO timing + strength testing\n", "");
}

static void print_arg_phy_addr(MAC_ENGINE *p_eng)
{
	uint8_t item[32] = "phy_addr[dec]";

	printf("%20s| 0~31: PHY Address (default:%d)\n", item, DEF_GPHY_ADR);
}

static void print_arg_ieee_select(MAC_ENGINE *p_eng) 
{
	uint8_t item[64] = "IEEE packet select (if test_mode == 1,2,3,4,5)";

	printf("%20s| 0/1/2... (default:0)\n", item);
}

static void print_arg_delay_scan_range(MAC_ENGINE *p_eng) 
{
	uint8_t item[32] = "delay-scan range";

	printf("%20s| 1/2/3/... only for test_mode 0 (default:%d)\n", item, DEF_GIOTIMINGBUND);
	printf("%20s| will scan from (orig - range) to (orig + range)\n", "");
	print_arg_ieee_select(p_eng);
}

static void print_arg_channel_num(MAC_ENGINE *p_eng) 
{
	uint8_t item[32] = "channel_num[dec]";

	printf("%20s| 1~32: Total Number of NCSI Channel (default:%d)\n", item,
	       DEF_GCHANNEL2NUM);
}

static void print_arg_package_num(MAC_ENGINE *p_eng) 
{
	uint8_t item[32] = "package_num[dec]";

	printf("%20s| 1~ 8: Total Number of NCSI Package (default:%d)\n", item,
	       DEF_GPACKAGE2NUM);
}

static void print_arg_loop(MAC_ENGINE *p_eng)
{
	uint8_t item[32] = "loop_max[dec]";

	printf("%20s| 1G  :  (default:%3d)\n", item, DEF_GLOOP_MAX * 20);
	printf("%20s| 100M:  (default:%3d)\n", "", DEF_GLOOP_MAX * 2);
	printf("%20s| 10M :  (default:%3d)\n", "", DEF_GLOOP_MAX);
}

static void print_arg_ctrl(MAC_ENGINE *p_eng)
{
	uint8_t item[32] = "ctrl[hex]";

	printf("%20s| default: 0x%04x\n", item, DEF_GCTRL);
	printf("%20s| bit0  : 1->single packet\n", "");
	printf("%20s| bit1  : 1->inverse RGMII RXCLK\n", "");
	printf("%20s| bit2  : 1->Disable recovering PHY status\n", "");
	printf("%20s| bit3  : 1->Enable PHY init\n", "");
	printf("%20s| bit4  : 1->PHY internal loopback\n", "");
	printf("%20s| bit5  : 1->Ignore PHY ID\n", "");	
	printf("%20s| bit6  : 1->full range scan\n", "");
	printf("%20s| bit7  : 1->Enable MAC int-loop\n", "");	
}

static void print_arg_speed(MAC_ENGINE *p_eng) 
{
	uint8_t item[32] = "speed[hex]";

	printf("%20s| bit[0]->1G  bit[1]->100M  bit[2]->10M "
	       "(default:0x%02lx)\n",
	       item, DEF_GSPEED);
}

static void print_arg_mdio_idx(MAC_ENGINE *p_eng) 
{
	uint8_t item[32] = "mdio_idx[dec]";

	printf("%20s| 0->MDIO1 1->MDIO2", item);
	
	if (p_eng->env.mac_num > 2) {
		printf(" 2->MDIO3 3->MDIO4");
	}
	printf("\n");
}

static void print_arg_mac_idx(MAC_ENGINE *p_eng) 
{
	uint8_t item[32] = "mac_idx[dec]";

	printf("%20s| 0->MAC1 1->MAC2", item);
	
	if (p_eng->env.mac_num > 2) {
		printf(" 2->MAC3 3->MAC4");
	}
	printf("\n");
}

static void print_usage(MAC_ENGINE *p_eng)
{
	if (MODE_DEDICATED == p_eng->arg.run_mode) {
		printf("mactest <idx> <run_speed> <ctrl> <loop_max> <test "
		       "mode> <phy addr> <timing boundary> <user data>\n");
		print_arg_mac_idx(p_eng);
		print_arg_mdio_idx(p_eng);
		print_arg_speed(p_eng);
		print_arg_ctrl(p_eng);
		print_arg_loop(p_eng);
		print_arg_test_mode(p_eng);
		print_arg_phy_addr(p_eng);
		print_arg_delay_scan_range(p_eng);
	} else if (MODE_NCSI == p_eng->arg.run_mode) {
		printf("ncsitest <idx> <packet num> <channel num> <test mode>"
		       "<timing boundary> <ctrl> <ARP num>\n");
		print_arg_mac_idx(p_eng);
		print_arg_mdio_idx(p_eng);
		print_arg_package_num(p_eng);
		print_arg_channel_num(p_eng);
		print_arg_test_mode(p_eng);
		print_arg_delay_scan_range(p_eng);
		print_arg_ctrl(p_eng);
	} else {
		printf("unknown run mode\n");
	}
}

static void finish_close(MAC_ENGINE *p_eng) 
{
	nt_log_func_name();		
}

char finish_check(MAC_ENGINE *p_eng, int value) 
{

	uint32_t reg;
	BYTE shift_value = 0;
	nt_log_func_name();

	if (p_eng->arg.run_mode == MODE_DEDICATED) {
		if (p_eng->dat.FRAME_LEN)
			free(p_eng->dat.FRAME_LEN);

		if (p_eng->dat.wp_lst)
			free(p_eng->dat.wp_lst);
	}

	p_eng->flg.Err_Flag = p_eng->flg.Err_Flag | value;

	if (DbgPrn_ErrFlg)
		printf("\nErr_Flag: [%08x]\n", p_eng->flg.Err_Flag);

	if (!p_eng->run.TM_Burst)
		FPri_ErrFlag(p_eng, FP_LOG);

	if (p_eng->run.TM_IOTiming)
		FPri_ErrFlag(p_eng, FP_IO);

	FPri_ErrFlag(p_eng, STD_OUT);

	if (!p_eng->run.TM_Burst)
		FPri_End(p_eng, FP_LOG);

	if (p_eng->run.TM_IOTiming)
		FPri_End(p_eng, FP_IO);

	FPri_End(p_eng, STD_OUT);

	if (!p_eng->run.TM_Burst)
		FPri_RegValue(p_eng, FP_LOG);
	if (p_eng->run.TM_IOTiming)
		FPri_RegValue(p_eng, FP_IO);

	finish_close(p_eng);

	if (p_eng->flg.Err_Flag) {
		return (1);
	} else {
		return (0);
	}
}

static uint32_t check_test_mode(MAC_ENGINE *p_eng)
{
	if (p_eng->arg.run_mode == MODE_NCSI ) {
		switch (p_eng->arg.test_mode) {
		case 0:
			break;
		case 1:
			p_eng->run.TM_NCSI_DiSChannel = 0;
			break;
		case 6:
			p_eng->run.TM_IOTiming = 1;
			break;
		case 7:
			p_eng->run.TM_IOTiming = 1;
			p_eng->run.TM_IOStrength = 1;
			break;
		default:
			printf("Error test_mode!!!\n");
			print_arg_test_mode(p_eng);
			return (1);
		}
	} else {
		switch (p_eng->arg.test_mode) {
		case 0:
			break;
		case 1:
		case 2:
		case 3:
		case 5:			
			p_eng->run.TM_RxDataEn = 0;
			p_eng->run.TM_Burst = 1;
			p_eng->run.TM_IEEE = 1;
			break;
		case 4:
			p_eng->run.TM_RxDataEn = 0;
			p_eng->run.TM_Burst = 1;
			p_eng->run.TM_IEEE = 0;
			break;
		case 6:
			p_eng->run.TM_IOTiming = 1;
			break;
		case 7:
			p_eng->run.TM_IOTiming = 1;
			p_eng->run.TM_IOStrength = 1;
			break;
#if 0			
		case 8:
			p_eng->run.TM_RxDataEn = 0;
			p_eng->run.TM_DefaultPHY = 1;
			break;
		case 9:
			p_eng->run.TM_TxDataEn = 0;
			p_eng->run.TM_DefaultPHY = 1;
			break;
		case 10:
			p_eng->run.TM_WaitStart = 1;
			break;
#endif			
		default:
			printf("Error test_mode!!!\n");
			print_arg_test_mode(p_eng);
			return (1);
		}
	}

	if (0 == p_eng->run.TM_IOStrength) {
		p_eng->io.drv_upper_bond = 0;
	}
	return 0;
}

/**
 * @brief enable/disable MAC
 * @param[in] p_eng - MAC_ENGINE
 * 
 * AST2600 uses synchronous reset scheme, so the bits for reset assert and 
 * deassert are the same
 * e.g. MAC#1: SCU04[11] = 1 --> MAC#1 reset assert
 *                       = 0 --> MAC#1 reset de-assert
 * 
 * AST2600 uses asynchronous reset scheme, so the bits for reset assert and 
 * deassert are different
 * e.g. MAC#1: SCU40[11] = 1 --> MAC#1 reset assert
 *             SCU44[11] = 1 --> MAC#1 reset de-assert
 * 
 * The same design concept is also adopted on clock stop/start.
 */
void scu_disable_mac(MAC_ENGINE *p_eng) 
{
	uint32_t mac_idx = p_eng->run.mac_idx;
	struct mac_ctrl_desc *p_mac = &mac_ctrl_lookup_tbl[mac_idx];
	uint32_t reg;

	debug("MAC%d:reset assert=0x%02x[%08x] deassert=0x%02x[%08x]\n",
	      mac_idx, p_mac->base_reset_assert, p_mac->bit_reset_assert,
	      p_mac->base_reset_deassert, p_mac->bit_reset_deassert);
	debug("MAC%d:clock stop=0x%02x[%08x] start=0x%02x[%08x]\n", mac_idx,
	      p_mac->base_clk_stop, p_mac->bit_clk_stop, p_mac->base_clk_start,
	      p_mac->bit_clk_start);

	reg = SCU_RD(p_mac->base_reset_assert);
	debug("reset reg: 0x%08x\n", reg);
	reg |= p_mac->bit_reset_assert;
	debug("reset reg: 0x%08x\n", reg);
	SCU_WR(reg, p_mac->base_reset_assert);
	/* issue a dummy read to ensure command is in order */
	reg = SCU_RD(p_mac->base_reset_assert);
	
	reg = SCU_RD(p_mac->base_clk_stop);
	debug("clock reg: 0x%08x\n", reg);
	reg |= p_mac->bit_clk_stop;
	debug("clock reg: 0x%08x\n", reg);
	SCU_WR(reg, p_mac->base_clk_stop);
	/* issue a dummy read to ensure command is in order */
	reg = SCU_RD(p_mac->base_clk_stop);
}

void scu_enable_mac(MAC_ENGINE *p_eng) 
{
	uint32_t mac_idx = p_eng->run.mac_idx;
	struct mac_ctrl_desc *p_mac = &mac_ctrl_lookup_tbl[mac_idx];
	uint32_t reg;

	debug("MAC%d:reset assert=0x%02x[%08x] deassert=0x%02x[%08x]\n",
	      mac_idx, p_mac->base_reset_assert, p_mac->bit_reset_assert,
	      p_mac->base_reset_deassert, p_mac->bit_reset_deassert);
	debug("MAC%d:clock stop=0x%02x[%08x] start=0x%02x[%08x]\n", mac_idx,
	      p_mac->base_clk_stop, p_mac->bit_clk_stop, p_mac->base_clk_start,
	      p_mac->bit_clk_start);

#ifdef CONFIG_ASPEED_AST2600
	reg = SCU_RD(p_mac->base_reset_deassert);
	debug("reset reg: 0x%08x\n", reg);
	reg |= p_mac->bit_reset_deassert;
	debug("reset reg: 0x%08x\n", reg);
	SCU_WR(reg, p_mac->base_reset_deassert);
	/* issue a dummy read to ensure command is in order */
	reg = SCU_RD(p_mac->base_reset_deassert);
	
	reg = SCU_RD(p_mac->base_clk_start);
	debug("clock reg: 0x%08x\n", reg);
	reg |= p_mac->bit_clk_start;
	debug("clock reg: 0x%08x\n", reg);
	SCU_WR(reg, p_mac->base_clk_start);
	/* issue a dummy read to ensure command is in order */
	reg = SCU_RD(p_mac->base_clk_start);
#else
	reg = SCU_RD(p_mac->base_reset_deassert);
	reg &= ~p_mac->bit_reset_deassert;
	SCU_WR(reg, p_mac->base_reset_deassert);
	/* issue a dummy read to ensure command is in order */
	reg = SCU_RD(p_mac->base_reset_deassert);
	
	reg = SCU_RD(p_mac->base_clk_start);
	reg &= ~p_mac->bit_clk_start;
	SCU_WR(reg, p_mac->base_clk_start);
	/* issue a dummy read to ensure command is in order */
	reg = SCU_RD(p_mac->base_clk_start);
#endif
}

/**
 * @brief setup mdc/mdio pinmix
 * @todo push/pop pinmux registers
*/
void scu_set_pinmux(MAC_ENGINE *p_eng)
{
	uint32_t reg;
	nt_log_func_name();

#ifdef CONFIG_ASPEED_AST2600
	/* MDC/MDIO pinmux */
	switch (p_eng->run.mdio_idx) {
	case 0:
		reg = SCU_RD(0x430) | GENMASK(17, 16);
		SCU_WR(reg, 0x430);
		break;
	case 1:
		reg = SCU_RD(0x470) & ~GENMASK(13, 12);
		SCU_WR(reg, 0x470);
		reg = SCU_RD(0x410) | GENMASK(13, 12);
		SCU_WR(reg, 0x410);
		break;
	case 2:
		reg = SCU_RD(0x470) & ~GENMASK(1, 0);
		SCU_WR(reg, 0x470);
		reg = SCU_RD(0x410) | GENMASK(1, 0);
		SCU_WR(reg, 0x410);
		break;
	case 3:
		reg = SCU_RD(0x470) & ~GENMASK(3, 2);
		SCU_WR(reg, 0x470);
		reg = SCU_RD(0x410) | GENMASK(3, 2);
		SCU_WR(reg, 0x410);
		break;
	default:
		printf("%s:undefined MDIO idx\n", __func__,
		       p_eng->run.mdio_idx);
	}

	switch (p_eng->run.mac_idx) {
	case 0:
#ifdef CONFIG_FPGA_ASPEED
		setbits_le32(SCU_BASE + 0x410, BIT(4));
#else
		setbits_le32(SCU_BASE + 0x400, GENMASK(11, 0));
		setbits_le32(SCU_BASE + 0x410, BIT(4));
		clrbits_le32(SCU_BASE + 0x470, BIT(4));
#endif
		break;
	case 1:
		setbits_le32(SCU_BASE + 0x400, GENMASK(23, 12));
		setbits_le32(SCU_BASE + 0x410, BIT(5));
		clrbits_le32(SCU_BASE + 0x470, BIT(5));
		break;
	case 2:
		setbits_le32(SCU_BASE + 0x410, GENMASK(27, 16));
		setbits_le32(SCU_BASE + 0x410, BIT(6));
		clrbits_le32(SCU_BASE + 0x470, BIT(6));
		break;
	case 3:
		clrbits_le32(SCU_BASE + 0x410, GENMASK(31, 28));
		setbits_le32(SCU_BASE + 0x4b0, GENMASK(31, 28));
		clrbits_le32(SCU_BASE + 0x474, GENMASK(7, 0));
		clrbits_le32(SCU_BASE + 0x414, GENMASK(7, 0));
		setbits_le32(SCU_BASE + 0x4b4, GENMASK(7, 0));
		setbits_le32(SCU_BASE + 0x410, BIT(7));
		clrbits_le32(SCU_BASE + 0x470, BIT(7));
		break;

	}

	debug("SCU410: %08x %08x %08x %08x\n", SCU_RD(0x410), SCU_RD(0x414), SCU_RD(0x418), SCU_RD(0x41c));
	debug("SCU430: %08x %08x %08x %08x\n", SCU_RD(0x430), SCU_RD(0x434), SCU_RD(0x438), SCU_RD(0x43c));
	debug("SCU470: %08x %08x %08x %08x\n", SCU_RD(0x470), SCU_RD(0x474), SCU_RD(0x478), SCU_RD(0x47c));
	debug("SCU4b0: %08x %08x %08x %08x\n", SCU_RD(0x4b0), SCU_RD(0x4b4), SCU_RD(0x4b8), SCU_RD(0x4bc));
#else
	/* MDC/MDIO pinmux */
	if (p_eng->run.mdio_idx == 0) {
		setbits_le32(SCU_BASE + 88, GENMASK(31, 30));
	} else {
		clrsetbits_le32(SCU_BASE + 90, BIT(6), BIT(2));
	}

	/* enable MAC#nLINK pin */
	setbits_le32(SCU_BASE + 80, BIT(p_eng->run.mac_idx));
#endif
}

static uint32_t check_mac_idx(MAC_ENGINE *p_eng)
{
	/* check if legal run_idx */
	if (p_eng->arg.mac_idx > p_eng->env.mac_num - 1) {
		printf("invalid run_idx = %d\n", p_eng->arg.mac_idx);	
		return 1;
	}
	
	return 0;
}

static void calc_loop_check_num(MAC_ENGINE *p_eng)
{
	nt_log_func_name();

#define ONE_MBYTE 1048576

	if (p_eng->run.IO_MrgChk ||
	    (p_eng->arg.run_speed == SET_1G_100M_10MBPS) ||
	    (p_eng->arg.run_speed == SET_100M_10MBPS)) {
		p_eng->run.LOOP_CheckNum = p_eng->run.LOOP_MAX;
	} else {
		switch (p_eng->arg.run_speed) {
		case SET_1GBPS:
			p_eng->run.CheckBuf_MBSize = MOVE_DATA_MB_SEC;
			break;
		case SET_100MBPS:
			p_eng->run.CheckBuf_MBSize = (MOVE_DATA_MB_SEC >> 3);
			break;
		case SET_10MBPS:
			p_eng->run.CheckBuf_MBSize = (MOVE_DATA_MB_SEC >> 6);
			break;
		}
		p_eng->run.LOOP_CheckNum =
		    (p_eng->run.CheckBuf_MBSize /
		     (((p_eng->dat.Des_Num * DMA_PakSize) / ONE_MBYTE) + 1));
	}
}

static uint32_t setup_running(MAC_ENGINE *p_eng)
{
	uint32_t n_desp_min;

	if (0 != check_mac_idx(p_eng)) {
		return 1;
	}
	p_eng->run.mac_idx = p_eng->arg.mac_idx;
	p_eng->run.mac_base = mac_base_lookup_tbl[p_eng->run.mac_idx];	
	
	p_eng->run.mdio_idx = p_eng->arg.mdio_idx;
	p_eng->run.mdio_base = mdio_base_lookup_tbl[p_eng->run.mdio_idx];

	p_eng->run.is_rgmii = p_eng->env.is_1g_valid[p_eng->run.mac_idx];

	/* 
	 * FIXME: too ugly...
	 * check if legal speed setup
	 * */
	switch (p_eng->arg.run_speed) {
	case SET_1GBPS:
		p_eng->run.speed_cfg[0] = 1;
		p_eng->run.speed_cfg[1] = 0;
		p_eng->run.speed_cfg[2] = 0;
		if (0 == p_eng->env.is_1g_valid[p_eng->run.mac_idx]) {
			printf("MAC%d doesn't support 1G\n",
			       p_eng->run.mac_idx);
			return 1;
		}
		break;
	case SET_100MBPS:
		p_eng->run.speed_cfg[0] = 0;
		p_eng->run.speed_cfg[1] = 1;
		p_eng->run.speed_cfg[2] = 0;
		break;
	case SET_10MBPS:
		p_eng->run.speed_cfg[0] = 0;
		p_eng->run.speed_cfg[1] = 0;
		p_eng->run.speed_cfg[2] = 1;
		break;
	case SET_1G_100M_10MBPS:
		p_eng->run.speed_cfg[0] = 1;
		p_eng->run.speed_cfg[1] = 1;
		p_eng->run.speed_cfg[2] = 1;
		break;
	case SET_100M_10MBPS:
		p_eng->run.speed_cfg[0] = 0;
		p_eng->run.speed_cfg[1] = 1;
		p_eng->run.speed_cfg[2] = 1;
		break;
	default:
		printf("Error speed!!!\n");
		print_arg_speed(p_eng);
		return (1);
	}	

	if (p_eng->arg.run_mode == MODE_NCSI) {
		/*
		 * [Arg]check GPackageTolNum
		 * [Arg]check GChannelTolNum
		 */
		if ((p_eng->arg.GPackageTolNum < 1) ||
		    (p_eng->arg.GPackageTolNum > 8)) {
			print_arg_package_num(p_eng);
			return (1);
		}
		if ((p_eng->arg.GChannelTolNum < 1) ||
		    (p_eng->arg.GChannelTolNum > 32)) {
			print_arg_channel_num(p_eng);
			return (1);
		}
	} else {
		/* [Arg]check ctrl */		
		if (p_eng->arg.ctrl.w & 0xfffffe00) {
			print_arg_ctrl(p_eng);
			return (1);
		}

		if (p_eng->arg.GPHYADR > 31) {
			printf("Error phy_adr!!!\n");
			print_arg_phy_addr(p_eng);
			return (1);
		}

		if (0 == p_eng->arg.loop_max) {
			switch (p_eng->arg.run_speed) {
			case SET_1GBPS:
				p_eng->arg.loop_max = DEF_GLOOP_MAX * 20;
				break;
			case SET_100MBPS:
				p_eng->arg.loop_max = DEF_GLOOP_MAX * 2;
				break;
			case SET_10MBPS:
				p_eng->arg.loop_max = DEF_GLOOP_MAX;
				break;
			case SET_1G_100M_10MBPS:
				p_eng->arg.loop_max = DEF_GLOOP_MAX * 20;
				break;
			case SET_100M_10MBPS:
				p_eng->arg.loop_max = DEF_GLOOP_MAX * 2;
				break;
			}
		}		
	}

	if (0 != check_test_mode(p_eng)) {
		return 1;
	}

	if (p_eng->run.TM_Burst) {
		p_eng->run.ieee_sel = p_eng->arg.ieee_sel;
		p_eng->run.delay_margin = 0;
	} else {
		p_eng->run.ieee_sel = 0;			
		p_eng->run.delay_margin = p_eng->arg.delay_scan_range;

		if (p_eng->run.delay_margin == 0) {
			printf("Error IO margin!!!\n");
			print_arg_delay_scan_range(p_eng);
			return(1);
		}						
	}

	if (!p_eng->env.is_1g_valid[p_eng->run.mac_idx])
		p_eng->run.speed_cfg[ 0 ] = 0;


	if (p_eng->arg.run_mode == MODE_NCSI) {
		if (p_eng->run.is_rgmii) {
			printf("\nNCSI must be RMII interface !!!\n");
			return (finish_check(p_eng, Err_Flag_MACMode));	
		}

#ifdef CONFIG_ASPEED_AST2600
		/**
		 * NCSI needs for 3.3V IO voltage but MAC#1 & MAC#2 only
		 * support 1.8V. So NCSI can only runs on MAC#3 or MAC#4
		 */
		if (p_eng->run.mac_idx < 2) {
			printf("\nNCSI must runs on MAC#3 or MAC#4\n");
			return (finish_check(p_eng, Err_Flag_MACMode));	
		}
#endif		
	}
	
	p_eng->run.tdes_base = (uint32_t)(&tdes_buf[0]);
	p_eng->run.rdes_base = (uint32_t)(&rdes_buf[0]);

	if (p_eng->run.TM_IOTiming || p_eng->run.delay_margin)
		p_eng->run.IO_MrgChk = 1;
	else
		p_eng->run.IO_MrgChk = 0;

	p_eng->phy.Adr         = p_eng->arg.GPHYADR;
	p_eng->phy.loop_phy    = p_eng->arg.ctrl.b.phy_int_loopback;
	p_eng->phy.default_phy = p_eng->run.TM_DefaultPHY;

	p_eng->run.LOOP_MAX = p_eng->arg.loop_max;
	calc_loop_check_num(p_eng);

	//------------------------------------------------------------
	// Descriptor Number
	//------------------------------------------------------------
	//------------------------------
	// [Dat]setup Des_Num
	// [Dat]setup DMABuf_Size
	// [Dat]setup DMABuf_Num
	//------------------------------
	if (p_eng->arg.run_mode == MODE_DEDICATED) {
		n_desp_min = p_eng->run.TM_IOTiming;

		if (p_eng->arg.ctrl.b.phy_skip_check &&
		    (p_eng->arg.test_mode == 0))
			/* for SMSC's LAN9303 issue */
			p_eng->dat.Des_Num = 114;
		else {
			switch (p_eng->arg.run_speed) {
			case SET_1GBPS:
				p_eng->dat.Des_Num =
				    p_eng->run.delay_margin
					? 100
					: (n_desp_min) ? 512 : 4096;
				break;
			case SET_100MBPS:
				p_eng->dat.Des_Num =
				    p_eng->run.delay_margin
					? 100
					: (n_desp_min) ? 512 : 4096;
				break;
			case SET_10MBPS:
				p_eng->dat.Des_Num =
				    p_eng->run.delay_margin
					? 100
					: (n_desp_min) ? 100 : 830;
				break;
			case SET_1G_100M_10MBPS:
				p_eng->dat.Des_Num =
				    p_eng->run.delay_margin
					? 100
					: (n_desp_min) ? 100 : 830;
				break;
			case SET_100M_10MBPS:
				p_eng->dat.Des_Num =
				    p_eng->run.delay_margin
					? 100
					: (n_desp_min) ? 100 : 830;
				break;
			}
		}
		/* keep in order: Des_Num -> DMABuf_Size -> DMABuf_Num */
		p_eng->dat.Des_Num_Org = p_eng->dat.Des_Num;
		p_eng->dat.DMABuf_Size = DMA_BufSize;
		p_eng->dat.DMABuf_Num = DMA_BufNum;

		if (DbgPrn_Info) {
			printf("CheckBuf_MBSize : %d\n",
			       p_eng->run.CheckBuf_MBSize);
			printf("LOOP_CheckNum   : %d\n",
			       p_eng->run.LOOP_CheckNum);
			printf("Des_Num         : %d\n", p_eng->dat.Des_Num);
			printf("DMA_BufSize     : %d bytes\n",
			       p_eng->dat.DMABuf_Size);
			printf("DMA_BufNum      : %d\n", p_eng->dat.DMABuf_Num);
			printf("DMA_PakSize     : %d\n", DMA_PakSize);
			printf("\n");
		}
		if (2 > p_eng->dat.DMABuf_Num)
			return (finish_check(p_eng, Err_Flag_DMABufNum));
	}

	return 0;
}

/**
 * @brief setup environment according to HW strap registers
*/
static uint32_t setup_interface(MAC_ENGINE *p_eng)
{
#ifdef CONFIG_ASPEED_AST2600
	hw_strap1_t strap1;
	hw_strap2_t strap2;
	
	strap1.w = SCU_RD(0x500);
	strap2.w = SCU_RD(0x510);

	p_eng->env.is_1g_valid[0] = strap1.b.mac1_interface;
	p_eng->env.is_1g_valid[1] = strap1.b.mac2_interface;
	p_eng->env.is_1g_valid[2] = strap2.b.mac3_interface;
	p_eng->env.is_1g_valid[3] = strap2.b.mac4_interface;
	
	p_eng->env.at_least_1g_valid =
	    p_eng->env.is_1g_valid[0] | p_eng->env.is_1g_valid[1] |
	    p_eng->env.is_1g_valid[2] | p_eng->env.is_1g_valid[3];
#else
	hw_strap1_t strap1;
	strap1.w = SCU_RD(0x70);
	p_eng->env.is_1g_valid[0] = strap1.b.mac1_interface;
	p_eng->env.is_1g_valid[1] = strap1.b.mac2_interface;

	p_eng->env.at_least_1g_valid =
	    p_eng->env.is_1g_valid[0] | p_eng->env.is_1g_valid[1];
#endif
	return 0;
}

/**
 * @brief setup chip compatibility accoriding to the chip ID register
*/
static uint32_t setup_chip_compatibility(MAC_ENGINE *p_eng)
{
	uint32_t reg_addr;
	uint32_t id, version;
	uint32_t is_valid;

	p_eng->env.ast2600 = 0;
	p_eng->env.ast2500 = 0;

#if defined(CONFIG_ASPEED_AST2600)
	reg_addr = 0x04;
#else
	reg_addr = 0x7c;
#endif
	is_valid = 0;
	id = (SCU_RD(reg_addr) & GENMASK(31, 24)) >> 24;
	version = (SCU_RD(reg_addr) & GENMASK(23, 16)) >> 16;

	if (id == 0x5) {
		printf("chip: AST2600 A%d\n", version);
		p_eng->env.ast2600 = 1;
		p_eng->env.ast2500 = 1;
		p_eng->env.mac_num = 4;
		p_eng->env.is_new_mdio_reg[0] = 1;
		p_eng->env.is_new_mdio_reg[1] = 1;
		p_eng->env.is_new_mdio_reg[2] = 1;
		p_eng->env.is_new_mdio_reg[3] = 1;
		is_valid = 1;
	} else if (id == 0x4) {
		printf("chip: AST2500 A%d\n", version);
		p_eng->env.ast2500 = 1;
		p_eng->env.mac_num = 2;
		p_eng->env.is_new_mdio_reg[0] = MAC1_RD(0x40) >> 31;
		p_eng->env.is_new_mdio_reg[1] = MAC2_RD(0x40) >> 31;
		is_valid = 1;
	}

	if (0 == is_valid) {
		printf("unknown chip\n");
		return 1;
	}

	return 0;
}

/**
 * @brief setup environment accoriding to the HW strap and chip ID
*/
static uint32_t setup_env(MAC_ENGINE *p_eng)
{
	if (0 != setup_chip_compatibility(p_eng)) {
		return 1;
	}
	
	setup_interface(p_eng);
	return 0;
}

static void push_reg(MAC_ENGINE *p_eng)
{
	/* SCU delay settings */
	p_eng->io.mac12_1g_delay.value.w = readl(p_eng->io.mac12_1g_delay.addr);
	p_eng->io.mac12_100m_delay.value.w = readl(p_eng->io.mac12_100m_delay.addr);
	p_eng->io.mac12_10m_delay.value.w = readl(p_eng->io.mac12_10m_delay.addr);
		
#ifdef CONFIG_ASPEED_AST2600
	p_eng->io.mac34_1g_delay.value.w = readl(p_eng->io.mac34_1g_delay.addr);
	p_eng->io.mac34_100m_delay.value.w = readl(p_eng->io.mac34_100m_delay.addr);
	p_eng->io.mac34_10m_delay.value.w = readl(p_eng->io.mac34_10m_delay.addr);
	
	p_eng->io.mac34_drv_reg.value.w = readl(p_eng->io.mac34_drv_reg.addr);
#else
	p_eng->io.mac12_drv_reg.value.w = readl(p_eng->io.mac12_drv_reg.addr);
#endif

	/* MAC registers */
	p_eng->reg.maccr.w = mac_reg_read(p_eng, 0x50);

	p_eng->reg.mac_madr = mac_reg_read(p_eng, 0x08);
	p_eng->reg.mac_ladr = mac_reg_read(p_eng, 0x0c);
	p_eng->reg.mac_fear = mac_reg_read(p_eng, 0x40);
}

static void pop_reg(MAC_ENGINE *p_eng)
{
	/* SCU delay settings */
	writel(p_eng->io.mac12_1g_delay.value.w, p_eng->io.mac12_1g_delay.addr);
	writel(p_eng->io.mac12_100m_delay.value.w, p_eng->io.mac12_100m_delay.addr);
	writel(p_eng->io.mac12_10m_delay.value.w, p_eng->io.mac12_10m_delay.addr);	

#ifdef CONFIG_ASPEED_AST2600	
	writel(p_eng->io.mac34_1g_delay.value.w, p_eng->io.mac34_1g_delay.addr);
	writel(p_eng->io.mac34_100m_delay.value.w, p_eng->io.mac34_100m_delay.addr);
	writel(p_eng->io.mac34_10m_delay.value.w, p_eng->io.mac34_10m_delay.addr);
	
	writel(p_eng->io.mac34_drv_reg.value.w, p_eng->io.mac34_drv_reg.addr);
#else
	writel(p_eng->io.mac12_drv_reg.value.w, p_eng->io.mac12_drv_reg.addr);
#endif

	/* MAC registers */
	mac_reg_write(p_eng, 0x50, p_eng->reg.maccr.w);
	mac_reg_write(p_eng, 0x08, p_eng->reg.mac_madr);
	mac_reg_write(p_eng, 0x0c, p_eng->reg.mac_ladr);
	mac_reg_write(p_eng, 0x40, p_eng->reg.mac_fear);
}
static uint32_t init_mac_engine(MAC_ENGINE *p_eng, uint32_t mode)
{
	memset(p_eng, 0, sizeof(MAC_ENGINE));

	if (0 != setup_env(p_eng)) {
		return 1;
	}
	
	p_eng->arg.run_mode = mode;
	p_eng->arg.delay_scan_range = DEF_GIOTIMINGBUND;
	p_eng->arg.test_mode = DEF_GTESTMODE;

	if (p_eng->arg.run_mode == MODE_NCSI ) {
		p_eng->arg.GARPNumCnt = DEF_GARPNUMCNT;
		p_eng->arg.GChannelTolNum = DEF_GCHANNEL2NUM;
		p_eng->arg.GPackageTolNum = DEF_GPACKAGE2NUM;
		p_eng->arg.ctrl.w = 0;
		p_eng->arg.run_speed = SET_100MBPS;        // In NCSI mode, we set to 100M bps
	} else {
		p_eng->arg.GUserDVal  = DEF_GUSER_DEF_PACKET_VAL;
		p_eng->arg.GPHYADR  = DEF_GPHY_ADR;
		p_eng->arg.loop_inf = 0;
		p_eng->arg.loop_max = 0;
		p_eng->arg.ctrl.w = DEF_GCTRL;
		p_eng->arg.run_speed = DEF_GSPEED;
	}

	p_eng->flg.print_en  = 1;
	p_eng->run.TIME_OUT_Des_PHYRatio = 1;
	
	p_eng->run.TM_TxDataEn = 1;
	p_eng->run.TM_RxDataEn = 1;
	p_eng->run.TM_NCSI_DiSChannel = 1;

	/* setup 
	 * 1. delay control register
	 * 2. driving strength control register and upper/lower bond
	 * 3. MAC control register
	 */
#ifdef CONFIG_ASPEED_AST2600
	p_eng->io.mac12_1g_delay.addr = SCU_BASE + 0x340;
	p_eng->io.mac12_1g_delay.tx_min = 0;
	p_eng->io.mac12_1g_delay.tx_max = 63;
	p_eng->io.mac12_1g_delay.rx_min = -63;
	p_eng->io.mac12_1g_delay.rx_max = 63;
	p_eng->io.mac12_1g_delay.rmii_tx_min = 0;
	p_eng->io.mac12_1g_delay.rmii_tx_max = 1;
	p_eng->io.mac12_1g_delay.rmii_rx_min = 0;
	p_eng->io.mac12_1g_delay.rmii_rx_max = 63;

	p_eng->io.mac12_100m_delay.addr = SCU_BASE + 0x348;
	p_eng->io.mac12_100m_delay.tx_min = 0;
	p_eng->io.mac12_100m_delay.tx_max = 63;
	p_eng->io.mac12_100m_delay.rx_min = -63;
	p_eng->io.mac12_100m_delay.rx_max = 63;
	p_eng->io.mac12_10m_delay.addr = SCU_BASE + 0x34c;
	p_eng->io.mac12_10m_delay.tx_min = 0;
	p_eng->io.mac12_10m_delay.tx_max = 63;
	p_eng->io.mac12_10m_delay.rx_min = -63;
	p_eng->io.mac12_10m_delay.rx_max = 63;

	p_eng->io.mac34_1g_delay.addr = SCU_BASE + 0x350;
	p_eng->io.mac34_1g_delay.tx_min = 0;
	p_eng->io.mac34_1g_delay.tx_max = 63;
	p_eng->io.mac34_1g_delay.rx_min = -63;
	p_eng->io.mac34_1g_delay.rx_max = 63;
	p_eng->io.mac34_1g_delay.rmii_tx_min = 0;
	p_eng->io.mac34_1g_delay.rmii_tx_max = 1;
	p_eng->io.mac34_1g_delay.rmii_rx_min = 0;
	p_eng->io.mac34_1g_delay.rmii_rx_max = 63;
	p_eng->io.mac34_100m_delay.addr = SCU_BASE + 0x358;
	p_eng->io.mac34_100m_delay.tx_min = 0;
	p_eng->io.mac34_100m_delay.tx_max = 63;
	p_eng->io.mac34_100m_delay.rx_min = -63;
	p_eng->io.mac34_100m_delay.rx_max = 63;
	p_eng->io.mac34_10m_delay.addr = SCU_BASE + 0x35c;
	p_eng->io.mac34_10m_delay.tx_min = 0;
	p_eng->io.mac34_10m_delay.tx_max = 63;
	p_eng->io.mac34_10m_delay.rx_min = -63;
	p_eng->io.mac34_10m_delay.rx_max = 63;

	p_eng->io.mac34_drv_reg.addr = SCU_BASE + 0x458;
	p_eng->io.mac34_drv_reg.drv_max = 0x3;
	p_eng->io.drv_upper_bond = 0x3;
	p_eng->io.drv_lower_bond = 0;
#else
	p_eng->io.mac12_1g_delay.addr = SCU_BASE + 0x48;
	p_eng->io.mac12_1g_delay.tx_min = 0;
	p_eng->io.mac12_1g_delay.tx_max = 63;
	p_eng->io.mac12_1g_delay.rx_min = 0;
	p_eng->io.mac12_1g_delay.rx_max = 63;
	p_eng->io.mac12_1g_delay.rmii_tx_min = 0;
	p_eng->io.mac12_1g_delay.rmii_tx_max = 1;
	p_eng->io.mac12_1g_delay.rmii_rx_min = 0;
	p_eng->io.mac12_1g_delay.rmii_rx_max = 63;
	p_eng->io.mac12_100m_delay.addr = SCU_BASE + 0xb8;
	p_eng->io.mac12_100m_delay.tx_min = 0;
	p_eng->io.mac12_100m_delay.tx_max = 63;
	p_eng->io.mac12_100m_delay.rx_min = 0;
	p_eng->io.mac12_100m_delay.rx_max = 63;
	p_eng->io.mac12_10m_delay.addr = SCU_BASE + 0xbc;
	p_eng->io.mac12_10m_delay.tx_min = 0;
	p_eng->io.mac12_10m_delay.tx_max = 63;
	p_eng->io.mac12_10m_delay.rx_min = 0;
	p_eng->io.mac12_10m_delay.rx_max = 63;

	p_eng->io.mac34_1g_delay.addr = 0;
	p_eng->io.mac34_100m_delay.addr = 0;
	p_eng->io.mac34_10m_delay.addr = 0;

	p_eng->io.mac12_drv_reg.addr = SCU_BASE + 0x90;
	p_eng->io.mac12_drv_reg.drv_max = 0x1;
	p_eng->io.drv_upper_bond = 0x1;
	p_eng->io.drv_lower_bond = 0;
#endif
	return 0;
}

static uint32_t parse_arg_dedicated(int argc, char *const argv[],
				    MAC_ENGINE *p_eng) 
{
	switch (argc) {
	case 10:
		p_eng->arg.GUserDVal = simple_strtol(argv[9], NULL, 16);
	case 9:
		p_eng->arg.delay_scan_range = simple_strtol(argv[8], NULL, 10);
		p_eng->arg.ieee_sel = p_eng->arg.delay_scan_range;
	case 8:
		p_eng->arg.GPHYADR = simple_strtol(argv[7], NULL, 10);
	case 7:
		p_eng->arg.test_mode = simple_strtol(argv[6], NULL, 16);
		printf("test mode = %d\n", p_eng->arg.test_mode);
	case 6:
		if (0 == strcmp(argv[5], "#")) {
			p_eng->arg.loop_inf = 1;
			printf("loop max = INF\n");
		} else {
			p_eng->arg.loop_max = simple_strtol(argv[5], NULL, 10);
			printf("loop max = %d\n", p_eng->arg.loop_max);
		}
	case 5:
		p_eng->arg.ctrl.w = simple_strtol(argv[4], NULL, 16);
		printf("ctrl=0x%02x\n", p_eng->arg.ctrl.w);
	case 4:
		p_eng->arg.run_speed = simple_strtol(argv[3], NULL, 16);
		printf("run_speed=0x%02x\n", p_eng->arg.run_speed);
	case 3:
		p_eng->arg.mdio_idx = simple_strtol(argv[2], NULL, 10);
		printf("mdio_idx=%d\n", p_eng->arg.mdio_idx);		
	}

	return 0;
}

static uint32_t parse_arg_ncsi(int argc, char *const argv[], MAC_ENGINE *p_eng) 
{
	switch (argc) {
	case 8:
		p_eng->arg.GARPNumCnt = simple_strtol(argv[7], NULL, 10);
	case 7:
		p_eng->arg.ctrl.w = simple_strtol(argv[6], NULL, 16);
		printf("ctrl=0x%02x\n", p_eng->arg.ctrl.w);
	case 6:
		p_eng->arg.delay_scan_range = simple_strtol(argv[5], NULL, 10);		
	case 5:
		p_eng->arg.test_mode = simple_strtol(argv[4], NULL, 16);
	case 4:
		p_eng->arg.GChannelTolNum  = simple_strtol(argv[3], NULL, 10);
	case 3:
		p_eng->arg.GPackageTolNum  = simple_strtol(argv[2], NULL, 10);
	}
	return 0;
}


static void disable_wdt(MAC_ENGINE *p_eng)
{
	/* FIXME */
	return;
}

static uint32_t setup_data(MAC_ENGINE *p_eng)
{
	if (p_eng->arg.run_mode == MODE_DEDICATED) {
		if (p_eng->run.TM_Burst)
			setup_arp(p_eng);
		
		p_eng->dat.FRAME_LEN =
		    (uint32_t *)malloc(p_eng->dat.Des_Num * sizeof(uint32_t));
		p_eng->dat.wp_lst =
		    (uint32_t *)malloc(p_eng->dat.Des_Num * sizeof(uint32_t));

		if (!p_eng->dat.FRAME_LEN)
			return (finish_check(p_eng, Err_Flag_MALLOC_FrmSize));
		if (!p_eng->dat.wp_lst)
			return (finish_check(p_eng, Err_Flag_MALLOC_LastWP));

		TestingSetup(p_eng);
	} else {
		if (p_eng->arg.GARPNumCnt != 0)
			setup_arp(p_eng);
	}

	init_iodelay(p_eng);
	p_eng->run.speed_idx = 0;
	if (!p_eng->run.is_rgmii)
		if (mac_set_scan_boundary(p_eng))
			return (finish_check(p_eng, 0));

	return 0;			
}

static uint32_t get_time_out_desc(MAC_ENGINE *p_eng)
{
	uint32_t time_out;
	uint32_t ratio = p_eng->run.TIME_OUT_Des_PHYRatio;
	
	if (p_eng->run.speed_sel[0])		
		time_out = ratio * TIME_OUT_Des_1G;
	else if (p_eng->run.speed_sel[1])
		time_out = ratio * TIME_OUT_Des_100M;
	else
		time_out = ratio * TIME_OUT_Des_10M;

	if (p_eng->run.TM_WaitStart)
		time_out = time_out * 10000;
	
	return time_out;		
}
void test_start(MAC_ENGINE *p_eng, PHY_ENGINE *p_phy_eng)
{
	int i, j;
	uint32_t drv, speed;
	int td, rd, tbegin, rbegin, tend, rend;
	int tstep, rstep;

	uint32_t wrn_flag_allspeed = 0;
	uint32_t err_flag_allspeed = 0;
	uint32_t des_flag_allspeed = 0;
	uint32_t ncsi_flag_allspeed = 0;

	for (speed = 0; speed < 3; speed++) {
		p_eng->flg.print_en = 1;
		p_eng->run.speed_idx = speed;
		if (p_eng->run.speed_sel[speed]) {
			p_eng->run.TIME_OUT_Des = get_time_out_desc(p_eng);

			if (p_eng->arg.run_mode ==  MODE_DEDICATED) {
				if ((p_eng->arg.run_speed ==
				     SET_1G_100M_10MBPS) ||
				    (p_eng->arg.run_speed == SET_100M_10MBPS)) {
					if (p_eng->run.speed_sel[0])
						p_eng->run.LOOP_MAX =
						    p_eng->arg.loop_max;
					else if (p_eng->run.speed_sel[1])
						p_eng->run.LOOP_MAX =
						    p_eng->arg.loop_max / 100;
					else
						p_eng->run.LOOP_MAX =
						    p_eng->arg.loop_max / 1000;

					if (!p_eng->run.LOOP_MAX)
						p_eng->run.LOOP_MAX = 1;

					calc_loop_check_num(p_eng);
				}

				//------------------------------
				// PHY Initial
				//------------------------------
				if (p_phy_eng->fp_set != 0) {
					init_phy(p_eng, p_phy_eng);  
				}

				if (p_eng->flg.Err_Flag)
					return(finish_check(p_eng, 0));
			}

			//------------------------------
			// [Start] The loop of different IO strength
			//------------------------------
			printf("drirving scan range: lo:%d up:%d\n",
			       p_eng->io.drv_lower_bond,
			       p_eng->io.drv_upper_bond);
			for (drv = p_eng->io.drv_lower_bond; drv <= p_eng->io.drv_upper_bond; drv++) {
				printf("drv_curr:%d\n", drv);
				p_eng->io.drv_curr = drv;
				if (p_eng->run.is_rgmii)
					if (mac_set_scan_boundary(p_eng))
						return(finish_check(p_eng, 0));

				if (p_eng->run.IO_MrgChk) {
					if (p_eng->run.TM_IOStrength) {
						mac_set_driving_strength(p_eng, drv);
					}

					if (p_eng->run.delay_margin)
						PrintIO_Header(p_eng, FP_LOG);
					if (p_eng->run.TM_IOTiming)
						PrintIO_Header(p_eng, FP_IO);
					PrintIO_Header(p_eng, STD_OUT);
				} else {
					if (p_eng->arg.run_mode == MODE_DEDICATED) {
						Print_Header(p_eng, STD_OUT);
					}
				} // End if (p_eng->run.IO_MrgChk)

				//------------------------------
				// [Start] The loop of different IO out delay
				//------------------------------
				tbegin = p_eng->io.tx_delay_scan.begin;
				tend = p_eng->io.tx_delay_scan.end;
				tstep = p_eng->io.tx_delay_scan.step;

				rbegin = p_eng->io.rx_delay_scan.begin;
				rend = p_eng->io.rx_delay_scan.end;
				rstep = p_eng->io.rx_delay_scan.step;

				for (td = tbegin; td <= tend; td += tstep) {
					p_eng->io.Dly_out = td;
					p_eng->io.Dly_out_selval  = td;
					if (p_eng->run.IO_MrgChk) {
						PrintIO_LineS(p_eng, STD_OUT);
					} // End if (p_eng->run.IO_MrgChk)


					//------------------------------
					// [Start] The loop of different IO in delay
					//------------------------------
					for (rd = rbegin; rd <= rend; rd += rstep) {
						p_eng->io.Dly_in = rd;
						if (p_eng->run.IO_MrgChk) {
							p_eng->io.Dly_in_selval  = rd;
							scu_disable_mac(p_eng);
							mac_set_delay(p_eng, rd, td);
							scu_enable_mac(p_eng);

						} // End if (p_eng->run.IO_MrgChk)

						//------------------------------
						// MAC Initial
						//------------------------------
						init_mac(p_eng);
						if (p_eng->flg.Err_Flag)
							return(finish_check(p_eng, 0));

						if (p_eng->arg.run_mode == MODE_NCSI) {
							p_eng->io.Dly_result = phy_ncsi(p_eng);
						}
						else {
							p_eng->io.Dly_result = TestingLoop(p_eng, p_eng->run.LOOP_CheckNum);
						}
						
						p_eng->io.dlymap[rd + 64][td] = p_eng->io.Dly_result;

						// Display to Log file and monitor
						if (p_eng->run.IO_MrgChk) {
							PrintIO_Line(p_eng, STD_OUT);

							FPri_ErrFlag(p_eng, FP_LOG);

							p_eng->flg.Wrn_Flag  = 0;
							p_eng->flg.Err_Flag  = 0;
							p_eng->flg.Des_Flag  = 0;
							p_eng->flg.NCSI_Flag = 0;
						}
					}


					if (p_eng->run.IO_MrgChk) {
						if (p_eng->run.TM_IOTiming) {
							PRINTF(FP_IO, "\n");
						}
						printf("\n");
					}
				} // End for (td = p_eng->io.tx_delay_scan.begin; td <= p_eng->io.tx_delay_scan.end; td+=p_eng->io.tx_delay_scan.step)


				//------------------------------
				// End
				//------------------------------				
#if 0				
				if (p_eng->run.IO_MrgChk) {
					for (td = p_eng->io.Dly_out_min; td <= p_eng->io.Dly_out_max; td++)
						for (rd = p_eng->io.Dly_in_min; rd <= p_eng->io.Dly_in_max; rd++)
							if (p_eng->io.dlymap[rd][td]) {
								if (p_eng->run.TM_IOTiming) {
									for (j = p_eng->io.Dly_out_min; j <= p_eng->io.Dly_out_max; j++) {
										for (i = p_eng->io.Dly_in_min; i <= p_eng->io.Dly_in_max; i++)
											if (p_eng->io.dlymap[i][j])
												{ printf("x "); }
											else
												{ printf("o "); }
										printf("\n");
									}
								} // End if (p_eng->run.TM_IOTiming)

								FindErr(p_eng, Err_Flag_IOMargin);
								goto Find_Err_Flag_IOMargin;
							} // End if (p_eng->io.dlymap[rd][td])
				} // End if (p_eng->run.IO_MrgChk)
#endif				
Find_Err_Flag_IOMargin:
				if (!p_eng->run.TM_Burst)
					FPri_ErrFlag(p_eng, FP_LOG);
				if (p_eng->run.TM_IOTiming)
					FPri_ErrFlag(p_eng, FP_IO);

				FPri_ErrFlag(p_eng, STD_OUT);

				wrn_flag_allspeed  = wrn_flag_allspeed  | p_eng->flg.Wrn_Flag;
				err_flag_allspeed  = err_flag_allspeed  | p_eng->flg.Err_Flag;
				des_flag_allspeed  = des_flag_allspeed  | p_eng->flg.Err_Flag;
				ncsi_flag_allspeed = ncsi_flag_allspeed | p_eng->flg.Err_Flag;
				p_eng->flg.Wrn_Flag  = 0;
				p_eng->flg.Err_Flag  = 0;
				p_eng->flg.Des_Flag  = 0;
				p_eng->flg.NCSI_Flag = 0;
			}

			if (p_eng->arg.run_mode == MODE_DEDICATED) {
				if (p_phy_eng->fp_clr != 0)
					recov_phy(p_eng, p_phy_eng);
			}

			p_eng->run.speed_sel[speed] = 0;
		} // End if (p_eng->run.speed_sel[speed])

		p_eng->flg.print_en = 0;
	} // End for (speed = 0; speed < 3; speed++)

	p_eng->flg.Wrn_Flag  = wrn_flag_allspeed;
	p_eng->flg.Err_Flag  = err_flag_allspeed;
	p_eng->flg.Des_Flag  = des_flag_allspeed;
	p_eng->flg.NCSI_Flag = ncsi_flag_allspeed;
}

void dump_setting(MAC_ENGINE *p_eng)
{
	/* dump env */
	printf("===================\n");
	printf("p_eng->env\n");
	printf("ast2600 = %d\n", p_eng->env.ast2600);
	printf("ast2500 = %d\n", p_eng->env.ast2500);
	printf("mac_num = %d\n", p_eng->env.mac_num);
	printf("is_new_mdio_reg = %d %d %d %d\n",
	       p_eng->env.is_new_mdio_reg[0],
	       p_eng->env.is_new_mdio_reg[1],
	       p_eng->env.is_new_mdio_reg[2],
	       p_eng->env.is_new_mdio_reg[3]);
	printf("is_1g_valid = %d %d %d %d\n",
	       p_eng->env.is_1g_valid[0],
	       p_eng->env.is_1g_valid[1],
	       p_eng->env.is_1g_valid[2],
	       p_eng->env.is_1g_valid[3]);
	printf("at_least_1g_valid = %d\n", p_eng->env.at_least_1g_valid);
	printf("===================\n");


}
/**
 * @brief nettest main function
*/
int mac_test(int argc, char * const argv[], uint32_t mode)
{
	MAC_ENGINE mac_eng;
	PHY_ENGINE phy_eng;	

	if (0 != init_mac_engine(&mac_eng, mode)) {
		printf("init MAC engine fail\n");
		return 1;
	}
	
	if (argc <= 1) {
		print_usage(&mac_eng);
		return 1;
	}

	mac_eng.arg.mac_idx = simple_strtol(argv[1], NULL, 16);

	/* default mdio_idx = mac_idx */
	mac_eng.arg.mdio_idx = mac_eng.arg.mac_idx;
	if (MODE_DEDICATED == mode)
		parse_arg_dedicated(argc, argv, &mac_eng);
	else		
		parse_arg_ncsi(argc, argv, &mac_eng);

	setup_running(&mac_eng);

	dump_setting(&mac_eng);

	/* init PHY engine */
	phy_eng.fp_set = NULL;
	phy_eng.fp_clr = NULL;

	if (mac_eng.arg.ctrl.b.rmii_50m_out && 0 == mac_eng.run.is_rgmii ) {
		mac_set_rmii_50m_output_enable(&mac_eng);
	}

	push_reg(&mac_eng);
	disable_wdt(&mac_eng);

	mac_set_addr(&mac_eng);
	if (mac_eng.arg.ctrl.b.mac_int_loopback)
		mac_set_interal_loopback(&mac_eng);
	
	if (mac_eng.arg.run_mode == MODE_DEDICATED)
		scu_set_pinmux(&mac_eng);

	scu_disable_mac(&mac_eng);
	scu_enable_mac(&mac_eng);
	if (mac_eng.arg.run_mode == MODE_DEDICATED) {
		if (TRUE == phy_find_addr(&mac_eng)) {
			phy_sel(&mac_eng, &phy_eng);		
		}
	}

	/* Data Initial */
	setup_data(&mac_eng);

	mac_eng.flg.all_fail = 1;
	for(int i = 0; i < 3; i++)
		mac_eng.run.speed_sel[i] = mac_eng.run.speed_cfg[i];

	//------------------------------
	// [Start] The loop of different speed
	//------------------------------
	test_start(&mac_eng, &phy_eng);
	pop_reg(&mac_eng);

	return(finish_check(&mac_eng, 0));
}
