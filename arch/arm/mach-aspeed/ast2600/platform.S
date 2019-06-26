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
/*
 * Board specific setup info
 *
 ******************************************************************************
 * ASPEED Technology Inc.
 * AST26x0 DDR3/DDR4 SDRAM controller initialization sequence for FPGA
 *
 * Version     : 2
 * Release date: 2019.02.19
 *
 * Priority of fix item:
 * [P1] = critical
 * [P2] = nice to have
 * [P3] = minor
 *
 * Change List :
 * V0 |2018.03.28 : 1.[P1] Initial release for simulation
 *
 * Optional define variable
 * 1. ECC Function enable
 *    ASTMMC_DRAM_ECC             // define to enable ECC function
 *    ASTMMC_DRAM_ECC_SIZE        // define the ECC protected memory size
 * 2. UART5 message output        //
 *    ASTMMC_UART_BASE            // select UART port base
 * 3. DRAM Type
 *    ASTMMC_DDR4_8GX8            // DDR4 (16Gb) 8Gbit X8 stacked part
 ******************************************************************************
 */

#include <config.h>
#include <version.h>
#include <asm/secure.h>
#include <asm/armv7.h>

#ifdef CONFIG_CPU_ARM1176
#define ASTMMC_DDR_DDR3
#else
#define ASTMMC_READ_TRAINING
#endif

/******************************************************************************
 Calibration Macro Start
 Usable registers:
  r0, r1, r2, r3, r5, r6, r7, r8, r9, r10, r11
 ******************************************************************************/
#define ASTMMC_INIT_VER      0x02                // 8bit verison number
#define ASTMMC_INIT_DATE     0x20190219          // Release date

/* PATTERN_TABLE,
   init_delay_timer,
   check_delay_timer,
   clear_delay_timer,
   print_hex_char,
   print_hex_byte,
   print_hex_word,
   print_hex_dword,
   are for DRAM calibration */

#define ASTMMC_UART_BASE     0x1E784000

#ifdef CONFIG_DRAM_ECC
#define ASTMMC_DRAM_ECC
#define ASTMMC_DRAM_ECC_SIZE CONFIG_DRAM_ECC_SIZE
#else
#define ASTMMC_DRAM_ECC_SIZE 0x0
#endif

#define ASTMMC_REG_MCR10     0x00
#define ASTMMC_REG_MCR14     0x04
#define ASTMMC_REG_MCR18     0x08
#define ASTMMC_REG_MCR1C     0x0C
#define ASTMMC_REG_MCR20     0x10
#define ASTMMC_REG_MCR24     0x14
#define ASTMMC_REG_MCR28     0x18
#define ASTMMC_REG_MCR2C     0x1C
#define ASTMMC_REG_RFC       0x20

TIME_TABLE_DDR3:
    .word   0x02070306       // MCR10
    .word   0x05021133       // MCR14
    .word   0x06010200       // MCR18
    .word   0x00000020       // MCR1C
    .word   0x00071320       // MCR20
    .word   0x00000200       // MCR24
    .word   0x00000000       // MCR28
    .word   0x00000000       // MCR2C
    .word   0x17263434       // MCRFC
TIME_TABLE_DDR4:
    .word   0x030C0207       // MCR10
    .word   0x04451133       // MCR14
    .word   0x0E010200       // MCR18
    .word   0x00000140       // MCR1C
    .word   0x03010100       // MCR20
    .word   0x00000000       // MCR24
    .word   0x04C00000       // MCR28
    .word   0x00000050       // MCR2C
    .word   0x17263434       // MCRFC

