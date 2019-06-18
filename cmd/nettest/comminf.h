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

#ifndef COMMINF_H
#define COMMINF_H

#include "swfunc.h"

#include "typedef.h"
#include "mac.h"
#include "physpecial.h"
#include "phygpio.h"

//---------------------------------------------------------
// Print Message
//---------------------------------------------------------
// for function
#define FP_LOG                                   0
#define FP_IO                                    1
#define STD_OUT                                  2

#define PRINTF(i, ...)                                          \
   do {                                                         \
       if (i == STD_OUT) {                                      \
           fprintf(stdout, __VA_ARGS__);                        \
           break;                                               \
       }                                                        \
       if ( (display_lantest_log_msg != 0) && (i == FP_LOG) ) { \
           fprintf(stdout, "[Log]:   ");                        \
           fprintf(stdout, __VA_ARGS__);                        \
       }                                                        \
   } while ( 0 );

//---------------------------------------------------------
// Function
//---------------------------------------------------------
  #define SWAP_4B( x )                                                             \
                                                 ( ( ( ( x ) & 0xff000000 ) >> 24) \
                                                 | ( ( ( x ) & 0x00ff0000 ) >>  8) \
                                                 | ( ( ( x ) & 0x0000ff00 ) <<  8) \
                                                 | ( ( ( x ) & 0x000000ff ) << 24) \
                                                 )
  #define SWAP_2B( x )                                                             \
                                                 ( ( ( ( x ) & 0xff00     ) >>  8) \
                                                 | ( ( ( x ) & 0x00ff     ) <<  8) \
                                                 )

  #define SWAP_2B_BEDN( x )                      ( SWAP_2B ( x ) )
  #define SWAP_2B_LEDN( x )                      ( x )
  #define SWAP_4B_BEDN( x )                      ( SWAP_4B ( x ) )
  #define SWAP_4B_LEDN( x )                      ( x )

  #define SWAP_4B_BEDN_NCSI( x )                 ( SWAP_4B( x ) )
  #define SWAP_4B_LEDN_NCSI( x )                 ( x )

#if defined(ENABLE_BIG_ENDIAN_MEM)
  #define SWAP_4B_LEDN_MEM( x )                  ( SWAP_4B( x ) )
#else
  #define SWAP_4B_LEDN_MEM( x )                  ( x )
#endif
#if defined(ENABLE_BIG_ENDIAN_REG)
  #define SWAP_4B_LEDN_REG( x )                  ( SWAP_4B( x ) )
#else
  #define SWAP_4B_LEDN_REG( x )                  ( x )
#endif

#define DELAY( x )                       	udelay( ( x ) * 1000 )
#define GET_CAHR                         	getc

//---------------------------------------------------------
// Default argument
//---------------------------------------------------------
#define  DEF_GUSER_DEF_PACKET_VAL                0x66666666     //0xff00ff00, 0xf0f0f0f0, 0xcccccccc, 0x55aa55aa, 0x5a5a5a5a, 0x66666666
#define  DEF_GIOTIMINGBUND                       5              //0/1/3/5/7
#define  DEF_GPHY_ADR                            0
#define  DEF_GTESTMODE                           0              //[0]0: no burst mode, 1: 0xff, 2: 0x55, 3: random, 4: ARP, 5: ARP, 6: IO timing, 7: IO timing+IO Strength
#define  DEF_GLOOP_MAX                           1
#define  DEF_MAC_LOOP_BACK                       0              //GCtrl bit6
#define  DEF_SKIP_CHECK_PHY                      0              //GCtrl bit5
#define  DEF_INTERNAL_LOOP_PHY                   0              //GCtrl bit4
#define  DEF_INIT_PHY                            1              //GCtrl bit3
#define  DEF_DIS_RECOVPHY                        0              //GCtrl bit2
#define  DEF_GCTRL                               (( DEF_MAC_LOOP_BACK << 6 ) | ( DEF_SKIP_CHECK_PHY << 5 ) | ( DEF_INTERNAL_LOOP_PHY << 4 ) | ( DEF_INIT_PHY << 3 ) | ( DEF_DIS_RECOVPHY << 2 ))

#define  SET_1GBPS                               0              // 1G bps
#define  SET_100MBPS                             1              // 100M bps
#define  SET_10MBPS                              2              // 10M bps
#define  SET_1G_100M_10MBPS                      3              // 1G and 100M and 10M bps
#define  SET_100M_10MBPS                         4              // 100M and 10M bps
#ifdef Enable_MAC_ExtLoop
#define  DEF_GSPEED                              SET_1GBPS
#else
#define  DEF_GSPEED                              SET_1G_100M_10MBPS
#endif
#define  DEF_GARPNUMCNT                          0

//---------------------------------------------------------
// MAC information
//---------------------------------------------------------
#define MAC_BASE1                              0x1e660000
#define MAC_BASE2                              0x1e680000
#define MAC_BASE3                              0x1e670000
#define MAC_BASE4                              0x1e690000
#define MDC_Thres                                0x3f
#define MAC_PHYWr                                0x08000000
#define MAC_PHYRd                                0x04000000

#define MAC_PHYWr_New                            0x00009400
#define MAC_PHYRd_New                            0x00009800
#define MAC_PHYBusy_New                          0x00008000

#define MAC_PHYWr_AST2600                         0x94000000
#define MAC_PHYRd_AST2600                         0x98000000
#define MAC_PHYBusy_AST2600                       0x80000000
#define MAC_PHYIDLE_AST2600                       0x00010000

//#define MAC_030_def                              0x00001010
//#define MAC_034_def                              0x00000000
//#define MAC_038_def                              0x00d22f00 //default 0x22f00
//#define MAC_038_def                              0x00022f00 //default 0x22f00
//#define MAC_038_def                              0x00022400 //default 0x22500 (AST2500)
//#define MAC_040_def                              0x00000000

#ifdef Enable_BufMerge
    #define MAC_048_def                          0x007702F1 //default 0xf1
#else
    #define MAC_048_def                          0x000002F1 //default 0xf1
#endif
//#define MAC_058_def                              0x00000040 //0x000001c0

//---------------------------------------------------------
// Data information
//---------------------------------------------------------
#if defined(Enable_MAC_ExtLoop) || defined(Enable_MAC_ExtLoop_PakcegMode) || defined(SelectSimpleBoundary) || defined(PHY_SPECIAL)
      #define ZeroCopy_OFFSET                    0
#else
      #define ZeroCopy_OFFSET                    (( eng->run.TM_Burst ) ? 0 : 2)
#endif

