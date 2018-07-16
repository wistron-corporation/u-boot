/*
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000,2001,2002 Wolfgang Denk <wd@denx.de>
 *  Copyright (C) 2007 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *  Copyright (c) 2008 Nuovation System Designs, LLC
 *    Grant Erickson <gerickson@nuovations.com>
 *
 * SPDX-License-Identifier:	GPL-2.0	IBM-pibs
 */

/*
 * Startup code for IBM/AMCC PowerPC 4xx (PPC4xx) based boards
 *
 * The following description only applies to the NOR flash style booting.
 * NAND booting is different. For more details about NAND booting on 4xx
 * take a look at doc/README.nand-boot-ppc440.
 *
 * The CPU starts at address 0xfffffffc (last word in the address space).
 * The U-Boot image therefore has to be located in the "upper" area of the
 * flash (e.g. 512MiB - 0xfff80000 ... 0xffffffff). The default value for
 * the boot chip-select (CS0) is quite big and covers this area. On the
 * 405EX this is for example 0xffe00000 ... 0xffffffff. U-Boot will
 * reconfigure this CS0 (and other chip-selects as well when configured
 * this way) in the boot process to the "correct" values matching the
 * board layout.
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/ppc4xx.h>
#include <version.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>
#include <asm/ppc4xx-isram.h>

#define PPC_128MB_SACR_BIT(addr)	((addr) >> 27)
#define PPC_128MB_SACR_VALUE(addr)	PPC_REG_VAL(PPC_128MB_SACR_BIT(addr),1)
/* tlbtab should be nop in case of FSP2 because tlb already set in IPL,
   just dummy call */
#if defined(CONFIG_SOC_IBM_FSP2)
.section .bootpg,"ax"
.globl tlbtab

tlbtab:
	nop
#endif

#ifdef CONFIG_SYS_INIT_DCACHE_CS
# if (CONFIG_SYS_INIT_DCACHE_CS == 0)
#  define PBxAP PB1AP
#  define PBxCR PB0CR
#  if (defined(CONFIG_SYS_EBC_PB0AP) && defined(CONFIG_SYS_EBC_PB0CR))
#   define PBxAP_VAL CONFIG_SYS_EBC_PB0AP
#   define PBxCR_VAL CONFIG_SYS_EBC_PB0CR
#  endif
# endif
# if (CONFIG_SYS_INIT_DCACHE_CS == 1)
#  define PBxAP PB1AP
#  define PBxCR PB1CR
#  if (defined(CONFIG_SYS_EBC_PB1AP) && defined(CONFIG_SYS_EBC_PB1CR))
#   define PBxAP_VAL CONFIG_SYS_EBC_PB1AP
#   define PBxCR_VAL CONFIG_SYS_EBC_PB1CR
#  endif
# endif
# if (CONFIG_SYS_INIT_DCACHE_CS == 2)
#  define PBxAP PB2AP
#  define PBxCR PB2CR
#  if (defined(CONFIG_SYS_EBC_PB2AP) && defined(CONFIG_SYS_EBC_PB2CR))
#   define PBxAP_VAL CONFIG_SYS_EBC_PB2AP
#   define PBxCR_VAL CONFIG_SYS_EBC_PB2CR
#  endif
# endif
# if (CONFIG_SYS_INIT_DCACHE_CS == 3)
#  define PBxAP PB3AP
#  define PBxCR PB3CR
#  if (defined(CONFIG_SYS_EBC_PB3AP) && defined(CONFIG_SYS_EBC_PB3CR))
#   define PBxAP_VAL CONFIG_SYS_EBC_PB3AP
#   define PBxCR_VAL CONFIG_SYS_EBC_PB3CR
#  endif
# endif
# if (CONFIG_SYS_INIT_DCACHE_CS == 4)
#  define PBxAP PB4AP
#  define PBxCR PB4CR
#  if (defined(CONFIG_SYS_EBC_PB4AP) && defined(CONFIG_SYS_EBC_PB4CR))
#   define PBxAP_VAL CONFIG_SYS_EBC_PB4AP
#   define PBxCR_VAL CONFIG_SYS_EBC_PB4CR
#  endif
# endif
# if (CONFIG_SYS_INIT_DCACHE_CS == 5)
#  define PBxAP PB5AP
#  define PBxCR PB5CR
#  if (defined(CONFIG_SYS_EBC_PB5AP) && defined(CONFIG_SYS_EBC_PB5CR))
#   define PBxAP_VAL CONFIG_SYS_EBC_PB5AP
#   define PBxCR_VAL CONFIG_SYS_EBC_PB5CR
#  endif
# endif
# if (CONFIG_SYS_INIT_DCACHE_CS == 6)
#  define PBxAP PB6AP
#  define PBxCR PB6CR
#  if (defined(CONFIG_SYS_EBC_PB6AP) && defined(CONFIG_SYS_EBC_PB6CR))
#   define PBxAP_VAL CONFIG_SYS_EBC_PB6AP
#   define PBxCR_VAL CONFIG_SYS_EBC_PB6CR
#  endif
# endif
# if (CONFIG_SYS_INIT_DCACHE_CS == 7)
#  define PBxAP PB7AP
#  define PBxCR PB7CR
#  if (defined(CONFIG_SYS_EBC_PB7AP) && defined(CONFIG_SYS_EBC_PB7CR))
#   define PBxAP_VAL CONFIG_SYS_EBC_PB7AP
#   define PBxCR_VAL CONFIG_SYS_EBC_PB7CR
#  endif
# endif
# ifndef PBxAP_VAL
#  define PBxAP_VAL	0
# endif
# ifndef PBxCR_VAL
#  define PBxCR_VAL	0
# endif
/*
 * Memory Bank x (nothingness) initialization CONFIG_SYS_INIT_RAM_ADDR + 64 MiB
 * used as temporary stack pointer for the primordial stack
 */