PATTERN_TABLE:
    .word   0xff00ff00
    .word   0xcc33cc33
    .word   0xaa55aa55
    .word   0x88778877
    .word   0x92cc4d6e       // 5
    .word   0x543d3cde
    .word   0xf1e843c7
    .word   0x7c61d253
    .word   0x00000000       // 8

    .macro init_delay_timer
    ldr   r0, =0x1e782024                        // Set Timer3 Reload
    str   r2, [r0]

    ldr   r0, =0x1e782034                        // Clear Timer3 ISR
    ldr   r1, =0x00000004
    str   r1, [r0]

    ldr   r0, =0x1e782030                        // Enable Timer3
    mov   r2, #7
    mov   r1, r2, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e782034                        // Check ISR for Timer3 timeout
    .endm

    .macro check_delay_timer
    ldr   r1, [r0]
    bic   r1, r1, #0xFFFFFFFB
    mov   r2, r1, lsr #2
    cmp   r2, #0x01
    .endm

    .macro clear_delay_timer
    ldr   r0, =0x1e78203C                        // Disable Timer3
    mov   r2, #0xF
    mov   r1, r2, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e782034                        // Clear Timer3 ISR
    ldr   r1, =0x00000004
    str   r1, [r0]
    .endm

    .macro print_hex_char
    and   r1, r1, #0xF
    cmp   r1, #9
    addgt r1, r1, #0x37
    addle r1, r1, #0x30
    str   r1, [r0]
    .endm

    .macro print_hex_byte
    ldr   r0, =ASTMMC_UART_BASE
    mov   r1, r2, lsr #4
    print_hex_char
    mov   r1, r2
    print_hex_char
    .endm

    .macro print_hex_word
    ldr   r0, =ASTMMC_UART_BASE
    mov   r1, r2, lsr #12
    print_hex_char
    mov   r1, r2, lsr #8
    print_hex_char
    mov   r1, r2, lsr #4
    print_hex_char
    mov   r1, r2
    print_hex_char
    .endm

    .macro print_hex_dword
    ldr   r0, =ASTMMC_UART_BASE
    mov   r1, r2, lsr #28
    print_hex_char
    mov   r1, r2, lsr #24
    print_hex_char
    mov   r1, r2, lsr #20
    print_hex_char
    mov   r1, r2, lsr #16
    print_hex_char
    mov   r1, r2, lsr #12
    print_hex_char
    mov   r1, r2, lsr #8
    print_hex_char
    mov   r1, r2, lsr #4
    print_hex_char
    mov   r1, r2
    print_hex_char
    .endm

/******************************************************************************
 Calibration Macro End
 ******************************************************************************/

.globl lowlevel_init
lowlevel_init:

#ifndef CONFIG_CPU_ARM1176
  /* Put secondary core to sleep */
  mrc   p15, 0, r0, c0, c0, 5                  @; Read CPU ID register
  ands  r0, r0, #0x03                          @; Mask off, leaving the CPU ID field
#ifdef CONFIG_ASPEED_NONSECUR_MODE
  blne	secondary_cpu_init
#else
  blne  relocate
  @blne  wait_for_kickup
  b     init_uart
#endif

#if 1
relocate:
    adrl  r0, wait_for_kickup
    ldr   r1, =0x1000f000     //  ; r1 = pointer to destination block
    mov   r2, #0x20           //  ; r2 = number of words to copy
wordcopy:
    ldr   r3, [r0], #4         // ; load a word from the source and
    str   r3, [r1], #4         // ; store it to the destination
    subs  r2, r2, #1           // ; decrement the counter
    bne   wordcopy             //1 ; ... copy more

   	ldr r0, =0x1E6E2180
	LDR r1, =0x1e784000   	
	ldr r4, =0xABBAADDA
	ldr r3, =0x1E6E2184

   	ldr   r5, =0x10000000
   	ldr   r6, =0x1000f000
   	str   r6, [r5]
	mov   lr, r6
	mov   pc, lr
#endif
#endif

init_uart:

#ifdef CONFIG_SPL_BUILD
    /* leave the rest to U-Boot proper */
    mov   r1, #0
    ldr   r0, =0x1e78500c
    str   r1, [r0]
    ldr   r0, =0x1e78504c
    str   r1, [r0]
    ldr   r0, =0x1e78508c
    str   r1, [r0]
    ldr   r0, =0x1e7850cc
    str   r1, [r0]

    mov   pc, lr
#endif

    /* save lr */
    mov   r4, lr

    /*Initialize the Debug UART here*/
    ldr   r0, =(ASTMMC_UART_BASE | 0x0c)
    mov   r1, #0x83
    str   r1, [r0]

    ldr   r0, =(ASTMMC_UART_BASE | 0x00)
    mov   r1, #0x01
    str   r1, [r0]

    ldr   r0, =(ASTMMC_UART_BASE | 0x04)
    mov   r1, #0x00
    str   r1, [r0]

    ldr   r0, =(ASTMMC_UART_BASE | 0x0c)
    mov   r1, #0x03
    str   r1, [r0]

    ldr   r0, =(ASTMMC_UART_BASE | 0x08)
    mov   r1, #0x07
    str   r1, [r0]

