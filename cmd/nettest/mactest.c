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

#define ARGV_MAC_IDX		1
#define ARGV_MDIO_IDX		2
#define ARGV_SPEED		3
#define ARGV_CTRL		4
#define ARGV_LOOP		5
#define ARGV_TEST_MODE		6
#define ARGV_PHY_ADDR		7
#define ARGV_TIMING_MARGIN	8


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

#if 0
void Print_Header (MAC_ENGINE *eng, BYTE option) 
{

	if      ( eng->run.Speed_sel[ 0 ] ) { PRINTF( option, " 1G   " ); }
	else if ( eng->run.Speed_sel[ 1 ] ) { PRINTF( option, " 100M " ); }
	else                                { PRINTF( option, " 10M  " ); }

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
	uint8_t item[32] = "IEEE packet select (if test_mode == 1,2,3,4,5)";

	printf("%20s| 0/1/2... (default:0)\n", item);
}

static void print_arg_delay_scan_boundary(MAC_ENGINE *p_eng) 
{
	uint8_t item[32] = "delay-scan boundary (if test_mode == 0)";

	printf("%20s| 0/1/3/5/7/... (default:%d)\n", item, DEF_GIOTIMINGBUND);
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
	printf("%20s| bit1  : 1->swap phy address\n", "");
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

	printf("%20s| bit[2]->1G  bit[1]->100M  bit[0]->10M "
	       "(default:0x%02lx)\n",
	       item, DEF_GSPEED);
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
		print_arg_speed(p_eng);
		print_arg_ctrl(p_eng);
		print_arg_loop(p_eng);
		print_arg_test_mode(p_eng);
		print_arg_phy_addr(p_eng);
		print_arg_delay_scan_boundary(p_eng);
	} else if (MODE_NCSI == p_eng->arg.run_mode) {
		printf("ncsitest <idx> <packet num> <channel num> <test mode>"
		       "<timing boundary> <ctrl> <ARP num>\n");
		print_arg_mac_idx(p_eng);
		print_arg_package_num(p_eng);
		print_arg_channel_num(p_eng);
		print_arg_test_mode(p_eng);
		print_arg_delay_scan_boundary(p_eng);
		print_arg_ctrl(p_eng);
	} else {
		printf("unknown run mode\n");
	}
}