# ifndef CONFIG_SYS_INIT_DCACHE_PBxAR
#  define CONFIG_SYS_INIT_DCACHE_PBxAR	(EBC_BXAP_BME_DISABLED			| \
				 EBC_BXAP_TWT_ENCODE(7)			| \
				 EBC_BXAP_BCE_DISABLE			| \
				 EBC_BXAP_BCT_2TRANS			| \
				 EBC_BXAP_CSN_ENCODE(0)			| \
				 EBC_BXAP_OEN_ENCODE(0)			| \
				 EBC_BXAP_WBN_ENCODE(0)			| \
				 EBC_BXAP_WBF_ENCODE(0)			| \
				 EBC_BXAP_TH_ENCODE(2)			| \
				 EBC_BXAP_RE_DISABLED			| \
				 EBC_BXAP_SOR_NONDELAYED		| \
				 EBC_BXAP_BEM_WRITEONLY			| \
				 EBC_BXAP_PEN_DISABLED)
# endif /* CONFIG_SYS_INIT_DCACHE_PBxAR */
# ifndef CONFIG_SYS_INIT_DCACHE_PBxCR
#  define CONFIG_SYS_INIT_DCACHE_PBxCR	(EBC_BXCR_BAS_ENCODE(CONFIG_SYS_INIT_RAM_ADDR)	| \
				 EBC_BXCR_BS_64MB			| \
				 EBC_BXCR_BU_RW				| \
				 EBC_BXCR_BW_16BIT)
# endif /* CONFIG_SYS_INIT_DCACHE_PBxCR */
# ifndef CONFIG_SYS_INIT_RAM_PATTERN
#  define CONFIG_SYS_INIT_RAM_PATTERN	0xDEADDEAD
# endif
#endif /* CONFIG_SYS_INIT_DCACHE_CS */

#if (defined(CONFIG_SYS_INIT_RAM_DCACHE) && (CONFIG_SYS_INIT_RAM_SIZE > (4 << 10)))
#error Only 4k of init-ram is supported - please adjust CONFIG_SYS_INIT_RAM_SIZE!
#endif

/*
 * Unless otherwise overridden, enable two 128MB cachable instruction regions
 * at CONFIG_SYS_SDRAM_BASE and another 128MB cacheable instruction region covering
 * NOR flash at CONFIG_SYS_FLASH_BASE. Disable all cacheable data regions.
 */
#if !defined(CONFIG_SYS_FLASH_BASE)
/* If not already defined, set it to the "last" 128MByte region */
# define CONFIG_SYS_FLASH_BASE		0xf8000000
#endif
#if !defined(CONFIG_SYS_ICACHE_SACR_VALUE)
# define CONFIG_SYS_ICACHE_SACR_VALUE		\
		(PPC_128MB_SACR_VALUE(CONFIG_SYS_SDRAM_BASE + (  0 << 20)) | \
		 PPC_128MB_SACR_VALUE(CONFIG_SYS_SDRAM_BASE + (128 << 20)) | \
		 PPC_128MB_SACR_VALUE(CONFIG_SYS_FLASH_BASE))
#endif /* !defined(CONFIG_SYS_ICACHE_SACR_VALUE) */

#if !defined(CONFIG_SYS_DCACHE_SACR_VALUE)
# define CONFIG_SYS_DCACHE_SACR_VALUE		\
		(0x00000000)
#endif /* !defined(CONFIG_SYS_DCACHE_SACR_VALUE) */

#if !defined(CONFIG_SYS_TLB_FOR_BOOT_FLASH)
#define CONFIG_SYS_TLB_FOR_BOOT_FLASH	0	/* use TLB 0 as default */
#endif

#define function_prolog(func_name)	.text; \
					.align 2; \
					.globl func_name; \
					func_name:
#define function_epilog(func_name)	.type func_name,@function; \
					.size func_name,.-func_name

/* We don't want the  MMU yet.
*/
#undef	MSR_KERNEL
#define MSR_KERNEL ( MSR_ME  )	/* Machine Check */


	.extern ext_bus_cntlr_init

/*
 * Set up GOT: Global Offset Table
 *
 * Use r12 to access the GOT
 */
#if !defined(CONFIG_SPL_BUILD)
	START_GOT
	GOT_ENTRY(_GOT2_TABLE_)
	GOT_ENTRY(_FIXUP_TABLE_)

	GOT_ENTRY(_start)
	GOT_ENTRY(_start_of_vectors)
	GOT_ENTRY(_end_of_vectors)
	GOT_ENTRY(transfer_to_handler)

	GOT_ENTRY(__init_end)
	GOT_ENTRY(__bss_end)
	GOT_ENTRY(__bss_start)
	END_GOT
#endif /* CONFIG_SPL_BUILD */

#if defined(CONFIG_SYS_RAMBOOT) || defined(CONFIG_BOOT_FROM_XMD)
	/*
	 * 4xx RAM-booting U-Boot image is started from offset 0
	 */
	.text
	bl	_start_440
#endif

#if defined(CONFIG_SPL) && !defined(CONFIG_SPL_BUILD)
	/*
	 * This is the entry of the real U-Boot from a board port
	 * that supports SPL booting on the PPC4xx. We only need
	 * to call board_init_f() here. Everything else has already
	 * been done in the SPL u-boot version.
	 */
	GET_GOT			/* initialize GOT access		*/
	bl	board_init_f	/* run 1st part of board init code (in Flash)*/
	/* NOTREACHED - board_init_f() does not return */
#endif