//      --------------------------------- DRAM_MapAdr            = TDES_BASE
//              | TX descriptor ring    |
//              ------------------------- DRAM_MapAdr + 0x040000 = RDES_BASE
//              | RX descriptor ring    |
//              -------------------------
//              | Reserved              |
//              -------------------------
//              | Reserved              |
//      --------------------------------- DRAM_MapAdr + 0x100000 = DMA_BASE    -------------------------
//              |   #1                  |  \                                   |     #1     Tx         |
//  DMA buffer  |                       |   DMA_BufSize                        |      LOOP = 0         |
// ( Tx/Rx )    -------------------------  /                                   --------------------------------------------------
//              |   #2                  |                                      |     #2     Rx         |  #2     Tx             |
//              |                       |                                      |      LOOP = 0         |   LOOP = 1             |
//              -------------------------                                      --------------------------------------------------
//              |   #3                  |                                                              |  #3     Rx             |
//              |                       |                                                              |   LOOP = 1             |
//              -------------------------                                                              -------------------------
//              |   #4                  |                                                                                     ..........
//              |                       |
//              -------------------------
//              |   #5                  |
//              |                       |
//              -------------------------
//              |   #6                  |
//              |                       |
//              -------------------------
//                           .
//                           .
//              -------------------------
//              |   #n, n = DMA_BufNum  |
//              |                       |
//      ---------------------------------
#ifdef Enable_MAC_ExtLoop
  #define DRAM_OFS_BUF                         0x00000000
#else
  #define DRAM_OFS_BUF                         0x04000000
  // #define DRAM_OFS_BUF                         0x06000000 // in-house setting
#endif

#define DRAM_OFS_WINDOW                      0x80000000
#define DRAM_OFS_REMAP                       0x00000000

  #define TDES_BASE1                             ( 0x00000000 + DRAM_OFS_BUF - DRAM_OFS_REMAP + DRAM_OFS_WINDOW )
#ifdef Enable_MAC_ExtLoop
  #define RDES_BASE1                             ( 0x00080000 + DRAM_OFS_BUF - DRAM_OFS_REMAP + DRAM_OFS_WINDOW )
#else
  #define RDES_BASE1                             ( 0x00040000 + DRAM_OFS_BUF - DRAM_OFS_REMAP + DRAM_OFS_WINDOW )
#endif
  #define DMA_BASE                               ( 0x00100000 + DRAM_OFS_BUF - DRAM_OFS_REMAP + DRAM_OFS_WINDOW )

  #define TDES_IniVal                            ( 0xb0000000 + eng->dat.FRAME_LEN_Cur )
  #define RDES_IniVal                            ( 0x00000fff )
  #define EOR_IniVal                             ( 0x40008000 )
  #define HWOwnTx(dat)                           ( dat & 0x80000000      )
  #define HWOwnRx(dat)                           ((dat & 0x80000000) == 0)
  #define HWEOR(dat)                             ( dat & 0x40000000      )

  #define AT_MEMRW_BUF( x )                      ( ( x ) + DRAM_OFS_REMAP - DRAM_OFS_WINDOW )
  #define AT_BUF_MEMRW( x )                      ( ( x ) - DRAM_OFS_REMAP + DRAM_OFS_WINDOW )

//---------------------------------------------------------
// Error Flag Bits
//---------------------------------------------------------
#define Err_Flag_MACMode                              ( 1 <<  0 )   // MAC interface mode mismatch
#define Err_Flag_PHY_Type                             ( 1 <<  1 )   // Unidentifiable PHY
#define Err_Flag_MALLOC_FrmSize                       ( 1 <<  2 )   // Malloc fail at frame size buffer
#define Err_Flag_MALLOC_LastWP                        ( 1 <<  3 )   // Malloc fail at last WP buffer
#define Err_Flag_Check_Buf_Data                       ( 1 <<  4 )   // Received data mismatch
#define Err_Flag_Check_Des                            ( 1 <<  5 )   // Descriptor error
#define Err_Flag_NCSI_LinkFail                        ( 1 <<  6 )   // NCSI packet retry number over flows
#define Err_Flag_NCSI_Check_TxOwnTimeOut              ( 1 <<  7 )   // Time out of checking Tx owner bit in NCSI packet
#define Err_Flag_NCSI_Check_RxOwnTimeOut              ( 1 <<  8 )   // Time out of checking Rx owner bit in NCSI packet
#define Err_Flag_NCSI_Check_ARPOwnTimeOut             ( 1 <<  9 )   // Time out of checking ARP owner bit in NCSI packet
#define Err_Flag_NCSI_No_PHY                          ( 1 << 10 )   // Can not find NCSI PHY
#define Err_Flag_NCSI_Channel_Num                     ( 1 << 11 )   // NCSI Channel Number Mismatch
#define Err_Flag_NCSI_Package_Num                     ( 1 << 12 )   // NCSI Package Number Mismatch
#define Err_Flag_PHY_TimeOut_RW                       ( 1 << 13 )   // Time out of read/write PHY register
#define Err_Flag_PHY_TimeOut_Rst                      ( 1 << 14 )   // Time out of reset PHY register
#define Err_Flag_RXBUF_UNAVA                          ( 1 << 15 )   // MAC00h[2]:Receiving buffer unavailable
#define Err_Flag_RPKT_LOST                            ( 1 << 16 )   // MAC00h[3]:Received packet lost due to RX FIFO full
#define Err_Flag_NPTXBUF_UNAVA                        ( 1 << 17 )   // MAC00h[6]:Normal priority transmit buffer unavailable
#define Err_Flag_TPKT_LOST                            ( 1 << 18 )   // MAC00h[7]:Packets transmitted to Ethernet lost
#define Err_Flag_DMABufNum                            ( 1 << 19 )   // DMA Buffer is not enough
#define Err_Flag_IOMargin                             ( 1 << 20 )   // IO timing margin is not enough
#define Err_Flag_IOMarginOUF                          ( 1 << 21 )   // IO timing testing out of boundary
#define Err_Flag_MHCLK_Ratio                          ( 1 << 22 )   // Error setting of MAC AHB bus clock (SCU08[18:16])

#define Wrn_Flag_IOMarginOUF                          ( 1 <<  0 )   // IO timing testing out of boundary
#define Wrn_Flag_RxErFloatting                        ( 1 <<  1 )   // NCSI RXER pin may be floatting to the MAC
//#define Wrn_Flag_RMIICK_IOMode                        ( 1 <<  2 )   // The PHY's RMII refreence clock input/output mode

#define PHY_Flag_RMIICK_IOMode_RTL8201E               ( 1 <<  0 )
#define PHY_Flag_RMIICK_IOMode_RTL8201F               ( 1 <<  1 )

