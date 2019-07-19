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
//#define MAC_DEBUG_REGRW_MAC
//#define MAC_DEBUG_REGRW_PHY
//#define MAC_DEBUG_REGRW_SCU
//#define MAC_DEBUG_REGRW_WDT
//#define MAC_DEBUG_REGRW_SDR
//#define MAC_DEBUG_REGRW_SMB
//#define MAC_DEBUG_REGRW_TIMER
//#define MAC_DEBUG_REGRW_GPIO
//#define MAC_DEBUG_MEMRW_Dat
//#define MAC_DEBUG_MEMRW_Des

#define MAC_C

#include "swfunc.h"

#include "comminf.h"
#include "stduboot.h"
#include <command.h>
#include <common.h>
#include <malloc.h>

// -------------------------------------------------------------
const uint32_t ARP_org_data[16] = {
    0xffffffff,
    0x0000ffff, // SA:00-00-
    0x12345678, // SA:78-56-34-12
    0x01000608, // ARP(0x0806)
    0x04060008,
    0x00000100, // sender MAC Address: 00 00
    0x12345678, // sender MAC Address: 12 34 56 78
    0xeb00a8c0, // sender IP Address:  192.168.0.235 (C0.A8.0.EB)
    0x00000000, // target MAC Address: 00 00 00 00
    0xa8c00000, // target MAC Address: 00 00, target IP Address:192.168
    0x00005c00, // target IP Address:  0.92 (C0.A8.0.5C)
		//	0x00000100, // target IP Address:  0.1 (C0.A8.0.1)
		//	0x0000de00, // target IP Address:  0.222 (C0.A8.0.DE)
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc68e2bd5};

//------------------------------------------------------------
// Read Memory
//------------------------------------------------------------
uint32_t Read_Mem_Dat_NCSI_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_MEMRW_Dat
	printf("[MEMRd-Dat] %08x = %08x\n", addr, SWAP_4B_LEDN_MEM( ReadSOC_DD(addr) ) );
#endif
	return ( SWAP_4B_LEDN_MEM( ReadSOC_DD(addr) ) );
}

uint32_t Read_Mem_Des_NCSI_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_MEMRW_Des
	printf("[MEMRd-Des] %08x = %08x\n", addr,
	       SWAP_4B_LEDN_MEM(ReadSOC_DD(addr)));
#endif
	return (SWAP_4B_LEDN_MEM(ReadSOC_DD(addr)));
}

uint32_t Read_Mem_Dat_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_MEMRW_Dat
	printf("[MEMRd-Dat] %08x = %08x\n", addr,
	       SWAP_4B_LEDN_MEM(ReadSOC_DD(addr)));
#endif
	return (SWAP_4B_LEDN_MEM(ReadSOC_DD(addr)));
}

uint32_t Read_Mem_Des_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_MEMRW_Des
	printf("[MEMRd-Des] %08x = %08x\n", addr,
	       SWAP_4B_LEDN_MEM(ReadSOC_DD(addr)));
#endif
	return (SWAP_4B_LEDN_MEM(ReadSOC_DD(addr)));
}

//------------------------------------------------------------
// Read Register
//------------------------------------------------------------
uint32_t Read_Reg_MAC_DD(MAC_ENGINE *eng, uint32_t addr)
{
#ifdef MAC_DEBUG_REGRW_MAC
	printf("[RegRd-MAC] %08x = %08x\n", eng->run.mac_base + addr,
	       SWAP_4B_LEDN_REG(ReadSOC_DD(eng->run.mac_base + addr)));
#endif
	return (SWAP_4B_LEDN_REG(ReadSOC_DD(eng->run.mac_base + addr)));
}

uint32_t Read_Reg_PHY_DD(MAC_ENGINE *eng, uint32_t addr)
{
#ifdef MAC_DEBUG_REGRW_PHY
	printf("[RegRd-PHY] %08x = %08x\n", eng->phy.PHY_BASE + addr,
	       SWAP_4B_LEDN_REG(ReadSOC_DD(eng->phy.PHY_BASE + addr)));
#endif
#ifdef CONFIG_ASPEED_AST2600
	if (addr == 0x60) {
		if (eng->run.is_rgmii)
			return (SWAP_4B_LEDN_REG(ReadSOC_DD(0x1e650000)));
		else
			return (SWAP_4B_LEDN_REG(ReadSOC_DD(0x1e650008)));
	} else {
		if (eng->run.is_rgmii)
			return (SWAP_4B_LEDN_REG(ReadSOC_DD(0x1e650004)));
		else
			return (SWAP_4B_LEDN_REG(ReadSOC_DD(0x1e65000c)));
	}
#else
	return (SWAP_4B_LEDN_REG(ReadSOC_DD(eng->phy.PHY_BASE + addr)));
#endif
}

uint32_t Read_Reg_SCU_DD_AST2600(uint32_t addr)
{
#ifdef MAC_DEBUG_REGRW_SCU
	printf("[RegRd-SCU] %08x = %08x\n", SCU_BASE + addr,
	       SWAP_4B_LEDN_REG(ReadSOC_DD(SCU_BASE + addr)));
#endif
	return (SWAP_4B_LEDN_REG(ReadSOC_DD(SCU_BASE + addr)));
}

uint32_t Read_Reg_SCU_DD(uint32_t addr)
{
#ifdef CONFIG_ASPEED_AST2600
	return (0);
#else
#ifdef MAC_DEBUG_REGRW_SCU
	printf("[RegRd-SCU] %08x = %08x\n", SCU_BASE + addr,
	       SWAP_4B_LEDN_REG(ReadSOC_DD(SCU_BASE + addr)));
#endif
	return (SWAP_4B_LEDN_REG(ReadSOC_DD(SCU_BASE + addr)));
#endif
}

uint32_t Read_Reg_WDT_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_REGRW_WDT
	printf("[RegRd-WDT] %08x = %08x\n", WDT_BASE + addr,
	       SWAP_4B_LEDN_REG(ReadSOC_DD(WDT_BASE + addr)));
#endif
	return (SWAP_4B_LEDN_REG(ReadSOC_DD(WDT_BASE + addr)));
}

uint32_t Read_Reg_SDR_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_REGRW_SDR
	printf("[RegRd-SDR] %08x = %08x\n", SDR_BASE + addr,
	       SWAP_4B_LEDN_REG(ReadSOC_DD(SDR_BASE + addr)));
#endif
	return (SWAP_4B_LEDN_REG(ReadSOC_DD(SDR_BASE + addr)));
}

uint32_t Read_Reg_TIMER_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_REGRW_TIMER
	printf("[RegRd-TIMER] %08x = %08x\n", TIMER_BASE + addr,
	       SWAP_4B_LEDN_REG(ReadSOC_DD(TIMER_BASE + addr)));
#endif
	return (SWAP_4B_LEDN_REG(ReadSOC_DD(TIMER_BASE + addr)));
}

uint32_t Read_Reg_GPIO_DD(uint32_t addr)
{
#ifdef MAC_DEBUG_REGRW_GPIO
	printf("[RegRd-GPIO] %08x = %08x\n", GPIO_BASE + addr,
	       SWAP_4B_LEDN_REG(ReadSOC_DD(GPIO_BASE + addr)));
#endif
	return (SWAP_4B_LEDN_REG(ReadSOC_DD(GPIO_BASE + addr)));
}

//------------------------------------------------------------
// Write Memory
//------------------------------------------------------------
void Write_Mem_Dat_NCSI_DD (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_MEMRW_Dat
	printf("[MEMWr-Dat] %08x = %08x\n", addr, SWAP_4B_LEDN_MEM( data ) );
#endif
	WriteSOC_DD( addr, SWAP_4B_LEDN_MEM( data ) );
}
void Write_Mem_Des_NCSI_DD (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_MEMRW_Des
	printf("[MEMWr-Des] %08x = %08x\n", addr, SWAP_4B_LEDN_MEM( data ) );
#endif
	WriteSOC_DD( addr, SWAP_4B_LEDN_MEM( data ) );
}
void Write_Mem_Dat_DD (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_MEMRW_Dat
	printf("[MEMWr-Dat] %08x = %08x\n", addr, SWAP_4B_LEDN_MEM( data ) );
#endif
	WriteSOC_DD( addr, SWAP_4B_LEDN_MEM( data ) );
}
void Write_Mem_Des_DD (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_MEMRW_Des
	printf("[MEMWr-Des] %08x = %08x\n", addr, SWAP_4B_LEDN_MEM( data ) );
#endif
	WriteSOC_DD( addr, SWAP_4B_LEDN_MEM( data ) );
}

//------------------------------------------------------------
// Write Register
//------------------------------------------------------------
void Write_Reg_MAC_DD (MAC_ENGINE *eng, uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_REGRW_MAC
	printf("[RegWr-MAC] %08x = %08x\n", eng->run.mac_base + addr, SWAP_4B_LEDN_REG( data ));
#endif
	WriteSOC_DD( eng->run.mac_base + addr, SWAP_4B_LEDN_REG( data ) );
}
void Write_Reg_PHY_DD (MAC_ENGINE *eng, uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_REGRW_PHY
	printf("[RegWr-PHY] %08x = %08x\n", eng->phy.PHY_BASE + addr, SWAP_4B_LEDN_REG( data ));
#endif
#ifdef CONFIG_ASPEED_AST2600
	if (addr == 0x60) {
		if (eng->run.is_rgmii)
			WriteSOC_DD( 0x1e650000, SWAP_4B_LEDN_REG( data ) );
		else
			WriteSOC_DD( 0x1e650008, SWAP_4B_LEDN_REG( data ) );
	} else {
		if (eng->run.is_rgmii)
			WriteSOC_DD( 0x1e650004, SWAP_4B_LEDN_REG( data ) );
		else
			WriteSOC_DD( 0x1e65000c, SWAP_4B_LEDN_REG( data ) );
	}
#else
	WriteSOC_DD( eng->phy.PHY_BASE + addr, SWAP_4B_LEDN_REG( data ) );
#endif
}
void Write_Reg_SCU_DD_AST2600 (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_REGRW_SCU
	printf("[RegWr-SCU] %08x = %08x\n", SCU_BASE + addr, SWAP_4B_LEDN_REG( data ));
#endif
	WriteSOC_DD( SCU_BASE + addr, SWAP_4B_LEDN_REG( data ) );
}
void Write_Reg_SCU_DD (uint32_t addr, uint32_t data) {
#ifdef CONFIG_ASPEED_AST2600
#else
#ifdef MAC_DEBUG_REGRW_SCU
	printf("[RegWr-SCU] %08x = %08x\n", SCU_BASE + addr, SWAP_4B_LEDN_REG( data ));
#endif
	WriteSOC_DD( SCU_BASE + addr, SWAP_4B_LEDN_REG( data ) );
#endif
}
void Write_Reg_WDT_DD (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_REGRW_WDT
	printf("[RegWr-WDT] %08x = %08x\n", WDT_BASE + addr, SWAP_4B_LEDN_REG( data ));
#endif
	WriteSOC_DD( WDT_BASE + addr, SWAP_4B_LEDN_REG( data ) );
}
void Write_Reg_TIMER_DD (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_REGRW_TIMER
	printf("[RegWr-TIMER] %08x = %08x\n", TIMER_BASE + addr, SWAP_4B_LEDN_REG( data ));
#endif
	WriteSOC_DD( TIMER_BASE + addr, SWAP_4B_LEDN_REG( data ) );
}
void Write_Reg_GPIO_DD (uint32_t addr, uint32_t data) {
#ifdef MAC_DEBUG_REGRW_GPIO
	printf("[RegWr-GPIO]%08x: %08x\n", GPIO_BASE + addr, SWAP_4B_LEDN_REG( data ));
#endif
	WriteSOC_DD( GPIO_BASE + addr, SWAP_4B_LEDN_REG( data ) );
}

//------------------------------------------------------------
// Others
//------------------------------------------------------------
void debug_pause (void) {
#ifdef DbgPrn_Enable_Debug_pause
	GET_CAHR();
#endif
}

//------------------------------------------------------------
void dump_mac_ROreg (MAC_ENGINE *eng) {
#ifdef Delay_MACDump
	DELAY( Delay_MACDump );
#endif
	printf("\n");
	printf("[MAC-H] ROReg A0h~ACh: %08x %08x %08x %08x\n", Read_Reg_MAC_DD( eng, 0xA0 ), Read_Reg_MAC_DD( eng, 0xA4 ), Read_Reg_MAC_DD( eng, 0xA8 ), Read_Reg_MAC_DD( eng, 0xAC ));
	printf("[MAC-H] ROReg B0h~BCh: %08x %08x %08x %08x\n", Read_Reg_MAC_DD( eng, 0xB0 ), Read_Reg_MAC_DD( eng, 0xB4 ), Read_Reg_MAC_DD( eng, 0xB8 ), Read_Reg_MAC_DD( eng, 0xBC ));
	printf("[MAC-H] ROReg C0h~C8h: %08x %08x %08x      \n", Read_Reg_MAC_DD( eng, 0xC0 ), Read_Reg_MAC_DD( eng, 0xC4 ), Read_Reg_MAC_DD( eng, 0xC8 ));
}