/*
 * 440 Startup -- on reset only the top 4k of the effective
 * address space is mapped in by an entry in the instruction
 * and data shadow TLB. The .bootpg section is located in the
 * top 4k & does only what's necessary to map in the the rest
 * of the boot rom. Once the boot rom is mapped in we can
 * proceed with normal startup.
 *
 * NOTE: CS0 only covers the top 2MB of the effective address
 * space after reset.
 */

    .section .bootpg,"ax"
    .globl _start_440

/**************************************************************************/
_start_440:
	/*----------------------------------------------------------------+
	| Core bug fix.  Clear the esr
	+-----------------------------------------------------------------*/
	li	r0,0
	mtspr	SPRN_ESR,r0
	/*----------------------------------------------------------------*/
	/* Clear and set up some registers. */
	/*----------------------------------------------------------------*/
	iccci	r0,r0		/* NOTE: operands not used for 440 */
	dccci	r0,r0		/* NOTE: operands not used for 440 */
	sync
	li	r0,0
	mtspr	SPRN_SRR0,r0
	mtspr	SPRN_SRR1,r0
	mtspr	SPRN_CSRR0,r0
	mtspr	SPRN_CSRR1,r0
	mtspr	SPRN_MCSRR0,r0
	mtspr	SPRN_MCSRR1,r0
	mfspr	r1,SPRN_MCSR
	mtspr	SPRN_MCSR_C,r1

	/*----------------------------------------------------------------*/
	/* CCR0 init */
	/*----------------------------------------------------------------*/
	/* Disable store gathering & broadcast, guarantee inst/data
	* cache block touch, force load/store alignment
	* (see errata 1.12: 440_33)
	*/
	lis	r1,0x0030	/* store gathering & broadcast disable */
	ori	r1,r1,0x6000	/* cache touch */
#if defined(CONFIG_SOC_IBM_FSP2)
	ori	r1,r1,0x0040	/* btac disable for ppc476 */
#endif
	mtspr	SPRN_CCR0,r1

	/*----------------------------------------------------------------*/
	/* Initialize debug */
	/*----------------------------------------------------------------*/
	mfspr	r1,SPRN_DBCR0
	andis.	r1, r1, 0x8000	/* test DBCR0[EDM] bit			*/
	bne	skip_debug_init	/* if set, don't clear debug register	*/
	mfspr	r1,SPRN_CCR0
	ori	r1,r1,CCR0_DTB@l /* Disable Trace Broadcast */
	mtspr	SPRN_CCR0,r1
	mtspr	SPRN_DBCR0,r0
	mtspr	SPRN_DBCR1,r0
	mtspr	SPRN_DBCR2,r0
	mtspr	SPRN_IAC1,r0
	mtspr	SPRN_IAC2,r0
	mtspr	SPRN_IAC3,r0
	mtspr	SPRN_DAC1,r0
	mtspr	SPRN_DAC2,r0
	mtspr	SPRN_DVC1,r0
	mtspr	SPRN_DVC2,r0

	mfspr	r1,SPRN_DBSR
#if defined(CONFIG_SOC_IBM_FSP2)
	lis	r0,0x3000	/* don't clear DBSR bits on FSP2 */
	andc	r1,r1,r0
	li	r0,0
#endif


	mtspr	SPRN_DBSR,r1	/* Clear all valid bits */
skip_debug_init:

	/*----------------------------------------------------------------*/
	/* Setup interrupt vectors */
	/*----------------------------------------------------------------*/
	mtspr	SPRN_IVPR,r0		/* Vectors start at 0x0000_0000 */
	li	r1,0x0100
	mtspr	SPRN_IVOR0,r1	/* Critical input */
	li	r1,0x0200
	mtspr	SPRN_IVOR1,r1	/* Machine check */
	li	r1,0x0300
	mtspr	SPRN_IVOR2,r1	/* Data storage */
	li	r1,0x0400
	mtspr	SPRN_IVOR3,r1	/* Instruction storage */
	li	r1,0x0500
	mtspr	SPRN_IVOR4,r1	/* External interrupt */
	li	r1,0x0600
	mtspr	SPRN_IVOR5,r1	/* Alignment */
	li	r1,0x0700
	mtspr	SPRN_IVOR6,r1	/* Program check */
	li	r1,0x0800
	mtspr	SPRN_IVOR7,r1	/* Floating point unavailable */
	li	r1,0x0c00
	mtspr	SPRN_IVOR8,r1	/* System call */
	li	r1,0x0a00
	mtspr	SPRN_IVOR9,r1	/* Auxiliary Processor unavailable */
	li	r1,0x0900
	mtspr	SPRN_IVOR10,r1	/* Decrementer */
	li	r1,0x1300
	mtspr	SPRN_IVOR13,r1	/* Data TLB error */
	li	r1,0x1400
	mtspr	SPRN_IVOR14,r1	/* Instr TLB error */
	li	r1,0x2000
	mtspr	SPRN_IVOR15,r1	/* Debug */

	/*----------------------------------------------------------------*/
	/* Configure cache regions  */
	/*----------------------------------------------------------------*/
#ifndef CONFIG_SOC_IBM_FSP2
	mtspr	SPRN_INV0,r0
	mtspr	SPRN_INV1,r0
	mtspr	SPRN_INV2,r0
	mtspr	SPRN_INV3,r0
	mtspr	SPRN_DNV0,r0
	mtspr	SPRN_DNV1,r0
	mtspr	SPRN_DNV2,r0
	mtspr	SPRN_DNV3,r0
	mtspr	SPRN_ITV0,r0
	mtspr	SPRN_ITV1,r0
	mtspr	SPRN_ITV2,r0
	mtspr	SPRN_ITV3,r0
	mtspr	SPRN_DTV0,r0
	mtspr	SPRN_DTV1,r0
	mtspr	SPRN_DTV2,r0
	mtspr	SPRN_DTV3,r0
	/*----------------------------------------------------------------*/
	/* Cache victim limits */
	/*----------------------------------------------------------------*/
	/* floors 0, ceiling max to use the entire cache -- nothing locked
	*/
	lis	r1,0x0001
	ori	r1,r1,0xf800
	mtspr	SPRN_IVLIM,r1
	mtspr	SPRN_DVLIM,r1