#define Des_Flag_TxOwnTimeOut                         ( 1 <<  0 )   // Time out of checking Tx owner bit
#define Des_Flag_RxOwnTimeOut                         ( 1 <<  1 )   // Time out of checking Rx owner bit
#define Des_Flag_FrameLen                             ( 1 <<  2 )   // Frame length mismatch
#define Des_Flag_RxErr                                ( 1 <<  3 )   // Input signal RxErr
#define Des_Flag_CRC                                  ( 1 <<  4 )   // CRC error of frame
#define Des_Flag_FTL                                  ( 1 <<  5 )   // Frame too long
#define Des_Flag_Runt                                 ( 1 <<  6 )   // Runt packet
#define Des_Flag_OddNibble                            ( 1 <<  7 )   // Nibble bit happen
#define Des_Flag_RxFIFOFull                           ( 1 <<  8 )   // Rx FIFO full

#define NCSI_Flag_Get_Version_ID                      ( 1 <<  0 )   // Time out when Get Version ID
#define NCSI_Flag_Get_Capabilities                    ( 1 <<  1 )   // Time out when Get Capabilities
#define NCSI_Flag_Select_Active_Package               ( 1 <<  2 )   // Time out when Select Active Package
#define NCSI_Flag_Enable_Set_MAC_Address              ( 1 <<  3 )   // Time out when Enable Set MAC Address
#define NCSI_Flag_Enable_Broadcast_Filter             ( 1 <<  4 )   // Time out when Enable Broadcast Filter
#define NCSI_Flag_Enable_Network_TX                   ( 1 <<  5 )   // Time out when Enable Network TX
#define NCSI_Flag_Enable_Channel                      ( 1 <<  6 )   // Time out when Enable Channel
#define NCSI_Flag_Disable_Network_TX                  ( 1 <<  7 )   // Time out when Disable Network TX
#define NCSI_Flag_Disable_Channel                     ( 1 <<  8 )   // Time out when Disable Channel
#define NCSI_Flag_Select_Package                      ( 1 <<  9 )   // Time out when Select Package
#define NCSI_Flag_Deselect_Package                    ( 1 << 10 )   // Time out when Deselect Package
#define NCSI_Flag_Set_Link                            ( 1 << 11 )   // Time out when Set Link
#define NCSI_Flag_Get_Controller_Packet_Statistics    ( 1 << 12 )   // Time out when Get Controller Packet Statistics
#define NCSI_Flag_Reset_Channel                       ( 1 << 13 )   // Time out when Reset Channel

//---------------------------------------------------------
// SCU information
//---------------------------------------------------------
#define SMB_BASE				0x1e720000
#define SCU_BASE				0x1e6e2000
#define SDR_BASE				0x1e6e0000
#define WDT_BASE				0x1e785000
#define TIMER_BASE				0x1e782000
#define GPIO_BASE				0x1e780000

#define SCU_48h_AST1010				0x00000200
#define SCU_48h_AST2300				0x00222255
#define SCU_48h_AST2500				0x00082208
#define SCU_B8h_AST2500				0x00082208
#define SCU_BCh_AST2500				0x00082208

//#define SCU_80h                                  0x0000000f     //AST2300[3:0]MAC1~4 PHYLINK
//#define SCU_88h                                  0xc0000000     //AST2300[31]MAC1 MDIO, [30]MAC1 MDC
//#define SCU_90h                                  0x00000004     //AST2300[2 ]MAC2 MDC/MDIO
//#define SCU_74h                                  0x06300000     //AST3000[20]MAC2 MDC/MDIO, [21]MAC2 MII, [25]MAC1 PHYLINK, [26]MAC2 PHYLINK

//---------------------------------------------------------
// DMA Buffer information
//---------------------------------------------------------
#ifdef FPGA
  #define DRAM_KByteSize			(56 * 1024)
#else  
  #define DRAM_KByteSize			(16500)         //16.11328125 K
#endif


#ifdef Enable_Jumbo
  #define DMA_PakSize                            ( 10 * 1024 )
#else
  #define DMA_PakSize                            ( 2 * 1024 ) // The size of one LAN packet
#endif

#ifdef SelectSimpleBoundary
  #define DMA_BufSize                            (     ( ( ( ( eng->dat.Des_Num + 15 ) * DMA_PakSize ) >> 2 ) << 2 ) ) //vary by Des_Num
#else
  #define DMA_BufSize                            ( 4 + ( ( ( ( eng->dat.Des_Num + 15 ) * DMA_PakSize ) >> 2 ) << 2 ) ) //vary by Des_Num
#endif

#define DMA_BufNum                               ( ( DRAM_KByteSize * 1024 ) / ( eng->dat.DMABuf_Size ) )                //vary by eng->dat.Des_Num
#define GET_DMA_BASE_SETUP                       ( DMA_BASE )
//#define GET_DMA_BASE(x)                          ( DMA_BASE + ( ( ( ( x ) % eng->dat.DMABuf_Num ) + 1 ) * eng->dat.DMABuf_Size ) )//vary by eng->dat.Des_Num
#define GET_DMA_BASE(x)                          ( DMA_BASE + ( ( ( ( x ) % eng->dat.DMABuf_Num ) + 1 ) * eng->dat.DMABuf_Size ) + ( ( ( x ) % 7 ) * DMA_PakSize ) )//vary by eng->dat.Des_Num

#define SEED_START                               8
#define DATA_SEED(seed)                          ( ( seed ) | (( seed + 1 ) << 16 ) )
#define DATA_IncVal                              0x00020001
//#define DATA_IncVal                              0x01000001     //fail
//#define DATA_IncVal                              0x10000001     //fail
//#define DATA_IncVal                              0x10000000     //fail
//#define DATA_IncVal                              0x80000000     //fail
//#define DATA_IncVal                              0x00000001     //ok
//#define DATA_IncVal                              0x01000100     //ok
//#define DATA_IncVal                              0x01010000     //ok
//#define DATA_IncVal                              0x01010101     //ok
//#define DATA_IncVal                              0x00000101     //ok
//#define DATA_IncVal                              0x00001111     //fail
//#define DATA_IncVal                              0x00000011     //fail
//#define DATA_IncVal                              0x10100101     //fail
//#define DATA_IncVal                              0xfeff0201
//#define DATA_IncVal                              0x00010001
#define PktByteSize                              ( ( ( ( ZeroCopy_OFFSET + eng->dat.FRAME_LEN_Cur - 1 ) >> 2 ) + 1) << 2 )

//---------------------------------------------------------
// Delay (ms)
//---------------------------------------------------------
//#define Delay_DesGap                             1    //off
//#define Delay_CntMax                             40
//#define Delay_CntMax                             1000
//#define Delay_CntMax                             8465
//#define Delay_CntMaxIncVal                       50000
#define Delay_CntMaxIncVal                       47500


//#define Delay_ChkRxOwn                           1
//#define Delay_ChkTxOwn                           1

#define Delay_PHYRst                             100
//#define Delay_PHYRd                              5
#define Delay_PHYRd                              1         //20150423

//#define Delay_SCU                                11
#define Delay_SCU                                1         //20150423
#define Delay_MACRst                             1
#define Delay_MACDump                            1

