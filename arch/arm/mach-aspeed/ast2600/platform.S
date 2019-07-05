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
 *
 ******************************************************************************
 */

#include <config.h>
#include <version.h>
#include <asm/secure.h>
#include <asm/armv7.h>

/*
 *       SMP mailbox
 * +----------------------+
 * |                      |
 * | mailbox insn. for    |
 * | cpuN polling SMP go  |
 * |                      |
 * +----------------------+ 0xC
 * | mailbox ready signal |
 * +----------------------+ 0x8
 * | cpuN GO signal       |
 * +----------------------+ 0x4
 * | cpuN entrypoint      |
 * +----------------------+ AST_SMP_MAILBOX_BASE
 */

#define AST_SMP_MAILBOX_BASE            0x1e6e2180
#define AST_SMP_MBOX_FIELD_ENTRY        (AST_SMP_MAILBOX_BASE + 0x0)
#define AST_SMP_MBOX_FIELD_GOSIGN       (AST_SMP_MAILBOX_BASE + 0x4)
#define AST_SMP_MBOX_FIELD_READY        (AST_SMP_MAILBOX_BASE + 0x8)
#define AST_SMP_MBOX_FIELD_POLLINSN     (AST_SMP_MAILBOX_BASE + 0xC)

#define AST_UART_BASE       0x1e784000

#define AST_SCU_BASE        0x1e6e2000
#define AST_SCU_PROT_KEY    (AST_SCU_BASE)
#define AST_SCU_REV_ID      (AST_SCU_BASE + 0x014)

#define REV_ID_AST2600A0    0x05000303

.macro timer_init
#ifdef CONFIG_FPGA_ASPEED
    movw    r0, #0x0
    movt    r0, #0x2500
#else
    ldr     r0, =AST_SCU_REV_ID
    ldr     r0, [r0]

    ldr     r1, =REV_ID_AST2600A0
    cmp     r0, r1

    movweq  r0, #0x0800
    movteq  r0, #0x2faf
    movwne  r0, #0x8c00
    movtne  r0, #0x4786
#endif
    /* write CNTFRQ */
    mcr     p15, 0, r0, c14, c0, 0
.endm


.globl lowlevel_init

lowlevel_init:

#if defined(CONFIG_SPL) && !defined(CONFIG_SPL_BUILD)
    mov   pc, lr
#else
    /* setup timer frequency for ARM generic timer */
    timer_init

    /*
     * we treat cpu0 as the primary core and
     * put secondary core (cpuN) to sleep
     */
    mrc   p15, 0, r0, c0, c0, 5                  @; Read CPU ID register
    ands  r0, r0, #0x03                          @; Mask off, leaving the CPU ID field

    beq   do_primary_core_setup

    /* hold cpuN until mailbox is ready */
poll_mailbox_ready:
    wfe
    ldr   r0, =AST_SMP_MBOX_FIELD_READY
    ldr   r1, =0xBABECAFE
    ldr   r2, [r0]
    cmp   r1, r2
    bne   poll_mailbox_ready

    /* parameters for relocated SMP go polling insn. */
    ldr   r0, =AST_SMP_MBOX_FIELD_GOSIGN
    ldr   r1, =AST_SMP_MBOX_FIELD_ENTRY
    ldr   r2, =0xABBAADDA
    ldr   r3, =AST_UART_BASE

    /* no return */
    ldr   pc, =AST_SMP_MBOX_FIELD_POLLINSN

do_primary_core_setup:

#ifdef CONFIG_FPGA_ASPEED
    /* fpga unlock scu */
    ldr   r0, =AST_SCU_PROT_KEY
    ldr   r1, =0x1688a8a8
    str   r1, [r0]
#endif

    /* disable WDT */
    mov   r1, #0
    ldr   r0, =0x1e78500c
    str   r1, [r0]
    ldr   r0, =0x1e78504c
    str   r1, [r0]
    ldr   r0, =0x1e78508c
    str   r1, [r0]
    ldr   r0, =0x1e7850cc
    str   r1, [r0]

    /* relocate mailbox insn. for cpuN polling SMP go signal */
    adrl  r0, mailbox_insn
    adrl  r1, mailbox_insn_end

    ldr   r2, =#AST_SMP_MBOX_FIELD_POLLINSN

relocate_mailbox_insn:
    ldr   r3, [r0], #0x4
    str   r3, [r2], #0x4
    cmp   r0, r1
    bne   relocate_mailbox_insn

    /* notify cpuN mailbox is ready */
    ldr   r0, =AST_SMP_MBOX_FIELD_READY
    ldr   r1, =0xBABECAFE
    str   r1, [r0]
    sev

    /* back to arch calling code */
    mov   pc, lr

/*
 * insn. inside mailbox to poll SMP go signal.
 *
 * Note that as this code will be relocated, any
 * pc-relative assembly should NOT be used.
 */
mailbox_insn:
    /*
     * r0 ~ r3 are parameters:
     *   r0 = AST_SMP_MBOX_FIELD_GOSIGN
     *   r1 = AST_SMP_MBOX_FIELD_ENTRY
     *   r2 = 0xABBAADDA
     *   r3 = AST_UART_BASE (for debug purpose)
     */
poll_mailbox_smp_go:
    wfe
    ldr   r4, [r0]
    cmp   r2, r4
    bne   poll_mailbox_smp_go

    /* debug message */
    mov   r4, #'C'
    str   r4, [r3]
    mov   r4, #'P'
    str   r4, [r3]
    mov   r4, #'U'
    str   r4, [r3]
    mov   r4, #'1'
    str   r4, [r3]
    mov   r4, #'\r'
    str   r4, [r3]
    mov   r4, #'\n'
    str   r4, [r3]

    /* SMP GO signal confirmed, release cpuN */
    ldr   pc, [r1]

mailbox_insn_end:
    /* should never reach */
    b .

#endif