#endif
	/*----------------------------------------------------------------+
	|Initialize MMUCR[STID] = 0.
	+-----------------------------------------------------------------*/
	mfspr	r0,SPRN_MMUCR
	addis	r1,0,0xFFFF
	ori	r1,r1,0xFF00
	and	r0,r0,r1
	mtspr	SPRN_MMUCR,r0

/* The IPL has already cleared/setup the TLB for FSP2 */
#ifndef CONFIG_SOC_IBM_FSP2
	/*----------------------------------------------------------------*/
	/* Clear all TLB entries -- TID = 0, TS = 0 */
	/*----------------------------------------------------------------*/
	addis	r0,0,0x0000
#ifdef CONFIG_SYS_RAMBOOT
	li	r4,0		/* Start with TLB #0 */
#else
	li	r4,1		/* Start with TLB #1 */
#endif
	li	r1,64		/* 64 TLB entries */
	sub	r1,r1,r4	/* calculate last TLB # */
	mtctr	r1
rsttlb:
#ifdef CONFIG_SYS_RAMBOOT
	tlbre	r3,r4,0		/* Read contents from TLB word #0 to get EPN */
	rlwinm.	r3,r3,0,0xfffffc00	/* Mask EPN */
	beq	tlbnxt		/* Skip EPN=0 TLB, this is the SDRAM TLB */
#endif
	tlbwe	r0,r4,0		/* Invalidate all entries (V=0)*/
	tlbwe	r0,r4,1
	tlbwe	r0,r4,2
tlbnxt:	addi	r4,r4,1		/* Next TLB */
	bdnz	rsttlb

	/*----------------------------------------------------------------*/
	/* TLB entry setup -- step thru tlbtab */
	/*----------------------------------------------------------------*/
	bl	tlbtab		/* Get tlbtab pointer */
	mr	r5,r0
	li	r1,0x003f	/* 64 TLB entries max */
	mtctr	r1
	li	r4,0		/* TLB # */

	addi	r5,r5,-4
1:
#ifdef CONFIG_SYS_RAMBOOT
	tlbre	r3,r4,0		/* Read contents from TLB word #0 */
	rlwinm.	r3,r3,0,0x00000200	/* Mask V (valid) bit */
	bne	tlbnx2		/* Skip V=1 TLB, this is the SDRAM TLB */
#endif
	lwzu	r0,4(r5)
	cmpwi	r0,0
	beq	2f		/* 0 marks end */
	lwzu	r1,4(r5)
	lwzu	r2,4(r5)
	tlbwe	r0,r4,0		/* TLB Word 0 */
	tlbwe	r1,r4,1		/* TLB Word 1 */
	tlbwe	r2,r4,2		/* TLB Word 2 */
tlbnx2:	addi	r4,r4,1		/* Next TLB */
	bdnz	1b
#endif /* CONFIG_SOC_IBM_FSP2 */

	/*----------------------------------------------------------------*/
	/* Continue from 'normal' start */
	/*----------------------------------------------------------------*/
2:
	bl	3f
	b	_start

3:	li	r0,0
	mtspr	SPRN_SRR1,r0		/* Keep things disabled for now */
	mflr	r1
	mtspr	SPRN_SRR0,r1
	rfi

/*
 * r3 - 1st arg to board_init(): IMMP pointer
 * r4 - 2nd arg to board_init(): boot flag
 */
#if !defined(CONFIG_SPL_BUILD)
	.text
	.long	0x27051956		/* U-Boot Magic Number			*/
	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION_STRING, "\0"

	. = EXC_OFF_SYS_RESET
	.globl	_start_of_vectors
_start_of_vectors:

/* Critical input. */
	CRIT_EXCEPTION(0x100, CritcalInput, UnknownException)

/* Machine check */
	MCK_EXCEPTION(0x200, MachineCheck, MachineCheckException)

/* Data Storage exception. */
	STD_EXCEPTION(0x300, DataStorage, UnknownException)

/* Instruction Storage exception. */
	STD_EXCEPTION(0x400, InstStorage, UnknownException)

/* External Interrupt exception. */
	STD_EXCEPTION(0x500, ExtInterrupt, external_interrupt)

/* Alignment exception. */
	. = 0x600
Alignment:
	EXCEPTION_PROLOG(SRR0, SRR1)
	mfspr	r4,DAR
	stw	r4,_DAR(r21)
	mfspr	r5,DSISR
	stw	r5,_DSISR(r21)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	EXC_XFER_TEMPLATE(Alignment, AlignmentException, MSR_KERNEL, COPY_EE)

/* Program check exception */
	. = 0x700
ProgramCheck:
	EXCEPTION_PROLOG(SRR0, SRR1)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	EXC_XFER_TEMPLATE(ProgramCheck, ProgramCheckException,
		MSR_KERNEL, COPY_EE)

	STD_EXCEPTION(0x800, FPUnavailable, UnknownException)
	STD_EXCEPTION(0x900, Decrementer, DecrementerPITException)
	STD_EXCEPTION(0xa00, APU, UnknownException)
	STD_EXCEPTION(0xc00, SystemCall, UnknownException)

	STD_EXCEPTION(0x1300, DataTLBError, UnknownException)
	STD_EXCEPTION(0x1400, InstructionTLBError, UnknownException)
	CRIT_EXCEPTION(0x2000, DebugBreakpoint, DebugException )

	.globl	_end_of_vectors
_end_of_vectors:
	. = _START_OFFSET
