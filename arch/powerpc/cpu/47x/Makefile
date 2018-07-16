#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	= start.o
extra-y	+= resetvec.o

obj-y	+= cache.o
obj-y	+= dcr.o

obj-y	+= 44x_spd_ddr.o
obj-y	+= 4xx_uart.o
obj-y	+= cpu.o
obj-y	+= cpu_init.o
obj-y	+= interrupts.o
obj-y	+= sdram.o
obj-y	+= speed.o
obj-y	+= tlb.o
obj-y	+= traps.o
obj-y	+= uic.o
ifdef CONFIG_SPL_BUILD
obj-y	+= spl_boot.o
endif