static void finish_close(MAC_ENGINE *p_eng) 
{
	nt_log_func_name();
	if (p_eng->reg.SCU_oldvld)
		recov_scu(p_eng);
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

#if defined(CONFIG_ASPEED_AST2500)
	reg = Read_Reg_SCU_DD(0x40);
	if (eng->arg.run_mode == MODE_DEDICATED)
		shift_value = 18 + p_eng->run.mac_idx;
	else
		shift_value = 16 + p_eng->run.mac_idx;
#endif

	if (p_eng->flg.Err_Flag) {
		// Fail
#if defined(CONFIG_ASPEED_AST2500)
		reg = reg & ~(1 << shift_value);
		Write_Reg_SCU_DD(0x40, reg);
#endif
		return (1);
	} else {
		// PASS
#if defined(CONFIG_ASPEED_AST2500)
		reg |= (1 << shift_value);
		Write_Reg_SCU_DD(0x40, reg);
#endif
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
static uint32_t check_mac_idx(MAC_ENGINE *p_eng)
{
	/* check if legal run_idx */
	if (p_eng->arg.mac_idx > p_eng->env.mac_num) {
		printf("invalid run_idx = %d\n", p_eng->arg.mac_idx);	
		return 1;
	}
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
		p_eng->run.Speed_org[0] = 1;
		p_eng->run.Speed_org[1] = 0;
		p_eng->run.Speed_org[2] = 0;
		if (0 == p_eng->env.is_1g_valid[p_eng->run.mac_idx]) {
			printf("MAC%d doesn't support 1G\n",
			       p_eng->arg.mac_idx);
			return 1;
		}
		break;
	case SET_100MBPS:
		p_eng->run.Speed_org[0] = 0;
		p_eng->run.Speed_org[1] = 1;
		p_eng->run.Speed_org[2] = 0;
		break;
	case SET_10MBPS:
		p_eng->run.Speed_org[0] = 0;
		p_eng->run.Speed_org[1] = 0;
		p_eng->run.Speed_org[2] = 1;
		break;
	case SET_1G_100M_10MBPS:
		p_eng->run.Speed_org[0] = 1;
		p_eng->run.Speed_org[1] = 1;
		p_eng->run.Speed_org[2] = 1;
		break;
	case SET_100M_10MBPS:
		p_eng->run.Speed_org[0] = 0;
		p_eng->run.Speed_org[1] = 1;
		p_eng->run.Speed_org[2] = 1;
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

		if (0 != p_eng->arg.loop_max) {
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
		p_eng->run.IO_Bund = 0;
	} else {
		p_eng->run.ieee_sel = 0;			
		p_eng->run.IO_Bund = p_eng->arg.delay_scan_boundary;

		if (!((p_eng->run.IO_Bund & 0x1) ||(p_eng->run.IO_Bund == 0))) {
			printf("Error IO margin!!!\n");
			print_arg_delay_scan_boundary(p_eng);
			return(1);
		}						
	}

	if (!p_eng->env.is_1g_valid[p_eng->run.mac_idx])
		p_eng->run.Speed_org[ 0 ] = 0;


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

	p_eng->run.tdes_base = TDES_BASE1;
	p_eng->run.rdes_base = RDES_BASE1;

	if (p_eng->run.TM_IOTiming || p_eng->run.IO_Bund )
		p_eng->run.IO_MrgChk = 1;
	else
		p_eng->run.IO_MrgChk = 0;

	p_eng->phy.Adr         = p_eng->arg.GPHYADR;
	p_eng->phy.loop_phy    = p_eng->arg.ctrl.b.phy_int_loopback;
	p_eng->phy.default_phy = p_eng->run.TM_DefaultPHY;

	p_eng->run.LOOP_MAX = p_eng->arg.loop_max;
	calc_loop_check_num( p_eng );	
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
	
	setup_interface(&p_eng);	

	return 0;
}

static push_reg(MAC_ENGINE *p_eng)
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
	
}
static uint32_t init_mac_engine(MAC_ENGINE *p_eng, uint32_t mode)
{
	memset(p_eng, 0, sizeof(MAC_ENGINE));

	if (0 != setup_env(p_eng)) {
		return 1;
	}
	
	p_eng->arg.run_mode = mode;
	p_eng->arg.delay_scan_boundary = DEF_GIOTIMINGBUND;
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

	p_eng->flg.Flag_PrintEn  = 1;
	p_eng->run.TIME_OUT_Des_PHYRatio = 1;
	
	p_eng->run.TM_TxDataEn = 1;
	p_eng->run.TM_RxDataEn = 1;
	p_eng->run.TM_NCSI_DiSChannel = 1;

	/* setup 
	 * 1. delay control register
	 * 2. driving strength control register and upper/lower bond
	 */
#ifdef CONFIG_ASPEED_AST2600
	p_eng->io.mac12_1g_delay.addr = SCU_BASE + 0x340;
	p_eng->io.mac12_1g_delay.tx_min_delay = 0;
	p_eng->io.mac12_1g_delay.tx_max_delay = 63;
	p_eng->io.mac12_1g_delay.rx_min_delay = -63;
	p_eng->io.mac12_1g_delay.rx_max_delay = 63;
	p_eng->io.mac12_100m_delay.addr = SCU_BASE + 0x348;
	p_eng->io.mac12_100m_delay.tx_min_delay = 0;
	p_eng->io.mac12_100m_delay.tx_max_delay = 63;
	p_eng->io.mac12_100m_delay.rx_min_delay = -63;
	p_eng->io.mac12_100m_delay.rx_max_delay = 63;
	p_eng->io.mac12_10m_delay.addr = SCU_BASE + 0x34c;
	p_eng->io.mac12_10m_delay.tx_min_delay = 0;
	p_eng->io.mac12_10m_delay.tx_max_delay = 63;
	p_eng->io.mac12_10m_delay.rx_min_delay = -63;
	p_eng->io.mac12_10m_delay.rx_max_delay = 63;

	p_eng->io.mac34_1g_delay.addr = SCU_BASE + 0x350;
	p_eng->io.mac34_1g_delay.tx_min_delay = 0;
	p_eng->io.mac34_1g_delay.tx_max_delay = 63;
	p_eng->io.mac34_1g_delay.rx_min_delay = -63;
	p_eng->io.mac34_1g_delay.rx_max_delay = 63;
	p_eng->io.mac34_100m_delay.addr = SCU_BASE + 0x358;
	p_eng->io.mac34_100m_delay.tx_min_delay = 0;
	p_eng->io.mac34_100m_delay.tx_max_delay = 63;
	p_eng->io.mac34_100m_delay.rx_min_delay = -63;
	p_eng->io.mac34_100m_delay.rx_max_delay = 63;
	p_eng->io.mac34_10m_delay.addr = SCU_BASE + 0x35c;
	p_eng->io.mac34_10m_delay.tx_min_delay = 0;
	p_eng->io.mac34_10m_delay.tx_max_delay = 63;
	p_eng->io.mac34_10m_delay.rx_min_delay = -63;
	p_eng->io.mac34_10m_delay.rx_max_delay = 63;

	p_eng->io.mac34_drv_reg.addr = SCU_BASE + 0x458;
	p_eng->io.mac34_drv_reg.drv_max = 0x3;
	p_eng->io.drv_upper_bond = 0x3;
	p_eng->io.drv_lower_bond = 0;
#else
	p_eng->io.mac12_1g_delay.addr = SCU_BASE + 0x48;
	p_eng->io.mac12_1g_delay.tx_min_delay = 0;
	p_eng->io.mac12_1g_delay.tx_max_delay = 63;
	p_eng->io.mac12_1g_delay.rx_min_delay = 0;
	p_eng->io.mac12_1g_delay.rx_max_delay = 63;
	p_eng->io.mac12_100m_delay.addr = SCU_BASE + 0xb8;
	p_eng->io.mac12_100m_delay.tx_min_delay = 0;
	p_eng->io.mac12_100m_delay.tx_max_delay = 63;
	p_eng->io.mac12_100m_delay.rx_min_delay = 0;
	p_eng->io.mac12_100m_delay.rx_max_delay = 63;
	p_eng->io.mac12_10m_delay.addr = SCU_BASE + 0xbc;
	p_eng->io.mac12_10m_delay.tx_min_delay = 0;
	p_eng->io.mac12_10m_delay.tx_max_delay = 63;
	p_eng->io.mac12_10m_delay.rx_min_delay = 0;
	p_eng->io.mac12_10m_delay.rx_max_delay = 63;

	p_eng->io.mac34_1g_delay.addr = 0;
	p_eng->io.mac34_100m_delay.addr = 0;
	p_eng->io.mac34_10m_delay.addr = 0;

	p_eng->io.mac12_drv_reg.addr = SCU_BASE + 0x90;
	p_eng->io.mac12_drv_reg.drv_max = 0x1;
	p_eng->io.drv_upper_bond = 0x1;
	p_eng->io.drv_lower_bond = 0;
#endif

	if (0 == p_eng->run.TM_IOStrength) {
		p_eng->io.drv_upper_bond = 0;
	}

	return 0;
}

static uint32_t parse_arg_dedicated(int argc, char *const argv[],
				    MAC_ENGINE *p_eng) 
{
	switch (argc) {
	case 9:
		p_eng->arg.GUserDVal = simple_strtol(argv[8], NULL, 16);
	case 8:
		p_eng->arg.delay_scan_boundary = simple_strtol(argv[7], NULL, 10);
		p_eng->arg.ieee_sel = p_eng->arg.delay_scan_boundary;
	case 7:
		p_eng->arg.GPHYADR = simple_strtol(argv[6], NULL, 10);
	case 6:
		p_eng->arg.test_mode = simple_strtol(argv[5], NULL, 16);
		printf("test mode = %d\n", p_eng->arg.test_mode);
	case 5:
		if (0 == strcmp(argv[4], "#")) {
			p_eng->arg.loop_inf = 1;
			printf("loop max = INF\n");
		} else {
			p_eng->arg.loop_max = simple_strtol(argv[4], NULL, 10);
			printf("loop max = %d\n", p_eng->arg.loop_max);
		}
	case 4:
		p_eng->arg.ctrl.w = simple_strtol(argv[3], NULL, 16);
		printf("ctrl=0x%02x\n", p_eng->arg.ctrl.w);
	case 3:
		p_eng->arg.run_speed = simple_strtol(argv[2], NULL, 16);
		printf("run_speed=0x%02x\n", p_eng->arg.run_speed);
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
		p_eng->arg.delay_scan_boundary = simple_strtol(argv[5], NULL, 10);		
	case 5:
		p_eng->arg.test_mode = simple_strtol(argv[4], NULL, 16);
	case 4:
		p_eng->arg.GChannelTolNum  = simple_strtol(argv[3], NULL, 10);
	case 3:
		p_eng->arg.GPackageTolNum  = simple_strtol(argv[2], NULL, 10);
	}
	return 0;
}



/**
 * @brief nettest main function
*/
int mac_test(int argc, char * const argv[], uint32_t mode)
{
	MAC_ENGINE mac_eng;
	PHY_ENGINE phy_eng;
	uint32_t wrn_flag_allspeed = 0;
	uint32_t err_flag_allspeed = 0;
	uint32_t des_flag_allspeed = 0;
	uint32_t ncsi_flag_allspeed = 0;

	if (0 != init_mac_engine(&mac_eng, mode)) {
		printf("init MAC engine fail\n");
		return 1;
	}
	
	if (argc <= 1) {
		print_usage(&mac_eng);
		return 1;
	}

	mac_eng.arg.mac_idx = simple_strtol(argv[1], NULL, 16);

	/* FIXME: add new argv to indicate MDIO index */
	mac_eng.arg.mdio_idx = mac_eng.arg.mac_idx;
	if (MODE_DEDICATED == mode)
		parse_arg_dedicated(argc, argv, &mac_eng);
	else		
		parse_arg_ncsi(argc, argv, &mac_eng);

	setup_running(&mac_eng);

	/* init PHY engine */
	phy_eng.fp_set = NULL;
	phy_eng.fp_clr = NULL;

	push_reg(&mac_eng);

	scu_disable_mac(&mac_eng);
	scu_enable_mac(&mac_eng);
	if (mac_eng.arg.run_mode == MODE_DEDICATED) {
		if (TRUE == phy_find_addr(&mac_eng))
			phy_sel(&mac_eng, &phy_eng);
	}

	pop_reg(&mac_eng);

#if 0
	int                  DES_LowNumber;		

	int                  i;
	int                  j;
	uint32_t                temp;

//------------------------------------------------------------
// Get Chip Feature
//------------------------------------------------------------
	read_scu( eng );

//------------------------------------------------------------
// Parameter Initial
//------------------------------------------------------------
		
		//------------------------------
		// [Reg]setup SCU_048_mix
		// [Reg]setup SCU_048_check
		// [Reg]setup SCU_048_default
		// [Reg]setup SCU_074_mix
		//------------------------------
		eng->reg.SCU_048_mix     = ( eng->reg.SCU_048 & 0xfc000000 );
		eng->reg.SCU_048_check   = ( eng->reg.SCU_048 & 0x03ffffff );
		eng->reg.SCU_048_default =   SCU_48h_AST2500  & 0x03ffffff;

		if ( eng->arg.ctrl.b.rmii_50m_out && 0 == eng->run.is_rgmii ) {
			switch ( eng->run.mac_idx ) {
				case 1: eng->reg.SCU_048_mix = eng->reg.SCU_048_mix | 0x40000000; break;
				case 0: eng->reg.SCU_048_mix = eng->reg.SCU_048_mix | 0x20000000; break;
			}
		}
		eng->reg.SCU_074_mix = eng->reg.SCU_074;

		//------------------------------
		// [Reg]setup MAC_050
		//------------------------------
		if ( eng->arg.run_mode == MODE_NCSI )
			// Set to 100Mbps and Enable RX broabcast packets and CRC_APD and Full duplex
			eng->reg.MAC_050 = 0x000a0500;// [100Mbps] RX_BROADPKT_EN & CRC_APD & Full duplex
//			eng->reg.MAC_050 = 0x000a4500;// [100Mbps] RX_BROADPKT_EN & RX_ALLADR & CRC_APD & Full duplex
		else {

			eng->reg.MAC_050 = 0x00004500;// RX_ALLADR & CRC_APD & Full duplex
#ifdef Enable_Runt
			eng->reg.MAC_050 = eng->reg.MAC_050 | 0x00001000;
#endif

		} // End if ( eng->arg.run_mode == MODE_NCSI )
#endif

#if 0
//------------------------------------------------------------
// Descriptor Number
//------------------------------------------------------------
	//------------------------------
	// [Dat]setup Des_Num
	// [Dat]setup DMABuf_Size
	// [Dat]setup DMABuf_Num
	//------------------------------
	if ( eng->arg.run_mode == MODE_DEDICATED ) {
		DES_LowNumber = eng->run.TM_IOTiming;

		if ( eng->arg.ctrl.b.phy_skip_check && ( eng->arg.test_mode == 0 ) )
			eng->dat.Des_Num = 114;//for SMSC's LAN9303 issue
		else {
			switch ( eng->arg.run_speed ) {
				case SET_1GBPS          : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 512 : 4096; break;
				case SET_100MBPS        : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 512 : 4096; break;
				case SET_10MBPS         : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 100 :  830; break;
				case SET_1G_100M_10MBPS : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 100 :  830; break;
				case SET_100M_10MBPS    : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 100 :  830; break;
			}
		} // End if ( eng->arg.ctrl.b.phy_skip_check && ( eng->arg.test_mode == 0 ) )


		eng->dat.Des_Num_Org = eng->dat.Des_Num;
		eng->dat.DMABuf_Size = DMA_BufSize; //keep in order: Des_Num --> DMABuf_Size --> DMABuf_Num
		eng->dat.DMABuf_Num  = DMA_BufNum;  //keep in order: Des_Num --> DMABuf_Size --> DMABuf_Num

		if ( DbgPrn_Info ) {
			printf("CheckBuf_MBSize : %d\n",       eng->run.CheckBuf_MBSize);
			printf("LOOP_CheckNum   : %d\n",       eng->run.LOOP_CheckNum);
			printf("Des_Num         : %d\n",       eng->dat.Des_Num);
			printf("DMA_BufSize     : %d bytes\n", eng->dat.DMABuf_Size);
			printf("DMA_BufNum      : %d\n",       eng->dat.DMABuf_Num);
			printf("DMA_PakSize     : %d\n",        DMA_PakSize);
			printf("\n");
		}
			if ( 2 > eng->dat.DMABuf_Num )
				return( finish_check( eng, Err_Flag_DMABufNum ) );
	} // End if ( eng->arg.run_mode == MODE_DEDICATED )

//------------------------------------------------------------
// Setup Running Parameter
//------------------------------------------------------------

#if 0
	eng->run.tdes_base = TDES_BASE1;
	eng->run.rdes_base = RDES_BASE1;

	if ( eng->run.TM_IOTiming || eng->run.IO_Bund )
		eng->run.IO_MrgChk = 1;
	else
		eng->run.IO_MrgChk = 0;

	eng->phy.Adr         = eng->arg.GPHYADR;
	eng->phy.loop_phy    = eng->arg.ctrl.b.phy_int_loopback;
	eng->phy.default_phy = eng->run.TM_DefaultPHY;

	eng->run.LOOP_MAX = eng->arg.loop_max;
	calc_loop_check_num( eng );	
#endif
//------------------------------------------------------------
// SCU Initial
//------------------------------------------------------------
	get_mac_info( eng );
	Setting_scu( eng );
	init_scu1( eng );
	

#if 0
	scu_disable_mac( eng );
	scu_enable_mac( eng );
	if ( eng->arg.run_mode ==  MODE_DEDICATED ) {
		eng->phy.phy_addr_valid = phy_find_addr( eng );
		if ( eng->phy.phy_addr_valid == TRUE )
			phy_sel( eng, phyeng );
	}
#endif	

//------------------------------------------------------------
// Data Initial
//------------------------------------------------------------
	if ( eng->arg.run_mode ==  MODE_DEDICATED ) {
		if ( eng->run.TM_Burst )
			setup_arp ( eng );
		eng->dat.FRAME_LEN = (uint32_t *)malloc( eng->dat.Des_Num    * sizeof( uint32_t ) );
		eng->dat.wp_lst    = (uint32_t *)malloc( eng->dat.Des_Num    * sizeof( uint32_t ) );

		if ( !eng->dat.FRAME_LEN )
			return( finish_check( eng, Err_Flag_MALLOC_FrmSize ) );
		if ( !eng->dat.wp_lst )
			return( finish_check( eng, Err_Flag_MALLOC_LastWP ) );

			// Setup data and length

		TestingSetup ( eng );
	} else {
			if ( eng->arg.GARPNumCnt != 0 )
				setup_arp ( eng );
	}// End if ( eng->arg.run_mode ==  MODE_DEDICATED )

	init_iodelay( eng );
	eng->run.speed_idx = 0;
	if (!eng->run.is_rgmii)
		if ( get_iodelay( eng ) )
			return( finish_check( eng, 0 ) );	

//------------------------------------------------------------
// main
//------------------------------------------------------------
	nt_log_func_name();

	eng->flg.AllFail = 1;
	for ( eng->run.speed_idx = 0; eng->run.speed_idx < 3; eng->run.speed_idx++ )
		eng->run.Speed_sel[ (int)eng->run.speed_idx ] = eng->run.Speed_org[ (int)eng->run.speed_idx ];

	//------------------------------
	// [Start] The loop of different speed
	//------------------------------
	for ( eng->run.speed_idx = 0; eng->run.speed_idx < 3; eng->run.speed_idx++ ) {
		eng->flg.Flag_PrintEn = 1;
		if ( eng->run.Speed_sel[ (int)eng->run.speed_idx ] ) {
			// Setting speed of LAN
			if      ( eng->run.Speed_sel[ 0 ] ) eng->reg.MAC_050_Speed = eng->reg.MAC_050 | 0x0000020f;
			else if ( eng->run.Speed_sel[ 1 ] ) eng->reg.MAC_050_Speed = eng->reg.MAC_050 | 0x0008000f;
			else                                eng->reg.MAC_050_Speed = eng->reg.MAC_050 | 0x0000000f;

			// Setting check owner time out
			if      ( eng->run.Speed_sel[ 0 ] ) eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des_PHYRatio * TIME_OUT_Des_1G;
			else if ( eng->run.Speed_sel[ 1 ] ) eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des_PHYRatio * TIME_OUT_Des_100M;
			else                                eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des_PHYRatio * TIME_OUT_Des_10M;

			if ( eng->run.TM_WaitStart )
				eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des * 10000;

				// Setting the LAN speed
			if ( eng->arg.run_mode ==  MODE_DEDICATED ) {
				// Test three speed of LAN, we will modify loop number
				if ( ( eng->arg.run_speed == SET_1G_100M_10MBPS ) || ( eng->arg.run_speed == SET_100M_10MBPS ) ) {
					if      ( eng->run.Speed_sel[ 0 ] ) eng->run.LOOP_MAX = eng->arg.loop_max;
					else if ( eng->run.Speed_sel[ 1 ] ) eng->run.LOOP_MAX = eng->arg.loop_max / 100;
					else                                eng->run.LOOP_MAX = eng->arg.loop_max / 1000;

					if ( !eng->run.LOOP_MAX )
						eng->run.LOOP_MAX = 1;

					calc_loop_check_num( eng );
				}

				//------------------------------
				// PHY Initial
				//------------------------------
				if ( phyeng->fp_set != 0 ) {
					init_phy( eng, phyeng );  
				}

				if ( eng->flg.Err_Flag )
					return( finish_check( eng, 0 ) );
			} // End if ( eng->arg.run_mode ==  MODE_DEDICATED )

			//------------------------------
			// [Start] The loop of different IO strength
			//------------------------------
			for (eng->io.drv_curr = eng->io.drv_lower_bond; eng->io.drv_curr <= eng->io.drv_upper_bond; eng->io.drv_curr++ ) {
				//------------------------------
				// Print Header of report to monitor and log file
				//------------------------------
				if (eng->run.is_rgmii)
					if ( get_iodelay( eng ) )
						return( finish_check( eng, 0 ) );

				if ( eng->run.IO_MrgChk ) {
					if ( eng->run.TM_IOStrength ) {
						mac_set_driving_strength(eng, eng->io.drv_curr);
					}

					if ( eng->run.IO_Bund )
						PrintIO_Header( eng, FP_LOG );
					if ( eng->run.TM_IOTiming )
						PrintIO_Header( eng, FP_IO );
					PrintIO_Header( eng, STD_OUT );
				} else {
					if ( eng->arg.run_mode == MODE_DEDICATED ) {
						if ( !eng->run.TM_Burst )
							Print_Header( eng, FP_LOG );
						Print_Header( eng, STD_OUT );
					}
				} // End if ( eng->run.IO_MrgChk )

				//------------------------------
				// [Start] The loop of different IO out delay
				//------------------------------
				for ( eng->io.Dly_out = eng->io.tx_delay_scan_begin; eng->io.Dly_out <= eng->io.tx_delay_scan_end; eng->io.Dly_out+=eng->io.Dly_out_cval ) {
					if ( eng->run.IO_MrgChk ) {
						eng->io.Dly_out_reg_hit = ( eng->io.Dly_out_reg == eng->io.value_ary[ eng->io.Dly_out ]) ? 1 : 0;

						if ( eng->run.TM_IOTiming )
							PrintIO_LineS( eng, FP_IO );
						PrintIO_LineS( eng, STD_OUT );
					} // End if ( eng->run.IO_MrgChk )


					//------------------------------
					// [Start] The loop of different IO in delay
					//------------------------------
					for ( eng->io.Dly_in = eng->io.rx_delay_scan_begin; eng->io.Dly_in <= eng->io.rx_delay_scan_end; eng->io.Dly_in+=eng->io.Dly_in_cval ) {
						if ( eng->run.IO_MrgChk ) {
							eng->io.Dly_in_selval  = eng->io.value_ary[ eng->io.Dly_in ];
							scu_disable_mac(eng);
							mac_set_delay(eng, eng->io.Dly_in_selval, eng->io.Dly_out_selval);							
							scu_enable_mac(eng);

						} // End if ( eng->run.IO_MrgChk )

						//------------------------------
						// MAC Initial
						//------------------------------
						init_mac( eng );
						if ( eng->flg.Err_Flag )
							return( finish_check( eng, 0 ) );

						if ( eng->arg.run_mode == MODE_NCSI )
							eng->io.Dly_result = phy_ncsi( eng );
						else
						{							
							eng->io.Dly_result = TestingLoop( eng, eng->run.LOOP_CheckNum );
						}
						eng->io.dlymap[ eng->io.Dly_in ][ eng->io.Dly_out ] = eng->io.Dly_result;

						// Display to Log file and monitor
						if ( eng->run.IO_MrgChk ) {
							if ( eng->run.TM_IOTiming )
								PrintIO_Line( eng, FP_IO );
							PrintIO_Line( eng, STD_OUT );

							FPri_ErrFlag( eng, FP_LOG );
							PrintIO_Line_LOG( eng );

							eng->flg.Wrn_Flag  = 0;
							eng->flg.Err_Flag  = 0;
							eng->flg.Des_Flag  = 0;
							eng->flg.NCSI_Flag = 0;
						} //End if ( eng->run.IO_MrgChk )
					} // End for ( eng->io.Dly_in = eng->io.rx_delay_scan_begin; eng->io.Dly_in <= eng->io.rx_delay_scan_end; eng->io.Dly_in+=eng->io.Dly_in_cval )


					if ( eng->run.IO_MrgChk ) {
						if ( eng->run.TM_IOTiming ) {
							PRINTF( FP_IO, "\n" );
						}
						printf("\n");
					}
				} // End for ( eng->io.Dly_out = eng->io.tx_delay_scan_begin; eng->io.Dly_out <= eng->io.tx_delay_scan_end; eng->io.Dly_out+=eng->io.Dly_out_cval )


				//------------------------------
				// End
				//------------------------------
				if ( eng->run.IO_MrgChk ) {
					for ( eng->io.Dly_out = eng->io.Dly_out_min; eng->io.Dly_out <= eng->io.Dly_out_max; eng->io.Dly_out++ )
						for ( eng->io.Dly_in = eng->io.Dly_in_min; eng->io.Dly_in <= eng->io.Dly_in_max; eng->io.Dly_in++ )
							if ( eng->io.dlymap[ eng->io.Dly_in ][ eng->io.Dly_out ] ) {
								if ( eng->run.TM_IOTiming ) {
									for ( j = eng->io.Dly_out_min; j <= eng->io.Dly_out_max; j++ ) {
										for ( i = eng->io.Dly_in_min; i <= eng->io.Dly_in_max; i++ )
											if ( eng->io.dlymap[i][j] )
												{ PRINTF( FP_IO, "x " ); }
											else
												{ PRINTF( FP_IO, "o " ); }
										PRINTF( FP_IO, "\n" );
									}
								} // End if ( eng->run.TM_IOTiming )

								FindErr( eng, Err_Flag_IOMargin );
								goto Find_Err_Flag_IOMargin;
							} // End if ( eng->io.dlymap[ eng->io.Dly_in ][ eng->io.Dly_out ] )
				} // End if ( eng->run.IO_MrgChk )
Find_Err_Flag_IOMargin:
				if ( !eng->run.TM_Burst )
					FPri_ErrFlag( eng, FP_LOG );
				if ( eng->run.TM_IOTiming )
					FPri_ErrFlag( eng, FP_IO );

				FPri_ErrFlag( eng, STD_OUT );

				wrn_flag_allspeed  = wrn_flag_allspeed  | eng->flg.Wrn_Flag;
				err_flag_allspeed  = err_flag_allspeed  | eng->flg.Err_Flag;
				des_flag_allspeed  = des_flag_allspeed  | eng->flg.Err_Flag;
				ncsi_flag_allspeed = ncsi_flag_allspeed | eng->flg.Err_Flag;
				eng->flg.Wrn_Flag  = 0;
				eng->flg.Err_Flag  = 0;
				eng->flg.Des_Flag  = 0;
				eng->flg.NCSI_Flag = 0;
			}

			if ( eng->arg.run_mode == MODE_DEDICATED ) {
				if ( phyeng->fp_clr != 0 )
					recov_phy( eng, phyeng );
			}

			eng->run.Speed_sel[ (int)eng->run.speed_idx ] = 0;
		} // End if ( eng->run.Speed_sel[ eng->run.speed_idx ] )

		eng->flg.Flag_PrintEn = 0;
	} // End for ( eng->run.speed_idx = 0; eng->run.speed_idx < 3; eng->run.speed_idx++ )

	eng->flg.Wrn_Flag  = wrn_flag_allspeed;
	eng->flg.Err_Flag  = err_flag_allspeed;
	eng->flg.Des_Flag  = des_flag_allspeed;
	eng->flg.NCSI_Flag = ncsi_flag_allspeed;


	return(finish_check(eng, 0));
#else
	return 0;
#endif	
}