#endif
	.globl	_start
_start:

#if defined(CONFIG_SPL) && !defined(CONFIG_SPL_BUILD)
	/*
	 * This is the entry of the real U-Boot from a board port
	 * that supports SPL booting on the PPC4xx. We only need
	 * to call board_init_f() here. Everything else has already
	 * been done in the SPL u-boot version.
	 */
	GET_GOT			/* initialize GOT access		*/
	bl	board_init_f	/* run 1st part of board init code (in Flash)*/
	/* NOTREACHED - board_init_f() does not return */
#endif

	/*----------------------------------------------------------------*/
	/* Clear and set up some registers. */
	/*----------------------------------------------------------------*/
	li	r0,0x0000
	lis	r1,0xffff
	mtspr	SPRN_DEC,r0			/* prevent dec exceptions */
	mtspr	SPRN_TBWL,r0			/* prevent fit & wdt exceptions */
	mtspr	SPRN_TBWU,r0
	mtspr	SPRN_TSR,r1			/* clear all timer exception status */
	mtspr	SPRN_TCR,r0			/* disable all */
	mtspr	SPRN_ESR,r0			/* clear exception syndrome register */
	mtxer	r0			/* clear integer exception register */

	/*----------------------------------------------------------------*/
	/* Debug setup -- some (not very good) ice's need an event*/
	/* to establish control :-( Define CONFIG_SYS_INIT_DBCR to the dbsr */
	/* value you need in this case 0x8cff 0000 should do the trick */
	/*----------------------------------------------------------------*/
#if defined(CONFIG_SYS_INIT_DBCR)
	lis	r1,0xffff
	ori	r1,r1,0xffff
	mtspr	SPRN_DBSR,r1			/* Clear all status bits */
	lis	r0,CONFIG_SYS_INIT_DBCR@h
	ori	r0,r0,CONFIG_SYS_INIT_DBCR@l
	mtspr	SPRN_DBCR0,r0
	isync
#endif

	/*----------------------------------------------------------------*/
	/* Setup the internal SRAM */
	/*----------------------------------------------------------------*/
	li	r0,0

#ifdef CONFIG_SYS_INIT_RAM_DCACHE
	/* Clear Dcache to use as RAM */
	addis	r3,r0,CONFIG_SYS_INIT_RAM_ADDR@h
	ori	r3,r3,CONFIG_SYS_INIT_RAM_ADDR@l
	addis	r4,r0,CONFIG_SYS_INIT_RAM_SIZE@h
	ori	r4,r4,CONFIG_SYS_INIT_RAM_SIZE@l
	rlwinm. r5,r4,0,27,31
	rlwinm	r5,r4,27,5,31
	beq	..d_ran
	addi	r5,r5,0x0001
..d_ran:
	mtctr	r5
..d_ag:
	dcbz	r0,r3
	addi	r3,r3,32
	bdnz	..d_ag

	/*
	 * Lock the init-ram/stack in d-cache, so that other regions
	 * may use d-cache as well
	 * Note, that this current implementation locks exactly 4k
	 * of d-cache, so please make sure that you don't define a
	 * bigger init-ram area. Take a look at the lwmon5 440EPx
	 * implementation as a reference.
	 */
	msync
	isync
	/* 8. set TFLOOR/NFLOOR to 8 (-> 8*16*32 bytes locked -> 4k) */
	lis	r1,0x0201
	ori	r1,r1,0xf808
	mtspr	SPRN_DVLIM,r1
	lis	r1,0x0808
	ori	r1,r1,0x0808
	mtspr	SPRN_DNV0,r1
	mtspr	SPRN_DNV1,r1
	mtspr	SPRN_DNV2,r1
	mtspr	SPRN_DNV3,r1
	mtspr	SPRN_DTV0,r1
	mtspr	SPRN_DTV1,r1
	mtspr	SPRN_DTV2,r1
	mtspr	SPRN_DTV3,r1
	msync
	isync
#endif /* CONFIG_SYS_INIT_RAM_DCACHE */

#if !defined(CONFIG_SOC_IBM_FSP2)
	lis	r2,0x7fff
	ori	r2,r2,0xffff
	mfdcr	r1,ISRAM0_DPC
	and	r1,r1,r2		/* Disable parity check */
	mtdcr	ISRAM0_DPC,r1
	mfdcr	r1,ISRAM0_PMEG
	and	r1,r1,r2		/* Disable pwr mgmt */
	mtdcr	ISRAM0_PMEG,r1

	lis	r1,0x8000		/* BAS = 8000_0000 */
#endif /* #if !defined(CONFIG_FSP2) */

	/*----------------------------------------------------------------*/
	/* Setup the stack in internal SRAM */
	/*----------------------------------------------------------------*/
	lis	r1, (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)@h
	ori	r1, r1, (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)@l
	/*
	 * Reserve space for globals and store address for initialization
	 * with board_init_f_init_reserve() in r14
	 */
	mr	r3, r1
	bl	board_init_f_alloc_reserve
	mr	r1, r3
	mr	r14, r3
	li	r0,0
	stwu	r0,-4(r1)
	stwu	r0,-4(r1)		/* Terminate call chain */

	stwu	r1,-8(r1)		/* Save back chain and move SP */
	lis	r0,RESET_VECTOR@h	/* Address of reset vector */
	ori	r0,r0, RESET_VECTOR@l
	stwu	r1,-8(r1)		/* Save back chain and move SP */
	stw	r0,+12(r1)		/* Save return addr (underflow vect) */

#ifndef CONFIG_SPL_BUILD
	GET_GOT
#endif

	bl	cpu_init_f	/* run low-level CPU init code	   (from Flash) */
	/* address for globals was stored in r14 */
	mr	r3, r14
	bl	board_init_f_init_reserve
	li	r3, 0
	bl	board_init_f
	/* NOTREACHED - board_init_f() does not return */

