/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 */

/* Bus Clocks, derived from core clocks */
#define BCLK_LHCLK	102
#define BCLK_MACCLK	103
#define BCLK_SDCLK	104
#define BCLK_ARMCLK	105

/* Special clocks */
#define PCLK_UART1	501
#define PCLK_UART2	502
#define PCLK_UART3	503
#define PCLK_UART4	504
#define PCLK_UART5	505
#define PCLK_MAC1	506
#define PCLK_MAC2	507

/* come from linux kernel */

#define ASPEED_CLK_GATE_ECLK            0
#define ASPEED_CLK_GATE_GCLK            1
#define ASPEED_CLK_GATE_MCLK            2
#define ASPEED_CLK_GATE_VCLK            3
#define ASPEED_CLK_GATE_BCLK            4
#define ASPEED_CLK_GATE_DCLK            5
#define ASPEED_CLK_GATE_REFCLK          6
#define ASPEED_CLK_GATE_USBPORT2CLK     7
#define ASPEED_CLK_GATE_LCLK            8
#define ASPEED_CLK_GATE_USBUHCICLK      9
#define ASPEED_CLK_GATE_D1CLK           10
#define ASPEED_CLK_GATE_YCLK            11
#define ASPEED_CLK_GATE_USBPORT1CLK     12
#define ASPEED_CLK_GATE_UART1CLK        13
#define ASPEED_CLK_GATE_UART2CLK        14
#define ASPEED_CLK_GATE_UART5CLK        15
#define ASPEED_CLK_GATE_ESPICLK         16
#define ASPEED_CLK_GATE_MAC1CLK         17
#define ASPEED_CLK_GATE_MAC2CLK         18
#define ASPEED_CLK_GATE_RSACLK          19
#define ASPEED_CLK_GATE_UART3CLK        20
#define ASPEED_CLK_GATE_UART4CLK        21
#define ASPEED_CLK_GATE_SDCLK           22
#define ASPEED_CLK_GATE_LHCCLK          23
#define ASPEED_CLK_GATE_SDEXTCLK        24
#define ASPEED_CLK_GATE_EMMCCLK         25
#define ASPEED_CLK_GATE_EMMCEXTCLK      26

#define ASPEED_CLK_GATE_UART6CLK        27
#define ASPEED_CLK_GATE_UART7CLK        28
#define ASPEED_CLK_GATE_UART8CLK        29
#define ASPEED_CLK_GATE_UART9CLK        30
#define ASPEED_CLK_GATE_UART10CLK       31
#define ASPEED_CLK_GATE_UART11CLK       32
#define ASPEED_CLK_GATE_UART12CLK       33
#define ASPEED_CLK_GATE_UART13CLK       34

#define ASPEED_CLK_HPLL                 35
#define ASPEED_CLK_AHB                  36
#define ASPEED_CLK_APB                  37
#define ASPEED_CLK_UART                 38
#define ASPEED_CLK_SDIO                 39
#define ASPEED_CLK_ECLK                 40
#define ASPEED_CLK_ECLK_MUX             41
#define ASPEED_CLK_LHCLK                42
#define ASPEED_CLK_MAC                  43
#define ASPEED_CLK_BCLK                 44
#define ASPEED_CLK_MPLL                 45
#define ASPEED_CLK_24M                  46
#define ASPEED_CLK_EMMC                 47
#define ASPEED_CLK_UARTX                48

///
#define ASPEED_CLK_APLL					50
#define ASPEED_CLK_EPLL					51
#define ASPEED_CLK_DPLL					52