init_dram:

/* Test - DRAM initial time */
    ldr   r0, =0x1e78203c
    ldr   r1, =0x0000F000
    str   r1, [r0]

    ldr   r0, =0x1e782044
    ldr   r1, =0xFFFFFFFF
    str   r1, [r0]

    ldr   r0, =0x1e782030
    ldr   r1, =0x00003000
    str   r1, [r0]
/* Test - DRAM initial time */

    /*Set Scratch register Bit 7 before initialize*/
    ldr   r0, =0x1e6e2000
    ldr   r1, =0x1688a8a8
    str   r1, [r0]
    ldr   r0, =0x1e6e2010
    str   r1, [r0]

/*  ldr   r0, =0x1e6e2100
    ldr   r1, [r0]
    orr   r1, r1, #0x80
    str   r1, [r0]
*/
/******************************************************************************
 Disable WDT for SPI Address mode detection function
 ******************************************************************************/
    ldr   r0, =0x1e620060
    mov   r1, #0
    str   r1, [r0]

    ldr   r0, =0x1e620064
    mov   r1, #0
    str   r1, [r0]

    ldr   r0, =0x1e78500c
    mov   r1, #0
    str   r1, [r0]
    ldr   r0, =0x1e78504c
    str   r1, [r0]
    ldr   r0, =0x1e78508c
    str   r1, [r0]
    ldr   r0, =0x1e7850cc
    str   r1, [r0]

#ifdef CONFIG_CPU_ARM1176
    /* Enable AXI_P */
/*  ldr   r0, =0x00000016
    mrc   p15, 0, r1, c15, c2, 4
    mcr   p15, 0, r0, c15, c2, 4
*/
init_arm11:
    /* Start of ES40004A PLL init */
    /* Step 1. Program PLL_config and keep power down */
    ldr   r0, =0x33000000
    ldr   r1, =0x01000000
    str   r1, [r0]
    ldr   r1, =0x0102001A                        @ 324 MHz
    str   r1, [r0]

    /* Step 2. Wait 1us for PLL initialization */
    ldr   r2, =0x00000100
delay_ES40004A_pll_init:
    subs  r2, r2, #1
    bne   delay_ES40004A_pll_init

    /* Step 3. Program PLL_config to exit Power down */
    ldr   r1, =0x0002001A
    str   r1, [r0]

    /* Step 4. Check pll_ld = 1?. Read PLL_config, check bit 27. */
    ldr   r2, =0x08000000                        @ bit[27] PLL lock detection
check_pll_ld:
    ldr   r1, [r0]
    tst   r1, r2
    beq   check_pll_ld

    /* Step 5. Program aclk_div */
    ldr   r0, =0x33000004
    ldr   r1, =0x00000007                        @ CPU/AXI = 8/1
    str   r1, [r0]

    /* Step 6. Program set_pll */
    ldr   r1, =0x00010007
    str   r1, [r0]
    /* End of ES40004A PLL init */
#endif
    
    /* skip SDRAM initialization (will be done in C function) */
    b     platform_exit

platform_exit:
    /* restore lr */
    mov   lr, r4

    /* back to arch calling code */
    mov   pc, lr

secondary_cpu_init:
#ifdef CONFIG_ASPEED_NONSECUR_MODE
    mov   r6,pc
	bl    start_sec
#endif
wait_for_kickup:
	wfe
	ldr r2,[r3]
	cmp r2,r4
	bne wait_for_kickup

	MOV r2,#'['
	STR r2,[r1]
	MOV r2,#'1'
	STR r2,[r1]
	MOV r2,#'C'
	STR r2,[r1]
	MOV r2,#'P'
	STR r2,[r1]
	MOV r2,#'U'
	STR r2,[r1]
	MOV r2,#']'
	STR r2,[r1]
	MOV r2,#'\n'
	STR r2,[r1]
	MOV r2,#'\r'
	STR r2,[r1]
	ldr pc, [r0]
	ldr pc, [r0]
	b wait_for_kickup