#if !defined(CONFIG_SPL_BUILD)
/*
 * This code finishes saving the registers to the exception frame
 * and jumps to the appropriate handler for the exception.
 * Register r21 is pointer into trap frame, r1 has new stack pointer.
 */
	.globl	transfer_to_handler
transfer_to_handler:
	stw	r22,_NIP(r21)
	lis	r22,MSR_POW@h
	andc	r23,r23,r22
	stw	r23,_MSR(r21)
	SAVE_GPR(7, r21)
	SAVE_4GPRS(8, r21)
	SAVE_8GPRS(12, r21)
	SAVE_8GPRS(24, r21)
	mflr	r23
	andi.	r24,r23,0x3f00		/* get vector offset */
	stw	r24,TRAP(r21)
	li	r22,0
	stw	r22,RESULT(r21)
	mtspr	SPRG2,r22		/* r1 is now kernel sp */
	lwz	r24,0(r23)		/* virtual address of handler */
	lwz	r23,4(r23)		/* where to go when done */
	mtspr	SRR0,r24
	mtspr	SRR1,r20
	mtlr	r23
	SYNC
	rfi				/* jump to handler, enable MMU */

int_return:
	mfmsr	r28		/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r28,r28,r4
	SYNC			/* Some chip revs need this... */
	mtmsr	r28
	SYNC
	lwz	r2,_CTR(r1)
	lwz	r0,_LINK(r1)
	mtctr	r2
	mtlr	r0
	lwz	r2,_XER(r1)
	lwz	r0,_CCR(r1)
	mtspr	XER,r2
	mtcrf	0xFF,r0
	REST_10GPRS(3, r1)
	REST_10GPRS(13, r1)
	REST_8GPRS(23, r1)
	REST_GPR(31, r1)
	lwz	r2,_NIP(r1)	/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	SRR0,r2
	mtspr	SRR1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfi

crit_return:
	mfmsr	r28		/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r28,r28,r4
	SYNC			/* Some chip revs need this... */
	mtmsr	r28
	SYNC
	lwz	r2,_CTR(r1)
	lwz	r0,_LINK(r1)
	mtctr	r2
	mtlr	r0
	lwz	r2,_XER(r1)
	lwz	r0,_CCR(r1)
	mtspr	XER,r2
	mtcrf	0xFF,r0
	REST_10GPRS(3, r1)
	REST_10GPRS(13, r1)
	REST_8GPRS(23, r1)
	REST_GPR(31, r1)
	lwz	r2,_NIP(r1)	/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	SPRN_CSRR0,r2
	mtspr	SPRN_CSRR1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfci

mck_return:
	mfmsr	r28		/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r28,r28,r4
	SYNC			/* Some chip revs need this... */
	mtmsr	r28
	SYNC
	lwz	r2,_CTR(r1)
	lwz	r0,_LINK(r1)
	mtctr	r2
	mtlr	r0
	lwz	r2,_XER(r1)
	lwz	r0,_CCR(r1)
	mtspr	XER,r2
	mtcrf	0xFF,r0
	REST_10GPRS(3, r1)
	REST_10GPRS(13, r1)
	REST_8GPRS(23, r1)
	REST_GPR(31, r1)
	lwz	r2,_NIP(r1)	/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	SPRN_MCSRR0,r2
	mtspr	SPRN_MCSRR1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfmci
	.globl get_pvr
get_pvr:
	mfspr	r3, PVR
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out16 */
/* Description:	 Output 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	out16
out16:
	sth	r4,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out16r */
/* Description:	 Byte reverse and output 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	out16r
out16r:
	sthbrx	r4,r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out32r */
/* Description:	 Byte reverse and output 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	out32r
out32r:
	stwbrx	r4,r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in16 */
/* Description:	 Input 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	in16
in16:
	lhz	r3,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in16r */
/* Description:	 Input 16 bits and byte reverse */
/*------------------------------------------------------------------------------- */
	.globl	in16r
in16r:
	lhbrx	r3,r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in32r */
/* Description:	 Input 32 bits and byte reverse */
/*------------------------------------------------------------------------------- */
	.globl	in32r
in32r:
	lwbrx	r3,r0,r3
	blr

#if !defined(CONFIG_SPL_BUILD)
/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * r3 = Relocated stack pointer
 * r4 = Relocated global data pointer
 * r5 = Relocated text pointer
 */
	.globl	relocate_code
relocate_code:
#if defined(CONFIG_4xx_DCACHE) || defined(CONFIG_SYS_INIT_DCACHE_CS)
	/*
	 * We need to flush the initial global data (gd_t) and bd_info
	 * before the dcache will be invalidated.
	 */

	/* Save registers */
	mr	r9, r3
	mr	r10, r4
	mr	r11, r5

	/*
	 * Flush complete dcache, this is faster than flushing the
	 * ranges for global_data and bd_info instead.
	 */
	bl	flush_dcache

