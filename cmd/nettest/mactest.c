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
#include "io.h"
#include "stduboot.h"
#include <command.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <post.h>

int mac_test(int argc, char * const argv[], char mode)
{
	MAC_ENGINE           MACENG;
	MAC_ENGINE           *eng;
	PHY_ENGINE           PHYENG;
	PHY_ENGINE           *phyeng;
	int                  DES_LowNumber;

	CHAR                 *stop_at;
	uint32_t                Wrn_Flag_allapeed;
	uint32_t                Err_Flag_allapeed;
	uint32_t                Des_Flag_allapeed;
	uint32_t                NCSI_Flag_allapeed;

	int                  i;
	int                  j;
	uint32_t                temp;

//------------------------------------------------------------
// main Start
//------------------------------------------------------------
	eng    = &MACENG;
	phyeng = &PHYENG;
	phyeng->fp_set = 0;
	phyeng->fp_clr = 0;
#if defined(PHY_SPECIAL)
	special_PHY_init( eng );
#endif

	eng->ModeSwitch = mode;

//------------------------------------------------------------
// Bus Initial
//------------------------------------------------------------

//------------------------------------------------------------
// Get Chip Feature
//------------------------------------------------------------
	Wrn_Flag_allapeed  = 0;
	Err_Flag_allapeed  = 0;
	Des_Flag_allapeed  = 0;
	NCSI_Flag_allapeed = 0;
	eng->flg.Wrn_Flag              = 0;
	eng->flg.Err_Flag              = 0;
	eng->flg.Des_Flag              = 0;
	eng->flg.NCSI_Flag             = 0;
	eng->flg.Flag_PrintEn          = 1;
	eng->flg.AllFail               = 0;
	eng->run.TIME_OUT_Des_PHYRatio = 1;
	eng->run.Loop_ofcnt            = 0;
	eng->run.Loop                  = 0;
	eng->run.Loop_rl[0]            = 0;
	eng->run.Loop_rl[1]            = 0;
	eng->run.Loop_rl[2]            = 0;
	eng->dat.FRAME_LEN             = 0;
	eng->dat.wp_lst                = 0;
	eng->io.init_done              = 0;
	eng->env.VGAModeVld            = 0;
	eng->reg.SCU_oldvld            = 0;
	eng->phy.Adr                   = 0;
	eng->phy.loop_phy              = 0;
	eng->phy.default_phy           = 0;
	eng->phy.PHY_ID2               = 0;
	eng->phy.PHY_ID3               = 0;
	eng->phy.PHYName[0]            = 0;
	eng->phy.RMIICK_IOMode         = 0;
	eng->ncsi_cap.PCI_DID_VID      = 0;
	eng->ncsi_cap.ManufacturerID   = 0;
	read_scu( eng );

	if ( RUN_STEP >= 1 ) {
		//------------------------------
		// [Reg]check SCU_07c
		// [Env]setup ASTChipName
		// [Env]setup ASTChipType
		//------------------------------
		switch ( eng->reg.SCU_07c ) {
			default:
				sprintf( eng->env.ASTChipName, "[ ]" );
				temp = ( eng->reg.SCU_07c ) & 0xff000000;
				switch ( temp ) {
					case 0x04000000 : eng->env.ASTChipType = 8; goto PASS_CHIP_ID;
					default:
						printf("Error Silicon Revision ID(SCU7C) %08x [%08x]!!!\n", eng->reg.SCU_07c, temp);
				}
				return(1);
		} // End switch ( eng->reg.SCU_07c )
PASS_CHIP_ID:
		//------------------------------
		// [Env]check ASTChipType
		// [Env]setup AST1100
		// [Env]setup AST2300
		// [Env]setup AST2400
		// [Env]setup AST1010
		// [Env]setup AST2500
		// [Env]setup AST2500A1
		//------------------------------
		eng->env.AST1100   = 0;
		eng->env.AST2300   = 0;
		eng->env.AST2400   = 0;
		eng->env.AST1010   = 0;
		eng->env.AST2500   = 0;
		eng->env.AST2500A1 = 0;
		switch ( eng->env.ASTChipType ) {
			case 8  : eng->env.AST2500A1 = 1;
			case 7  : eng->env.AST2500   = 1;
			case 5  : eng->env.AST2400   = 1;
			case 4  : eng->env.AST2300   = 1; break;

			case 6  : eng->env.AST2300 = 1; eng->env.AST2400 = 1; eng->env.AST1010 = 1; break;
			case 1  : eng->env.AST1100 = 1; break;
			default : break;
		} // End switch ( eng->env.ASTChipType )

		//------------------------------
		// [Env]check ASTChipType
		// [Reg]check SCU_0f0
		// [Env]setup MAC34_vld
		//------------------------------
		if ( ( eng->env.ASTChipType == 4 ) && ( eng->reg.SCU_0f0 & 0x00000001 ) )//only AST2300
			eng->env.MAC34_vld = 1;
		else
			eng->env.MAC34_vld = 0;
#ifdef CONFIG_ASPEED_AST2600
		eng->env.MAC34_vld = 1;
#endif

//------------------------------------------------------------
// Get Argument Input
//------------------------------------------------------------
		//------------------------------
		// Load default value
		//------------------------------
		if ( eng->ModeSwitch == MODE_NSCI ) {
			eng->arg.GARPNumCnt     = DEF_GARPNUMCNT;
			eng->arg.GChannelTolNum = DEF_GCHANNEL2NUM;
			eng->arg.GPackageTolNum = DEF_GPACKAGE2NUM;
			eng->arg.GCtrl          = 0;
			eng->arg.GSpeed         = SET_100MBPS;        // In NCSI mode, we set to 100M bps
		}
		else {
			eng->arg.GUserDVal    = DEF_GUSER_DEF_PACKET_VAL;
			eng->arg.GPHYADR      = DEF_GPHY_ADR;
			eng->arg.GLOOP_INFINI = 0;
			eng->arg.GLOOP_MAX    = 0;
			eng->arg.GCtrl        = DEF_GCTRL;
			eng->arg.GSpeed       = DEF_GSPEED;
		}
		eng->arg.GChk_TimingBund = DEF_GIOTIMINGBUND;
		eng->arg.GTestMode       = DEF_GTESTMODE;

		//------------------------------
		// Get setting information by user
		//------------------------------
		eng->arg.GRun_Mode = (BYTE)atoi(argv[1]);
		if (argc > 1) {
			if ( eng->ModeSwitch == MODE_NSCI ) {
				switch ( argc ) {
					case 8: eng->arg.GARPNumCnt      = (uint32_t)atoi(argv[7]);
					case 7: eng->arg.GCtrl           = (BYTE)atoi(argv[6]);
					case 6: eng->arg.GChk_TimingBund = (BYTE)atoi(argv[5]);
					case 5: eng->arg.GTestMode       = (BYTE)atoi(argv[4]);
					case 4: eng->arg.GChannelTolNum  = (BYTE)atoi(argv[3]);
					case 3: eng->arg.GPackageTolNum  = (BYTE)atoi(argv[2]);
					default: break;
				}
			}
			else {
				eng->arg.GARPNumCnt = 0;
				switch ( argc ) {
					case 9: eng->arg.GUserDVal       = strtoul(argv[8], &stop_at, 16);
					case 8: eng->arg.GChk_TimingBund = (BYTE)atoi(argv[7]);
					case 7: eng->arg.GPHYADR         = (BYTE)atoi(argv[6]);
					case 6: eng->arg.GTestMode       = (BYTE)atoi(argv[5]);
					case 5: strcpy( eng->arg.GLOOP_Str, argv[4] );
						if ( !strcmp( eng->arg.GLOOP_Str, "#" ) )
							eng->arg.GLOOP_INFINI = 1;
						else
							eng->arg.GLOOP_MAX    = (uint32_t)atoi( eng->arg.GLOOP_Str );
					case 4: eng->arg.GCtrl           = (BYTE)atoi(argv[3]);
					case 3: eng->arg.GSpeed          = (BYTE)atoi(argv[2]);
					default: break;
				}
			} // End if ( eng->ModeSwitch == MODE_NSCI )
		}
		else {
			// Wrong parameter
			if ( eng->ModeSwitch == MODE_NSCI ) {
				if ( eng->env.AST2300 )
					printf("\nNCSITEST.exe  run_mode  <package_num>  <channel_num>  <test_mode>  <IO margin>\n\n");
				else
					printf("\nNCSITEST.exe  run_mode  <package_num>  <channel_num>  <test_mode>\n\n");
				PrintMode         ( eng );
				PrintPakNUm       ( eng );
				PrintChlNUm       ( eng );
				PrintTest         ( eng );
				PrintIOTimingBund ( eng );
			}
			else {
#ifdef Enable_MAC_ExtLoop
				printf("\nMACLOOP.exe  run_mode  <speed>\n\n");
#else
				if ( eng->env.AST2300 )
					printf("\nMACTEST.exe  run_mode  <speed>  <ctrl>  <loop_max>  <test_mode>  <phy_adr>  <IO margin>\n\n");
				else
					printf("\nMACTEST.exe  run_mode  <speed>  <ctrl>  <loop_max>  <test_mode>  <phy_adr>\n\n");
#endif
				PrintMode         ( eng );
				PrintSpeed        ( eng );
#ifndef Enable_MAC_ExtLoop
				PrintCtrl         ( eng );
				PrintLoop         ( eng );
				PrintTest         ( eng );
				PrintPHYAdr       ( eng );
				PrintIOTimingBund ( eng );
#endif
			} // End if ( eng->ModeSwitch == MODE_NSCI )
			Finish_Close( eng );

			return(1);
		} // End if (argc > 1)

#ifdef Enable_MAC_ExtLoop
		eng->arg.GChk_TimingBund = 0;
		eng->arg.GTestMode       = 0;
		eng->arg.GLOOP_INFINI    = 1;
//		eng->arg.GCtrl           = 0;
		eng->arg.GCtrl           = eng->arg.GCtrl & 0x08;
#endif

//------------------------------------------------------------
// Check Argument & Setup
//------------------------------------------------------------
		//------------------------------
		// [Env]check MAC34_vld
		// [Arg]check GRun_Mode
		// [Run]setup MAC_idx
		// [Run]setup MAC_BASE
		//------------------------------
		switch ( eng->arg.GRun_Mode ) {
			case 0:                            printf("\n[MAC1]\n"); eng->run.MAC_idx = 0; eng->run.MAC_BASE = MAC_BASE1; break;
			case 1:                            printf("\n[MAC2]\n"); eng->run.MAC_idx = 1; eng->run.MAC_BASE = MAC_BASE2; break;
			case 2: if ( eng->env.MAC34_vld ) {printf("\n[MAC3]\n"); eng->run.MAC_idx = 2; eng->run.MAC_BASE = MAC_BASE3; break;}
			        else
			            goto Error_GRun_Mode;
			case 3: if ( eng->env.MAC34_vld ) {printf("\n[MAC4]\n"); eng->run.MAC_idx = 3; eng->run.MAC_BASE = MAC_BASE4; break;}
			        else
			            goto Error_GRun_Mode;
			default:
Error_GRun_Mode:
				printf("Error run_mode!!!\n");
				PrintMode ( eng );
				return(1);
		} // End switch ( eng->arg.GRun_Mode )
#ifdef CONFIG_ASPEED_AST2600
switch ( eng->run.MAC_idx ) {
	case 0: Write_Reg_SCU_DD_AST2600( 0x10c , 0x00000000 | eng->reg.SCU_FPGASel ); break;
	case 1: Write_Reg_SCU_DD_AST2600( 0x10c , 0x50000000 | eng->reg.SCU_FPGASel ); break;
	case 2: Write_Reg_SCU_DD_AST2600( 0x10c , 0xa0000000 | eng->reg.SCU_FPGASel ); break;
	case 3: Write_Reg_SCU_DD_AST2600( 0x10c , 0xf0000000 | eng->reg.SCU_FPGASel ); break;
} // End switch ( eng->arg.GRun_Mode )
#endif
		//------------------------------
		// [Arg]check GSpeed
		// [Run]setup Speed_1G
		// [Run]setup Speed_org
		//------------------------------
		switch ( eng->arg.GSpeed ) {
			case SET_1GBPS          : eng->run.Speed_1G = 1; eng->run.Speed_org[ 0 ] = 1; eng->run.Speed_org[ 1 ] = 0; eng->run.Speed_org[ 2 ] = 0; break;
			case SET_100MBPS        : eng->run.Speed_1G = 0; eng->run.Speed_org[ 0 ] = 0; eng->run.Speed_org[ 1 ] = 1; eng->run.Speed_org[ 2 ] = 0; break;
			case SET_10MBPS         : eng->run.Speed_1G = 0; eng->run.Speed_org[ 0 ] = 0; eng->run.Speed_org[ 1 ] = 0; eng->run.Speed_org[ 2 ] = 1; break;
#ifndef Enable_MAC_ExtLoop
			case SET_1G_100M_10MBPS : eng->run.Speed_1G = 0; eng->run.Speed_org[ 0 ] = 1; eng->run.Speed_org[ 1 ] = 1; eng->run.Speed_org[ 2 ] = 1; break;
			case SET_100M_10MBPS    : eng->run.Speed_1G = 0; eng->run.Speed_org[ 0 ] = 0; eng->run.Speed_org[ 1 ] = 1; eng->run.Speed_org[ 2 ] = 1; break;
#endif
			default:
				printf("Error speed!!!\n");
				PrintSpeed ( eng );
				return(1);
		} // End switch ( eng->arg.GSpeed )


		if ( eng->ModeSwitch == MODE_NSCI ) {
			//------------------------------
			// [Arg]check GPackageTolNum
			// [Arg]check GChannelTolNum
			//------------------------------
			if (( eng->arg.GPackageTolNum < 1 ) || ( eng->arg.GPackageTolNum >  8 )) {
				PrintPakNUm ( eng );
				return(1);
			}
			if (( eng->arg.GChannelTolNum < 1 ) || ( eng->arg.GChannelTolNum > 32 )) {
				PrintChlNUm ( eng );
				return(1);
			}
			//------------------------------
			// [Arg]check GCtrl
			// [Arg]setup GEn_RMII_50MOut
			// [Arg]setup GEn_MACLoopback
			// [Arg]setup GEn_FullRange
			// [Arg]setup GEn_SkipRxEr
			// [Arg]setup GEn_PrintNCSI
			//------------------------------
			eng->arg.GEn_RMII_50MOut = ( eng->arg.GCtrl >> 8 ) & 0x1;
			eng->arg.GEn_MACLoopback = ( eng->arg.GCtrl >> 7 ) & 0x1;
			eng->arg.GEn_FullRange   = ( eng->arg.GCtrl >> 6 ) & 0x1;

			eng->arg.GEn_SkipRxEr    = ( eng->arg.GCtrl >> 1 ) & 0x1;
			eng->arg.GEn_PrintNCSI   = ( eng->arg.GCtrl      ) & 0x1;
		}
		else {
			//------------------------------
			// [Arg]check GCtrl
			// [Arg]setup GEn_RMIIPHY_IN
			// [Arg]setup GEn_RMII_50MOut
			// [Arg]setup GEn_MACLoopback
			// [Arg]setup GEn_FullRange
			// [Arg]setup GEn_SkipChkPHY
			// [Arg]setup GEn_IntLoopPHY
			// [Arg]setup GEn_InitPHY
			// [Arg]setup GDis_RecovPHY
			// [Arg]setup GEn_PHYAdrInv
			// [Arg]setup GEn_SinglePacket
			//------------------------------
			if ( eng->arg.GCtrl & 0xfffffe00 ) {
				printf("Error ctrl!!!\n");
				PrintCtrl ( eng );
				return(1);
			}
			else {
				eng->arg.GEn_RMIIPHY_IN   = ( eng->arg.GCtrl >> 9 ) & 0x1;
				eng->arg.GEn_RMII_50MOut  = ( eng->arg.GCtrl >> 8 ) & 0x1;
				eng->arg.GEn_MACLoopback  = ( eng->arg.GCtrl >> 7 ) & 0x1;
				eng->arg.GEn_FullRange    = ( eng->arg.GCtrl >> 6 ) & 0x1;
				eng->arg.GEn_SkipChkPHY   = ( eng->arg.GCtrl >> 5 ) & 0x1;
				eng->arg.GEn_IntLoopPHY   = ( eng->arg.GCtrl >> 4 ) & 0x1;
				eng->arg.GEn_InitPHY      = ( eng->arg.GCtrl >> 3 ) & 0x1;
				eng->arg.GDis_RecovPHY    = ( eng->arg.GCtrl >> 2 ) & 0x1;
				eng->arg.GEn_PHYAdrInv    = ( eng->arg.GCtrl >> 1 ) & 0x1;
				eng->arg.GEn_SinglePacket = ( eng->arg.GCtrl      ) & 0x1;
				if ( !eng->env.AST2400 && eng->arg.GEn_MACLoopback ) {
					printf("Error ctrl!!!\n");
					PrintCtrl ( eng );
					return(1);
				}
			} // End if ( eng->arg.GCtrl & 0xffffff83 )

			//------------------------------
			// [Arg]check GPHYADR
			//------------------------------
			if ( eng->arg.GPHYADR > 31 ) {
				printf("Error phy_adr!!!\n");
				PrintPHYAdr ( eng );
				return(1);
			} // End if ( eng->arg.GPHYADR > 31)
			//------------------------------
			// [Arg]check GLOOP_MAX
			// [Arg]check GSpeed
			// [Arg]setup GLOOP_MAX
			//------------------------------
			if ( !eng->arg.GLOOP_MAX )
				switch ( eng->arg.GSpeed ) {
					case SET_1GBPS         : eng->arg.GLOOP_MAX = DEF_GLOOP_MAX * 20; break;
					case SET_100MBPS       : eng->arg.GLOOP_MAX = DEF_GLOOP_MAX * 2 ; break;
					case SET_10MBPS        : eng->arg.GLOOP_MAX = DEF_GLOOP_MAX     ; break;
					case SET_1G_100M_10MBPS: eng->arg.GLOOP_MAX = DEF_GLOOP_MAX * 20; break;
					case SET_100M_10MBPS   : eng->arg.GLOOP_MAX = DEF_GLOOP_MAX * 2 ; break;
				}
		} // End if ( eng->ModeSwitch == MODE_NSCI )

//------------------------------------------------------------
// Check Argument & Setup Running Parameter
//------------------------------------------------------------
		//------------------------------
		// [Env]check AST2300
		// [Arg]check GTestMode
		// [Run]setup TM_IOTiming
		// [Run]setup TM_IOStrength
		// [Run]setup TM_TxDataEn
		// [Run]setup TM_RxDataEn
		// [Run]setup TM_NCSI_DiSChannel
		// [Run]setup TM_Burst
		// [Run]setup TM_IEEE
		// [Run]setup TM_WaitStart
		//------------------------------
		eng->run.TM_IOTiming        = 0;
		eng->run.TM_IOStrength      = 0;
		eng->run.TM_TxDataEn        = 1;
		eng->run.TM_RxDataEn        = 1;
		eng->run.TM_NCSI_DiSChannel = 1; // For ncsitest function
		eng->run.TM_Burst           = 0; // For mactest function
		eng->run.TM_IEEE            = 0; // For mactest function
		eng->run.TM_WaitStart       = 0; // For mactest function
		eng->run.TM_DefaultPHY      = 0; // For mactest function
		if ( eng->ModeSwitch == MODE_NSCI ) {
			switch ( eng->arg.GTestMode ) {
				case 0 :     break;
				case 1 :     eng->run.TM_NCSI_DiSChannel = 0; break;
				case 6 : if ( eng->env.AST2300 ) {
					     eng->run.TM_IOTiming = 1; break;}
					 else
					     goto Error_GTestMode_NCSI;
				case 7 : if ( eng->env.AST2300 ) {
					     eng->run.TM_IOTiming = 1; eng->run.TM_IOStrength = 1; break;}
					 else
					     goto Error_GTestMode_NCSI;
				default:
Error_GTestMode_NCSI:
					printf("Error test_mode!!!\n");
					PrintTest ( eng );
					return(1);
			} // End switch ( eng->arg.GTestMode )
		}
		else {
			switch ( eng->arg.GTestMode ) {
				case  0 :     break;
				case  1 :     eng->run.TM_RxDataEn = 0; eng->run.TM_Burst = 1; eng->run.TM_IEEE = 1; break;
				case  2 :     eng->run.TM_RxDataEn = 0; eng->run.TM_Burst = 1; eng->run.TM_IEEE = 1; break;
				case  3 :     eng->run.TM_RxDataEn = 0; eng->run.TM_Burst = 1; eng->run.TM_IEEE = 1; break;
				case  4 :     eng->run.TM_RxDataEn = 0; eng->run.TM_Burst = 1; eng->run.TM_IEEE = 0; break;
				case  5 :     eng->run.TM_RxDataEn = 0; eng->run.TM_Burst = 1; eng->run.TM_IEEE = 1; break;
				case  6 : if ( eng->env.AST2300 ) {
				              eng->run.TM_IOTiming = 1; break;}
				          else
				              goto Error_GTestMode;
				case  7 : if ( eng->env.AST2300 ) {
				              eng->run.TM_IOTiming = 1; eng->run.TM_IOStrength = 1; break;}
				          else
				              goto Error_GTestMode;
				case  8 :     eng->run.TM_RxDataEn = 0; eng->run.TM_DefaultPHY = 1; break;
				case  9 :     eng->run.TM_TxDataEn = 0; eng->run.TM_DefaultPHY = 1; break;
				case 10 :     eng->run.TM_WaitStart = 1; break;
				case 11 :     break;
				default:
Error_GTestMode:
					printf("Error test_mode!!!\n");
					PrintTest ( eng );
					return(1);
			} // End switch ( eng->arg.GTestMode )
#ifdef Enable_MAC_ExtLoop
			eng->run.TM_DefaultPHY = 1;
#endif
		} // End if ( eng->ModeSwitch == MODE_NSCI )

		//------------------------------
		// [Env]check AST2300
		// [Arg]check GChk_TimingBund
		// [Run]check TM_Burst
		// [Arg]setup GIEEE_sel
		// [Run]setup IO_Bund
		//------------------------------
		if ( eng->run.TM_Burst ) {
			eng->arg.GIEEE_sel = eng->arg.GChk_TimingBund;
			eng->run.IO_Bund = 0;
		}
		else {
			eng->arg.GIEEE_sel = 0;
			if ( eng->env.AST2300 ) {
				eng->run.IO_Bund = eng->arg.GChk_TimingBund;

//				if ( !( ( ( 7 >= eng->run.IO_Bund ) && ( eng->run.IO_Bund & 0x1 ) ) ||
//				        ( eng->run.IO_Bund == 0                                   )
//				       ) ) {
//					printf("Error IO margin!!!\n");
//					PrintIOTimingBund ( eng );
//					return(1);
//				}
				if ( !( ( eng->run.IO_Bund & 0x1 ) ||
				        ( eng->run.IO_Bund == 0  )
				       ) ) {
					printf("Error IO margin!!!\n");
					PrintIOTimingBund ( eng );
					return(1);
				}
			}
			else {
				eng->run.IO_Bund = 0;
			}
		} // End if ( eng->run.TM_Burst )
//------------------------------------------------------------
// Setup Environment
//------------------------------------------------------------
		//------------------------------
		// [Env]check AST1010
		// [Env]check AST2300
		// [Reg]check SCU_070
		// [Env]setup MAC_Mode
		// [Env]setup MAC1_1Gvld
		// [Env]setup MAC2_1Gvld
		// [Env]setup MAC1_RMII
		// [Env]setup MAC2_RMII
		// [Env]setup MAC2_vld
		//------------------------------
		if ( eng->env.AST2300 ) {
			eng->env.MAC_Mode   = ( eng->reg.SCU_070 >> 6 ) & 0x3;
			eng->env.MAC1_1Gvld = ( eng->env.MAC_Mode & 0x1 ) ? 1 : 0;//1:RGMII, 0:RMII
			eng->env.MAC2_1Gvld = ( eng->env.MAC_Mode & 0x2 ) ? 1 : 0;//1:RGMII, 0:RMII
			eng->env.MAC1_RMII  = !eng->env.MAC1_1Gvld;
			eng->env.MAC2_RMII  = !eng->env.MAC2_1Gvld;
			eng->env.MAC2_vld   = 1;
#ifdef CONFIG_ASPEED_AST2600
			eng->env.MAC3_1Gvld = ( eng->reg.SCU_510 & 0x1 ) ? 1 : 0;//1:RGMII, 0:RMII
			eng->env.MAC4_1Gvld = ( eng->reg.SCU_510 & 0x2 ) ? 1 : 0;//1:RGMII, 0:RMII
			eng->env.MAC3_RMII  = !eng->env.MAC3_1Gvld;
			eng->env.MAC4_RMII  = !eng->env.MAC4_1Gvld;
#endif
		}
		else {
			eng->env.MAC_Mode   = ( eng->reg.SCU_070 >> 6 ) & 0x7;
			eng->env.MAC1_1Gvld = ( eng->env.MAC_Mode == 0x0 ) ? 1 : 0;
			eng->env.MAC2_1Gvld = 0;

			switch ( eng->env.MAC_Mode ) {
				case 0 : eng->env.MAC1_RMII = 0; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 0; break; //000: Select GMII(MAC#1) only
				case 1 : eng->env.MAC1_RMII = 0; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 1; break; //001: Select MII (MAC#1) and MII(MAC#2)
				case 2 : eng->env.MAC1_RMII = 1; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 1; break; //010: Select RMII(MAC#1) and MII(MAC#2)
				case 3 : eng->env.MAC1_RMII = 0; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 0; break; //011: Select MII (MAC#1) only
				case 4 : eng->env.MAC1_RMII = 1; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 0; break; //100: Select RMII(MAC#1) only
//				case 5 : eng->env.MAC1_RMII = 0; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 0; break; //101: Reserved
				case 6 : eng->env.MAC1_RMII = 1; eng->env.MAC2_RMII = 1; eng->env.MAC2_vld = 1; break; //110: Select RMII(MAC#1) and RMII(MAC#2)
//				case 7 : eng->env.MAC1_RMII = 0; eng->env.MAC2_RMII = 0; eng->env.MAC2_vld = 0; break; //111: Disable dual MAC
				default:
					return( Finish_Check( eng, Err_Flag_MACMode ) );
			}
		} // End if ( eng->env.AST2300 )
		eng->env.MAC_atlast_1Gvld = eng->env.MAC1_1Gvld | eng->env.MAC2_1Gvld;

//------------------------------------------------------------
// Check & Setup Environment
//------------------------------------------------------------
		//------------------------------
		// [Phy]setup PHY_BASE
		// [Env]setup MAC_1Gvld
		// [Env]setup MAC_RMII
		//------------------------------
		if ( eng->run.MAC_idx == 0 ) {
			if ( eng->arg.GEn_PHYAdrInv ) {
				eng->phy.PHY_BASE    = MAC_BASE2;
				eng->run.MAC_idx_PHY = 1;
			} else {
				eng->phy.PHY_BASE    = MAC_BASE1;
				eng->run.MAC_idx_PHY = 0;
			}
			eng->env.MAC_1Gvld = eng->env.MAC1_1Gvld;
			eng->env.MAC_RMII  = eng->env.MAC1_RMII;

			if ( eng->run.Speed_1G & !eng->env.MAC1_1Gvld ) {
				printf("\nMAC1 don't support 1Gbps !!!\n");
				return( Finish_Check( eng, Err_Flag_MACMode ) );
			}
		}
		else if ( eng->run.MAC_idx == 1 ) {
			if ( eng->arg.GEn_PHYAdrInv ) {
				eng->phy.PHY_BASE    = MAC_BASE1;
				eng->run.MAC_idx_PHY = 0;
			} else {
				eng->phy.PHY_BASE    = MAC_BASE2;
				eng->run.MAC_idx_PHY = 1;
			}
			eng->env.MAC_1Gvld = eng->env.MAC2_1Gvld;
			eng->env.MAC_RMII  = eng->env.MAC2_RMII;

			if ( eng->run.Speed_1G & !eng->env.MAC2_1Gvld ) {
				printf("\nMAC2 don't support 1Gbps !!!\n");
				return( Finish_Check( eng, Err_Flag_MACMode ) );
			}
			if ( !eng->env.MAC2_vld ) {
				printf("\nMAC2 not valid !!!\n");
				return( Finish_Check( eng, Err_Flag_MACMode ) );
			}
		}
		else {
			if ( eng->run.MAC_idx == 2 ) {
				if ( eng->arg.GEn_PHYAdrInv ) {
					eng->phy.PHY_BASE    = MAC_BASE4;
					eng->run.MAC_idx_PHY = 3;
				} else {
					eng->phy.PHY_BASE    = MAC_BASE3;
					eng->run.MAC_idx_PHY = 2;
				}
#ifdef CONFIG_ASPEED_AST2600
				eng->env.MAC_1Gvld = eng->env.MAC3_1Gvld;
				eng->env.MAC_RMII  = eng->env.MAC3_RMII;

				if ( eng->run.Speed_1G & !eng->env.MAC3_1Gvld ) {
					printf("\nMAC3 don't support 1Gbps !!!\n");
					return( Finish_Check( eng, Err_Flag_MACMode ) );
				}
#endif
			} else {
				if ( eng->arg.GEn_PHYAdrInv ) {
					eng->phy.PHY_BASE    = MAC_BASE3;
					eng->run.MAC_idx_PHY = 2;
				} else {
					eng->phy.PHY_BASE    = MAC_BASE4;
					eng->run.MAC_idx_PHY = 3;
				}
#ifdef CONFIG_ASPEED_AST2600
				eng->env.MAC_1Gvld = eng->env.MAC4_1Gvld;
				eng->env.MAC_RMII  = eng->env.MAC4_RMII;

				if ( eng->run.Speed_1G & !eng->env.MAC4_1Gvld ) {
					printf("\nMAC4 don't support 1Gbps !!!\n");
					return( Finish_Check( eng, Err_Flag_MACMode ) );
				}
#endif
			}
#ifdef CONFIG_ASPEED_AST2600
#else
			eng->env.MAC_1Gvld = 0;
			eng->env.MAC_RMII  = 1;

			if ( eng->run.Speed_1G ) {
				printf("\nMAC3/MAC4 don't support 1Gbps !!!\n");
				return( Finish_Check( eng, Err_Flag_MACMode ) );
			}
#endif			
		} // End if ( eng->run.MAC_idx == 0 )

		if ( !eng->env.MAC_1Gvld )
			eng->run.Speed_org[ 0 ] = 0;

		if ( ( eng->ModeSwitch == MODE_NSCI ) && ( !eng->env.MAC_RMII ) ) {
			printf("\nNCSI must be RMII interface !!!\n");
			return( Finish_Check( eng, Err_Flag_MACMode ) );
		}

		//------------------------------
		// [Env]setup MHCLK_Ratio
		//------------------------------
#ifdef CONFIG_ASPEED_AST2600
#else
		eng->env.MHCLK_Ratio = ( eng->reg.SCU_008 >> 16 ) & 0x7;
		if ( eng->env.MAC_atlast_1Gvld ) {
			if ( eng->env.MHCLK_Ratio != 2 ) {
				FindErr( eng, Err_Flag_MHCLK_Ratio );
//				return( Finish_Check( eng, Err_Flag_MHCLK_Ratio ) );
			}
		}
		else {
			if ( eng->env.MHCLK_Ratio != 4 ) {
				FindErr( eng, Err_Flag_MHCLK_Ratio );
//				return( Finish_Check( eng, Err_Flag_MHCLK_Ratio ) );
			}
		}
#endif

//------------------------------------------------------------
// Parameter Initial
//------------------------------------------------------------
		//------------------------------
		// [Reg]setup SCU_004_rstbit
		// [Reg]setup SCU_004_mix
		// [Reg]setup SCU_004_dis
		// [Reg]setup SCU_004_en
		//------------------------------
		if ( eng->arg.GEn_PHYAdrInv ) {
			eng->reg.SCU_004_rstbit = 0x00001800; //Reset Engine
		}
		else {
			if ( eng->run.MAC_idx == 1 )
				eng->reg.SCU_004_rstbit = 0x00001000; //Reset Engine
			else
				eng->reg.SCU_004_rstbit = 0x00000800; //Reset Engine
		}
		eng->reg.SCU_004_mix = eng->reg.SCU_004;
		eng->reg.SCU_004_en  = eng->reg.SCU_004_mix & (~eng->reg.SCU_004_rstbit);
		eng->reg.SCU_004_dis = eng->reg.SCU_004_mix |   eng->reg.SCU_004_rstbit;

		//------------------------------
		// [Reg]setup SCU_00c_clkbit
		// [Reg]setup SCU_00c_mix
		// [Reg]setup SCU_00c_dis
		// [Reg]setup SCU_00c_en
		//------------------------------
		if ( eng->env.AST2300 ) {
//			if ( eng->arg.GEn_PHYAdrInv ) {
				if ( eng->env.MAC34_vld )
					eng->reg.SCU_00c_clkbit = 0x00f00000; //Clock Stop Control
				else
					eng->reg.SCU_00c_clkbit = 0x00300000; //Clock Stop Control
//			}
//			else {
//				switch ( eng->run.MAC_idx ) {
//					case 3: eng->reg.SCU_00c_clkbit = 0x00800000; break; //Clock Stop Control
//					case 2: eng->reg.SCU_00c_clkbit = 0x00400000; break; //Clock Stop Control
//					case 1: eng->reg.SCU_00c_clkbit = 0x00200000; break; //Clock Stop Control
//					case 0: eng->reg.SCU_00c_clkbit = 0x00100000; break; //Clock Stop Control
//			}
		}
		else {
			eng->reg.SCU_00c_clkbit = 0x00000000;
		} // End if ( eng->env.AST2300 )
		eng->reg.SCU_00c_mix = eng->reg.SCU_00c;
		eng->reg.SCU_00c_en  = eng->reg.SCU_00c_mix & (~eng->reg.SCU_00c_clkbit);
		eng->reg.SCU_00c_dis = eng->reg.SCU_00c_mix |   eng->reg.SCU_00c_clkbit;

		//------------------------------
		// [Reg]setup SCU_048_mix
		// [Reg]setup SCU_048_check
		// [Reg]setup SCU_048_default
		// [Reg]setup SCU_074_mix
		//------------------------------
		eng->reg.SCU_048_mix     = ( eng->reg.SCU_048 & 0xfc000000 );
		eng->reg.SCU_048_check   = ( eng->reg.SCU_048 & 0x03ffffff );
		eng->reg.SCU_048_default =   SCU_48h_AST2500  & 0x03ffffff;

		if ( eng->arg.GEn_RMII_50MOut & eng->env.MAC_RMII ) {
			switch ( eng->run.MAC_idx ) {
				case 1: eng->reg.SCU_048_mix = eng->reg.SCU_048_mix | 0x40000000; break;
				case 0: eng->reg.SCU_048_mix = eng->reg.SCU_048_mix | 0x20000000; break;
			}
		}
		eng->reg.SCU_074_mix = eng->reg.SCU_074;

		//------------------------------
		// [Reg]setup MAC_050
		//------------------------------
		if ( eng->ModeSwitch == MODE_NSCI )
			// Set to 100Mbps and Enable RX broabcast packets and CRC_APD and Full duplex
			eng->reg.MAC_050 = 0x000a0500;// [100Mbps] RX_BROADPKT_EN & CRC_APD & Full duplex
//			eng->reg.MAC_050 = 0x000a4500;// [100Mbps] RX_BROADPKT_EN & RX_ALLADR & CRC_APD & Full duplex
		else {
#ifdef Enable_MAC_ExtLoop
//			eng->reg.MAC_050 = 0x00000100;// Full duplex
			eng->reg.MAC_050 = 0x00004100;// RX_ALLADR & Full duplex
#else
			eng->reg.MAC_050 = 0x00004500;// RX_ALLADR & CRC_APD & Full duplex
#endif
#ifdef Enable_Runt
			eng->reg.MAC_050 = eng->reg.MAC_050 | 0x00001000;
#endif
#if defined(PHY_SPECIAL)
			eng->reg.MAC_050 = eng->reg.MAC_050 | 0x00002000;
#elif defined(Enable_Jumbo)
			eng->reg.MAC_050 = eng->reg.MAC_050 | 0x00002000;
#endif
		} // End if ( eng->ModeSwitch == MODE_NSCI )

//------------------------------------------------------------
// Descriptor Number
//------------------------------------------------------------
		//------------------------------
		// [Dat]setup Des_Num
		// [Dat]setup DMABuf_Size
		// [Dat]setup DMABuf_Num
		//------------------------------
		if ( eng->ModeSwitch == MODE_DEDICATED ) {
#ifdef Enable_Jumbo
			DES_LowNumber = 1;
#else
			DES_LowNumber = eng->run.TM_IOTiming;
#endif
			if ( eng->arg.GEn_SkipChkPHY && ( eng->arg.GTestMode == 0 ) )
				eng->dat.Des_Num = 114;//for SMSC's LAN9303 issue
			else {
				switch ( eng->arg.GSpeed ) {
					case SET_1GBPS          : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 512 : 4096; break;
					case SET_100MBPS        : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 512 : 4096; break;
					case SET_10MBPS         : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 100 :  830; break;
					case SET_1G_100M_10MBPS : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 100 :  830; break;
					case SET_100M_10MBPS    : eng->dat.Des_Num = ( eng->run.IO_Bund ) ? 100 : ( DES_LowNumber ) ? 100 :  830; break;
				}
			} // End if ( eng->arg.GEn_SkipChkPHY && ( eng->arg.GTestMode == 0 ) )
#ifdef SelectDesNumber
			eng->dat.Des_Num = SelectDesNumber;
#endif
#ifdef ENABLE_ARP_2_WOL
			if ( eng->arg.GTestMode == 4 )
				eng->dat.Des_Num = 1;
#endif
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
				return( Finish_Check( eng, Err_Flag_DMABufNum ) );
		} // End if ( eng->ModeSwitch == MODE_DEDICATED )

//------------------------------------------------------------
// Setup Running Parameter
//------------------------------------------------------------
#ifdef Enable_MAC_ExtLoop
		eng->run.TDES_BASE = RDES_BASE1;
		eng->run.RDES_BASE = RDES_BASE1;
#else
		eng->run.TDES_BASE = TDES_BASE1;
		eng->run.RDES_BASE = RDES_BASE1;
#endif

		if ( eng->run.TM_IOTiming || eng->run.IO_Bund )
			eng->run.IO_MrgChk = 1;
		else
			eng->run.IO_MrgChk = 0;

		eng->phy.Adr         = eng->arg.GPHYADR;
		eng->phy.loop_phy    = eng->arg.GEn_IntLoopPHY;
		eng->phy.default_phy = eng->run.TM_DefaultPHY;

		eng->run.LOOP_MAX = eng->arg.GLOOP_MAX;
		Calculate_LOOP_CheckNum( eng );

	} // End if (RUN_STEP >= 1)

//------------------------------------------------------------
// SCU Initial
//------------------------------------------------------------
	if ( RUN_STEP >= 2 ) {
		get_mac_info( eng );
		Setting_scu( eng );
		init_scu1( eng );
	}

	if ( RUN_STEP >= 3 ) {
//		init_scu_macrst( eng );
		init_scu_macdis( eng );
		init_scu_macen( eng );
		if ( eng->ModeSwitch ==  MODE_DEDICATED ) {
#if defined(PHY_GPIO)
			phy_gpio_init( eng );
#endif
#if defined(PHY_SPECIAL)
			special_PHY_MDIO_init( eng );
#endif
			eng->phy.PHYAdrValid = find_phyadr( eng );
			if ( eng->phy.PHYAdrValid == TRUE )
				phy_sel( eng, phyeng );
		}
	}

//------------------------------------------------------------
// Data Initial
//------------------------------------------------------------
	if (RUN_STEP >= 4) {
		if ( eng->ModeSwitch ==  MODE_DEDICATED ) {
			if ( eng->run.TM_Burst )
				setup_arp ( eng );
			eng->dat.FRAME_LEN = (uint32_t *)malloc( eng->dat.Des_Num    * sizeof( uint32_t ) );
			eng->dat.wp_lst    = (uint32_t *)malloc( eng->dat.Des_Num    * sizeof( uint32_t ) );

			if ( !eng->dat.FRAME_LEN )
				return( Finish_Check( eng, Err_Flag_MALLOC_FrmSize ) );
			if ( !eng->dat.wp_lst )
				return( Finish_Check( eng, Err_Flag_MALLOC_LastWP ) );

			// Setup data and length
#if defined(PHY_SPECIAL)
			special_PHY_buf_init( eng );
#endif
			TestingSetup ( eng );
		} 
		else {
			if ( eng->arg.GARPNumCnt != 0 )
				setup_arp ( eng );
		}// End if ( eng->ModeSwitch ==  MODE_DEDICATED )

		init_iodelay( eng );
		eng->run.Speed_idx = 0;
		if ( !eng->io.Dly_3Regiser )
			if ( get_iodelay( eng ) )
				return( Finish_Check( eng, 0 ) );

	} // End if (RUN_STEP >= 4)

//------------------------------------------------------------
// main
//------------------------------------------------------------
	if (RUN_STEP >= 5) {
		nt_log_func_name();

		eng->flg.AllFail = 1;
#ifdef Enable_LOOP_INFINI
LOOP_INFINI:;
#endif
		for ( eng->run.Speed_idx = 0; eng->run.Speed_idx < 3; eng->run.Speed_idx++ )
			eng->run.Speed_sel[ (int)eng->run.Speed_idx ] = eng->run.Speed_org[ (int)eng->run.Speed_idx ];

		//------------------------------
		// [Start] The loop of different speed
		//------------------------------
		for ( eng->run.Speed_idx = 0; eng->run.Speed_idx < 3; eng->run.Speed_idx++ ) {
			eng->flg.Flag_PrintEn = 1;
			if ( eng->run.Speed_sel[ (int)eng->run.Speed_idx ] ) {
				// Setting speed of LAN
				if      ( eng->run.Speed_sel[ 0 ] ) eng->reg.MAC_050_Speed = eng->reg.MAC_050 | 0x0000020f;
				else if ( eng->run.Speed_sel[ 1 ] ) eng->reg.MAC_050_Speed = eng->reg.MAC_050 | 0x0008000f;
				else                                eng->reg.MAC_050_Speed = eng->reg.MAC_050 | 0x0000000f;
#ifdef Enable_CLK_Stable
//				init_scu_macdis( eng );
//				init_scu_macen( eng );
				Write_Reg_SCU_DD( 0x0c, eng->reg.SCU_00c_dis );//Clock Stop Control
				Read_Reg_SCU_DD( 0x0c );
				Write_Reg_MAC_DD( eng, 0x50, eng->reg.MAC_050_Speed & 0xfffffff0 );
				Write_Reg_SCU_DD( 0x0c, eng->reg.SCU_00c_en );//Clock Stop Control
#endif

				// Setting check owner time out
				if      ( eng->run.Speed_sel[ 0 ] ) eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des_PHYRatio * TIME_OUT_Des_1G;
				else if ( eng->run.Speed_sel[ 1 ] ) eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des_PHYRatio * TIME_OUT_Des_100M;
				else                                eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des_PHYRatio * TIME_OUT_Des_10M;

				if ( eng->run.TM_WaitStart )
					eng->run.TIME_OUT_Des = eng->run.TIME_OUT_Des * 10000;

				// Setting the LAN speed
				if ( eng->ModeSwitch ==  MODE_DEDICATED ) {
					// Test three speed of LAN, we will modify loop number
					if ( ( eng->arg.GSpeed == SET_1G_100M_10MBPS ) || ( eng->arg.GSpeed == SET_100M_10MBPS ) ) {
						if      ( eng->run.Speed_sel[ 0 ] ) eng->run.LOOP_MAX = eng->arg.GLOOP_MAX;
						else if ( eng->run.Speed_sel[ 1 ] ) eng->run.LOOP_MAX = eng->arg.GLOOP_MAX / 100;
						else                                eng->run.LOOP_MAX = eng->arg.GLOOP_MAX / 1000;

						if ( !eng->run.LOOP_MAX )
							eng->run.LOOP_MAX = 1;

						Calculate_LOOP_CheckNum( eng );
					}

					//------------------------------
					// PHY Initial
					//------------------------------
					if ( eng->env.AST1100 )
						init_scu2 ( eng );

#ifdef SUPPORT_PHY_LAN9303
					if ( eng->arg.GEn_InitPHY )
						LAN9303( LAN9303_I2C_BUSNUM, eng->arg.GPHYADR, eng->run.Speed_idx, eng->arg.GEn_IntLoopPHY | (eng->run.TM_Burst<<1) | eng->run.TM_IEEE );
#elif defined(PHY_SPECIAL)
					if ( eng->arg.GEn_InitPHY )
						special_PHY_reg_init( eng );
#else
					if ( phyeng->fp_set != 0 ) {
						init_phy( eng, phyeng );
  #ifdef Delay_PHYRst
//						DELAY( Delay_PHYRst * 10 );
  #endif
					}
#endif

					if ( eng->env.AST1100 )
						init_scu3 ( eng );

					if ( eng->flg.Err_Flag )
						return( Finish_Check( eng, 0 ) );
				} // End if ( eng->ModeSwitch ==  MODE_DEDICATED )

				//------------------------------
				// [Start] The loop of different IO strength
				//------------------------------
				for ( eng->io.Str_i = 0; eng->io.Str_i <= eng->io.Str_max; eng->io.Str_i++ ) {
					//------------------------------
					// Print Header of report to monitor and log file
					//------------------------------
					if ( eng->io.Dly_3Regiser )
						if ( get_iodelay( eng ) )
							return( Finish_Check( eng, 0 ) );

					if ( eng->run.IO_MrgChk ) {
						if ( eng->run.TM_IOStrength ) {
							eng->io.Str_val = eng->io.Str_reg_mask | ( eng->io.Str_i << eng->io.Str_shf );
//printf("\nIOStrength_val= %08x, ", eng->io.Str_val);
//printf("SCU90h: %08x ->", Read_Reg_SCU_DD( 0x90 ));
							Write_Reg_SCU_DD( eng->io.Str_reg_idx, eng->io.Str_val );
//printf(" %08x\n", Read_Reg_SCU_DD( 0x90 ));
						}

						if ( eng->run.IO_Bund )
							PrintIO_Header( eng, FP_LOG );
						if ( eng->run.TM_IOTiming )
							PrintIO_Header( eng, FP_IO );
						PrintIO_Header( eng, STD_OUT );
					}
					else {
						if ( eng->ModeSwitch == MODE_DEDICATED ) {
							if ( !eng->run.TM_Burst )
								Print_Header( eng, FP_LOG );
							Print_Header( eng, STD_OUT );
						}
					} // End if ( eng->run.IO_MrgChk )

					//------------------------------
					// [Start] The loop of different IO out delay
					//------------------------------
					for ( eng->io.Dly_out = eng->io.Dly_out_str; eng->io.Dly_out <= eng->io.Dly_out_end; eng->io.Dly_out+=eng->io.Dly_out_cval ) {
						if ( eng->run.IO_MrgChk ) {
							eng->io.Dly_out_selval  = eng->io.value_ary[ eng->io.Dly_out ];
							eng->io.Dly_out_reg_hit = ( eng->io.Dly_out_reg == eng->io.Dly_out_selval ) ? 1 : 0;
#ifdef Enable_Fast_SCU
							init_scu_macdis( eng );
							Write_Reg_SCU_DD( eng->io.Dly_reg_idx, eng->reg.SCU_048_mix | ( eng->io.Dly_out_selval << eng->io.Dly_out_shf ) );
							init_scu_macen( eng );
#endif
							if ( eng->run.TM_IOTiming )
								PrintIO_LineS( eng, FP_IO );
							PrintIO_LineS( eng, STD_OUT );
						} // End if ( eng->run.IO_MrgChk )

#ifdef Enable_Fast_SCU
						//------------------------------
						// SCU Initial
						//------------------------------
//						init_scu_macrst( eng );

						//------------------------------
						// MAC Initial
						//------------------------------
						init_mac( eng );
						if ( eng->flg.Err_Flag )
							return( Finish_Check( eng, 0 ) );
#endif
						//------------------------------
						// [Start] The loop of different IO in delay
						//------------------------------
						for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval ) {
							if ( eng->run.IO_MrgChk ) {
								eng->io.Dly_in_selval  = eng->io.value_ary[ eng->io.Dly_in ];
								eng->io.Dly_val = ( eng->io.Dly_in_selval  << eng->io.Dly_in_shf  )
								                | ( eng->io.Dly_out_selval << eng->io.Dly_out_shf );

//printf("\nDly_val= %08x, ", eng->io.Dly_val);
//printf("SCU%02xh: %08x ->", eng->io.Dly_reg_idx, Read_Reg_SCU_DD( eng->io.Dly_reg_idx ) );
								init_scu_macdis( eng );
								Write_Reg_SCU_DD( eng->io.Dly_reg_idx, eng->reg.SCU_048_mix | eng->io.Dly_val );
								init_scu_macen( eng );
//printf(" %08x\n", Read_Reg_SCU_DD( eng->io.Dly_reg_idx ) );
							} // End if ( eng->run.IO_MrgChk )
#ifdef Enable_Fast_SCU
#else
							//------------------------------
							// SCU Initial
							//------------------------------
//							init_scu_macrst( eng );

							//------------------------------
							// MAC Initial
							//------------------------------
							init_mac( eng );
							if ( eng->flg.Err_Flag )
								return( Finish_Check( eng, 0 ) );
#endif
							// Testing
							if ( eng->ModeSwitch == MODE_NSCI )
								eng->io.Dly_result = phy_ncsi( eng );
							else
							{
								if ((eng->arg.GTestMode == 11) && !(
								((eng->io.Dly_out == eng->io.Dly_out_str) && (eng->io.Dly_in == eng->io.Dly_in_str)) ||
								((eng->io.Dly_out == eng->io.Dly_out_str) && (eng->io.Dly_in == eng->io.Dly_in_end)) ||
								((eng->io.Dly_out == eng->io.Dly_out_end) && (eng->io.Dly_in == eng->io.Dly_in_str)) ||
								((eng->io.Dly_out == eng->io.Dly_out_end) && (eng->io.Dly_in == eng->io.Dly_in_end)) ||
								((eng->io.Dly_out == eng->io.Dly_out_reg) && (eng->io.Dly_in == eng->io.Dly_in_reg))))
									eng->io.Dly_result = 0;
								else
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
						} // End for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval )


						if ( eng->run.IO_MrgChk ) {
							if ( eng->run.TM_IOTiming ) {
								PRINTF( FP_IO, "\n" );
							}
							printf("\n");
						}
					} // End for ( eng->io.Dly_out = eng->io.Dly_out_str; eng->io.Dly_out <= eng->io.Dly_out_end; eng->io.Dly_out+=eng->io.Dly_out_cval )


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

					Wrn_Flag_allapeed  = Wrn_Flag_allapeed  | eng->flg.Wrn_Flag;
					Err_Flag_allapeed  = Err_Flag_allapeed  | eng->flg.Err_Flag;
					Des_Flag_allapeed  = Des_Flag_allapeed  | eng->flg.Err_Flag;
					NCSI_Flag_allapeed = NCSI_Flag_allapeed | eng->flg.Err_Flag;
					eng->flg.Wrn_Flag  = 0;
					eng->flg.Err_Flag  = 0;
					eng->flg.Des_Flag  = 0;
					eng->flg.NCSI_Flag = 0;
				} // End for ( eng->io.Str_i = 0; eng->io.Str_i <= eng->io.Str_max; eng->io.Str_i++ ) {

				if ( eng->ModeSwitch == MODE_DEDICATED ) {
#ifdef PHY_SPECIAL
					if ( eng->arg.GEn_InitPHY )
						special_PHY_recov( eng );
#else
//					if ( ( eng->io.Dly_result == 0 ) & ( phyeng->fp_clr != 0 ) )
					if ( phyeng->fp_clr != 0 )
						recov_phy( eng, phyeng );
#endif
				}

				eng->run.Speed_sel[ (int)eng->run.Speed_idx ] = 0;
			} // End if ( eng->run.Speed_sel[ eng->run.Speed_idx ] )

			eng->flg.Flag_PrintEn = 0;
		} // End for ( eng->run.Speed_idx = 0; eng->run.Speed_idx < 3; eng->run.Speed_idx++ )

		eng->flg.Wrn_Flag  = Wrn_Flag_allapeed;
		eng->flg.Err_Flag  = Err_Flag_allapeed;
		eng->flg.Des_Flag  = Des_Flag_allapeed;
		eng->flg.NCSI_Flag = NCSI_Flag_allapeed;

#ifdef Enable_LOOP_INFINI
		if ( eng->flg.Err_Flag == 0 ) {
			goto LOOP_INFINI;
		}
#endif
	} // End if (RUN_STEP >= 5)

	return( Finish_Check( eng, 0 ) );
}
