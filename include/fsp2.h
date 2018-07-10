/*
 * Copyright (c) 2017 IBM
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __FSP2_H__
#define __FSP2_H__

#define MEM_SCRUBBED	1
#define MEM_FILLED	2
#define MEM_STEP	512*1024

int mem_init(unsigned long, unsigned long, int *);

#endif /* __FSP2_H__ */