#if defined(CONFIG_SYS_INIT_DCACHE_CS)
	/*
	 * Undo the earlier data cache set-up for the primordial stack and
	 * data area. First, invalidate the data cache and then disable data
	 * cacheability for that area. Finally, restore the EBC values, if
	 * any.
	 */

	/* Invalidate the primordial stack and data area in cache */
	lis	r3, CONFIG_SYS_INIT_RAM_ADDR@h
	ori	r3, r3, CONFIG_SYS_INIT_RAM_ADDR@l

	lis	r4, CONFIG_SYS_INIT_RAM_SIZE@h
	ori	r4, r4, CONFIG_SYS_INIT_RAM_SIZE@l
	add	r4, r4, r3

	bl	invalidate_dcache_range

	/* Disable cacheability for the region */
	mfdccr	r3
	lis     r4, ~PPC_128MB_SACR_VALUE(CONFIG_SYS_INIT_RAM_ADDR)@h
	ori     r4, r4, ~PPC_128MB_SACR_VALUE(CONFIG_SYS_INIT_RAM_ADDR)@l
	and     r3, r3, r4
	mtdccr  r3

	/* Restore the EBC parameters */
	li	r3, PBxAP
	mtdcr	EBC0_CFGADDR, r3
	lis	r3, PBxAP_VAL@h
	ori	r3, r3, PBxAP_VAL@l
	mtdcr	EBC0_CFGDATA, r3

	li	r3, PBxCR
	mtdcr	EBC0_CFGADDR, r3
	lis	r3, PBxCR_VAL@h
	ori	r3, r3, PBxCR_VAL@l
	mtdcr	EBC0_CFGDATA, r3
#endif /* defined(CONFIG_SYS_INIT_DCACHE_CS) */

	/* Restore registers */
	mr	r3, r9
	mr	r4, r10
	mr	r5, r11
#endif /* defined(CONFIG_4xx_DCACHE) || defined(CONFIG_SYS_INIT_DCACHE_CS) */

#ifdef CONFIG_SYS_INIT_RAM_DCACHE
	/*
	 * Unlock the previously locked d-cache
	 */
	msync
	isync
	/* set TFLOOR/NFLOOR to 0 again */
	lis	r6,0x0001
	ori	r6,r6,0xf800
	mtspr	SPRN_DVLIM,r6
	lis	r6,0x0000
	ori	r6,r6,0x0000
	mtspr	SPRN_DNV0,r6
	mtspr	SPRN_DNV1,r6
	mtspr	SPRN_DNV2,r6
	mtspr	SPRN_DNV3,r6
	mtspr	SPRN_DTV0,r6
	mtspr	SPRN_DTV1,r6
	mtspr	SPRN_DTV2,r6
	mtspr	SPRN_DTV3,r6
	msync
	isync

	/* Invalidate data cache, now no longer our stack */
	dccci	0,0
	sync
	isync
#endif /* CONFIG_SYS_INIT_RAM_DCACHE */

	/*
	 * On some 440er platforms the cache is enabled in the first TLB (Boot-CS)
	 * to speed up the boot process. Now this cache needs to be disabled.
	 */
	/* Clear all potential pending exceptions */
	mfspr	r1,SPRN_MCSR
	mtspr	SPRN_MCSR_C,r1
#if !defined(CONFIG_SOC_IBM_FSP2)
	addi	r1,r0,CONFIG_SYS_TLB_FOR_BOOT_FLASH	/* Use defined TLB */
	tlbre	r0,r1,0x0002		/* Read contents */
	ori	r0,r0,0x0c00		/* Or in the inhibit, write through bit */
	tlbwe	r0,r1,0x0002		/* Save it out */
	sync
	isync
#endif /* !defined(CONFIG_SOC_IBM_FSP2) */
	mr	r1,  r3		/* Set new stack pointer		*/
	mr	r9,  r4		/* Save copy of Init Data pointer	*/
	mr	r10, r5		/* Save copy of Destination Address	*/

	GET_GOT
	mr	r3,  r5				/* Destination Address	*/
	lis	r4, CONFIG_SYS_MONITOR_BASE@h		/* Source      Address	*/
	ori	r4, r4, CONFIG_SYS_MONITOR_BASE@l
	lwz	r5, GOT(__init_end)
	sub	r5, r5, r4
	li	r6, L1_CACHE_BYTES		/* Cache Line Size	*/

	/*
	 * Fix GOT pointer:
	 *
	 * New GOT-PTR = (old GOT-PTR - CONFIG_SYS_MONITOR_BASE) + Destination Address
	 *
	 * Offset:
	 */
	sub	r15, r10, r4

	/* First our own GOT */
	add	r12, r12, r15
	/* then the one used by the C code */
	add	r30, r30, r15

	/*
	 * Now relocate code
	 */

	cmplw	cr1,r3,r4
	addi	r0,r5,3
	srwi.	r0,r0,2
	beq	cr1,4f		/* In place copy is not necessary	*/
	beq	7f		/* Protect against 0 count		*/
	mtctr	r0
	bge	cr1,2f

	la	r8,-4(r4)
	la	r7,-4(r3)
1:	lwzu	r0,4(r8)
	stwu	r0,4(r7)
	bdnz	1b
	b	4f

2:	slwi	r0,r0,2
	add	r8,r4,r0
	add	r7,r3,r0
3:	lwzu	r0,-4(r8)
	stwu	r0,-4(r7)
	bdnz	3b

/*
 * Now flush the cache: note that we must start from a cache aligned
 * address. Otherwise we might miss one cache line.
 */
4:	cmpwi	r6,0
	add	r5,r3,r5
	beq	7f		/* Always flush prefetch queue in any case */
	subi	r0,r6,1
	andc	r3,r3,r0
	mr	r4,r3
5:	dcbst	0,r4
	add	r4,r4,r6
	cmplw	r4,r5
	blt	5b
	sync			/* Wait for all dcbst to complete on bus */
	mr	r4,r3
6:	icbi	0,r4
	add	r4,r4,r6
	cmplw	r4,r5
	blt	6b
7:	sync			/* Wait for all icbi to complete on bus */
	isync

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */

	addi	r0, r10, in_ram - _start + _START_OFFSET
	mtlr	r0
	blr				/* NEVER RETURNS! */

in_ram:

	/*
	 * Relocation Function, r12 point to got2+0x8000
	 *
	 * Adjust got2 pointers, no need to check for 0, this code
	 * already puts a few entries in the table.
	 */
	li	r0,__got2_entries@sectoff@l
	la	r3,GOT(_GOT2_TABLE_)
	lwz	r11,GOT(_GOT2_TABLE_)
	mtctr	r0
	sub	r11,r3,r11
	addi	r3,r3,-4