//------------------------------------------------------------
// IO delay
//------------------------------------------------------------
void init_iodelay (MAC_ENGINE *eng) 
{
	int        index;

	nt_log_func_name();

	if (eng->run.is_rgmii)
		eng->io.Dly_3Regiser = 1;
	else
		eng->io.Dly_3Regiser = 0;
	//------------------------------
	// IO Strength Max.
	//------------------------------
	//------------------------------
	// [IO]setup Str_reg_idx
	// [IO]setup Str_reg_value
	// [IO]setup Str_reg_mask
	// [IO]setup Str_max
	// [IO]setup Str_shf
	//------------------------------
	// Get bit (shift) of IO driving strength register
	eng->io.Str_reg_idx   = 0x90;
	eng->io.Str_reg_Lbit  =  8;
	eng->io.Str_reg_Hbit  = 11;
	eng->io.Str_reg_value = ( eng->reg.SCU_090 >> eng->io.Str_reg_Lbit ) & 0xf;
	eng->io.Str_reg_mask  = ( eng->reg.SCU_090 & 0xfffff0ff );
	eng->io.Str_max       = 1;//0~1
	if (!eng->run.is_rgmii) {
		switch ( eng->run.mac_idx ) {
			case 0  : eng->io.Str_shf =  9; break;
			case 1  : eng->io.Str_shf = 11; break;
		}
	}
	else {//AST2500 RGMII only support high drive RGMIITXCK
		if ( eng->run.TM_IOStrength )
			printf("The RGMII driving strength testing v1.0\n");
		switch ( eng->run.mac_idx ) {
			case 0  : eng->io.Str_shf =  8; break;
			case 1  : eng->io.Str_shf = 10; break;
		}
	}

	if ( !eng->run.TM_IOStrength )
		eng->io.Str_max = 0;

	//------------------------------
	// IO Delay Stage/Step
	//------------------------------
	//------------------------------
	// [IO]setup Dly_stagebit
	// [IO]setup Dly_stage_in
	// [IO]setup Dly_stage_out
	// [IO]setup Dly_step
	// [IO]setup Dly_mask_bit_in
	// [IO]setup Dly_mask_bit_out
	//------------------------------
	eng->io.Dly_stage_shf_i = (eng->arg.ctrl.b.full_range) ? 0 : AST2500_IOStageShiftBit_In ;
	eng->io.Dly_stage_shf_o = (eng->arg.ctrl.b.full_range) ? 0 : AST2500_IOStageShiftBit_Out;

	eng->io.Dly_stagebit  = 6;
	eng->io.Dly_stage     =   ( 1 << eng->io.Dly_stagebit );
	eng->io.Dly_stage_in  = ( eng->io.Dly_stage >> eng->io.Dly_stage_shf_i );
	eng->io.Dly_stage_out = ( eng->io.Dly_stage >> eng->io.Dly_stage_shf_o );
	eng->io.Dly_step      = AST2500_IOStageStep;

	eng->io.Dly_mask_bit_in = eng->io.Dly_stage - 1;
	if (!eng->run.is_rgmii)
		eng->io.Dly_mask_bit_out = 1;
	else
		eng->io.Dly_mask_bit_out = eng->io.Dly_mask_bit_in;

	//------------------------------
	// IO-Delay Register Bit Position
	//------------------------------
	//------------------------------
	// [IO]setup Dly_out_shf
	// [IO]setup Dly_in_shf
	// [IO]setup Dly_in_shf_regH
	// [IO]setup Dly_out_shf_regH
	//------------------------------
	if (!eng->run.is_rgmii) {
		switch ( eng->run.mac_idx ) {
			case 0  : eng->io.Dly_out_shf = 24; eng->io.Dly_in_shf = 12; break;
			case 1  : eng->io.Dly_out_shf = 25; eng->io.Dly_in_shf = 18; break;
		}
	}
	else {
		switch ( eng->run.mac_idx ) {
			case 0  : eng->io.Dly_out_shf =  0; eng->io.Dly_in_shf  = 12; break;
			case 1  : eng->io.Dly_out_shf =  6; eng->io.Dly_in_shf  = 18; break;
		}
	} // End if (!eng->run.is_rgmii)

	eng->io.Dly_in_shf_regH  = eng->io.Dly_in_shf  + eng->io.Dly_stagebit - 1;
	eng->io.Dly_out_shf_regH = eng->io.Dly_out_shf + eng->io.Dly_stagebit - 1;

	//------------------------------
	// [IO]setup Dly_mask_pos
	// [Reg]setup SCU_048_mix
	//------------------------------
	eng->io.Dly_mask_pos = ( eng->io.Dly_mask_bit_in  << eng->io.Dly_in_shf  )
	                     | ( eng->io.Dly_mask_bit_out << eng->io.Dly_out_shf );
	eng->reg.SCU_048_mix = eng->reg.SCU_048_mix | ( eng->reg.SCU_048_check & ( ~eng->io.Dly_mask_pos ) );

	//------------------------------
	// [IO]setup value_ary
	//------------------------------
	if (eng->reg.SCU_07c == 0x01000003) {
		for (index = 0; index < 16; index++) {
			eng->io.value_ary[ index ] = IOValue_Array_A0[ index ];
		}
	}
	else {
		for (index = 0; index < eng->io.Dly_stage; index++)
			eng->io.value_ary[ index ] = index;
	}

	eng->io.init_done = 1;
}

//------------------------------------------------------------
int get_iodelay (MAC_ENGINE *eng) {
	int        index;
	int        index_max;

	nt_log_func_name();

	//------------------------------
	// IO Delay Register Setting
	//------------------------------
	//------------------------------
	// [IO]setup Dly_reg_idx
	// [IO]setup Dly_reg_value
	//------------------------------
	switch (eng->run.Speed_idx) {
	case 0:
		eng->io.Dly_reg_idx = 0x48;
		eng->io.Dly_reg_value = eng->reg.SCU_048;
		break;
	case 1:
		eng->io.Dly_reg_idx = 0xb8;
		eng->io.Dly_reg_value = eng->reg.SCU_0b8;
		break;
	case 2:
		eng->io.Dly_reg_idx = 0xbc;
		eng->io.Dly_reg_value = eng->reg.SCU_0bc;
		break;
	}

	//------------------------------
	// [IO]setup Dly_reg_name_tx
	// [IO]setup Dly_reg_name_rx
	// [IO]setup Dly_reg_name_tx_new
	// [IO]setup Dly_reg_name_rx_new
	//------------------------------
	if (!eng->run.is_rgmii)
		sprintf( eng->io.Dly_reg_name_tx, "Tx:SCU%2X[   %2d]=",  eng->io.Dly_reg_idx,                           eng->io.Dly_out_shf );
	else
		sprintf( eng->io.Dly_reg_name_tx, "Tx:SCU%2X[%2d:%2d]=", eng->io.Dly_reg_idx, eng->io.Dly_out_shf_regH, eng->io.Dly_out_shf );
	sprintf( eng->io.Dly_reg_name_rx, "Rx:SCU%2X[%2d:%2d]=", eng->io.Dly_reg_idx, eng->io.Dly_in_shf_regH,  eng->io.Dly_in_shf );

	if (!eng->run.is_rgmii)
		sprintf(eng->io.Dly_reg_name_tx_new,
			"Tx[   %2d]=", eng->io.Dly_out_shf);
	else
		sprintf(eng->io.Dly_reg_name_tx_new,
			"Tx[%2d:%2d]=", eng->io.Dly_out_shf_regH,
			eng->io.Dly_out_shf);
	sprintf(eng->io.Dly_reg_name_rx_new,
		"Rx[%2d:%2d]=", eng->io.Dly_in_shf_regH, eng->io.Dly_in_shf);

	//------------------------------
	// [IO]setup Dly_in_reg
	// [IO]setup Dly_out_reg
	//------------------------------
	// Get current clock delay value of TX(out) and RX(in) in the SCU48 register
	// and setting test range
	eng->io.Dly_in_reg  = ( eng->io.Dly_reg_value >> eng->io.Dly_in_shf  ) & eng->io.Dly_mask_bit_in;
	eng->io.Dly_out_reg = ( eng->io.Dly_reg_value >> eng->io.Dly_out_shf ) & eng->io.Dly_mask_bit_out;

	//------------------------------
	// [IO]setup Dly_in_reg_idx
	// [IO]setup Dly_in_min
	// [IO]setup Dly_in_max
	// [IO]setup Dly_out_reg_idx
	// [IO]setup Dly_out_min
	// [IO]setup Dly_out_max
	//------------------------------
	// Find the coordinate in X-Y axis
	index_max = ( eng->io.Dly_stage_in << eng->io.Dly_stage_shf_i );
	for ( index = 0; index < index_max; index++ )
		if ( eng->io.Dly_in_reg == eng->io.value_ary[ index ] ) {
			eng->io.Dly_in_reg_idx = index;
			eng->io.Dly_in_min     = index - ( eng->run.IO_Bund >> 1 );
			eng->io.Dly_in_max     = index + ( eng->run.IO_Bund >> 1 );
			break;
		}

	index_max = ( eng->io.Dly_stage_out << eng->io.Dly_stage_shf_o );
	for ( index = 0; index < index_max; index++ )
		if ( eng->io.Dly_out_reg == eng->io.value_ary[ index ] ) {
			eng->io.Dly_out_reg_idx = index;
			if (!eng->run.is_rgmii) {
				eng->io.Dly_out_min = index;
				eng->io.Dly_out_max = index;
			}
			else {
				eng->io.Dly_out_min = index - ( eng->run.IO_Bund >> 1 );
				eng->io.Dly_out_max = index + ( eng->run.IO_Bund >> 1 );
			}
			break;
		}

	if ( eng->run.IO_MrgChk ) {
		if ( eng->io.Dly_in_reg_idx >= eng->io.Dly_stage_in )
			return( ( eng->flg.Err_Flag = eng->flg.Err_Flag | Err_Flag_IOMarginOUF ) );
		if ( eng->io.Dly_out_reg_idx >= eng->io.Dly_stage_out )
			return( ( eng->flg.Err_Flag = eng->flg.Err_Flag | Err_Flag_IOMarginOUF ) );
#ifdef Enable_No_IOBoundary
		if ( eng->io.Dly_in_min  <  0                     ) { eng->flg.Wrn_Flag = eng->flg.Wrn_Flag | Wrn_Flag_IOMarginOUF; eng->io.Dly_in_min  = 0                     ;}
		if ( eng->io.Dly_in_max  >= eng->io.Dly_stage_in  ) { eng->flg.Wrn_Flag = eng->flg.Wrn_Flag | Wrn_Flag_IOMarginOUF; eng->io.Dly_in_max  = eng->io.Dly_stage_in-1;}

		if ( eng->io.Dly_out_min <  0                     ) { eng->flg.Wrn_Flag = eng->flg.Wrn_Flag | Wrn_Flag_IOMarginOUF; eng->io.Dly_out_min = 0                      ;}
		if ( eng->io.Dly_out_max >= eng->io.Dly_stage_out ) { eng->flg.Wrn_Flag = eng->flg.Wrn_Flag | Wrn_Flag_IOMarginOUF; eng->io.Dly_out_max = eng->io.Dly_stage_out-1;}
#else
		if ( ( eng->io.Dly_in_min < 0 ) || ( eng->io.Dly_in_max >= eng->io.Dly_stage_in ) )
			return( ( eng->flg.Err_Flag = eng->flg.Err_Flag | Err_Flag_IOMarginOUF ) );

		if ( ( eng->io.Dly_out_min < 0 ) || ( eng->io.Dly_out_max >= eng->io.Dly_stage_out ) )
			return( ( eng->flg.Err_Flag = eng->flg.Err_Flag | Err_Flag_IOMarginOUF ) );
#endif
	}

	//------------------------------
	// IO Delay Testing Boundary
	//------------------------------
	//------------------------------
	// [IO]setup Dly_in_cval
	// [IO]setup Dly_out_cval
	// [IO]setup Dly_in_str
	// [IO]setup Dly_in_end
	// [IO]setup Dly_out_str
	// [IO]setup Dly_out_end
	//------------------------------
	// Get the range for testmargin block
	if ( eng->run.TM_IOTiming ) {
		eng->io.Dly_in_cval  = eng->io.Dly_step;
		eng->io.Dly_out_cval = eng->io.Dly_step;
		eng->io.Dly_in_str   = 0;
		eng->io.Dly_in_end   = eng->io.Dly_stage_in-1;
		eng->io.Dly_out_str  = 0;
		if (!eng->run.is_rgmii)
			eng->io.Dly_out_end  = 1;
		else
			eng->io.Dly_out_end  = eng->io.Dly_stage_out-1;
	}
	else if ( eng->run.IO_Bund ) {
		eng->io.Dly_in_cval  = eng->io.Dly_step;
		eng->io.Dly_out_cval = eng->io.Dly_step;
		eng->io.Dly_in_str   = eng->io.Dly_in_min;
		eng->io.Dly_in_end   = eng->io.Dly_in_max;
		eng->io.Dly_out_str  = eng->io.Dly_out_min;
		eng->io.Dly_out_end  = eng->io.Dly_out_max;
	}
	else {
		eng->io.Dly_in_cval  = 1;
		eng->io.Dly_out_cval = 1;
		eng->io.Dly_in_str   = 0;
		eng->io.Dly_in_end   = 0;
		eng->io.Dly_out_str  = 0;
		eng->io.Dly_out_end  = 0;
	} // End if ( eng->run.TM_IOTiming )

	return(0);
}

//------------------------------------------------------------
// SCU
//------------------------------------------------------------
void recov_scu (MAC_ENGINE *eng) {
	nt_log_func_name();

	//MAC
//	Write_Reg_MAC_DD( eng, 0x08, eng->reg.MAC_008 );
//	Write_Reg_MAC_DD( eng, 0x0c, eng->reg.MAC_00c );
//	Write_Reg_MAC_DD( eng, 0x40, eng->reg.MAC_040 );

	//SCU
//	Write_Reg_SCU_DD( 0x004, eng->reg.SCU_004 );
	Write_Reg_SCU_DD( 0x008, eng->reg.SCU_008 );
//	Write_Reg_SCU_DD( 0x00c, eng->reg.SCU_00c );
	Write_Reg_SCU_DD( 0x048, eng->reg.SCU_048 );

	Write_Reg_SCU_DD( 0x074, eng->reg.SCU_074 );
	Write_Reg_SCU_DD( 0x080, eng->reg.SCU_080 );
	Write_Reg_SCU_DD( 0x088, eng->reg.SCU_088 );
	Write_Reg_SCU_DD( 0x090, eng->reg.SCU_090 );
	Write_Reg_SCU_DD( 0x0b8, eng->reg.SCU_0b8 );
	Write_Reg_SCU_DD( 0x0bc, eng->reg.SCU_0bc );
} // End void recov_scu (MAC_ENGINE *eng)