//#define Delay_DES                                1
#ifdef Enable_MAC_ExtLoop_PakcegMode
  #define Delay_CheckData                        100
  #define Delay_CheckData_LoopNum                100
#else
//  #define Delay_CheckData                        100
//  #define Delay_CheckData_LoopNum                100
#endif

//---------------------------------------------------------
// Time Out
//---------------------------------------------------------
#ifdef Enable_MAC_ExtLoop_PakcegMode
//    #define TIME_OUT_Des_1G                      40000
//    #define TIME_OUT_Des_100M                    400000
//    #define TIME_OUT_Des_10M                     2000000
    #define TIME_OUT_Des_1G                      0
    #define TIME_OUT_Des_100M                    0
    #define TIME_OUT_Des_10M                     0
    #define TIME_OUT_NCSI                        0
    #define TIME_OUT_PHY_RW                      10000
    #define TIME_OUT_PHY_Rst                     10000
#else
    #define TIME_OUT_Des_1G                      10000     //400
    #define TIME_OUT_Des_100M                    20000     //4000
    #define TIME_OUT_Des_10M                     50000     //20000
    #define TIME_OUT_NCSI                        100000    //40000
    #define TIME_OUT_PHY_RW                      2000000   //100000
    #define TIME_OUT_PHY_Rst                     20000     //1000
#endif
//#define TIME_OUT_PHY_RW                          10000
//#define TIME_OUT_PHY_Rst                         10000

////#define TIME_OUT_NCSI                            300000
//#define TIME_OUT_NCSI                            30000     //20150423

//---------------------------------------------------------
// Others
//---------------------------------------------------------
#define Loop_OverFlow                            0x7fffffff

//---------------------------------------------------------
// Chip memory MAP
//---------------------------------------------------------
#define LITTLE_ENDIAN_ADDRESS                    0
#define BIG_ENDIAN_ADDRESS                       1

typedef struct {
    uint32_t StartAddr;
    uint32_t EndAddr;
}  LittleEndian_Area;

static const LittleEndian_Area LittleEndianArea[] = {{ 0xFFFFFFFF, 0xFFFFFFFF }};

// ========================================================
// For ncsi.c

#define DEF_GPACKAGE2NUM                         1         // Default value
#define DEF_GCHANNEL2NUM                         2         // Default value

//---------------------------------------------------------
// Variable
//---------------------------------------------------------
//NC-SI Command Packet
typedef struct {
//Ethernet Header
	unsigned char        DA[6];                        // Destination Address
	unsigned char        SA[6];                        // Source Address
	uint16_t       EtherType;                    // DMTF NC-SI, it should be 0x88F8
//NC-SI Control Packet
	unsigned char        MC_ID;                        // Management Controller should set this field to 0x00
	unsigned char        Header_Revision;              // For NC-SI 1.0 spec, this field has to set 0x01
	unsigned char        Reserved_1;                   // Reserved has to set to 0x00
	unsigned char        IID;                          // Instance ID
	unsigned char        Command;
//	unsigned char        Channel_ID;
	unsigned char        ChID;
	uint16_t	Payload_Length;               // Payload Length = 12 bits, 4 bits are reserved
	uint32_t	Reserved_2;
	uint32_t	Reserved_3;

	uint16_t	Reserved_4;
	uint16_t	Reserved_5;
	uint16_t	Response_Code;
	uint16_t       Reason_Code;
	unsigned char        Payload_Data[64];
#if !defined(SLT_UBOOT)
}  NCSI_Command_Packet;
#else
}  __attribute__ ((__packed__)) NCSI_Command_Packet;
#endif

//NC-SI Response Packet
typedef struct {
	unsigned char        DA[6];
	unsigned char        SA[6];
	uint16_t       EtherType;                    //DMTF NC-SI
//NC-SI Control Packet
	unsigned char        MC_ID;                        //Management Controller should set this field to 0x00
	unsigned char        Header_Revision;              //For NC-SI 1.0 spec, this field has to set 0x01
	unsigned char        Reserved_1;                   //Reserved has to set to 0x00
	unsigned char        IID;                          //Instance ID
	unsigned char        Command;
//	unsigned char        Channel_ID;
	unsigned char        ChID;
	uint16_t       Payload_Length;               //Payload Length = 12 bits, 4 bits are reserved
	uint16_t       Reserved_2;
	uint16_t       Reserved_3;
	uint16_t       Reserved_4;
	uint16_t       Reserved_5;

	uint16_t       Response_Code;
	uint16_t       Reason_Code;
	unsigned char        Payload_Data[64];
#if !defined(SLT_UBOOT)
}  NCSI_Response_Packet;
#else
}  __attribute__ ((__packed__)) NCSI_Response_Packet;
#endif

