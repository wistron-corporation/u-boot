/*
 * (C) Copyright 2008
 * Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@gmail.com
 * This work has been supported by: QTechnology  http://qtec.com/
 * Based on interrupts.c Wolfgang Denk-DENX Software Engineering-wd@denx.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef INTERRUPT_H
#define INTERRUPT_H

void external_interrupt(struct pt_regs *regs);
void pic_irq_disable(unsigned int vec);
void pic_enable(void);
void pic_irq_ack(unsigned int vec);
void pic_irq_enable(unsigned int vec);
void interrupt_run_handler(int vec);
void interrupt_init_cpu(unsigned *decrementer_count);
void timer_interrupt_cpu(struct pt_regs *regs);

#endif