//------------------------------------------------------------
void read_scu (MAC_ENGINE *eng) 
{
	nt_log_func_name();

	if ( !eng->reg.SCU_oldvld ) {
		//SCU
		eng->reg.SCU_004 = Read_Reg_SCU_DD( 0x004 );
		eng->reg.SCU_008 = Read_Reg_SCU_DD( 0x008 );
		eng->reg.SCU_00c = Read_Reg_SCU_DD( 0x00c );
		eng->reg.SCU_048 = Read_Reg_SCU_DD( 0x048 );
		eng->reg.SCU_070 = Read_Reg_SCU_DD( 0x070 );
		eng->reg.SCU_074 = Read_Reg_SCU_DD( 0x074 );
		eng->reg.SCU_07c = Read_Reg_SCU_DD( 0x07c );
		eng->reg.SCU_080 = Read_Reg_SCU_DD( 0x080 );
		eng->reg.SCU_088 = Read_Reg_SCU_DD( 0x088 );
		eng->reg.SCU_090 = Read_Reg_SCU_DD( 0x090 );
		eng->reg.SCU_09c = Read_Reg_SCU_DD( 0x09c );
		eng->reg.SCU_0b8 = Read_Reg_SCU_DD( 0x0b8 );
		eng->reg.SCU_0bc = Read_Reg_SCU_DD( 0x0bc );
		eng->reg.SCU_0f0 = Read_Reg_SCU_DD( 0x0f0 );

		//WDT
		eng->reg.WDT_00c = Read_Reg_WDT_DD( 0x00c );
		eng->reg.WDT_02c = Read_Reg_WDT_DD( 0x02c );
		eng->reg.WDT_04c = Read_Reg_WDT_DD( 0x04c );

		eng->reg.SCU_oldvld = 1;
	} // End if ( !eng->reg.SCU_oldvld )


#ifdef CONFIG_ASPEED_AST2600
	eng->reg.SCU_048 = 0x00082208;
	eng->reg.SCU_0b8 = 0x00082208;
	eng->reg.SCU_0bc = 0x00082208;
	eng->reg.SCU_07c = 0x04000000;

	eng->reg.SCU_FPGASel = Read_Reg_SCU_DD_AST2600( 0x10c ) & 0x0fffffff;
	eng->reg.SCU_070     = Read_Reg_SCU_DD_AST2600( 0x500 ) & 0x000000c0;
	eng->reg.SCU_510     = Read_Reg_SCU_DD_AST2600( 0x510 ) & 0x00000003;

	Write_Reg_SCU_DD_AST2600( 0x000 , 0x1688a8a8 );
	Write_Reg_SCU_DD_AST2600( 0x010 , 0x1688a8a8 );
//(clock enable) --------------------
	Write_Reg_SCU_DD_AST2600( 0x084 , 0x00300000 );
	Write_Reg_SCU_DD_AST2600( 0x094 , 0x00300000 );
//(Reset) --------------------
	Write_Reg_SCU_DD_AST2600( 0x040 , 0x00001800 );
//	Write_Reg_SCU_DD_AST2600( 0x050 , 0x00001800 );//Old
	Write_Reg_SCU_DD_AST2600( 0x050 , 0x00300000 );//New
//PMI --------------------
//	Write_Reg_SCU_DD_AST2600( 0x054 , 0x00002000 );//Old
	Write_Reg_SCU_DD_AST2600( 0x054 , 0x00000008 );//New
//(RGMII) --------------------
/*	Write_Reg_SCU_DD_AST2600( 0x500 , 0x000000c0 );
	Write_Reg_SCU_DD_AST2600( 0x510 , 0x00000003 );
*/
//(RMII) --------------------
/*
	Write_Reg_SCU_DD_AST2600( 0x504 , 0x000000c0 );
	Write_Reg_SCU_DD_AST2600( 0x514 , 0x00000003 );
*/
//(Reset) --------------------
	Write_Reg_SCU_DD_AST2600( 0x044 , 0x00001800 );
//	Write_Reg_SCU_DD_AST2600( 0x054 , 0x00001800 );//Old
	Write_Reg_SCU_DD_AST2600( 0x054 , 0x00300000 );//New
#endif
} // End read_scu(MAC_ENGINE *eng)

//------------------------------------------------------------
void Setting_scu (MAC_ENGINE *eng) 
{
	nt_log_func_name();

	//------------------------------
	// [WDT]Disable Timer
	//------------------------------	

	Write_Reg_WDT_DD( 0x00c, eng->reg.WDT_00c & 0xfffffffc );
	Write_Reg_WDT_DD( 0x02c, eng->reg.WDT_02c & 0xfffffffc );
	Write_Reg_WDT_DD( 0x04c, eng->reg.WDT_04c & 0xfffffffc );

	Write_Reg_WDT_DD( 0x01c, Read_Reg_WDT_DD( 0x01c ) & 0xffffff9f );
	Write_Reg_WDT_DD( 0x03c, Read_Reg_WDT_DD( 0x03c ) & 0xffffff9f );
	Write_Reg_WDT_DD( 0x05c, Read_Reg_WDT_DD( 0x05c ) & 0xffffff9f );
}

//------------------------------------------------------------
void init_scu1 (MAC_ENGINE *eng) 
{
	nt_log_func_name();

	if ( eng->arg.run_mode == MODE_DEDICATED )
		init_scu_macio ( eng );

#ifdef Enable_BufMerge
	Write_Reg_SCU_DD( 0xf0, 0x66559959 );//MAC buffer merge
#endif
} // End void init_scu1 (MAC_ENGINE *eng)

//------------------------------------------------------------
void init_scu_macio (MAC_ENGINE *eng) 
{
	nt_log_func_name();
	switch (eng->run.MAC_idx_PHY) {
	case 0:
		Write_Reg_SCU_DD(0x88,
				 (eng->reg.SCU_088 & 0x3fffffff) | 0xc0000000);
		break; //[31]MAC1 MDIO, [30]MAC1 MDC
	case 1:
		Write_Reg_SCU_DD(0x90,
				 (eng->reg.SCU_090 & 0xfffffffb) | 0x00000004);
		break; //[2 ]MAC2 MDC/MDIO

	default:
		break;
	}
} // End void init_scu_macio (MAC_ENGINE *eng)

//------------------------------------------------------------
void init_scu_macrst (MAC_ENGINE *eng) 
{
	nt_log_func_name();

	Write_Reg_SCU_DD( 0x04, eng->reg.SCU_004_dis );//Rst
#ifdef Delay_SCU
	DELAY( Delay_SCU );
#endif
	Write_Reg_SCU_DD( 0x04, eng->reg.SCU_004_en );//Enable Engine
} // End void init_scu_macrst (MAC_ENGINE *eng)

//------------------------------------------------------------
void init_scu_macdis (MAC_ENGINE *eng) 
{
	nt_log_func_name();

	Write_Reg_SCU_DD( 0x04, eng->reg.SCU_004_dis );//Rst
	Read_Reg_SCU_DD( 0x04 );//delay

	Write_Reg_SCU_DD( 0x0c, eng->reg.SCU_00c_dis );//Clock
	Read_Reg_SCU_DD( 0x0c );//delay
} // End void init_scu_macdis (MAC_ENGINE *eng)

//------------------------------------------------------------
void init_scu_macen (MAC_ENGINE *eng) 
{

	nt_log_func_name();

	Read_Reg_SCU_DD( 0x0c );//delay
	Write_Reg_SCU_DD( 0x0c, eng->reg.SCU_00c_en );//Clock
	Read_Reg_SCU_DD( 0x0c );//delay

	Read_Reg_SCU_DD( 0x04 );//delay
	Write_Reg_SCU_DD( 0x04, eng->reg.SCU_004_en );//Enable Engine
	Read_Reg_SCU_DD( 0x04 );//delay
} // End void init_scu_macen (MAC_ENGINE *eng)

//------------------------------------------------------------
void init_scu2 (MAC_ENGINE *eng) 
{
#ifdef SCU_74h
  	nt_log_func_name();

	Write_Reg_SCU_DD( 0x74, eng->reg.SCU_074 | SCU_74h );//PinMux
  #ifdef Delay_SCU
	DELAY( Delay_SCU );
  #endif
#endif
} // End void init_scu2 (MAC_ENGINE *eng)

//------------------------------------------------------------
void init_scu3 (MAC_ENGINE *eng) 
{
#ifdef SCU_74h
  	nt_log_func_name();

	Write_Reg_SCU_DD( 0x74, eng->reg.SCU_074 | (SCU_74h & 0xffefffff) );//PinMux
  #ifdef Delay_SCU
	DELAY( Delay_SCU );
  #endif
#endif
} // End void init_scu3 (MAC_ENGINE *eng)

//------------------------------------------------------------
// MAC
//------------------------------------------------------------
void get_mac_info (MAC_ENGINE *eng) 
{
	nt_log_func_name();

	//------------------------------
	// [Inf]setup SA
	//------------------------------
	eng->reg.MAC_008 = Read_Reg_MAC_DD( eng, 0x08 );
	eng->reg.MAC_00c = Read_Reg_MAC_DD( eng, 0x0c );
	if (  (( eng->reg.MAC_008 == 0x0000 ) && ( eng->reg.MAC_00c == 0x00000000 ))
	   || (( eng->reg.MAC_008 == 0xffff ) && ( eng->reg.MAC_00c == 0xffffffff ))
	   )
	{
//		eng->reg.MAC_008 = 0x00000057;//MSB(0x00)
//		eng->reg.MAC_00c = 0x89568838;//LSB(0x38)
		eng->reg.MAC_008 = 0x0000000a;//MSB(0x00)
		eng->reg.MAC_00c = 0xf7837dd4;//LSB(0xd4)
	}

	eng->inf.SA[ 0 ] = ( eng->reg.MAC_008 >>  8 ) & 0xff;//MSB
	eng->inf.SA[ 1 ] = ( eng->reg.MAC_008       ) & 0xff;
	eng->inf.SA[ 2 ] = ( eng->reg.MAC_00c >> 24 ) & 0xff;
	eng->inf.SA[ 3 ] = ( eng->reg.MAC_00c >> 16 ) & 0xff;
	eng->inf.SA[ 4 ] = ( eng->reg.MAC_00c >>  8 ) & 0xff;
	eng->inf.SA[ 5 ] = ( eng->reg.MAC_00c       ) & 0xff;//LSB

	//------------------------------
	// [Reg]setup MAC_040_new
	//------------------------------
	eng->reg.MAC_040 = Read_Reg_MAC_DD( eng, 0x40 );
	if ( eng->arg.ctrl.b.mac_int_loopback )
		eng->reg.MAC_040_new = eng->reg.MAC_040 | 0x40000000;
	else
		eng->reg.MAC_040_new = eng->reg.MAC_040;

  #ifdef MAC_040_def
	Write_Reg_MAC_DD( eng, 0x40, eng->reg.MAC_040_new | MAC_040_def );
  #else
	Write_Reg_MAC_DD( eng, 0x40, eng->reg.MAC_040_new );
  #endif
}

//------------------------------------------------------------
void init_mac (MAC_ENGINE *eng) 
{
	nt_log_func_name();

#ifdef Enable_MAC_SWRst
	Write_Reg_MAC_DD( eng, 0x50, 0x80000000 | eng->reg.MAC_050_Speed );

	while (0x80000000 & Read_Reg_MAC_DD( eng, 0x50 )) {
//printf(".");
  #ifdef Delay_MACRst
		DELAY( Delay_MACRst );
  #endif
	}
  #ifdef Delay_MACRst
	DELAY( Delay_MACRst );
  #endif
#endif

//	Write_Reg_MAC_DD( eng, 0x20, ( eng->run.TDES_BASE + CPU_BUS_ADDR_SDRAM_OFFSET ) ); // 20130730
//	Write_Reg_MAC_DD( eng, 0x24, ( eng->run.RDES_BASE + CPU_BUS_ADDR_SDRAM_OFFSET ) ); // 20130730
	Write_Reg_MAC_DD( eng, 0x20, AT_MEMRW_BUF( eng->run.TDES_BASE ) ); // 20130730
	Write_Reg_MAC_DD( eng, 0x24, AT_MEMRW_BUF( eng->run.RDES_BASE ) ); // 20130730

	Write_Reg_MAC_DD( eng, 0x08, eng->reg.MAC_008 );
	Write_Reg_MAC_DD( eng, 0x0c, eng->reg.MAC_00c );

#ifdef MAC_030_def
	Write_Reg_MAC_DD( eng, 0x30, MAC_030_def );//Int Thr/Cnt
#endif
#ifdef MAC_034_def
	Write_Reg_MAC_DD( eng, 0x34, MAC_034_def );//Poll Cnt
#endif
#ifdef MAC_038_def
	Write_Reg_MAC_DD( eng, 0x38, MAC_038_def );
#endif
#ifdef MAC_048_def
	Write_Reg_MAC_DD( eng, 0x48, MAC_048_def );
#endif
#ifdef MAC_058_def
	Write_Reg_MAC_DD( eng, 0x58, MAC_058_def );
#endif

	if ( eng->arg.run_mode == MODE_NCSI )
		Write_Reg_MAC_DD( eng, 0x4c, NCSI_RxDMA_PakSize );
	else
		Write_Reg_MAC_DD( eng, 0x4c, DMA_PakSize );

	Write_Reg_MAC_DD( eng, 0x50, eng->reg.MAC_050_Speed );
#ifdef Delay_MACRst
#endif
	DELAY( Delay_MACRst );
} // End void init_mac (MAC_ENGINE *eng)

//------------------------------------------------------------
// Basic
//------------------------------------------------------------
void FPri_RegValue (MAC_ENGINE *eng, BYTE option) 
{
	nt_log_func_name();

	PRINTF( option, "[SDR] Date:%08x\n", Read_Reg_SDR_DD( 0x88 ) );
	PRINTF( option, "[SDR]  80:%08x %08x %08x %08x\n", Read_Reg_SDR_DD( 0x80 ), Read_Reg_SDR_DD( 0x84 ), Read_Reg_SDR_DD( 0x88 ), Read_Reg_SDR_DD( 0x8c ) );

	PRINTF( option, "[SCU]  04:%08x  08:%08x  0c:%08x\n",           eng->reg.SCU_004, eng->reg.SCU_008, eng->reg.SCU_00c );
	PRINTF( option, "[SCU]  1c:%08x  2c:%08x  48:%08x  4c:%08x\n", Read_Reg_SCU_DD( 0x01c ), Read_Reg_SCU_DD( 0x02c ), eng->reg.SCU_048, Read_Reg_SCU_DD( 0x04c ) );
	PRINTF( option, "[SCU]  70:%08x  74:%08x  7c:%08x  f0:%08x\n", eng->reg.SCU_070, eng->reg.SCU_074, eng->reg.SCU_07c, eng->reg.SCU_0f0 );
	PRINTF( option, "[SCU]  80:%08x  88:%08x  90:%08x  9c:%08x\n", eng->reg.SCU_080, eng->reg.SCU_088, eng->reg.SCU_090, eng->reg.SCU_09c );
	PRINTF( option, "[SCU]  a0:%08x  a4:%08x  b8:%08x  bc:%08x\n", Read_Reg_SCU_DD( 0x0a0 ), Read_Reg_SCU_DD( 0x0a4 ), eng->reg.SCU_0b8, eng->reg.SCU_0bc );

	PRINTF( option, "[SCU] 13c:%08x 140:%08x 144:%08x 1dc:%08x\n", Read_Reg_SCU_DD( 0x13c ), Read_Reg_SCU_DD( 0x140 ), Read_Reg_SCU_DD( 0x144 ), Read_Reg_SCU_DD( 0x1dc ) );
	PRINTF( option, "[WDT]  0c:%08x  2c:%08x  4c:%08x\n", eng->reg.WDT_00c, eng->reg.WDT_02c, eng->reg.WDT_04c );
	PRINTF( option, "[MAC]  08:%08x  0c:%08x\n", eng->reg.MAC_008, eng->reg.MAC_00c );
	PRINTF( option, "[MAC]  A0|%08x %08x %08x %08x\n", Read_Reg_MAC_DD( eng, 0xa0 ), Read_Reg_MAC_DD( eng, 0xa4 ), Read_Reg_MAC_DD( eng, 0xa8 ), Read_Reg_MAC_DD( eng, 0xac ) );
	PRINTF( option, "[MAC]  B0|%08x %08x %08x %08x\n", Read_Reg_MAC_DD( eng, 0xb0 ), Read_Reg_MAC_DD( eng, 0xb4 ), Read_Reg_MAC_DD( eng, 0xb8 ), Read_Reg_MAC_DD( eng, 0xbc ) );
	PRINTF( option, "[MAC]  C0|%08x %08x %08x\n",       Read_Reg_MAC_DD( eng, 0xc0 ), Read_Reg_MAC_DD( eng, 0xc4 ), Read_Reg_MAC_DD( eng, 0xc8 ) );

} // End void FPri_RegValue (MAC_ENGINE *eng, BYTE *fp)

