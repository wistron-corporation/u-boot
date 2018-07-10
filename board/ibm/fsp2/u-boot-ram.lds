/*
 * Copyright (c) 2017 IBM
 * SPDX-License-Identifier:     GPL-2.0+
 */

OUTPUT_ARCH(powerpc)
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = + SIZEOF_HEADERS;
  .text      :
  {
    arch/powerpc/cpu/47x/start.o	(.text*)
    board/ibm/fsp2/init.o		(.text*)

    *(.text*)
  }
  _etext = .;
  PROVIDE (etext = .);
  .rodata    :
  {
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
  }

  /* Read-write section, merged into data segment: */
  . = (. + 0x00FF) & 0xFFFFFF00;
  _erotext = .;
  PROVIDE (erotext = .);
  .reloc   :
  {
    KEEP(*(.got))
    _GOT2_TABLE_ = .;
    KEEP(*(.got2))
    _FIXUP_TABLE_ = .;
    KEEP(*(.fixup))
  }
  __got2_entries = (_FIXUP_TABLE_ - _GOT2_TABLE_) >>2;
  __fixup_entries = (. - _FIXUP_TABLE_)>>2;

  .data    :
  {
    *(.data*)
    *(.sdata*)
  }
  _edata  =  .;
  PROVIDE (edata = .);

  . = .;
  __u_boot_cmd_start = .;
  .u_boot_cmd : { *(.u_boot_cmd) }
  __u_boot_cmd_end = .;

  . = ALIGN(4);
  .u_boot_list : { KEEP(*(SORT(.u_boot_list*))); }

  . = .;
  __start___ex_table = .;
  __ex_table : { *(__ex_table) }
  __stop___ex_table = .;

  . = ALIGN(256);
  __init_begin = .;
  .text.init : { *(.text.init) }
  .data.init : { *(.data.init) }
  . = ALIGN(256);
  __init_end = .;

  __bss_start = .;
  .bss (NOLOAD)       :
  {
   *(.bss*)
   *(.sbss*)
   *(COMMON)
   . = ALIGN(4);
  }

  __bss_end = . ;
  PROVIDE (end = .);
}