typedef struct {
	unsigned char        All_ID                                   ;//__attribute__ ((aligned (4)));
	unsigned char        Package_ID                               ;//__attribute__ ((aligned (4)));
	unsigned char        Channel_ID                               ;//__attribute__ ((aligned (4)));
	uint32_t Capabilities_Flags                       ;//__attribute__ ((aligned (4)));
	uint32_t Broadcast_Packet_Filter_Capabilities     ;//__attribute__ ((aligned (4)));
	uint32_t Multicast_Packet_Filter_Capabilities     ;//__attribute__ ((aligned (4)));
	uint32_t Buffering_Capabilities                   ;//__attribute__ ((aligned (4)));
	uint32_t AEN_Control_Support                      ;//__attribute__ ((aligned (4)));
	unsigned char        VLAN_Filter_Count                        ;//__attribute__ ((aligned (4)));
	unsigned char        Mixed_Filter_Count                       ;//__attribute__ ((aligned (4)));
	unsigned char        Multicast_Filter_Count                   ;//__attribute__ ((aligned (4)));
	unsigned char        Unicast_Filter_Count                     ;//__attribute__ ((aligned (4)));
	unsigned char        VLAN_Mode_Support                        ;//__attribute__ ((aligned (4)));
	unsigned char        Channel_Count                            ;//__attribute__ ((aligned (4)));
	uint32_t PCI_DID_VID                              ;//__attribute__ ((aligned (4)));
	uint32_t ManufacturerID                           ;//__attribute__ ((aligned (4)));
} NCSI_Capability;
typedef struct {
#ifdef CONFIG_ASPEED_AST2600
	uint32_t SCU_FPGASel                   ;//__attribute__ ((aligned (4)));
	uint32_t SCU_510                       ;//__attribute__ ((aligned (4)));
#endif
	uint32_t MAC_000                       ;//__attribute__ ((aligned (4)));
	uint32_t MAC_008                       ;//__attribute__ ((aligned (4)));
	uint32_t MAC_00c                       ;//__attribute__ ((aligned (4)));
	uint32_t MAC_040                       ;//__attribute__ ((aligned (4)));
	uint32_t MAC_040_new                   ;//__attribute__ ((aligned (4)));
	uint32_t MAC_050                       ;//__attribute__ ((aligned (4)));
	uint32_t MAC_050_Speed                 ;//__attribute__ ((aligned (4)));
	uint32_t SCU_004                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_004_mix                   ;//__attribute__ ((aligned (4)));
	uint32_t SCU_004_rstbit                ;//__attribute__ ((aligned (4)));
	uint32_t SCU_004_dis                   ;//__attribute__ ((aligned (4)));
	uint32_t SCU_004_en                    ;//__attribute__ ((aligned (4)));
	uint32_t SCU_008                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_00c                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_00c_mix                   ;//__attribute__ ((aligned (4)));
	uint32_t SCU_00c_clkbit                ;//__attribute__ ((aligned (4)));
	uint32_t SCU_00c_dis                   ;//__attribute__ ((aligned (4)));
	uint32_t SCU_00c_en                    ;//__attribute__ ((aligned (4)));
	uint32_t SCU_048                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_048_mix                   ;//__attribute__ ((aligned (4)));
	uint32_t SCU_048_default               ;//__attribute__ ((aligned (4)));
	uint32_t SCU_048_check                 ;//__attribute__ ((aligned (4)));
	uint32_t SCU_070                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_074                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_074_mix                   ;//__attribute__ ((aligned (4)));
	uint32_t SCU_07c                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_080                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_088                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_090                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_09c                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_0b8                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_0bc                       ;//__attribute__ ((aligned (4)));
	uint32_t SCU_0f0                       ;//__attribute__ ((aligned (4)));
	uint32_t WDT_00c                       ;//__attribute__ ((aligned (4)));
	uint32_t WDT_02c                       ;//__attribute__ ((aligned (4)));
	uint32_t WDT_04c                       ;//__attribute__ ((aligned (4)));

	CHAR                 SCU_oldvld                    ;//__attribute__ ((aligned (4)));
} MAC_Register;
typedef struct {
	CHAR                 ASTChipType                   ;//__attribute__ ((aligned (4)));
	CHAR                 ASTChipName[64]               ;//__attribute__ ((aligned (4)));
	CHAR                 AST1100                       ;//__attribute__ ((aligned (4)));//Different in phy & dram initiation & dram size & RMII
	CHAR                 AST2300                       ;//__attribute__ ((aligned (4)));
	CHAR                 AST2400                       ;//__attribute__ ((aligned (4)));
	CHAR                 AST1010                       ;//__attribute__ ((aligned (4)));
	CHAR                 AST2500                       ;//__attribute__ ((aligned (4)));
	CHAR                 AST2500A1                     ;//__attribute__ ((aligned (4)));

	CHAR                 MAC_Mode                      ;//__attribute__ ((aligned (4)));
	CHAR                 MAC1_1Gvld                    ;//__attribute__ ((aligned (4)));
	CHAR                 MAC2_1Gvld                    ;//__attribute__ ((aligned (4)));
#ifdef CONFIG_ASPEED_AST2600
	CHAR                 MAC3_1Gvld                    ;//__attribute__ ((aligned (4)));
	CHAR                 MAC4_1Gvld                    ;//__attribute__ ((aligned (4)));
#endif
	CHAR                 MAC1_RMII                     ;//__attribute__ ((aligned (4)));
	CHAR                 MAC2_RMII                     ;//__attribute__ ((aligned (4)));
#ifdef CONFIG_ASPEED_AST2600
	CHAR                 MAC3_RMII                     ;//__attribute__ ((aligned (4)));
	CHAR                 MAC4_RMII                     ;//__attribute__ ((aligned (4)));
#endif
	CHAR                 MAC_atlast_1Gvld              ;//__attribute__ ((aligned (4)));
	CHAR                 MAC2_vld                      ;//__attribute__ ((aligned (4)));
	CHAR                 MAC34_vld                     ;//__attribute__ ((aligned (4)));

	CHAR                 MAC_1Gvld                     ;//__attribute__ ((aligned (4)));
	CHAR                 MAC_RMII                      ;//__attribute__ ((aligned (4)));

	CHAR                 MHCLK_Ratio                   ;//__attribute__ ((aligned (4)));

	uint32_t VGAMode                       ;//__attribute__ ((aligned (4)));
	char                 VGAModeVld                    ;//__attribute__ ((aligned (4)));
} MAC_Environment;
typedef struct {
	uint32_t GARPNumCnt                    ;//__attribute__ ((aligned (4)));//argv     [6]
	uint32_t GUserDVal                     ;//__attribute__ ((aligned (4)));//argv[8]
	BYTE                 GChk_TimingBund               ;//__attribute__ ((aligned (4)));//argv[7]  [5]
	CHAR                 GPHYADR                       ;//__attribute__ ((aligned (4)));//argv[6]
	BYTE                 GTestMode                     ;//__attribute__ ((aligned (4)));//argv[5]  [4]
	CHAR                 GLOOP_Str[32]                 ;//__attribute__ ((aligned (4)));//argv[4]
	BYTE                 GCtrl                         ;//__attribute__ ((aligned (4)));//argv[3]  [7]
	BYTE                 GSpeed                        ;//__attribute__ ((aligned (4)));//argv[2]
	BYTE                 GChannelTolNum                ;//__attribute__ ((aligned (4)));//argv     [3]
	BYTE                 GPackageTolNum                ;//__attribute__ ((aligned (4)));//argv     [2]
	BYTE                 GRun_Mode                     ;//__attribute__ ((aligned (4)));//argv[1]  [1]

	CHAR                 GIEEE_sel                     ;//__attribute__ ((aligned (4)));//argv[7]
	CHAR                 GLOOP_INFINI                  ;//__attribute__ ((aligned (4)));//argv[4]
	uint32_t GLOOP_MAX                     ;//__attribute__ ((aligned (4)));//argv[4]

	CHAR                 GEn_SkipRxEr                  ;//__attribute__ ((aligned (4)));//GCtrl    [1]
	CHAR                 GEn_PrintNCSI                 ;//__attribute__ ((aligned (4)));//GCtrl    [0]
	CHAR                 GEn_RMIIPHY_IN                ;//__attribute__ ((aligned (4)));//GCtrl[9]
	CHAR                 GEn_RMII_50MOut               ;//__attribute__ ((aligned (4)));//GCtrl[8] [8]
	CHAR                 GEn_MACLoopback               ;//__attribute__ ((aligned (4)));//GCtrl[7] [7]
	CHAR                 GEn_FullRange                 ;//__attribute__ ((aligned (4)));//GCtrl[6] [6]
	CHAR                 GEn_SkipChkPHY                ;//__attribute__ ((aligned (4)));//GCtrl[5]
	CHAR                 GEn_IntLoopPHY                ;//__attribute__ ((aligned (4)));//GCtrl[4]
	CHAR                 GEn_InitPHY                   ;//__attribute__ ((aligned (4)));//GCtrl[3]
	CHAR                 GDis_RecovPHY                 ;//__attribute__ ((aligned (4)));//GCtrl[2]
	CHAR                 GEn_PHYAdrInv                 ;//__attribute__ ((aligned (4)));//GCtrl[1]
	CHAR                 GEn_SinglePacket              ;//__attribute__ ((aligned (4)));//GCtrl[0]
} MAC_Argument;
typedef struct {
	CHAR                 MAC_idx                       ;//__attribute__ ((aligned (4)));//GRun_Mode
	CHAR                 MAC_idx_PHY                   ;//__attribute__ ((aligned (4)));//GRun_Mode
	uint32_t MAC_BASE                      ;//__attribute__ ((aligned (4)));//GRun_Mode

	CHAR                 Speed_1G                      ;//__attribute__ ((aligned (4)));//GSpeed
	CHAR                 Speed_org[3]                  ;//__attribute__ ((aligned (4)));//GSpeed
	CHAR                 Speed_sel[3]                  ;//__attribute__ ((aligned (4)));
	CHAR                 Speed_idx                     ;//__attribute__ ((aligned (4)));

	CHAR                 TM_Burst                      ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_IEEE                       ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_IOTiming                   ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_IOStrength                 ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_TxDataEn                   ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_RxDataEn                   ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_WaitStart                  ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_DefaultPHY                 ;//__attribute__ ((aligned (4)));//GTestMode
	CHAR                 TM_NCSI_DiSChannel            ;//__attribute__ ((aligned (4)));//GTestMode

	BYTE                 IO_Bund                       ;//__attribute__ ((aligned (4)));
	CHAR                 IO_MrgChk                     ;//__attribute__ ((aligned (4)));

	uint32_t TDES_BASE                     ;//__attribute__ ((aligned (4)));
	uint32_t RDES_BASE                     ;//__attribute__ ((aligned (4)));

	uint32_t NCSI_TxDesBase                ;//__attribute__ ((aligned (4)));
	uint32_t NCSI_RxDesBase                ;//__attribute__ ((aligned (4)));
	int                  NCSI_RxTimeOutScale           ;//__attribute__ ((aligned (4)));

	int                  LOOP_MAX                      ;//__attribute__ ((aligned (4)));
	uint32_t LOOP_CheckNum                 ;//__attribute__ ((aligned (4)));
	uint32_t CheckBuf_MBSize               ;//__attribute__ ((aligned (4)));
	uint32_t TIME_OUT_Des                  ;//__attribute__ ((aligned (4)));
	uint32_t TIME_OUT_Des_PHYRatio         ;//__attribute__ ((aligned (4)));

	int                  Loop_ofcnt                    ;//__attribute__ ((aligned (4)));
	int                  Loop                          ;//__attribute__ ((aligned (4)));
	int                  Loop_rl[3]                    ;//__attribute__ ((aligned (4)));
} MAC_Running;
typedef struct {
	CHAR                 SA[6]                         ;//__attribute__ ((aligned (4)));
	CHAR                 NewMDIO                       ;//__attribute__ ((aligned (4))); //start from AST2300
} MAC_Information;
typedef struct {
	uint32_t PHY_BASE                      ;//__attribute__ ((aligned (4)));
	int                  loop_phy                      ;//__attribute__ ((aligned (4)));
	CHAR                 default_phy                   ;//__attribute__ ((aligned (4)));
	CHAR                 Adr                           ;//__attribute__ ((aligned (4)));

	CHAR                 PHYName[64]                   ;//__attribute__ ((aligned (4)));
	uint32_t PHY_ID3                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_ID2                       ;//__attribute__ ((aligned (4)));

	uint32_t PHY_00h                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_06h                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_09h                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_0eh                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_10h                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_11h                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_12h                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_14h                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_15h                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_18h                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_19h                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_1ch                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_1eh                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_1fh                       ;//__attribute__ ((aligned (4)));
	uint32_t PHY_06hA[7]                   ;//__attribute__ ((aligned (4)));
	BOOLEAN              PHYAdrValid                   ;//__attribute__ ((aligned (4)));

	uint32_t RMIICK_IOMode                 ;//__attribute__ ((aligned (4)));
} MAC_PHY;
typedef struct {
	CHAR                 init_done                     ;//__attribute__ ((aligned (4)));

	BYTE                 Dly_MrgEn                     ;//__attribute__ ((aligned (4)));
	CHAR                 Dly_3Regiser                  ;//__attribute__ ((aligned (4)));

	uint32_t Str_reg_idx                   ;//__attribute__ ((aligned (4)));
	BYTE                 Str_reg_Lbit                  ;//__attribute__ ((aligned (4)));
	BYTE                 Str_reg_Hbit                  ;//__attribute__ ((aligned (4)));
	uint32_t Str_reg_value                 ;//__attribute__ ((aligned (4)));
	uint32_t Str_reg_mask                  ;//__attribute__ ((aligned (4)));
	BYTE                 Str_max                       ;//__attribute__ ((aligned (4)));
	BYTE                 Str_shf                       ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_stagebit                  ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_stage                     ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_stage_in                  ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_stage_out                 ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_step                      ;//__attribute__ ((aligned (4)));
	uint32_t Dly_mask_bit_in               ;//__attribute__ ((aligned (4)));
	uint32_t Dly_mask_bit_out              ;//__attribute__ ((aligned (4)));
	uint32_t Dly_mask_pos                  ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_shf                    ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_shf                   ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_shf_regH               ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_shf_regH              ;//__attribute__ ((aligned (4)));
	BYTE                 value_ary[64]                 ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_stage_shf_i               ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_stage_shf_o               ;//__attribute__ ((aligned (4)));

	uint32_t Dly_reg_idx                   ;//__attribute__ ((aligned (4)));
	uint32_t Dly_reg_value                 ;//__attribute__ ((aligned (4)));
	char                 Dly_reg_name_tx[32]           ;//__attribute__ ((aligned (4)));
	char                 Dly_reg_name_rx[32]           ;//__attribute__ ((aligned (4)));
	char                 Dly_reg_name_tx_new[32]       ;//__attribute__ ((aligned (4)));
	char                 Dly_reg_name_rx_new[32]       ;//__attribute__ ((aligned (4)));
	uint32_t Dly_in_reg                    ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_reg_idx                ;//__attribute__ ((aligned (4)));
	SCHAR                Dly_in_min                    ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_max                    ;//__attribute__ ((aligned (4)));
	uint32_t Dly_out_reg                   ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_reg_idx               ;//__attribute__ ((aligned (4)));
	SCHAR                Dly_out_min                   ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_max                   ;//__attribute__ ((aligned (4)));
	uint32_t Dly_in_cval                   ;//__attribute__ ((aligned (4)));
	SCHAR                Dly_in_str                    ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_end                    ;//__attribute__ ((aligned (4)));
	uint32_t Dly_out_cval                  ;//__attribute__ ((aligned (4)));
	SCHAR                Dly_out_str                   ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_end                   ;//__attribute__ ((aligned (4)));

	BYTE                 Str_i                         ;//__attribute__ ((aligned (4)));
	uint32_t Str_val                       ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in                        ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_in_selval                 ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out                       ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_selval                ;//__attribute__ ((aligned (4)));
	uint32_t Dly_val                       ;//__attribute__ ((aligned (4)));
	BYTE                 Dly_out_reg_hit               ;//__attribute__ ((aligned (4)));
	CHAR                 Dly_result                    ;//__attribute__ ((aligned (4)));
	CHAR                 dlymap[64][64]                ;//__attribute__ ((aligned (4)));
} MAC_IO;
typedef struct {
#ifdef Enable_ShowBW
	double               Total_frame_len               ;//__attribute__ ((aligned (8)));
#endif
	uint32_t Des_Num                       ;//__attribute__ ((aligned (4)));
	uint32_t Des_Num_Org                   ;//__attribute__ ((aligned (4)));
	uint32_t DMABuf_Size                   ;//__attribute__ ((aligned (4)));
	uint32_t DMABuf_Num                    ;//__attribute__ ((aligned (4)));

	uint32_t *FRAME_LEN                    ;//__attribute__ ((aligned (4)));
	uint32_t FRAME_LEN_Cur                 ;//__attribute__ ((aligned (4)));
	uint32_t *wp_lst                       ;//__attribute__ ((aligned (4)));
	uint32_t wp_fir                        ;//__attribute__ ((aligned (4)));

	uint32_t DMA_Base_Setup                 ;//__attribute__ ((aligned (4)));
	uint32_t DMA_Base_Tx                  ;//__attribute__ ((aligned (4)));
	uint32_t DMA_Base_Rx                   ;//__attribute__ ((aligned (4)));

	uint32_t ARP_data[16]                  ;//__attribute__ ((aligned (4)));
	uint32_t TxDes0DW                      ;//__attribute__ ((aligned (4)));
	uint32_t RxDes0DW                      ;//__attribute__ ((aligned (4)));
	uint32_t RxDes3DW                      ;//__attribute__ ((aligned (4)));

	BYTE                 number_chl                    ;//__attribute__ ((aligned (4)));
	BYTE                 number_pak                    ;//__attribute__ ((aligned (4)));
	char                 NCSI_RxEr                     ;//__attribute__ ((aligned (4)));
	uint32_t NCSI_TxDWBUF[512]             ;//__attribute__ ((aligned (4)));
	uint32_t NCSI_RxDWBUF[512]             ;//__attribute__ ((aligned (4)));
	char                 NCSI_CommandStr[512]          ;//__attribute__ ((aligned (4)));
	unsigned char        *NCSI_TxByteBUF               ;//__attribute__ ((aligned (4)));
	unsigned char        *NCSI_RxByteBUF               ;//__attribute__ ((aligned (4)));
	unsigned char        NCSI_Payload_Data[16]         ;//__attribute__ ((aligned (4)));
	uint32_t Payload_Checksum_NCSI         ;//__attribute__ ((aligned (4)));
} MAC_Data;
typedef struct {
	uint32_t Wrn_Flag                      ;//__attribute__ ((aligned (4)));
	uint32_t Err_Flag                      ;//__attribute__ ((aligned (4)));
	uint32_t Des_Flag                      ;//__attribute__ ((aligned (4)));
	uint32_t NCSI_Flag                     ;//__attribute__ ((aligned (4)));
	uint32_t Bak_Err_Flag                  ;//__attribute__ ((aligned (4)));
	uint32_t Bak_NCSI_Flag                 ;//__attribute__ ((aligned (4)));
	CHAR                 Flag_PrintEn                  ;//__attribute__ ((aligned (4)));
	uint32_t CheckDesFail_DesNum           ;//__attribute__ ((aligned (4)));
	CHAR                 AllFail                       ;//__attribute__ ((aligned (4)));
} MAC_Flag;
typedef struct {
	MAC_Register         reg;
	MAC_Environment      env;
	MAC_Argument         arg;
	MAC_Running          run;
	MAC_Information      inf;
	MAC_PHY              phy;
	MAC_IO               io;
	MAC_Data             dat;
	MAC_Flag             flg;
	NCSI_Command_Packet  ncsi_req;
	NCSI_Response_Packet ncsi_rsp;
	NCSI_Capability      ncsi_cap;

	PHY_GPIOstr          GPIO;
	PHY_BCMIMP           BCMIMP;
	CHAR                 ModeSwitch;
} MAC_ENGINE;
typedef void (* PHY_SETTING) (MAC_ENGINE *);
typedef struct {
	PHY_SETTING          fp_set;
	PHY_SETTING          fp_clr;
} PHY_ENGINE;

