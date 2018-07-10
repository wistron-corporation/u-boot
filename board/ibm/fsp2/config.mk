#
# Copyright (c) 2017 IBM
# SPDX-License-Identifier:     GPL-2.0+
#

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif

ifeq ($(dbcr),1)
PLATFORM_CPPFLAGS += -DCONFIG_SYS_INIT_DBCR=0x8cff0000
endif