//------------------------------------------------------------
void FPri_End (MAC_ENGINE *eng, BYTE option) 
{
	nt_log_func_name();
	if ((!eng->run.is_rgmii) && ( eng->phy.RMIICK_IOMode != 0 ) && eng->run.IO_MrgChk && eng->flg.AllFail ) {
		if ( eng->arg.ctrl.b.rmii_phy_in == 0 ) {
			PRINTF( option, "\n\n\n\n\n\n[Info] The PHY's RMII reference clock pin is setting to the OUTPUT mode now.\n" );
			PRINTF( option, "       Maybe you can run the INPUT mode command \"mactest  %d %d %d %d %d %d %d\".\n\n\n\n", eng->arg.run_idx, eng->arg.run_speed, (eng->arg.ctrl.w | 0x80), eng->arg.loop_max, eng->arg.test_mode, eng->arg.GPHYADR, eng->arg.GChk_TimingBund );
		}
		else {
			PRINTF( option, "\n\n\n\n\n\n[Info] The PHY's RMII reference clock pin is setting to the INPUT mode now.\n" );
			PRINTF( option, "       Maybe you can run the OUTPUT mode command \"mactest  %d %d %d %d %d %d %d\".\n\n\n\n", eng->arg.run_idx, eng->arg.run_speed, (eng->arg.ctrl.w & 0x7f), eng->arg.loop_max, eng->arg.test_mode, eng->arg.GPHYADR, eng->arg.GChk_TimingBund );
		}
	} // End if ( eng->env.MAC_RMII && ( eng->phy.RMIICK_IOMode != 0 ) && eng->run.IO_MrgChk && eng->flg.AllFail )

	if ( !eng->run.TM_RxDataEn ) {
	}
	else if ( eng->flg.Err_Flag ) {
		PRINTF( option, "                    \n----> fail !!!\n" );
	}
	else {
		PRINTF( option, "                    \n----> All Pass !!!\n" );
	}

	//------------------------------
	//[Warning] PHY Address
	//------------------------------
	if ( eng->arg.run_mode == MODE_DEDICATED ) {
		if ( eng->arg.GPHYADR != eng->phy.Adr )
			PRINTF( option, "\n[Warning] PHY Address change from %d to %d !!!\n", eng->arg.GPHYADR, eng->phy.Adr );
	}

	//------------------------------
	//[Warning] IO Strength
	//------------------------------
	if (eng->io.init_done && eng->io.Str_reg_value) {
		PRINTF(option,
		       "\n[Warning] SCU%02X[%2d:%2d] == 0x%02x is not the "
		       "suggestion value 0.\n",
		       eng->io.Str_reg_idx, eng->io.Str_reg_Hbit,
		       eng->io.Str_reg_Lbit, eng->io.Str_reg_value);
		PRINTF(option, "          This change at this platform must "
			       "been proven again by the ASPEED.\n");
	}

	//------------------------------
	//[Warning] IO Timing
	//------------------------------
	if ((eng->reg.SCU_048_check != eng->reg.SCU_048_default)) {
		PRINTF(option,
		       "\n[Warning] SCU48 == 0x%08x is not the suggestion "
		       "value 0x%08x.\n",
		       eng->reg.SCU_048, eng->reg.SCU_048_default);
		PRINTF(option, "          This change at this platform must "
			       "been proven again by the ASPEED.\n");
	}
	
		if ((eng->reg.SCU_0b8 != SCU_B8h_AST2500)) {
			PRINTF(option,
			       "\n[Warning] SCUB8 == 0x%08x is not the "
			       "suggestion value 0x%08x.\n",
			       eng->reg.SCU_0b8, SCU_B8h_AST2500);
			PRINTF(option,
			       "          This change at this platform must "
			       "been proven again by the ASPEED.\n");
		}
		if ((eng->reg.SCU_0bc != SCU_BCh_AST2500)) {
			PRINTF(option,
			       "\n[Warning] SCUBC == 0x%08x is not the "
			       "suggestion value 0x%08x.\n",
			       eng->reg.SCU_0bc, SCU_BCh_AST2500);
			PRINTF(option,
			       "          This change at this platform must "
			       "been proven again by the ASPEED.\n");
		}

	if ( eng->arg.run_mode == MODE_NCSI ) {
		PRINTF( option, "\n[Arg] %d %d %d %d %d %d %d {%d}\n", eng->arg.run_idx, eng->arg.GPackageTolNum, eng->arg.GChannelTolNum, eng->arg.test_mode, eng->arg.GChk_TimingBund, eng->arg.ctrl.w, eng->arg.GARPNumCnt, TIME_OUT_NCSI );

		switch ( eng->ncsi_cap.PCI_DID_VID ) {
			case PCI_DID_VID_Intel_82574L             : { PRINTF( option, "[NC]%08x %08x: Intel 82574L       \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82575_10d6         : { PRINTF( option, "[NC]%08x %08x: Intel 82575        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82575_10a7         : { PRINTF( option, "[NC]%08x %08x: Intel 82575        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82575_10a9         : { PRINTF( option, "[NC]%08x %08x: Intel 82575        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_10c9         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_10e6         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_10e7         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_10e8         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_1518         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_1526         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_150a         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82576_150d         : { PRINTF( option, "[NC]%08x %08x: Intel 82576        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82599_10fb         : { PRINTF( option, "[NC]%08x %08x: Intel 82599        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_82599_1557         : { PRINTF( option, "[NC]%08x %08x: Intel 82599        \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_I210_1533          : { PRINTF( option, "[NC]%08x %08x: Intel I210         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_I210_1537          : { PRINTF( option, "[NC]%08x %08x: Intel I210         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_I350_1521          : { PRINTF( option, "[NC]%08x %08x: Intel I350         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_I350_1523          : { PRINTF( option, "[NC]%08x %08x: Intel I350         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_X540               : { PRINTF( option, "[NC]%08x %08x: Intel X540         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_X550               : { PRINTF( option, "[NC]%08x %08x: Intel X550         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_Broadwell_DE       : { PRINTF( option, "[NC]%08x %08x: Intel Broadwell-DE \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Intel_X722_37d0          : { PRINTF( option, "[NC]%08x %08x: Intel X722         \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM5718         : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM5718   \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM5719         : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM5719   \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM5720         : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM5720   \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM5725         : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM5725   \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM57810S       : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM57810S \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_Cumulus         : { PRINTF( option, "[NC]%08x %08x: Broadcom Cumulus   \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM57302        : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM57302  \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Broadcom_BCM957452       : { PRINTF( option, "[NC]%08x %08x: Broadcom BCM957452 \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Mellanox_ConnectX_3_1003 : { PRINTF( option, "[NC]%08x %08x: Mellanox ConnectX-3\n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Mellanox_ConnectX_3_1007 : { PRINTF( option, "[NC]%08x %08x: Mellanox ConnectX-3\n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			case PCI_DID_VID_Mellanox_ConnectX_4      : { PRINTF( option, "[NC]%08x %08x: Mellanox ConnectX-4\n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			default:
			switch ( eng->ncsi_cap.manufacturer_id ) {
				case ManufacturerID_Intel    : { PRINTF( option, "[NC]%08x %08x: Intel              \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
				case ManufacturerID_Broadcom : { PRINTF( option, "[NC]%08x %08x: Broadcom           \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
				case ManufacturerID_Mellanox : { PRINTF( option, "[NC]%08x %08x: Mellanox           \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
				case ManufacturerID_Mellanox1: { PRINTF( option, "[NC]%08x %08x: Mellanox           \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
				case ManufacturerID_Emulex   : { PRINTF( option, "[NC]%08x %08x: Emulex             \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
				default                      : { PRINTF( option, "[NC]%08x %08x                     \n", eng->ncsi_cap.manufacturer_id, eng->ncsi_cap.PCI_DID_VID ); break; }
			} // End switch ( eng->ncsi_cap.manufacturer_id )
		} // End switch ( eng->ncsi_cap.PCI_DID_VID )
	}
	else {
#if 0		
		if (eng->arg.loop_inf) {
			PRINTF( option, "\n[Arg] %d %d %d # %d %d %d %x (%s){%d x:%d %d %d}[%d %d %d] %d\n"  , eng->arg.run_idx, eng->arg.run_speed, eng->arg.ctrl.w,                     eng->arg.test_mode, eng->arg.GPHYADR, eng->arg.GChk_TimingBund, eng->arg.GUserDVal,  eng->run.TIME_OUT_Des_PHYRatio, TIME_OUT_Des_1G, TIME_OUT_Des_100M, TIME_OUT_Des_10M, eng->run.Loop_rl[0], eng->run.Loop_rl[1], eng->run.Loop_rl[2], eng->dat.Des_Num );
		}
		else {
			PRINTF( option, "\n[Arg] %d %d %d %d %d %d %d %x (%s){%d x:%d %d %d}[%d %d %d] %d\n", eng->arg.run_idx, eng->arg.run_speed, eng->arg.ctrl.w, eng->arg.loop_max, eng->arg.test_mode, eng->arg.GPHYADR, eng->arg.GChk_TimingBund, eng->arg.GUserDVal, eng->run.TIME_OUT_Des_PHYRatio, TIME_OUT_Des_1G, TIME_OUT_Des_100M, TIME_OUT_Des_10M, eng->run.Loop_rl[0], eng->run.Loop_rl[1], eng->run.Loop_rl[2], eng->dat.Des_Num );
		}
#endif		

		PRINTF( option, "[PHY] Adr:%d ID2:%04x ID3:%04x (%s)\n", eng->phy.Adr, eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName );
	} // End if ( eng->arg.run_mode == MODE_NCSI )


	PRINTF( option, "[Ver II] %s\n", version_name );
} // End void FPri_End (MAC_ENGINE *eng, BYTE option)

//------------------------------------------------------------
void FPri_ErrFlag (MAC_ENGINE *eng, BYTE option) 
{
	nt_log_func_name();
	if ( eng->flg.Flag_PrintEn ) {
		if ( eng->flg.Wrn_Flag ) {
			if ( eng->flg.Wrn_Flag & Wrn_Flag_IOMarginOUF ) {
				PRINTF( option, "[Warning] IO timing testing range out of boundary\n" );
				if (!eng->run.is_rgmii) {
					PRINTF( option, "      (reg:%d,%d) %dx1(%d~%d,%d)\n", eng->io.Dly_in_reg_idx,
											      eng->io.Dly_out_reg_idx,
											      eng->run.IO_Bund,
											      eng->io.Dly_in_min,
											      eng->io.Dly_in_max,
											      eng->io.Dly_out_min );
				}
				else {
					PRINTF( option, "      (reg:%d,%d) %dx%d(%d~%d,%d~%d)\n", eng->io.Dly_in_reg_idx,
												  eng->io.Dly_out_reg_idx,
												  eng->run.IO_Bund,
												  eng->run.IO_Bund,
												  eng->io.Dly_in_min,
												  eng->io.Dly_in_max,
												  eng->io.Dly_out_min,
												  eng->io.Dly_out_max );
				}
			} // End if ( eng->flg.Wrn_Flag & Wrn_Flag_IOMarginOUF )
			if ( eng->flg.Wrn_Flag & Wrn_Flag_RxErFloatting ) {
				PRINTF( option, "[Warning] NCSI RXER pin may be floatting to the MAC !!!\n" );
				PRINTF( option, "          Please contact with the ASPEED Inc. for more help.\n" );
			} // End if ( eng->flg.Wrn_Flag & Wrn_Flag_RxErFloatting )
		} // End if ( eng->flg.Wrn_Flag )

		if ( eng->flg.Err_Flag ) {
			PRINTF( option, "\n\n" );
//PRINTF( option, "Err_Flag: %x\n\n", eng->flg.Err_Flag );

			if ( eng->flg.Err_Flag & Err_Flag_PHY_Type                ) { PRINTF( option, "[Err] Unidentifiable PHY                                     \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_MALLOC_FrmSize          ) { PRINTF( option, "[Err] Malloc fail at frame size buffer                       \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_MALLOC_LastWP           ) { PRINTF( option, "[Err] Malloc fail at last WP buffer                          \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_Check_Buf_Data          ) { PRINTF( option, "[Err] Received data mismatch                                 \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_NCSI_Check_TxOwnTimeOut ) { PRINTF( option, "[Err] Time out of checking Tx owner bit in NCSI packet       \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_NCSI_Check_RxOwnTimeOut ) { PRINTF( option, "[Err] Time out of checking Rx owner bit in NCSI packet       \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_NCSI_Check_ARPOwnTimeOut) { PRINTF( option, "[Err] Time out of checking ARP owner bit in NCSI packet      \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_NCSI_No_PHY             ) { PRINTF( option, "[Err] Can not find NCSI PHY                                  \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_NCSI_Channel_Num        ) { PRINTF( option, "[Err] NCSI Channel Number Mismatch                           \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_NCSI_Package_Num        ) { PRINTF( option, "[Err] NCSI Package Number Mismatch                           \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_PHY_TimeOut_RW          ) { PRINTF( option, "[Err] Time out of read/write PHY register                    \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_PHY_TimeOut_Rst         ) { PRINTF( option, "[Err] Time out of reset PHY register                         \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_RXBUF_UNAVA             ) { PRINTF( option, "[Err] MAC00h[2]:Receiving buffer unavailable                 \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_RPKT_LOST               ) { PRINTF( option, "[Err] MAC00h[3]:Received packet lost due to RX FIFO full     \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_NPTXBUF_UNAVA           ) { PRINTF( option, "[Err] MAC00h[6]:Normal priority transmit buffer unavailable  \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_TPKT_LOST               ) { PRINTF( option, "[Err] MAC00h[7]:Packets transmitted to Ethernet lost         \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_DMABufNum               ) { PRINTF( option, "[Err] DMA Buffer is not enough                               \n" ); }
			if ( eng->flg.Err_Flag & Err_Flag_IOMargin                ) { PRINTF( option, "[Err] IO timing margin is not enough                         \n" ); }

			if ( eng->flg.Err_Flag & Err_Flag_MHCLK_Ratio             ) {
				PRINTF( option, "[Err] Error setting of MAC AHB bus clock (SCU08[18:16])      \n" );
				if ( eng->env.at_least_1g_valid )
					{ PRINTF( option, "      SCU08[18:16] == 0x%01x is not the suggestion value 2.\n", eng->env.MHCLK_Ratio ); }
				else
					{ PRINTF( option, "      SCU08[18:16] == 0x%01x is not the suggestion value 4.\n", eng->env.MHCLK_Ratio ); }
			} // End if ( eng->flg.Err_Flag & Err_Flag_MHCLK_Ratio             )

			if ( eng->flg.Err_Flag & Err_Flag_IOMarginOUF ) {
				PRINTF( option, "[Err] IO timing testing range out of boundary\n");
				if (!eng->run.is_rgmii) {
					PRINTF( option, "      (reg:%d,%d) %dx1(%d~%d,%d)\n", eng->io.Dly_in_reg_idx,
											      eng->io.Dly_out_reg_idx,
											      eng->run.IO_Bund,
											      eng->io.Dly_in_min,
											      eng->io.Dly_in_max,
											      eng->io.Dly_out_min );
				}
				else {
					PRINTF( option, "      (reg:%d,%d) %dx%d(%d~%d,%d~%d)\n", eng->io.Dly_in_reg_idx,
												  eng->io.Dly_out_reg_idx,
												  eng->run.IO_Bund,
												  eng->run.IO_Bund,
												  eng->io.Dly_in_min,
												  eng->io.Dly_in_max,
												  eng->io.Dly_out_min,
												  eng->io.Dly_out_max );
				}
			} // End if ( eng->flg.Err_Flag & Err_Flag_IOMarginOUF )

			if ( eng->flg.Err_Flag & Err_Flag_Check_Des ) {
				PRINTF( option, "[Err] Descriptor error\n");
				if ( eng->flg.Des_Flag & Des_Flag_TxOwnTimeOut ) { PRINTF( option, "[Des] Time out of checking Tx owner bit\n" ); }
				if ( eng->flg.Des_Flag & Des_Flag_RxOwnTimeOut ) { PRINTF( option, "[Des] Time out of checking Rx owner bit\n" ); }
				if ( eng->flg.Des_Flag & Des_Flag_FrameLen     ) { PRINTF( option, "[Des] Frame length mismatch            \n" ); }
				if ( eng->flg.Des_Flag & Des_Flag_RxErr        ) { PRINTF( option, "[Des] Input signal RxErr               \n" ); }
				if ( eng->flg.Des_Flag & Des_Flag_CRC          ) { PRINTF( option, "[Des] CRC error of frame               \n" ); }
				if ( eng->flg.Des_Flag & Des_Flag_FTL          ) { PRINTF( option, "[Des] Frame too long                   \n" ); }
				if ( eng->flg.Des_Flag & Des_Flag_Runt         ) { PRINTF( option, "[Des] Runt packet                      \n" ); }
				if ( eng->flg.Des_Flag & Des_Flag_OddNibble    ) { PRINTF( option, "[Des] Nibble bit happen                \n" ); }
				if ( eng->flg.Des_Flag & Des_Flag_RxFIFOFull   ) { PRINTF( option, "[Des] Rx FIFO full                     \n" ); }
			} // End if ( eng->flg.Err_Flag & Err_Flag_Check_Des )

			if ( eng->flg.Err_Flag & Err_Flag_MACMode ) {
				PRINTF( option, "[Err] MAC interface mode mismatch\n" );
				PRINTF(option, "");

				for (int i = 0; i < 4; i++) {
					if (eng->env.is_1g_valid[i]) {
						PRINTF(option,
						       "[MAC%d] is RGMII\n");
					} else {
						PRINTF(option,
						       "[MAC%d] RMII\n");
					}
				}
			} // End if ( eng->flg.Err_Flag & Err_Flag_MACMode )

			if ( eng->arg.run_mode == MODE_NCSI ) {
				if ( eng->flg.Err_Flag & Err_Flag_NCSI_LinkFail ) {
					PRINTF( option, "[Err] NCSI packet retry number over flows when find channel\n" );

					if ( eng->flg.NCSI_Flag & NCSI_Flag_Get_Version_ID                  ) { PRINTF( option, "[NCSI] Time out when Get Version ID                  \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Get_Capabilities                ) { PRINTF( option, "[NCSI] Time out when Get Capabilities                \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Select_Active_Package           ) { PRINTF( option, "[NCSI] Time out when Select Active Package           \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Enable_Set_MAC_Address          ) { PRINTF( option, "[NCSI] Time out when Enable Set MAC Address          \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Enable_Broadcast_Filter         ) { PRINTF( option, "[NCSI] Time out when Enable Broadcast Filter         \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Enable_Network_TX               ) { PRINTF( option, "[NCSI] Time out when Enable Network TX               \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Enable_Channel                  ) { PRINTF( option, "[NCSI] Time out when Enable Channel                  \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Disable_Network_TX              ) { PRINTF( option, "[NCSI] Time out when Disable Network TX              \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Disable_Channel                 ) { PRINTF( option, "[NCSI] Time out when Disable Channel                 \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Select_Package                  ) { PRINTF( option, "[NCSI] Time out when Select Package                  \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Deselect_Package                ) { PRINTF( option, "[NCSI] Time out when Deselect Package                \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Set_Link                        ) { PRINTF( option, "[NCSI] Time out when Set Link                        \n" ); }
					if ( eng->flg.NCSI_Flag & NCSI_Flag_Get_Controller_Packet_Statistics) { PRINTF( option, "[NCSI] Time out when Get Controller Packet Statistics\n" ); }
				}

				if ( eng->flg.Err_Flag & Err_Flag_NCSI_Channel_Num ) { PRINTF( option, "[NCSI] Channel number expected: %d, real: %d\n", eng->arg.GChannelTolNum, eng->dat.number_chl ); }
				if ( eng->flg.Err_Flag & Err_Flag_NCSI_Package_Num ) { PRINTF( option, "[NCSI] Peckage number expected: %d, real: %d\n", eng->arg.GPackageTolNum, eng->dat.number_pak ); }
			} // End if ( eng->arg.run_mode == MODE_NCSI )
		} // End if ( eng->flg.Err_Flag )
	} // End if ( eng->flg.Flag_PrintEn )
} // End void FPri_ErrFlag (MAC_ENGINE *eng, BYTE option)

//------------------------------------------------------------
void Finish_Close (MAC_ENGINE *eng) 
{
	nt_log_func_name();
#ifdef Enable_RecovSCU
	if ( eng->reg.SCU_oldvld )
		recov_scu( eng );
#endif

} // End void Finish_Close (MAC_ENGINE *eng)

//------------------------------------------------------------
char Finish_Check (MAC_ENGINE *eng, int value) 
{
#if defined(CONFIG_ASPEED_AST2500)
	uint32_t   reg;
	BYTE    shift_value = 0;
#endif
	nt_log_func_name();

#ifdef Disable_VGA
	if ( eng->env.VGAModeVld ) {
		outp(0x3d4, 0x17);
		outp(0x3d5, eng->env.VGAMode);
	}
#endif

	if ( eng->arg.run_mode ==  MODE_DEDICATED ) {
		if ( eng->dat.FRAME_LEN )
			free( eng->dat.FRAME_LEN );

		if ( eng->dat.wp_lst )
			free( eng->dat.wp_lst );
	}

	eng->flg.Err_Flag = eng->flg.Err_Flag | value;

	if ( DbgPrn_ErrFlg )
		printf("\nErr_Flag: [%08x]\n", eng->flg.Err_Flag);

	if ( !eng->run.TM_Burst )
		FPri_ErrFlag( eng, FP_LOG );

	if ( eng->run.TM_IOTiming )
		FPri_ErrFlag( eng, FP_IO );

	FPri_ErrFlag( eng, STD_OUT );

	if ( !eng->run.TM_Burst )
		FPri_End( eng, FP_LOG );

	if ( eng->run.TM_IOTiming )
		FPri_End( eng, FP_IO );

	FPri_End( eng, STD_OUT );


	if ( !eng->run.TM_Burst )
		FPri_RegValue( eng, FP_LOG );
	if ( eng->run.TM_IOTiming )
		FPri_RegValue( eng, FP_IO  );

	Finish_Close( eng );

#if defined(CONFIG_ASPEED_AST2500)
	reg = Read_Reg_SCU_DD( 0x40 );
	if ( eng->arg.run_mode == MODE_DEDICATED )
		shift_value = 18 + eng->run.mac_idx;
	else
		shift_value = 16 + eng->run.mac_idx;
#endif

	if ( eng->flg.Err_Flag )
	{
		// Fail
#if defined(CONFIG_ASPEED_AST2500)
		reg = reg & ~( 1 << shift_value );
		Write_Reg_SCU_DD( 0x40, reg );
#endif
		return( 1 );
	}
	else
	{
		// PASS
#if defined(CONFIG_ASPEED_AST2500)
		reg |= ( 1 << shift_value );
		Write_Reg_SCU_DD( 0x40, reg );
#endif
		return( 0 );
	}
} // End char Finish_Check (MAC_ENGINE *eng, int value)

//------------------------------------------------------------
int FindErr (MAC_ENGINE *eng, int value) {
	eng->flg.Err_Flag = eng->flg.Err_Flag | value;

	if ( DbgPrn_ErrFlg )
		printf("\nErr_Flag: [%08x]\n", eng->flg.Err_Flag);

	return(1);
}

//------------------------------------------------------------
int FindErr_Des (MAC_ENGINE *eng, int value) {
	eng->flg.Err_Flag = eng->flg.Err_Flag | Err_Flag_Check_Des;
	eng->flg.Des_Flag = eng->flg.Des_Flag | value;
	if ( DbgPrn_ErrFlg )
		printf("\nErr_Flag: [%08x] Des_Flag: [%08x]\n", eng->flg.Err_Flag, eng->flg.Des_Flag);

	return(1);
}

//------------------------------------------------------------
// Get and Check status of Interrupt
//------------------------------------------------------------
int check_int (MAC_ENGINE *eng, char *type ) 
{
	nt_log_func_name();

	eng->reg.MAC_000 = Read_Reg_MAC_DD( eng, 0x00 );//Interrupt Status
#ifdef CheckRxbufUNAVA
	if ( eng->reg.MAC_000 & 0x00000004 ) {
		PRINTF( FP_LOG, "[%sIntStatus] Receiving buffer unavailable               : %08x [loop[%d]:%d]\n", type, eng->reg.MAC_000, eng->run.Loop_ofcnt, eng->run.Loop );
		FindErr( eng, Err_Flag_RXBUF_UNAVA );
	}
#endif

#ifdef CheckRPktLost
	if ( eng->reg.MAC_000 & 0x00000008 ) {
		PRINTF( FP_LOG, "[%sIntStatus] Received packet lost due to RX FIFO full   : %08x [loop[%d]:%d]\n", type, eng->reg.MAC_000, eng->run.Loop_ofcnt, eng->run.Loop );
		FindErr( eng, Err_Flag_RPKT_LOST );
	}
#endif

#ifdef CheckNPTxbufUNAVA
	if ( eng->reg.MAC_000 & 0x00000040 ) {
		PRINTF( FP_LOG, "[%sIntStatus] Normal priority transmit buffer unavailable: %08x [loop[%d]:%d]\n", type, eng->reg.MAC_000, eng->run.Loop_ofcnt, eng->run.Loop );
		FindErr( eng, Err_Flag_NPTXBUF_UNAVA );
	}
#endif

#ifdef CheckTPktLost
	if ( eng->reg.MAC_000 & 0x00000080 ) {
		PRINTF( FP_LOG, "[%sIntStatus] Packets transmitted to Ethernet lost       : %08x [loop[%d]:%d]\n", type, eng->reg.MAC_000, eng->run.Loop_ofcnt, eng->run.Loop );
		FindErr( eng, Err_Flag_TPKT_LOST );
	}
#endif

	if ( eng->flg.Err_Flag )
		return(1);
	else
		return(0);
} // End int check_int (MAC_ENGINE *eng, char *type)


//------------------------------------------------------------
// Buffer
//------------------------------------------------------------
void setup_framesize (MAC_ENGINE *eng) 
{
	int32_t       des_num;

	nt_log_func_name();

	//------------------------------
	// Fill Frame Size out descriptor area
	//------------------------------
	if (0) {
		for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
			if ( RAND_SIZE_SIMPLE )
				switch( rand() % 5 ) {
					case 0 : eng->dat.FRAME_LEN[ des_num ] = 0x4e ; break;
					case 1 : eng->dat.FRAME_LEN[ des_num ] = 0x4ba; break;
					default: eng->dat.FRAME_LEN[ des_num ] = 0x5ea; break;
				}
			else
//				eng->dat.FRAME_LEN[ des_num ] = ( rand() + RAND_SIZE_MIN ) % ( RAND_SIZE_MAX + 1 );
				eng->dat.FRAME_LEN[ des_num ] = RAND_SIZE_MIN + ( rand() % ( RAND_SIZE_MAX - RAND_SIZE_MIN + 1 ) );

			if ( DbgPrn_FRAME_LEN )
				PRINTF( FP_LOG, "[setup_framesize] FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]\n", eng->dat.FRAME_LEN[ des_num ], des_num, eng->run.Loop_ofcnt, eng->run.Loop );
		}
	}
	else {
		for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
#ifdef SelectSimpleLength
			if ( des_num % FRAME_SELH_PERD )
				eng->dat.FRAME_LEN[ des_num ] = FRAME_LENH;
			else
				eng->dat.FRAME_LEN[ des_num ] = FRAME_LENL;
#else
			if ( eng->run.TM_Burst ) {
				if ( eng->run.TM_IEEE )
					eng->dat.FRAME_LEN[ des_num ] = 1514;
				else
  #ifdef ENABLE_ARP_2_WOL
					eng->dat.FRAME_LEN[ des_num ] = 164;
  #else
					eng->dat.FRAME_LEN[ des_num ] = 60;
  #endif
			}
			else {
  #ifdef SelectLengthInc
				eng->dat.FRAME_LEN[ des_num ] = 1514 - ( des_num % 1455 );
  #else
				if ( des_num % FRAME_SELH_PERD )
					eng->dat.FRAME_LEN[ des_num ] = FRAME_LENH;
				else
					eng->dat.FRAME_LEN[ des_num ] = FRAME_LENL;
  #endif
			} // End if ( eng->run.TM_Burst )
#endif
			if ( DbgPrn_FRAME_LEN )
				PRINTF( FP_LOG, "[setup_framesize] FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]\n", eng->dat.FRAME_LEN[ des_num ], des_num, eng->run.Loop_ofcnt, eng->run.Loop );

		} // End for (des_num = 0; des_num < eng->dat.Des_Num; des_num++)
	} // End if ( ENABLE_RAND_SIZE )

	// Calculate average of frame size
#ifdef Enable_ShowBW
	eng->dat.Total_frame_len = 0;

	for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ )
		eng->dat.Total_frame_len += eng->dat.FRAME_LEN[ des_num ];
#endif

	//------------------------------
	// Write Plane
	//------------------------------
	switch( ZeroCopy_OFFSET & 0x3 ) {
		case 0: eng->dat.wp_fir = 0xffffffff; break;
		case 1: eng->dat.wp_fir = 0xffffff00; break;
		case 2: eng->dat.wp_fir = 0xffff0000; break;
		case 3: eng->dat.wp_fir = 0xff000000; break;
	}

	for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ )
		switch( ( ZeroCopy_OFFSET + eng->dat.FRAME_LEN[ des_num ] - 1 ) & 0x3 ) {
			case 0: eng->dat.wp_lst[ des_num ] = 0x000000ff; break;
			case 1: eng->dat.wp_lst[ des_num ] = 0x0000ffff; break;
			case 2: eng->dat.wp_lst[ des_num ] = 0x00ffffff; break;
			case 3: eng->dat.wp_lst[ des_num ] = 0xffffffff; break;
		}
} // End void setup_framesize (void)

//------------------------------------------------------------
void setup_arp (MAC_ENGINE *eng) 
{
	int        i;

	nt_log_func_name();
	for (i = 0; i < 16; i++ )
		eng->dat.ARP_data[ i ] = ARP_org_data[ i ];

	eng->dat.ARP_data[ 1 ] = 0x0000ffff
	                       | ( eng->inf.SA[ 0 ] << 16 )//MSB
	                       | ( eng->inf.SA[ 1 ] << 24 );

	eng->dat.ARP_data[ 2 ] = ( eng->inf.SA[ 2 ]       )
	                       | ( eng->inf.SA[ 3 ] <<  8 )
	                       | ( eng->inf.SA[ 4 ] << 16 )
	                       | ( eng->inf.SA[ 5 ] << 24 );//LSB

	eng->dat.ARP_data[ 5 ] = 0x00000100
	                       | ( eng->inf.SA[ 0 ] << 16 )//MSB
	                       | ( eng->inf.SA[ 1 ] << 24 );

	eng->dat.ARP_data[ 6 ] = ( eng->inf.SA[ 2 ]       )
	                       | ( eng->inf.SA[ 3 ] <<  8 )
	                       | ( eng->inf.SA[ 4 ] << 16 )
	                       | ( eng->inf.SA[ 5 ] << 24 );//LSB
} // End void setup_arp (MAC_ENGINE *eng)

//------------------------------------------------------------
void setup_buf (MAC_ENGINE *eng) 
{
	int32_t       des_num_max;
	int32_t       des_num;
	int        i;
	uint32_t      adr;
	uint32_t      adr_srt;
	uint32_t      adr_end;
	uint32_t      Current_framelen;
	uint32_t      gdata = 0;
	int        cnt;
	uint32_t      len;

#ifdef ENABLE_ARP_2_WOL
	int        DA[3];

	DA[ 0 ] =  ( ( SelectWOLDA_DatH >>  8 ) & 0x00ff ) |
	           ( ( SelectWOLDA_DatH <<  8 ) & 0xff00 );

	DA[ 1 ] =  ( ( SelectWOLDA_DatL >> 24 ) & 0x00ff ) |
	           ( ( SelectWOLDA_DatL >>  8 ) & 0xff00 );

	DA[ 2 ] =  ( ( SelectWOLDA_DatL >>  8 ) & 0x00ff ) |
	           ( ( SelectWOLDA_DatL <<  8 ) & 0xff00 );
#endif

	nt_log_func_name();

	// It need be multiple of 4
	eng->dat.DMA_Base_Setup = GET_DMA_BASE_SETUP & 0xfffffffc;
	adr_srt = eng->dat.DMA_Base_Setup;//base for read/write

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
				if ( DbgPrn_BufAdr )
					printf("[loop[%d]:%4d][des:%4d][setup_buf  ] %08x\n", eng->run.Loop_ofcnt, eng->run.Loop, des_num, adr_srt);
#ifdef ENABLE_DASA
				Write_Mem_Dat_DD( adr_srt    , 0xffffffff           );
				Write_Mem_Dat_DD( adr_srt + 4, eng->dat.ARP_data[1] );
				Write_Mem_Dat_DD( adr_srt + 8, eng->dat.ARP_data[2] );

				for ( adr = (adr_srt + 12); adr < (adr_srt + DMA_PakSize); adr += 4 )
#else
				for ( adr =  adr_srt;       adr < (adr_srt + DMA_PakSize); adr += 4 )
#endif
				{
					switch( eng->arg.test_mode ) {
						case 1: gdata = 0xffffffff;              break;
						case 2: gdata = 0x55555555;              break;
						case 3: gdata = rand() | (rand() << 16); break;
						case 5: gdata = eng->arg.GUserDVal;      break;
					}
					Write_Mem_Dat_DD( adr, gdata );
				} // End for()
				adr_srt += DMA_PakSize;
			} // End for (des_num = 0; des_num < eng->dat.Des_Num; des_num++)
		}
		else {
			printf("----->[ARP] 60 bytes\n");
			for (i = 0; i < 16; i++)
				printf("      [Tx%02d] %08x %08x\n", i, eng->dat.ARP_data[i], SWAP_4B( eng->dat.ARP_data[i] ) );

			for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
				if ( DbgPrn_BufAdr )
					printf("[loop[%d]:%4d][des:%4d][setup_buf  ] %08x\n", eng->run.Loop_ofcnt, eng->run.Loop, des_num, adr_srt);

				for (i = 0; i < 16; i++)
					Write_Mem_Dat_DD( adr_srt + ( i << 2 ), eng->dat.ARP_data[i] );

#ifdef ENABLE_ARP_2_WOL
				for (i = 16; i < 40; i += 3) {
					Write_Mem_Dat_DD( adr_srt + ( i << 2 ),     ( DA[ 1 ] << 16 ) |  DA[ 0 ] );
					Write_Mem_Dat_DD( adr_srt + ( i << 2 ) + 4, ( DA[ 0 ] << 16 ) |  DA[ 2 ] );
					Write_Mem_Dat_DD( adr_srt + ( i << 2 ) + 8, ( DA[ 2 ] << 16 ) |  DA[ 1 ] );
				}
#endif
				adr_srt += DMA_PakSize;
			} // End for (des_num = 0; des_num < eng->dat.Des_Num; des_num++)
		} // End if ( eng->run.TM_IEEE )
	}
	else {
		if ( eng->arg.ctrl.b.single_packet )
			des_num_max = 1;
		else
			des_num_max = eng->dat.Des_Num;

		for ( des_num = 0; des_num < des_num_max; des_num++ ) {
			if ( DbgPrn_BufAdr )
				printf("[loop[%d]:%4d][des:%4d][setup_buf  ] %08x\n", eng->run.Loop_ofcnt, eng->run.Loop, des_num, adr_srt);
  #ifdef SelectSimpleData
    #ifdef SimpleData_Fix
			switch( des_num % SimpleData_FixNum ) {
				case  0 : gdata = SimpleData_FixVal00; break;
				case  1 : gdata = SimpleData_FixVal01; break;
				case  2 : gdata = SimpleData_FixVal02; break;
				case  3 : gdata = SimpleData_FixVal03; break;
				case  4 : gdata = SimpleData_FixVal04; break;
				case  5 : gdata = SimpleData_FixVal05; break;
				case  6 : gdata = SimpleData_FixVal06; break;
				case  7 : gdata = SimpleData_FixVal07; break;
				case  8 : gdata = SimpleData_FixVal08; break;
				case  9 : gdata = SimpleData_FixVal09; break;
				case 10 : gdata = SimpleData_FixVal10; break;
				default : gdata = SimpleData_FixVal11; break;
			}
    #else
			gdata   = 0x11111111 * ((des_num + SEED_START) % 256);
    #endif
  #else
			gdata   = DATA_SEED( des_num + SEED_START );
  #endif
			Current_framelen = eng->dat.FRAME_LEN[ des_num ];

			if ( DbgPrn_FRAME_LEN )
				PRINTF( FP_LOG, "[setup_buf      ] Current_framelen:%08x[Des:%d][loop[%d]:%d]\n", Current_framelen, des_num, eng->run.Loop_ofcnt, eng->run.Loop );

			cnt     = 0;
			len     = ( ( ( Current_framelen - 14 ) & 0xff ) << 8) |
			            ( ( Current_framelen - 14 ) >> 8 );
			adr_end = adr_srt + DMA_PakSize;
			for ( adr = adr_srt; adr < adr_end; adr += 4 ) {
  #ifdef SelectSimpleDA
				cnt++;
				if      ( cnt == 1 ) Write_Mem_Dat_DD( adr, SelectSimpleDA_Dat0 );
				else if ( cnt == 2 ) Write_Mem_Dat_DD( adr, SelectSimpleDA_Dat1 );
				else if ( cnt == 3 ) Write_Mem_Dat_DD( adr, SelectSimpleDA_Dat2 );
				else if ( cnt == 4 ) Write_Mem_Dat_DD( adr, len | (len << 16)   );
				else
  #endif
				                     Write_Mem_Dat_DD( adr, gdata );
  #ifdef SelectSimpleData
				gdata = gdata ^ SimpleData_XORVal;
  #else
				gdata += DATA_IncVal;
  #endif
			}
			adr_srt += DMA_PakSize;
		} // End for (des_num = 0; des_num < eng->dat.Des_Num; des_num++)
	} // End if ( eng->run.TM_Burst )
} // End void setup_buf (MAC_ENGINE *eng)

//------------------------------------------------------------
// Check data of one packet
//------------------------------------------------------------
char check_Data (MAC_ENGINE *eng, uint32_t datbase, int32_t number) 
{
	int32_t       number_dat;
	int        index;
	uint32_t      rdata;
	uint32_t      wp_lst_cur;
	uint32_t      adr_las;
	uint32_t      adr;
	uint32_t      adr_srt;
	uint32_t      adr_end;
#ifdef SelectSimpleDA
	int        cnt;
	uint32_t      len;
	uint32_t      gdata_bak;
#endif
	uint32_t      gdata;

	uint32_t      wp;

	nt_log_func_name();

	if ( eng->arg.ctrl.b.single_packet )
		number_dat = 0;
	else
		number_dat = number;

	wp_lst_cur             = eng->dat.wp_lst[ number ];
	eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[ number_dat ];

	if ( DbgPrn_FRAME_LEN )
		PRINTF( FP_LOG, "[check_Data     ] FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]\n", eng->dat.FRAME_LEN_Cur, number, eng->run.Loop_ofcnt, eng->run.Loop );

	adr_srt = datbase;
	adr_end = adr_srt + PktByteSize;

#if defined(SelectSimpleData)
    #ifdef SimpleData_Fix
	switch( number_dat % SimpleData_FixNum ) {
		case  0 : gdata = SimpleData_FixVal00; break;
		case  1 : gdata = SimpleData_FixVal01; break;
		case  2 : gdata = SimpleData_FixVal02; break;
		case  3 : gdata = SimpleData_FixVal03; break;
		case  4 : gdata = SimpleData_FixVal04; break;
		case  5 : gdata = SimpleData_FixVal05; break;
		case  6 : gdata = SimpleData_FixVal06; break;
		case  7 : gdata = SimpleData_FixVal07; break;
		case  8 : gdata = SimpleData_FixVal08; break;
		case  9 : gdata = SimpleData_FixVal09; break;
		case 10 : gdata = SimpleData_FixVal10; break;
		default : gdata = SimpleData_FixVal11; break;
	}
    #else
	gdata   = 0x11111111 * (( number_dat + SEED_START ) % 256 );
    #endif
#else
	gdata   = DATA_SEED( number_dat + SEED_START );
#endif

//printf("check_buf: %08x - %08x [%08x]\n", adr_srt, adr_end, datbase);
	wp      = eng->dat.wp_fir;
	adr_las = adr_end - 4;
#ifdef SelectSimpleDA
	cnt     = 0;
	len     = ((( eng->dat.FRAME_LEN_Cur-14 ) & 0xff ) << 8 ) |
	          ( ( eng->dat.FRAME_LEN_Cur-14 )          >> 8 );
#endif

	if ( DbgPrn_Bufdat )
		PRINTF( FP_LOG, " Inf:%08x ~ %08x(%08x) %08x [Des:%d][loop[%d]:%d]\n", adr_srt, adr_end, adr_las, gdata, number, eng->run.Loop_ofcnt, eng->run.Loop );

	for ( adr = adr_srt; adr < adr_end; adr+=4 ) {
#ifdef SelectSimpleDA
		cnt++;
		if      ( cnt == 1 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat0; }
		else if ( cnt == 2 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat1; }
		else if ( cnt == 3 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat2; }
		else if ( cnt == 4 ) { gdata_bak = gdata; gdata = len | (len << 16);   }
#endif
		rdata = Read_Mem_Dat_DD( adr );
		if ( adr == adr_las )
			wp = wp & wp_lst_cur;

		if ( ( rdata & wp ) != ( gdata & wp ) ) {
			PRINTF( FP_LOG, "\nError: Adr:%08x[%3d] (%08x) (%08x:%08x) [Des:%d][loop[%d]:%d]\n", adr, ( adr - adr_srt ) / 4, rdata, gdata, wp, number, eng->run.Loop_ofcnt, eng->run.Loop );
			for ( index = 0; index < 6; index++ )
				PRINTF( FP_LOG, "Rep  : Adr:%08x      (%08x) (%08x:%08x) [Des:%d][loop[%d]:%d]\n", adr, Read_Mem_Dat_DD( adr ), gdata, wp, number, eng->run.Loop_ofcnt, eng->run.Loop );

			if ( DbgPrn_DumpMACCnt )
				dump_mac_ROreg( eng );

			return( FindErr( eng, Err_Flag_Check_Buf_Data ) );
		} // End if ( (rdata & wp) != (gdata & wp) )
		if ( DbgPrn_BufdatDetail )
			PRINTF( FP_LOG, " Adr:%08x[%3d] (%08x) (%08x:%08x) [Des:%d][loop[%d]:%d]\n", adr, ( adr - adr_srt ) / 4, rdata, gdata, wp, number, eng->run.Loop_ofcnt, eng->run.Loop );

#ifdef SelectSimpleDA
		if ( cnt <= 4 )
			gdata = gdata_bak;
#endif

#if defined(SelectSimpleData)
		gdata = gdata ^ SimpleData_XORVal;
#else
		gdata += DATA_IncVal;
#endif

		wp     = 0xffffffff;
	}
	return(0);
} // End char check_Data (MAC_ENGINE *eng, uint32_t datbase, int32_t number)

//------------------------------------------------------------
char check_buf (MAC_ENGINE *eng, int loopcnt) {
	int32_t       des_num;
	uint32_t      desadr;
	uint32_t      datbase;

	nt_log_func_name();

	desadr = eng->run.RDES_BASE + ( 16 * eng->dat.Des_Num ) - 4;//base for read/write
	for ( des_num = eng->dat.Des_Num - 1; des_num >= 0; des_num-- ) {
		datbase = AT_BUF_MEMRW( Read_Mem_Des_DD( desadr ) & 0xfffffffc);//base for read/write
		if ( check_Data( eng, datbase, des_num ) ) {
			check_int ( eng, "" );
			return(1);
		}
		desadr-= 16;
	}
	if ( check_int ( eng, "" ) )
		return(1);

#if defined(Delay_CheckData_LoopNum) && defined(Delay_CheckData)
	if ( ( loopcnt % Delay_CheckData_LoopNum ) == 0 ) DELAY( Delay_CheckData );
#endif
	return(0);
} // End char check_buf (MAC_ENGINE *eng, int loopcnt)

//------------------------------------------------------------
// Descriptor
//------------------------------------------------------------
void setup_txdes (MAC_ENGINE *eng, uint32_t desadr, uint32_t bufbase) 
{
	uint32_t      bufadr;
	uint32_t      bufadrgap;
	uint32_t      desval = 0;
	int32_t       des_num;

	nt_log_func_name();

	bufadr = bufbase;
	if ( eng->arg.ctrl.b.single_packet )
		bufadrgap = 0;
	else
		bufadrgap = DMA_PakSize;

	if ( eng->run.TM_TxDataEn ) {
		for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
			eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[ des_num ];
			desval                 = TDES_IniVal;
			Write_Mem_Des_DD( desadr + 0x04, 0      );
			Write_Mem_Des_DD( desadr + 0x08, 0      );
			Write_Mem_Des_DD( desadr + 0x0C, bufadr );
			Write_Mem_Des_DD( desadr       , desval );

			if ( DbgPrn_FRAME_LEN )
				PRINTF( FP_LOG, "[setup_txdes    ] FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]\n", eng->dat.FRAME_LEN_Cur, des_num, eng->run.Loop_ofcnt, eng->run.Loop );

			if ( DbgPrn_BufAdr )
				printf("[loop[%d]:%4d][des:%4d][setup_txdes] %08x [%08x]\n", eng->run.Loop_ofcnt, eng->run.Loop, des_num, desadr, bufadr);

			desadr += 16;
			bufadr += bufadrgap;
		}
		Write_Mem_Des_DD( desadr - 0x10, desval | EOR_IniVal );
	}
	else {
		Write_Mem_Des_DD( desadr, 0);
	} // End if ( eng->run.TM_TxDataEn )
} // End void setup_txdes (uint32_t desadr, uint32_t bufbase)

//------------------------------------------------------------
void setup_rxdes (MAC_ENGINE *eng, uint32_t desadr, uint32_t bufbase) {
	uint32_t      bufadr;
	uint32_t      desval;
	int32_t       des_num;

	nt_log_func_name();

	bufadr = bufbase;
	desval = RDES_IniVal;
	if ( eng->run.TM_RxDataEn ) {
		for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
			Write_Mem_Des_DD( desadr + 0x04, 0      );
			Write_Mem_Des_DD( desadr + 0x08, 0      );
			Write_Mem_Des_DD( desadr + 0x0C, bufadr );
			Write_Mem_Des_DD( desadr       , desval );

			if ( DbgPrn_BufAdr )
				printf("[loop[%d]:%4d][des:%4d][setup_rxdes] %08x [%08x]\n", eng->run.Loop_ofcnt, eng->run.Loop, des_num, desadr, bufadr);

			desadr += 16;
			bufadr += DMA_PakSize;
		}
		Write_Mem_Des_DD( desadr - 0x10, desval | EOR_IniVal );
	}
	else {
		Write_Mem_Des_DD( desadr, 0x80000000 );
	} // End if ( eng->run.TM_RxDataEn )
} // End void setup_rxdes (uint32_t desadr, uint32_t bufbase)

//------------------------------------------------------------
// First setting TX and RX information
//------------------------------------------------------------
void setup_des (MAC_ENGINE *eng, uint32_t bufnum) 
{

	if ( DbgPrn_BufAdr ) {
		printf("setup_des: %d\n", bufnum);
		debug_pause();
	}

//	eng->dat.DMA_Base_Tx = CPU_BUS_ADDR_SDRAM_OFFSET + ZeroCopy_OFFSET + eng->dat.DMA_Base_Setup; // 20130730
//	eng->dat.DMA_Base_Rx = CPU_BUS_ADDR_SDRAM_OFFSET + ZeroCopy_OFFSET + GET_DMA_BASE(0); // 20130730
	eng->dat.DMA_Base_Tx = ZeroCopy_OFFSET + eng->dat.DMA_Base_Setup; // 20130730
	eng->dat.DMA_Base_Rx = ZeroCopy_OFFSET + GET_DMA_BASE(0); // 20130730
	setup_txdes( eng, eng->run.TDES_BASE, AT_MEMRW_BUF( eng->dat.DMA_Base_Tx ) );//base for read/write //base of the descriptor
	setup_rxdes( eng, eng->run.RDES_BASE, AT_MEMRW_BUF( eng->dat.DMA_Base_Rx ) );//base for read/write //base of the descriptor
} // End void setup_des (uint32_t bufnum)

//------------------------------------------------------------
// Move buffer point of TX and RX descriptor to next DMA buffer
//------------------------------------------------------------
void setup_des_loop (MAC_ENGINE *eng, uint32_t bufnum) 
{
	int32_t des_num;
	uint32_t H_rx_desadr;
	uint32_t H_tx_desadr;
	uint32_t H_tx_bufadr;
	uint32_t H_rx_bufadr;

	nt_log_func_name();

	if ( eng->run.TM_RxDataEn ) {
		H_rx_bufadr = AT_MEMRW_BUF( eng->dat.DMA_Base_Rx );//base of the descriptor
		H_rx_desadr = eng->run.RDES_BASE;//base for read/write
		for ( des_num = 0; des_num < eng->dat.Des_Num - 1; des_num++ ) {
			Write_Mem_Des_DD( H_rx_desadr + 0x0C, H_rx_bufadr );
			Write_Mem_Des_DD( H_rx_desadr       , RDES_IniVal );
			if ( DbgPrn_BufAdr )
				printf("[loop[%d]:%4d][des:%4d][setup_rxdes] %08x [%08x]\n", eng->run.Loop_ofcnt, eng->run.Loop, des_num, H_rx_desadr, H_rx_bufadr);

			H_rx_bufadr += DMA_PakSize;
			H_rx_desadr += 16;
		}
		Write_Mem_Des_DD( H_rx_desadr + 0x0C, H_rx_bufadr );
		Write_Mem_Des_DD( H_rx_desadr       , RDES_IniVal | EOR_IniVal );
		if ( DbgPrn_BufAdr )
			printf("[loop[%d]:%4d][des:%4d][setup_rxdes] %08x [%08x]\n", eng->run.Loop_ofcnt, eng->run.Loop, des_num, H_rx_desadr, H_rx_bufadr);
	}
//	Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 ); // Rx Poll

	if ( eng->run.TM_TxDataEn ) {
		H_tx_bufadr = AT_MEMRW_BUF( eng->dat.DMA_Base_Tx );//base of the descriptor
		H_tx_desadr = eng->run.TDES_BASE;//base for read/write
		for ( des_num = 0; des_num < eng->dat.Des_Num - 1; des_num++ ) {
			eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[ des_num ];
			Write_Mem_Des_DD( H_tx_desadr + 0x0C, H_tx_bufadr );
			Write_Mem_Des_DD( H_tx_desadr       , TDES_IniVal );
			if ( DbgPrn_BufAdr )
				printf("[loop[%d]:%4d][des:%4d][setup_txdes] %08x [%08x]\n", eng->run.Loop_ofcnt, eng->run.Loop, des_num, H_tx_desadr, H_tx_bufadr);

			H_tx_bufadr += DMA_PakSize;
			H_tx_desadr += 16;
		}
		eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[ des_num ];
		Write_Mem_Des_DD( H_tx_desadr + 0x0C, H_tx_bufadr );
		Write_Mem_Des_DD( H_tx_desadr       , TDES_IniVal | EOR_IniVal );
		if ( DbgPrn_BufAdr )
			printf("[loop[%d]:%4d][des:%4d][setup_txdes] %08x [%08x]\n", eng->run.Loop_ofcnt, eng->run.Loop, des_num, H_tx_desadr, H_tx_bufadr);
	}
//	Write_Reg_MAC_DD( eng, 0x18, 0x00000000 ); // Tx Poll
} // End void setup_des_loop (uint32_t bufnum)

//------------------------------------------------------------
char check_des_header_Tx (MAC_ENGINE *eng, char *type, uint32_t adr, int32_t desnum) {
	int        timeout = 0;

	eng->dat.TxDes0DW = Read_Mem_Des_DD( adr );

	while ( HWOwnTx( eng->dat.TxDes0DW ) ) {
		// we will run again, if transfer has not been completed.
		if ( ( eng->run.TM_Burst || eng->run.TM_RxDataEn ) && ( ++timeout > eng->run.TIME_OUT_Des ) ) {
			PRINTF( FP_LOG, "[%sTxDesOwn] Address %08x = %08x [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.TxDes0DW, desnum, eng->run.Loop_ofcnt, eng->run.Loop );
			return( FindErr_Des( eng, Des_Flag_TxOwnTimeOut ) );
		}
//		Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 );//Rx Poll
//		Write_Reg_MAC_DD( eng, 0x18, 0x00000000 );//Tx Poll

#ifdef Delay_ChkTxOwn
		DELAY( Delay_ChkTxOwn );
#endif
		eng->dat.TxDes0DW = Read_Mem_Des_DD( adr );
	}

	return(0);
} // End char check_des_header_Tx (MAC_ENGINE *eng, char *type, uint32_t adr, int32_t desnum)

//------------------------------------------------------------
char check_des_header_Rx (MAC_ENGINE *eng, char *type, uint32_t adr, int32_t desnum) {
#ifdef CheckRxOwn
	int        timeout = 0;

	eng->dat.RxDes0DW = Read_Mem_Des_DD( adr );

	while ( HWOwnRx( eng->dat.RxDes0DW ) ) {
		// we will run again, if transfer has not been completed.
		if ( eng->run.TM_TxDataEn && ( ++timeout > eng->run.TIME_OUT_Des ) ) {
			PRINTF( FP_LOG, "[%sRxDesOwn] Address %08x = %08x [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, desnum, eng->run.Loop_ofcnt, eng->run.Loop );
			return( FindErr_Des( eng, Des_Flag_RxOwnTimeOut ) );
		}
//		Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 );//Rx Poll
//		Write_Reg_MAC_DD( eng, 0x18, 0x00000000 );//Tx Poll

  #ifdef Delay_ChkRxOwn
		DELAY( Delay_ChkRxOwn );
  #endif
		eng->dat.RxDes0DW = Read_Mem_Des_DD( adr );
	};


  #ifdef CheckRxLen
	if ( DbgPrn_FRAME_LEN )
		PRINTF( FP_LOG, "[%sRxDes          ] FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]\n", type, ( eng->dat.FRAME_LEN_Cur + 4 ), desnum, eng->run.Loop_ofcnt, eng->run.Loop );

	if ( ( eng->dat.RxDes0DW & 0x3fff ) != ( eng->dat.FRAME_LEN_Cur + 4 ) ) {
		eng->dat.RxDes3DW = Read_Mem_Des_DD( adr + 12 );
		PRINTF( FP_LOG, "[%sRxDes] Error Frame Length %08x:%08x %08x(%4d/%4d) [Des:%d][loop[%d]:%d]\n",   type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, ( eng->dat.RxDes0DW & 0x3fff ), ( eng->dat.FRAME_LEN_Cur + 4 ), desnum, eng->run.Loop_ofcnt, eng->run.Loop );
		FindErr_Des( eng, Des_Flag_FrameLen );
	}
  #endif // End CheckRxLen

	if ( eng->dat.RxDes0DW & Check_ErrMask_ALL ) {
		eng->dat.RxDes3DW = Read_Mem_Des_DD( adr + 12 );
  #ifdef CheckRxErr
		if ( eng->dat.RxDes0DW & Check_ErrMask_RxErr ) {
			PRINTF( FP_LOG, "[%sRxDes] Error RxErr        %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop_ofcnt, eng->run.Loop );
			FindErr_Des( eng, Des_Flag_RxErr );
		}
  #endif // End CheckRxErr

  #ifdef CheckCRC
		if ( eng->dat.RxDes0DW & Check_ErrMask_CRC ) {
			PRINTF( FP_LOG, "[%sRxDes] Error CRC          %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop_ofcnt, eng->run.Loop );
			FindErr_Des( eng, Des_Flag_CRC );
		}
  #endif // End CheckCRC

  #ifdef CheckFTL
		if ( eng->dat.RxDes0DW & Check_ErrMask_FTL ) {
			PRINTF( FP_LOG, "[%sRxDes] Error FTL          %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop_ofcnt, eng->run.Loop );
			FindErr_Des( eng, Des_Flag_FTL );
		}
  #endif // End CheckFTL

  #ifdef CheckRunt
		if ( eng->dat.RxDes0DW & Check_ErrMask_Runt) {
			PRINTF( FP_LOG, "[%sRxDes] Error Runt         %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop_ofcnt, eng->run.Loop );
			FindErr_Des( eng, Des_Flag_Runt );
		}
  #endif // End CheckRunt

  #ifdef CheckOddNibble
		if ( eng->dat.RxDes0DW & Check_ErrMask_OddNibble ) {
			PRINTF( FP_LOG, "[%sRxDes] Odd Nibble         %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop_ofcnt, eng->run.Loop );
			FindErr_Des( eng, Des_Flag_OddNibble );
		}
  #endif // End CheckOddNibble

  #ifdef CheckRxFIFOFull
		if ( eng->dat.RxDes0DW & Check_ErrMask_RxFIFOFull ) {
			PRINTF( FP_LOG, "[%sRxDes] Error Rx FIFO Full %08x:%08x %08x            [Des:%d][loop[%d]:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop_ofcnt, eng->run.Loop );
			FindErr_Des( eng, Des_Flag_RxFIFOFull );
		}
  #endif // End CheckRxFIFOFull
	}

#endif // End CheckRxOwn

	if ( eng->flg.Err_Flag )
		return(1);
	else
		return(0);
} // End char check_des_header_Rx (MAC_ENGINE *eng, char *type, uint32_t adr, int32_t desnum)

//------------------------------------------------------------
char check_des (MAC_ENGINE *eng, uint32_t bufnum, int checkpoint) {
	int32_t       desnum;
	CHAR       desnum_last;
	uint32_t      H_rx_desadr;
	uint32_t      H_tx_desadr;
	uint32_t      H_tx_bufadr;
	uint32_t      H_rx_bufadr;
#ifdef Delay_DesGap
	uint32_t      dly_cnt = 0;
	uint32_t      dly_max = Delay_CntMaxIncVal;
#endif

	nt_log_func_name();

	// Fire the engine to send and recvice
	Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 );//Rx Poll
	Write_Reg_MAC_DD( eng, 0x18, 0x00000000 );//Tx Poll

#ifndef SelectSimpleDes
	H_tx_bufadr = AT_MEMRW_BUF( eng->dat.DMA_Base_Tx );//base of the descriptor
	H_rx_bufadr = AT_MEMRW_BUF( eng->dat.DMA_Base_Rx );//base of the descriptor
#endif
	H_rx_desadr = eng->run.RDES_BASE;//base for read/write
	H_tx_desadr = eng->run.TDES_BASE;//base for read/write

#ifdef Delay_DES
	DELAY( Delay_DES );
#endif

	for ( desnum = 0; desnum < eng->dat.Des_Num; desnum++ ) {
		desnum_last = ( desnum == ( eng->dat.Des_Num - 1 ) ) ? 1 : 0;
		if ( DbgPrn_BufAdr ) {
			if ( checkpoint )
				printf("[loop[%d]:%4d][des:%4d][check_des  ] %08x %08x [%08x %08x]\n", eng->run.Loop_ofcnt, eng->run.Loop, desnum, ( H_tx_desadr ), ( H_rx_desadr ), Read_Mem_Des_DD( H_tx_desadr + 12 ), Read_Mem_Des_DD( H_rx_desadr + 12 ) );
			else
				printf("[loop[%d]:%4d][des:%4d][check_des  ] %08x %08x [%08x %08x]->[%08x %08x]\n", eng->run.Loop_ofcnt, eng->run.Loop, desnum, ( H_tx_desadr ), ( H_rx_desadr ), Read_Mem_Des_DD( H_tx_desadr + 12 ), Read_Mem_Des_DD( H_rx_desadr + 12 ), H_tx_bufadr, H_rx_bufadr );
		}

		//[Delay]--------------------
#ifdef Delay_DesGap
//		if ( dly_cnt++ > 3 ) {
		if ( dly_cnt > Delay_CntMax ) {
//			switch ( rand() % 12 ) {
//				case 1 : dly_max = 00000; break;
//				case 3 : dly_max = 20000; break;
//				case 5 : dly_max = 40000; break;
//				case 7 : dly_max = 60000; break;
//				defaule: dly_max = 70000; break;
//			}
//
//			dly_max += ( rand() % 4 ) * 14321;
//
//			while (dly_cnt < dly_max) {
//				dly_cnt++;
//			}
			DELAY( Delay_DesGap );
			dly_cnt = 0;
		}
		else {
			dly_cnt++;
//			timeout = 0;
//			while (timeout < 50000) {timeout++;};
		}
#endif // End Delay_DesGap

		//[Check Owner Bit]--------------------
		eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[ desnum ];
		if ( DbgPrn_FRAME_LEN )
			PRINTF( FP_LOG, "[check_des      ] FRAME_LEN_Cur:%08x[Des:%d][loop[%d]:%d]%d\n", eng->dat.FRAME_LEN_Cur, desnum, eng->run.Loop_ofcnt, eng->run.Loop, checkpoint );

		// Check the description of Tx and Rx
		if ( eng->run.TM_TxDataEn && check_des_header_Tx( eng, "", H_tx_desadr, desnum ) ) {
			eng->flg.CheckDesFail_DesNum = desnum;

			return(1);
		}
		if ( eng->run.TM_RxDataEn && check_des_header_Rx( eng, "", H_rx_desadr, desnum ) ) {
			eng->flg.CheckDesFail_DesNum = desnum;

			return(1);
		}

#ifndef SelectSimpleDes
		if ( !checkpoint ) {
			// Setting buffer address to description of Tx and Rx on next stage
			if ( eng->run.TM_RxDataEn ) {
				Write_Mem_Des_DD( H_rx_desadr + 0x0C, H_rx_bufadr );
				if ( desnum_last )
					Write_Mem_Des_DD( H_rx_desadr, RDES_IniVal | EOR_IniVal );
				else
					Write_Mem_Des_DD( H_rx_desadr, RDES_IniVal );
				Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 ); //Rx Poll
				H_rx_bufadr += DMA_PakSize;
			}
			if ( eng->run.TM_TxDataEn ) {
				Write_Mem_Des_DD( H_tx_desadr + 0x0C, H_tx_bufadr );
				if ( desnum_last )
					Write_Mem_Des_DD( H_tx_desadr, TDES_IniVal | EOR_IniVal );
				else
					Write_Mem_Des_DD( H_tx_desadr, TDES_IniVal );
				Write_Reg_MAC_DD( eng, 0x18, 0x00000000 ); //Tx Poll
				H_tx_bufadr += DMA_PakSize;
			}
		}
#endif // End SelectSimpleDes

		H_rx_desadr += 16;
		H_tx_desadr += 16;
	} // End for (desnum = 0; desnum < eng->dat.Des_Num; desnum++)

	return(0);
} // End char check_des (MAC_ENGINE *eng, uint32_t bufnum, int checkpoint)
//#endif

//------------------------------------------------------------
// Print
//-----------------------------------------------------------
void PrintIO_Header (MAC_ENGINE *eng, BYTE option) {

	if ( eng->run.TM_IOStrength ) {
		if ( eng->io.Str_max > 1 )
			{ PRINTF( option, "<IO Strength: SCU%02x[%2d:%2d]=%2d>", eng->io.Str_reg_idx, eng->io.Str_shf + 1, eng->io.Str_shf, eng->io.Str_i ); }
		else
			{ PRINTF( option, "<IO Strength: SCU%02x[%2d]=%2d>", eng->io.Str_reg_idx, eng->io.Str_shf, eng->io.Str_i ); }
	}

	if      ( eng->run.Speed_sel[ 0 ] ) { PRINTF( option, "[1G  ]========================================>\n" ); }
	else if ( eng->run.Speed_sel[ 1 ] ) { PRINTF( option, "[100M]========================================>\n" ); }
	else                                { PRINTF( option, "[10M ]========================================>\n" ); }

	if ( !(option == FP_LOG) ) {
		PRINTF( option, "   SCU%2X      ", eng->io.Dly_reg_idx );

		for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval ) {
			eng->io.Dly_in_selval = eng->io.value_ary[ eng->io.Dly_in ];
			PRINTF( option, "%1x", ( eng->io.Dly_in_selval >> 4 ) );
		}

		PRINTF( option, "\n%s    ", eng->io.Dly_reg_name_rx_new );
		for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval ) {
			eng->io.Dly_in_selval = eng->io.value_ary[ eng->io.Dly_in ];
			PRINTF( option, "%1x", eng->io.Dly_in_selval & 0xf );
		}

		PRINTF( option, "\n              " );
		for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval ) {
			if ( eng->io.Dly_in_reg_idx == eng->io.Dly_in ) { PRINTF( option, "|" ); }
			else                                            { PRINTF( option, " " ); }
		}
		PRINTF( option, "\n");
	} // End if ( !(option == FP_LOG) )
} // End void PrintIO_Header (MAC_ENGINE *eng, BYTE option)

//------------------------------------------------------------
void PrintIO_LineS (MAC_ENGINE *eng, BYTE option) {

	if ( eng->io.Dly_out_reg_hit )
		{ PRINTF( option, "%s%02x:-", eng->io.Dly_reg_name_tx_new, eng->io.Dly_out_selval ); }
	else
		{ PRINTF( option, "%s%02x: ", eng->io.Dly_reg_name_tx_new, eng->io.Dly_out_selval ); }
} // End void PrintIO_LineS (MAC_ENGINE *eng, BYTE option)

//------------------------------------------------------------
void PrintIO_Line (MAC_ENGINE *eng, BYTE option) {

	if ( ( eng->io.Dly_in_reg == eng->io.Dly_in_selval ) && eng->io.Dly_out_reg_hit ) {
		if ( eng->io.Dly_result ) { PRINTF( option, "X" ); }
		else                      { PRINTF( option, "O" ); }
	}
	else {
		if ( eng->io.Dly_result ) { PRINTF( option, "x" ); }
		else                      { PRINTF( option, "o" ); }
	}
} // End void PrintIO_Line (MAC_ENGINE *eng, BYTE option)

//------------------------------------------------------------
void PrintIO_Line_LOG (MAC_ENGINE *eng) {

	if ( eng->io.Dly_result ) {
		PRINTF( FP_LOG, "\n=====>[Check]%s%2x, %s%2x:  X\n", eng->io.Dly_reg_name_rx, eng->io.Dly_in_selval, eng->io.Dly_reg_name_tx, eng->io.Dly_out_selval );
	}
	else {
		PRINTF( FP_LOG, "\n=====>[Check]%s%2x, %s%2x:  O\n", eng->io.Dly_reg_name_rx, eng->io.Dly_in_selval, eng->io.Dly_reg_name_tx, eng->io.Dly_out_selval );
	}
}

//------------------------------------------------------------
// main
//------------------------------------------------------------
void Calculate_LOOP_CheckNum (MAC_ENGINE *eng) 
{
	nt_log_func_name();

#define ONE_MBYTE    1048576

#ifdef CheckDataEveryTime
	eng->run.LOOP_CheckNum = 1;
#else
	if ( eng->run.IO_MrgChk || ( eng->arg.run_speed == SET_1G_100M_10MBPS ) || ( eng->arg.run_speed == SET_100M_10MBPS ) ) {
		eng->run.LOOP_CheckNum = eng->run.LOOP_MAX;
	}
	else {
		switch ( eng->arg.run_speed ) {
			case SET_1GBPS    : eng->run.CheckBuf_MBSize =  MOVE_DATA_MB_SEC      ; break; // 1G
			case SET_100MBPS  : eng->run.CheckBuf_MBSize = (MOVE_DATA_MB_SEC >> 3); break; // 100M ~ 1G / 8
			case SET_10MBPS   : eng->run.CheckBuf_MBSize = (MOVE_DATA_MB_SEC >> 6); break; // 10M  ~ 1G / 64
		}
		eng->run.LOOP_CheckNum = ( eng->run.CheckBuf_MBSize / ( ((eng->dat.Des_Num * DMA_PakSize) / ONE_MBYTE ) + 1) );
	}
#endif
}

//------------------------------------------------------------
void TestingSetup (MAC_ENGINE *eng) 
{
	nt_log_func_name();


	//[Disable VGA]--------------------
#ifdef Disable_VGA
//	if ( eng->arg.loop_inf & ~( eng->run.TM_Burst || eng->run.TM_IOTiming ) ) {
		eng->env.VGAModeVld = 1;
		outp(0x3d4, 0x17);
		eng->env.VGAMode = inp(0x3d5);
		outp(0x3d4, 0x17);
		outp(0x3d5, 0);
//	}
#endif

	//[Setup]--------------------
	setup_framesize( eng );
	setup_buf( eng );
}

//------------------------------------------------------------
// Return 1 ==> fail
// Return 0 ==> PASS
//------------------------------------------------------------
char TestingLoop (MAC_ENGINE *eng, uint32_t loop_checknum) 
{
	char       checkprd;
	char       looplast;
	char       checken;

	nt_log_func_name();


	if ( DbgPrn_DumpMACCnt )
		dump_mac_ROreg( eng );

	//[Setup]--------------------
	eng->run.Loop = 0;
	checkprd = 0;
	checken  = 0;
	looplast = 0;


	setup_des( eng, 0 );

	if ( eng->run.TM_WaitStart ) {
		printf("Press any key to start...\n");
		GET_CAHR();
	}


	while ( ( eng->run.Loop < eng->run.LOOP_MAX ) || eng->arg.loop_inf ) {
		looplast = !eng->arg.loop_inf && ( eng->run.Loop == eng->run.LOOP_MAX - 1 );

#ifdef CheckRxBuf
		if ( !eng->run.TM_Burst )
			checkprd = ( ( eng->run.Loop % loop_checknum ) == ( loop_checknum - 1 ) );
		checken = looplast | checkprd;
#endif



		if (DbgPrn_BufAdr) {
			printf("for start ======> [%d]%d/%d(%d) looplast:%d "
			       "checkprd:%d checken:%d\n",
			       eng->run.Loop_ofcnt, eng->run.Loop,
			       eng->run.LOOP_MAX, eng->arg.loop_inf,
			       looplast, checkprd, checken);
			debug_pause();
		}


		if ( eng->run.TM_RxDataEn )
			eng->dat.DMA_Base_Tx = eng->dat.DMA_Base_Rx;

//		eng->dat.DMA_Base_Rx = CPU_BUS_ADDR_SDRAM_OFFSET + ZeroCopy_OFFSET + GET_DMA_BASE( eng->run.Loop + 1 ); // 20130730
		eng->dat.DMA_Base_Rx = ZeroCopy_OFFSET + GET_DMA_BASE( eng->run.Loop + 1 ); // 20130730
		//[Check DES]--------------------
		if ( check_des( eng, eng->run.Loop, checken ) ) {
			//descriptor error
#ifdef CheckRxBuf
			eng->dat.Des_Num = eng->flg.CheckDesFail_DesNum + 1;
			if ( checkprd )
				check_buf( eng, loop_checknum );
			else
				check_buf( eng, ( eng->run.LOOP_MAX % loop_checknum ) );
			eng->dat.Des_Num = eng->dat.Des_Num_Org;
#endif

			if ( DbgPrn_DumpMACCnt )
				dump_mac_ROreg ( eng );

			return(1);
		}

		//[Check Buf]--------------------
		if ( eng->run.TM_RxDataEn && checken ) {
			if ( checkprd ) {
#ifdef Enable_ShowBW  
				printf("[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", loop_checknum, ((double)loop_checknum * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused);
				PRINTF( FP_LOG, "[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", loop_checknum, ((double)loop_checknum * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused );  
#endif

#ifdef CheckRxBuf
				if ( check_buf( eng, loop_checknum ) )
					return(1);
#endif
			}
			else {
#ifdef Enable_ShowBW  
				printf("[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", (eng->run.LOOP_MAX % loop_checknum), ((double)(eng->run.LOOP_MAX % loop_checknum) * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused);
				PRINTF( FP_LOG, "[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", (eng->run.LOOP_MAX % loop_checknum), ((double)(eng->run.LOOP_MAX % loop_checknum) * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused );  
#endif

#ifdef CheckRxBuf
				if ( check_buf( eng, ( eng->run.LOOP_MAX % loop_checknum ) ) )
					return(1);
#endif
			} // End if ( checkprd )

#ifndef SelectSimpleDes
			if ( !looplast )
				setup_des_loop( eng, eng->run.Loop );
#endif

#ifdef Enable_ShowBW
			timeold = clock();
#endif
		} // End if ( eng->run.TM_RxDataEn && checken )

#ifdef SelectSimpleDes
		if ( !looplast )
			setup_des_loop( eng, eng->run.Loop );
#endif

		if ( eng->arg.loop_inf )
			printf("===============> Loop[%d]: %d  \r", eng->run.Loop_ofcnt, eng->run.Loop);
		else if ( eng->arg.test_mode == 0 ) {
			if ( !( DbgPrn_BufAdr || eng->run.IO_Bund ) )
				printf(" [%d]%d                        \r", eng->run.Loop_ofcnt, eng->run.Loop);
		}

		if (DbgPrn_BufAdr) {
			printf("for end   ======> [%d]%d/%d(%d)\n",
			       eng->run.Loop_ofcnt, eng->run.Loop,
			       eng->run.LOOP_MAX, eng->arg.loop_inf);
			debug_pause();
		}

		if ( eng->run.Loop >= Loop_OverFlow ) {
			printf("Over-flow\n");
			eng->run.Loop = 0;
			eng->run.Loop_ofcnt++;
		}
		else
			eng->run.Loop++;
	} // End while ( ( eng->run.Loop < eng->run.LOOP_MAX ) || eng->arg.loop_inf )
	eng->run.Loop_rl[ (int)eng->run.Speed_idx ] = eng->run.Loop;

	eng->flg.AllFail = 0;
	return(0);
} // End char TestingLoop (MAC_ENGINE *eng, uint32_t loop_checknum)