#undef GLOBAL
#ifdef NCSI_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

GLOBAL  char phy_ncsi (MAC_ENGINE *eng);

// ========================================================
// For mactest

#undef GLOBAL
#ifdef MACTEST_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

#define MODE_DEDICATED                           0x01
#define MODE_NSCI                                0x02

GLOBAL  UCHAR            *mmiobase;
GLOBAL  uint32_t ulPCIBaseAddress;
GLOBAL  uint32_t ulMMIOBaseAddress;

GLOBAL  BYTE             display_lantest_log_msg;

// ========================================================
// For mac.c
#undef GLOBAL
#ifdef MAC_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

#if defined(MAC_C)
static  const  char version_name[] = VER_NAME;
static  const  BYTE IOValue_Array_A0[16] = {8,1, 10,3, 12,5, 14,7, 0,9, 2,11, 4,13, 6,15}; // AST2300-A0
#endif

GLOBAL void    debug_pause (void);
GLOBAL uint32_t Read_Mem_Dat_NCSI_DD (uint32_t addr);
GLOBAL uint32_t Read_Mem_Des_NCSI_DD (uint32_t addr);
GLOBAL uint32_t Read_Mem_Dat_DD (uint32_t addr);
GLOBAL uint32_t Read_Mem_Des_DD (uint32_t addr);
GLOBAL uint32_t Read_Reg_MAC_DD (MAC_ENGINE *eng, uint32_t addr);
GLOBAL uint32_t Read_Reg_PHY_DD (MAC_ENGINE *eng, uint32_t addr);
GLOBAL uint32_t Read_Reg_SCU_DD_AST2600 (uint32_t addr);
GLOBAL uint32_t Read_Reg_SCU_DD (uint32_t addr);
GLOBAL uint32_t Read_Reg_WDT_DD (uint32_t addr);
GLOBAL uint32_t Read_Reg_SDR_DD (uint32_t addr);
GLOBAL uint32_t Read_Reg_SMB_DD (uint32_t addr);
GLOBAL uint32_t Read_Reg_TIMER_DD (uint32_t addr);
GLOBAL uint32_t Read_Reg_GPIO_DD (uint32_t addr);
GLOBAL void Write_Mem_Dat_NCSI_DD (uint32_t addr, uint32_t data);
GLOBAL void Write_Mem_Des_NCSI_DD (uint32_t addr, uint32_t data);
GLOBAL void Write_Mem_Dat_DD (uint32_t addr, uint32_t data);
GLOBAL void Write_Mem_Des_DD (uint32_t addr, uint32_t data);
GLOBAL void Write_Reg_MAC_DD (MAC_ENGINE *eng, uint32_t addr, uint32_t data);
GLOBAL void Write_Reg_PHY_DD (MAC_ENGINE *eng, uint32_t addr, uint32_t data);
GLOBAL void Write_Reg_SCU_DD_AST2600 (uint32_t addr, uint32_t data);
GLOBAL void Write_Reg_SCU_DD (uint32_t addr, uint32_t data);
GLOBAL void Write_Reg_WDT_DD (uint32_t addr, uint32_t data);
GLOBAL void Write_Reg_TIMER_DD (uint32_t addr, uint32_t data);
GLOBAL void Write_Reg_GPIO_DD (uint32_t addr, uint32_t data);
GLOBAL void    init_iodelay (MAC_ENGINE *eng);
GLOBAL int     get_iodelay (MAC_ENGINE *eng);
GLOBAL void    read_scu (MAC_ENGINE *eng);
GLOBAL void    Setting_scu (MAC_ENGINE *eng);
GLOBAL void    PrintMode (MAC_ENGINE *eng);
GLOBAL void    PrintPakNUm (MAC_ENGINE *eng);
GLOBAL void    PrintChlNUm (MAC_ENGINE *eng);
GLOBAL void    PrintTest (MAC_ENGINE *eng);
GLOBAL void    PrintIOTimingBund (MAC_ENGINE *eng);
GLOBAL void    PrintSpeed (MAC_ENGINE *eng);
GLOBAL void    PrintCtrl (MAC_ENGINE *eng);
GLOBAL void    PrintLoop (MAC_ENGINE *eng);
GLOBAL void    PrintPHYAdr (MAC_ENGINE *eng);
GLOBAL void    Finish_Close (MAC_ENGINE *eng);
GLOBAL void    Calculate_LOOP_CheckNum (MAC_ENGINE *eng);
GLOBAL char    Finish_Check (MAC_ENGINE *eng, int value);
GLOBAL void    init_scu1 (MAC_ENGINE *eng);
GLOBAL void    init_scu_macio (MAC_ENGINE *eng);
GLOBAL void    init_scu_macrst (MAC_ENGINE *eng);
GLOBAL void    init_scu_macdis (MAC_ENGINE *eng);
GLOBAL void    init_scu_macen (MAC_ENGINE *eng);
GLOBAL void    setup_arp (MAC_ENGINE *eng);
GLOBAL void    TestingSetup (MAC_ENGINE *eng);
GLOBAL void    init_scu2 (MAC_ENGINE *eng);
GLOBAL void    init_scu3 (MAC_ENGINE *eng);
GLOBAL void    get_mac_info (MAC_ENGINE *eng);
GLOBAL void    init_mac (MAC_ENGINE *eng);
GLOBAL char TestingLoop (MAC_ENGINE *eng, uint32_t loop_checknum);
GLOBAL void    PrintIO_Line_LOG (MAC_ENGINE *eng);
GLOBAL void    init_phy (MAC_ENGINE *eng, PHY_ENGINE *phyeng);
GLOBAL BOOLEAN find_phyadr (MAC_ENGINE *eng);
GLOBAL void phy_write (MAC_ENGINE *eng, int adr, uint32_t data);
GLOBAL uint32_t phy_read (MAC_ENGINE *eng, int adr);
GLOBAL void    phy_sel (MAC_ENGINE *eng, PHY_ENGINE *phyeng);
GLOBAL void    recov_phy (MAC_ENGINE *eng, PHY_ENGINE *phyeng);
GLOBAL int     FindErr (MAC_ENGINE *eng, int value);
GLOBAL int     FindErr_Des (MAC_ENGINE *eng, int value);
GLOBAL void    PrintIO_Header (MAC_ENGINE *eng, BYTE option);
GLOBAL void    Print_Header (MAC_ENGINE *eng, BYTE option);
GLOBAL void    PrintIO_LineS (MAC_ENGINE *eng, BYTE option);
GLOBAL void    PrintIO_Line (MAC_ENGINE *eng, BYTE option);
GLOBAL void    FPri_ErrFlag (MAC_ENGINE *eng, BYTE option);

