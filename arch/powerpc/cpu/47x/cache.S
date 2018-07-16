/*
 * This file contains miscellaneous low-level functions.
 *    Copyright (C) 1995-1996 Gary Thomas (gdt@linuxppc.org)
 *
 * Largely rewritten by Cort Dougan (cort@cs.nmt.edu)
 * and Paul Mackerras.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <config.h>
#include <asm/ppc4xx.h>
#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <asm/cache.h>
#include <asm/mmu.h>

/*
 * Flush instruction cache.
 */
_GLOBAL(invalidate_icache)
	iccci	r0,r0
	isync
	blr

/*
 * Write any modified data cache blocks out to memory
 * and invalidate the corresponding instruction cache blocks.
 *
 * flush_icache_range(unsigned long start, unsigned long stop)
 */
_GLOBAL(flush_icache_range)
	li	r5,L1_CACHE_BYTES-1
	andc	r3,r3,r5
	subf	r4,r3,r4
	add	r4,r4,r5
	srwi.	r4,r4,L1_CACHE_SHIFT
	beqlr
	mtctr	r4
	mr	r6,r3
1:	dcbst	0,r3
	addi	r3,r3,L1_CACHE_BYTES
	bdnz	1b
	sync				/* wait for dcbst's to get to ram */
	mtctr	r4
2:	icbi	0,r6
	addi	r6,r6,L1_CACHE_BYTES
	bdnz	2b
	sync				/* additional sync needed on g4 */
	isync
	blr

/*
 * Write any modified data cache blocks out to memory.
 * Does not invalidate the corresponding cache lines (especially for
 * any corresponding instruction cache).
 *
 * clean_dcache_range(unsigned long start, unsigned long stop)
 */
_GLOBAL(clean_dcache_range)
	li	r5,L1_CACHE_BYTES-1
	andc	r3,r3,r5
	subf	r4,r3,r4
	add	r4,r4,r5
	srwi.	r4,r4,L1_CACHE_SHIFT
	beqlr
	mtctr	r4

1:	dcbst	0,r3
	addi	r3,r3,L1_CACHE_BYTES
	bdnz	1b
	sync				/* wait for dcbst's to get to ram */
	blr



/*
 * Write any modified data cache blocks out to memory and invalidate them.
 * Does not invalidate the corresponding instruction cache blocks.
 *
 * wback_inv_dcache(unsigned long start, unsigned long stop)
 */
_GLOBAL(wback_inv_dcache_range)
        li      r5,L1_CACHE_BYTES-1
        andc    r3,r3,r5
        subf    r4,r3,r4
        add     r4,r4,r5
        srwi.   r4,r4,L1_CACHE_SHIFT
        beqlr
        mtctr   r4

1:      dcbf    0,r3                    /* data cache block flush */
        sync
        dcbi    0,r3                    /* data cache block invalidate */
        sync                            /* wait for dcbst's to get to ram */
        addi    r3,r3,L1_CACHE_BYTES
        bdnz    1b
        sync                            /* wait for dcbst's to get to ram */
        blr


/*
 * Write any modified L2 cache blocks out to memory and invalidate them.
 *
 * flush_l2cache_range(unsigned long start, unsigned long stop)
 */
_GLOBAL(flush_l2cache_range)
	li	r5,L2_CACHE_BYTES-1
	andc	r3,r3,r5
	subf	r4,r3,r4
	add	r4,r4,r5
	srwi.	r4,r4,L2_CACHE_SHIFT
	beqlr
	mtctr	r4

1:	dcbf	2,r3
	addi	r3,r3,L2_CACHE_BYTES
	bdnz	1b
	sync				/* wait for dcbf's to get to ram */
	blr

/*
 * Prime the L2 cache.
 *
 * touch_l2cache_range(unsigned long start, unsigned long stop)
 */
_GLOBAL(touch_l2cache_range)
	li	r5,L2_CACHE_BYTES-1
	andc	r3,r3,r5
	subf	r4,r3,r4
	add	r4,r4,r5
	srwi.	r4,r4,L2_CACHE_SHIFT
	beqlr
	mtctr	r4

1:	dcbt	2,r3
	addi	r3,r3,L2_CACHE_BYTES
	bdnz	1b
	sync				/* wait for dcbt's to get to cache */
	blr

/*
 * 40x cores have 8K or 16K dcache and 32 byte line size.
 * 44x has a 32K dcache and 32 byte line size.
 * 8xx has 1, 2, 4, 8K variants.
 * For now, cover the worst case of the 44x.
 * Must be called with external interrupts disabled.
 */
#define CACHE_NWAYS     64
#define CACHE_NLINES    32

_GLOBAL(flush_dcache)
	li	r4,(2 * CACHE_NWAYS * CACHE_NLINES)
	mtctr	r4
	lis	r5,0
1:	lwz	r3,0(r5)		/* Load one word from every line */
	addi	r5,r5,L1_CACHE_BYTES
	bdnz	1b
	sync
	blr

_GLOBAL(invalidate_dcache)
	addi	r6,0,0x0000		/* clear GPR 6 */
	/* Do loop for # of dcache congruence classes. */
	lis	r7,(CONFIG_SYS_DCACHE_SIZE / L1_CACHE_BYTES / 2)@ha	/* TBS for large sized cache */
	ori	r7,r7,(CONFIG_SYS_DCACHE_SIZE / L1_CACHE_BYTES / 2)@l
					/* NOTE: dccci invalidates both */
	mtctr	r7			/* ways in the D cache */
..dcloop:
	dccci	0,r6			/* invalidate line */
	addi	r6,r6,L1_CACHE_BYTES	/* bump to next line */
	bdnz	..dcloop
	sync
	blr

/*
 * Cache functions.
 *
 * NOTE: currently the 440s run with dcache _disabled_ once relocated to DRAM,
 * although for some cache-ralated calls stubs have to be provided to satisfy
 * symbols resolution.
 * Icache-related functions are used in POST framework.
 *
 */
       .globl  dcache_disable
       .globl  dcache_enable
       .globl  icache_disable
       .globl  icache_enable
dcache_disable:
dcache_enable:
icache_disable:
icache_enable:
	blr

	.globl	dcache_status
	.globl	icache_status
dcache_status:
icache_status:
	mr	r3,  0
	blr

