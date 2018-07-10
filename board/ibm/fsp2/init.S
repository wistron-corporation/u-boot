/*
 * Copyright (c) 2017 IBM
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <ppc_asm.tmpl>
#include <config.h>
#include <asm/mmu.h>
#include <asm/ppc4xx.h>

/**************************************************************************
 * TLB TABLE
 *
 * This table is used by the cpu boot code to setup the initial tlb
 * entries. Rather than make broad assumptions in the cpu source tree,
 * this table lets each board set things up however they like.
 *
 *  Pointer to the table is returned in r1
 *
 *************************************************************************/

/* Note that all of the TLB entries necessary have already been setup by
 * the IPL.
 * TODO : move tlb setup from ipl here.
 */