GLOBAL void init_hwtimer( void );
GLOBAL void delay_hwtimer(uint16_t msec);

#ifdef SUPPORT_PHY_LAN9303
// ========================================================
// For LAN9303.c
#undef GLOBAL
#ifdef LAN9303_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

GLOBAL void LAN9303(int num, int phy_adr, int speed, int int_loopback);
#endif // SUPPORT_PHY_LAN9303

// ========================================================
// For PHYGPIO.c
#undef GLOBAL
#ifdef PHYGPIO_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

#if defined(PHY_GPIO)
GLOBAL void    phy_gpio_init( MAC_ENGINE *eng );
GLOBAL void    phy_gpio_write( MAC_ENGINE *eng, int regadr, int wrdata );
GLOBAL uint32_t phy_gpio_read( MAC_ENGINE *eng, int regadr );
#endif

// ========================================================
// For PHYSPECIAL.c
#undef GLOBAL
#ifdef PHYMISC_C
#define GLOBAL
#else
#define GLOBAL    extern
#endif

#ifdef PHY_SPECIAL
GLOBAL void    special_PHY_init (MAC_ENGINE *eng);
GLOBAL void    special_PHY_MDIO_init (MAC_ENGINE *eng);
GLOBAL void    special_PHY_buf_init (MAC_ENGINE *eng);
GLOBAL void    special_PHY_recov (MAC_ENGINE *eng);
GLOBAL void    special_PHY_reg_init (MAC_ENGINE *eng);
GLOBAL void    special_PHY_debug (MAC_ENGINE *eng);
GLOBAL uint32_t special_PHY_FRAME_LEN (MAC_ENGINE *eng);
GLOBAL uint32_t *special_PHY_txpkt_ptr (MAC_ENGINE *eng);
GLOBAL uint32_t *special_PHY_rxpkt_ptr (MAC_ENGINE *eng);
#endif

#endif // End COMMINF_H