1:	lwzu	r0,4(r3)
	cmpwi	r0,0
	beq-	2f
	add	r0,r0,r11
	stw	r0,0(r3)
2:	bdnz	1b

	/*
	 * Now adjust the fixups and the pointers to the fixups
	 * in case we need to move ourselves again.
	 */
	li	r0,__fixup_entries@sectoff@l
	lwz	r3,GOT(_FIXUP_TABLE_)
	cmpwi	r0,0
	mtctr	r0
	addi	r3,r3,-4
	beq	4f
3:	lwzu	r4,4(r3)
	lwzux	r0,r4,r11
	cmpwi	r0,0
	add	r0,r0,r11
	stw	r4,0(r3)
	beq-	5f
	stw	r0,0(r4)
5:	bdnz	3b
4:
clear_bss:
	/*
	 * Now clear BSS segment
	 */
	lwz	r3,GOT(__bss_start)
	lwz	r4,GOT(__bss_end)

	cmplw	0, r3, r4
	beq	7f

	li	r0, 0

	andi.	r5, r4, 3
	beq	6f
	sub	r4, r4, r5
	mtctr	r5
	mr	r5, r4
5:	stb	r0, 0(r5)
	addi	r5, r5, 1
	bdnz	5b
6:
	stw	r0, 0(r3)
	addi	r3, r3, 4
	cmplw	0, r3, r4
	bne	6b

7:
	mr	r3, r9		/* Init Data pointer		*/
	mr	r4, r10		/* Destination Address		*/
	bl	board_init_r

	/*
	 * Copy exception vector code to low memory
	 *
	 * r3: dest_addr
	 * r7: source address, r8: end address, r9: target address
	 */
	.globl	trap_init
trap_init:
	mflr	r4			/* save link register		*/
	GET_GOT
	lwz	r7, GOT(_start_of_vectors)
	lwz	r8, GOT(_end_of_vectors)

	li	r9, 0x100		/* reset vector always at 0x100 */

	cmplw	0, r7, r8
	bgelr				/* return if r7>=r8 - just in case */
1:
	lwz	r0, 0(r7)
	stw	r0, 0(r9)
	addi	r7, r7, 4
	addi	r9, r9, 4
	cmplw	0, r7, r8
	bne	1b

	/*
	 * relocate `hdlr' and `int_return' entries
	 */
	li	r7, .L_MachineCheck - _start + _START_OFFSET
	li	r8, Alignment - _start + _START_OFFSET
2:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector */
	cmplw	0, r7, r8
	blt	2b

	li	r7, .L_Alignment - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_ProgramCheck - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_FPUnavailable - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_Decrementer - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_APU - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_InstructionTLBError - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_DataTLBError - _start + _START_OFFSET
	bl	trap_reloc
	li	r7, .L_DebugBreakpoint - _start + _START_OFFSET
	bl	trap_reloc

	addi	r7,r0,0x1000		/* set ME bit (Machine Exceptions) */
	oris	r7,r7,0x0002		/* set CE bit (Critical Exceptions) */
	mtmsr	r7			/* change MSR */
	mtlr	r4			/* restore link register	*/
	blr
#endif /* CONFIG_SPL_BUILD */

/*----------------------------------------------------------------------------+
| dcbz_area.
+----------------------------------------------------------------------------*/
	function_prolog(dcbz_area)
	rlwinm. r5,r4,0,27,31
	rlwinm	r5,r4,27,5,31
	beq	..d_ra2
	addi	r5,r5,0x0001
..d_ra2:mtctr	r5
..d_ag2:dcbz	r0,r3
	addi	r3,r3,32
	bdnz	..d_ag2
	sync
	blr
	function_epilog(dcbz_area)
#endif /* CONFIG_SPL_BUILD */

/*------------------------------------------------------------------------------- */
/* Function:	 in8 */
/* Description:	 Input 8 bits */
/*------------------------------------------------------------------------------- */
	.globl	in8
in8:
	lbz	r3,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out8 */
/* Description:	 Output 8 bits */
/*------------------------------------------------------------------------------- */
	.globl	out8
out8:
	stb	r4,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out32 */
/* Description:	 Output 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	out32
out32:
	stw	r4,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in32 */
/* Description:	 Input 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	in32
in32:
	lwz	3,0x0000(3)
	blr

/*----------------------------------------------------------------------------+
| mttlb3.
+----------------------------------------------------------------------------*/
	function_prolog(mttlb3)
	TLBWE(4,3,2)
	blr
	function_epilog(mttlb3)

/*----------------------------------------------------------------------------+
| mftlb3.
+----------------------------------------------------------------------------*/
	function_prolog(mftlb3)
	TLBRE(3,3,2)
	blr
	function_epilog(mftlb3)

/*----------------------------------------------------------------------------+
| mttlb2.
+----------------------------------------------------------------------------*/
	function_prolog(mttlb2)
	TLBWE(4,3,1)
	blr
	function_epilog(mttlb2)

/*----------------------------------------------------------------------------+
| mftlb2.
+----------------------------------------------------------------------------*/
	function_prolog(mftlb2)
	TLBRE(3,3,1)
	blr
	function_epilog(mftlb2)

/*----------------------------------------------------------------------------+
| mttlb1.
+----------------------------------------------------------------------------*/
	function_prolog(mttlb1)
	TLBWE(4,3,0)
	blr
	function_epilog(mttlb1)

/*----------------------------------------------------------------------------+
| mftlb1.
+----------------------------------------------------------------------------*/
	function_prolog(mftlb1)
	TLBRE(3,3,0)
	blr
	function_epilog(mftlb1)
