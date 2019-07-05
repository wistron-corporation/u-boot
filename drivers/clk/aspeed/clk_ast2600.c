// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <asm/arch/scu_ast2600.h>
#include <dt-bindings/clock/ast2600-clock.h>
#include <dt-bindings/reset/ast2600-reset.h>

/*
 * MAC Clock Delay settings, taken from Aspeed SDK
 */
#define RGMII_TXCLK_ODLY	8
#define RMII_RXCLK_IDLY		2

#define MAC_DEF_DELAY_1G	0x00410410
#define MAC_DEF_DELAY_100M	0x00410410
#define MAC_DEF_DELAY_10M	0x00410410

#define MAC34_DEF_DELAY_1G	0x00104208
#define MAC34_DEF_DELAY_100M	0x00104208
#define MAC34_DEF_DELAY_10M	0x00104208

/*
 * TGMII Clock Duty constants, taken from Aspeed SDK
 */
#define RGMII2_TXCK_DUTY	0x66
#define RGMII1_TXCK_DUTY	0x64

#define D2PLL_DEFAULT_RATE	(250 * 1000 * 1000)

DECLARE_GLOBAL_DATA_PTR;

/*
 * Clock divider/multiplier configuration struct.
 * For H-PLL and M-PLL the formula is
 * (Output Frequency) = CLKIN * ((M + 1) / (N + 1)) / (P + 1)
 * M - Numerator
 * N - Denumerator
 * P - Post Divider
 * They have the same layout in their control register.
 *
 * D-PLL and D2-PLL have extra divider (OD + 1), which is not
 * yet needed and ignored by clock configurations.
 */
union ast2600_pll_reg {
	unsigned int w;
	struct {
		unsigned int m : 13;	/* 12:0  */
		unsigned int n : 6;	/* 18:13  */
		unsigned int p : 4;	/* 22:19  */
		unsigned int reserved : 9; /* 31:20  */
	} b;
};

struct ast2600_pll_cfg {
	union ast2600_pll_reg reg;
	unsigned int ext_reg;
};

struct ast2600_pll_desc {
	u32 in;
	u32 out;
	struct ast2600_pll_cfg cfg;
};

static const struct ast2600_pll_desc ast2600_pll_lookup[] = {
    {.in = AST2600_CLK_IN, .out = 400000000,
    .cfg.reg.b.m = 95, .cfg.reg.b.n = 2, .cfg.reg.b.p = 1,
    .cfg.ext_reg = 0x31,
    },
    {.in = AST2600_CLK_IN, .out = 200000000,
    .cfg.reg.b.m = 127, .cfg.reg.b.n = 0, .cfg.reg.b.p = 15,
    .cfg.ext_reg = 0x3f
    },
    {.in = AST2600_CLK_IN, .out = 334000000,
    .cfg.reg.b.m = 667, .cfg.reg.b.n = 4, .cfg.reg.b.p = 9,
    .cfg.ext_reg = 0x14d
    },

    {.in = AST2600_CLK_IN, .out = 1000000000,
    .cfg.reg.b.m = 119, .cfg.reg.b.n = 2, .cfg.reg.b.p = 0,
    .cfg.ext_reg = 0x3d
    },

    {.in = AST2600_CLK_IN, .out = 50000000,
    .cfg.reg.b.m = 95, .cfg.reg.b.n = 2, .cfg.reg.b.p = 15,
    .cfg.ext_reg = 0x31
    },
};

extern u32 ast2600_get_pll_rate(struct ast2600_scu *scu, int pll_idx)
{
	u32 clkin = AST2600_CLK_IN;
	u32 pll_reg = 0;
	unsigned int mult, div = 1;

	switch(pll_idx) {
		case ASPEED_CLK_HPLL:	
			pll_reg = readl(&scu->h_pll_param);
			break;
		case ASPEED_CLK_MPLL:	
			pll_reg = readl(&scu->m_pll_param);
			break;
		case ASPEED_CLK_DPLL:	
			pll_reg = readl(&scu->d_pll_param);
			break;		
		case ASPEED_CLK_EPLL:	
			pll_reg = readl(&scu->e_pll_param);
			break;

	}
	if (pll_reg & BIT(24)) {
		/* Pass through mode */
		mult = div = 1;
	} else {
		/* F = 25Mhz * [(M + 2) / (n + 1)] / (p + 1) */
		u32 m = pll_reg  & 0x1fff;
		u32 n = (pll_reg >> 13) & 0x3f;
		u32 p = (pll_reg >> 19) & 0xf;
		mult = (m + 1) / (n + 1);
		div = (p + 1);
	}
	return ((clkin * mult)/div);
	
}

extern u32 ast2600_get_apll_rate(struct ast2600_scu *scu)
{
	u32 clkin = AST2600_CLK_IN;
	u32 apll_reg = readl(&scu->a_pll_param);
	unsigned int mult, div = 1;

	if (apll_reg & BIT(20)) {
		/* Pass through mode */
		mult = div = 1;
	} else {
		/* F = 25Mhz * (2-od) * [(m + 2) / (n + 1)] */
		u32 m = (apll_reg >> 5) & 0x3f;
		u32 od = (apll_reg >> 4) & 0x1;
		u32 n = apll_reg & 0xf;

		mult = (2 - od) * (m + 2);
		div = n + 1;
	}
	return ((clkin * mult)/div);
}

static u32 ast2600_a0_axi_ahb_div_table[] = {
	2, 2, 3, 5,
};

static u32 ast2600_a1_axi_ahb_div_table[] = {
	4, 6, 2, 4,
};

static u32 ast2600_get_hclk(struct ast2600_scu *scu)
{
	u32 hw_rev = readl(&scu->chip_id0);
	u32 hwstrap1 = readl(&scu->hwstrap1);
	u32 axi_div = 1;
	u32 ahb_div = 0;
	u32 rate = 0;
	
	if(hwstrap1 & BIT(16))
		axi_div = 1;
	else
		axi_div = 2;
	
	if (hw_rev & BIT(16))
		ahb_div = ast2600_a1_axi_ahb_div_table[(hwstrap1 >> 11) & 0x3];
	else
		ahb_div = ast2600_a0_axi_ahb_div_table[(hwstrap1 >> 11) & 0x3];
	
	rate = ast2600_get_pll_rate(scu, ASPEED_CLK_HPLL);

	return (rate / axi_div / ahb_div);
}

static u32 ast2600_hpll_pclk_div_table[] = {
	4, 8, 12, 16, 20, 24, 28, 32,
};

static u32 ast2600_get_pclk(struct ast2600_scu *scu)
{
	u32 clk_sel1 = readl(&scu->clk_sel1);
	u32 apb_div = ast2600_hpll_pclk_div_table[((clk_sel1 >> 23) & 0x7)];
	u32 rate = ast2600_get_pll_rate(scu, ASPEED_CLK_HPLL);

	return (rate / apb_div);
}

static u32 ast2600_get_uxclk_rate(struct ast2600_scu *scu)
{
	u32 clk_in = 0;
	u32 uxclk_sel = readl(&scu->clk_sel4);

	uxclk_sel &= 0x3;
	switch(uxclk_sel) {
		case 0:
			clk_in = ast2600_get_apll_rate(scu) / 4;
			break;
		case 1:
			clk_in = ast2600_get_apll_rate(scu) / 2;
			break;
		case 2:
			clk_in = ast2600_get_apll_rate(scu);
			break;
		case 3:
			clk_in = ast2600_get_hclk(scu);
			break;
	}

	return clk_in;
}

static u32 ast2600_get_huxclk_rate(struct ast2600_scu *scu)
{
	u32 clk_in = 0;
	u32 huclk_sel = readl(&scu->clk_sel4);

	huclk_sel = ((huclk_sel >> 3) & 0x3);
	switch(huclk_sel) {
		case 0:
			clk_in = ast2600_get_apll_rate(scu) / 4;
			break;
		case 1:
			clk_in = ast2600_get_apll_rate(scu) / 2;
			break;
		case 2:
			clk_in = ast2600_get_apll_rate(scu);
			break;
		case 3:
			clk_in = ast2600_get_hclk(scu);
			break;
	}

	return clk_in;
}

static u32 ast2600_get_uart_from_uxclk_rate(struct ast2600_scu *scu)
{
	u32 clk_in = ast2600_get_uxclk_rate(scu);
	u32 div_reg = readl(&scu->uart_24m_ref_uxclk);
	unsigned int mult, div;

	u32 n = (div_reg >> 8) & 0x3ff;
	u32 r = div_reg & 0xff;
	
	mult = r;
	div = (n * 4);
	return (clk_in * mult)/div;
}

static u32 ast2600_get_uart_from_huxclk_rate(struct ast2600_scu *scu)
{
	u32 clk_in = ast2600_get_huxclk_rate(scu);
	u32 div_reg = readl(&scu->uart_24m_ref_huxclk);

	unsigned int mult, div;

	u32 n = (div_reg >> 8) & 0x3ff;
	u32 r = div_reg & 0xff;
	
	mult = r;
	div = (n * 4);
	return (clk_in * mult)/div;
}

static u32 ast2600_get_sdio_clk_rate(struct ast2600_scu *scu)
{
	u32 clkin = 0;
	u32 clk_sel = readl(&scu->clk_sel4);
	u32 div = (clk_sel >> 28) & 0x7;

	if(clk_sel & BIT(8)) {
		clkin = ast2600_get_apll_rate(scu);
	} else {
		clkin = 200 * 1000 * 1000;
	}
	div = (div + 1) << 1;

	return (clkin / div);
}

static u32 ast2600_get_emmc_clk_rate(struct ast2600_scu *scu)
{
	u32 clkin = ast2600_get_pll_rate(scu, ASPEED_CLK_HPLL);
	u32 clk_sel = readl(&scu->clk_sel1);
	u32 div = (clk_sel >> 12) & 0x7;
	
	div = (div + 1) << 2;

	return (clkin / div);
}

static u32 ast2600_get_uart_clk_rate(struct ast2600_scu *scu, int uart_idx)
{
	u32 uart_sel = readl(&scu->clk_sel4);
	u32 uart_sel5 = readl(&scu->clk_sel5);	
	ulong uart_clk = 0;

	switch(uart_idx) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 6:
			if(uart_sel & BIT(uart_idx - 1))
				uart_clk = ast2600_get_uart_from_uxclk_rate(scu)/13 ;
			else
				uart_clk = ast2600_get_uart_from_huxclk_rate(scu)/13 ;
			break;
		case 5: //24mhz is come form usb phy 48Mhz
			{
			u8 uart5_clk_sel = 0;
			//high bit
			if (readl(&scu->misc_ctrl1) & BIT(12))
				uart5_clk_sel = 0x2;
			else
				uart5_clk_sel = 0x0;

			if (readl(&scu->clk_sel2) & BIT(14))
				uart5_clk_sel |= 0x1;
			
			switch(uart5_clk_sel) {
				case 0:
					uart_clk = 24000000;
					break;
				case 1:
					uart_clk = 0;
					break;
				case 2:
					uart_clk = 24000000/13;
					break;
				case 3:
					uart_clk = 192000000/13;
					break;
			}
			}
			break;
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:			
			if(uart_sel5 & BIT(uart_idx - 1))
				uart_clk = ast2600_get_uart_from_uxclk_rate(scu)/13 ;
			else
				uart_clk = ast2600_get_uart_from_huxclk_rate(scu)/13 ;
			break;
	}

	return uart_clk;
}

static ulong ast2600_clk_get_rate(struct clk *clk)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	switch (clk->id) {
	case ASPEED_CLK_HPLL:
	case ASPEED_CLK_EPLL:
	case ASPEED_CLK_DPLL:
	case ASPEED_CLK_MPLL:
		rate = ast2600_get_pll_rate(priv->scu, clk->id);
		break;
	case ASPEED_CLK_AHB:
		rate = ast2600_get_hclk(priv->scu);
		break;
	case ASPEED_CLK_APB:
		rate = ast2600_get_pclk(priv->scu);
		break;
	case ASPEED_CLK_APLL:
		rate = ast2600_get_apll_rate(priv->scu);
		break;
	case ASPEED_CLK_GATE_UART1CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 1);
		break;
	case ASPEED_CLK_GATE_UART2CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 2);
		break;
	case ASPEED_CLK_GATE_UART3CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 3);
		break;
	case ASPEED_CLK_GATE_UART4CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 4);
		break;
	case ASPEED_CLK_GATE_UART5CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 5);
		break;
	case ASPEED_CLK_SDIO:
		rate = ast2600_get_sdio_clk_rate(priv->scu);
		break;
	case ASPEED_CLK_EMMC:
		rate = ast2600_get_emmc_clk_rate(priv->scu);
		break;
	default:
		pr_debug("can't get clk rate \n");
		return -ENOENT;
		break;
	}

	return rate;
}

/**
 * @brief	lookup PLL divider config by input/output rate
 * @param[in]	*pll - PLL descriptor
 * @return	true - if PLL divider config is found, false - else
 * 
 * The function caller shall fill "pll->in" and "pll->out", then this function
 * will search the lookup table to find a valid PLL divider configuration.
 */
static bool ast2600_search_clock_config(struct ast2600_pll_desc *pll)
{
	u32 i;
	bool is_found = false;

	for (i = 0; i < ARRAY_SIZE(ast2600_pll_lookup); i++) {
		const struct ast2600_pll_desc *def_cfg = &ast2600_pll_lookup[i];
		if ((def_cfg->in == pll->in) && (def_cfg->out == pll->out)) {
			is_found = true;
			pll->cfg.reg.w = def_cfg->cfg.reg.w;
			pll->cfg.ext_reg = def_cfg->cfg.ext_reg;
			break;
		}
	}
	return is_found;
}

static u32 ast2600_configure_ddr(struct ast2600_scu *scu, ulong rate)
{
	u32 mpll_reg;
	struct ast2600_pll_desc mpll;	

	mpll.in = AST2600_CLK_IN;
	mpll.out = rate;
	if (false == ast2600_search_clock_config(&mpll)) {
		printf("error!! unable to find valid DDR clock setting\n");
		return 0;
	}

	mpll_reg = readl(&scu->m_pll_param);
	mpll_reg &= ~GENMASK(22, 0);
	mpll_reg |= mpll.cfg.reg.w;
	writel(mpll_reg, &scu->m_pll_param);	
	
	/* write extend parameter */
	writel(mpll.cfg.ext_reg, &scu->m_pll_ext_param);

	return ast2600_get_pll_rate(scu, ASPEED_CLK_MPLL);
}

static ulong ast2600_clk_set_rate(struct clk *clk, ulong rate)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);

	ulong new_rate;
	switch (clk->id) {
	case ASPEED_CLK_MPLL:
		new_rate = ast2600_configure_ddr(priv->scu, rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

#define SCU_CLKSTOP_MAC1		(20)
#define SCU_CLKSTOP_MAC2		(21)
#define SCU_CLKSTOP_MAC3		(20)
#define SCU_CLKSTOP_MAC4		(21)

static u32 ast2600_configure_mac12_clk(struct ast2600_scu *scu)
{
	u32 epll_reg;
	u32 clksel;
	u32 clkdelay;
	
	struct ast2600_pll_desc epll;

	epll.in = AST2600_CLK_IN;
	epll.out = 1000000000;
	if (false == ast2600_search_clock_config(&epll)) {
		printf(
		    "error!! unable to find valid ETHNET MAC clock setting\n");
		debug("%s: epll cfg = 0x%08x 0x%08x\n", __func__,
		      epll.cfg.reg.w, epll.cfg.ext_reg);
		debug("%s: epll cfg = %02x %02x %02x\n", __func__,
		      epll.cfg.reg.b.m, epll.cfg.reg.b.n, epll.cfg.reg.b.p);
		return 0;
	}

	epll_reg = readl(&scu->e_pll_param);
	epll_reg &= ~GENMASK(22, 0);
	epll_reg |= epll.cfg.reg.w;
	writel(epll_reg, &scu->e_pll_param);	
	
	/* write extend parameter */
	writel(epll.cfg.ext_reg, &scu->e_pll_ext_param);	

	/* select MAC#1 and MAC#2 clock source = EPLL / 8 */
	clksel = readl(&scu->clk_sel2);
	clksel &= ~BIT(23);
	clksel |= 0x7 << 20;
	writel(clksel, &scu->clk_sel2);

	/* 
	BIT(31): select RGMII 125M from internal source
	BIT(28): RGMII 125M output enable
	BIT(25:0): 1G default delay
	*/
	clkdelay = MAC_DEF_DELAY_1G | BIT(31) | BIT(28);
	writel(clkdelay, &scu->mac12_clk_delay);	

	/* set 100M/10M default delay */
	writel(MAC_DEF_DELAY_100M, &scu->mac12_clk_delay_100M);
	writel(MAC_DEF_DELAY_10M, &scu->mac12_clk_delay_10M);

	/* MAC AHB = HPLL / 6 */
	clksel = readl(&scu->clk_sel1);
	clksel &= ~GENMASK(18, 16);
	clksel |= 0x2 << 16;
	writel(clksel, &scu->clk_sel1);	

	return 0;
}

static u32 ast2600_configure_mac34_clk(struct ast2600_scu *scu)
{
	u32 reg;

	ast2600_configure_mac12_clk(scu);

	/* 
	BIT[31]   RGMII 125M source: 0 = from IO pin
	BIT[25:0] MAC 1G delay 
	*/
	reg = readl(&scu->mac34_clk_delay);
	reg &= ~(BIT(31) | GENMASK(25, 0));
	reg |= MAC34_DEF_DELAY_1G;
	writel(reg, &scu->mac34_clk_delay);
	writel(MAC34_DEF_DELAY_100M, &scu->mac34_clk_delay_100M);
	writel(MAC34_DEF_DELAY_10M, &scu->mac34_clk_delay_10M);

	/* clock source seletion and divider */
	reg = readl(&scu->clk_sel4);
	reg &= ~GENMASK(26, 24);	/* MAC AHB = HCLK / 2 */
	reg &= ~GENMASK(18, 16);
	reg |= 0x3 << 16;		/* RMII 50M = SLICLK_200M / 4 */
	writel(reg, &scu->clk_sel4);

	/* set driving strength */
	reg = readl(&scu->pinmux_ctrl16);
	reg &= GENMASK(3, 0);
	reg |= (0x2 << 0) | (0x2 << 2);
	writel(reg, &scu->pinmux_ctrl16);

	return 0;
}
#if 0
/**
 * WIP: ast2600 RGMII clock source tree
 * 
 *    125M from external PAD -------->|\
 *    HPLL -->|\                      | |---->RGMII 125M for MAC#1 & MAC#2
 *            | |---->| divider |---->|/                             +
 *    EPLL -->|/                                                     |
 *                                                                   |
 *    +---------<-----------|PAD output enable|<---------------------+
 *    |
 *    +--->|PAD input enable|----->|\
 *                                 | |----> RGMII 125M for MAC#3 & MAC#4
 *    SLICLK 200M -->|divider|---->|/
*/
struct ast2600_rgmii_clk_config {
	u32 mac_1_2_src;	/* 0=external PAD, 1=internal PLL */
	u32 int_clk_src;	/* 0=EPLL, 1=HPLL */
	u32 int_clk_div;
	
	u32 mac_3_4_src;	/* 0=external PAD, 1=SLICLK */
	u32 sli_clk_div;	/* reserved */
};

static void ast2600_init_rgmii_clk(struct ast2600_scu *scu, int index)
{
	debug("%s not ready\n", __func__);
}

static void ast2600_init_rmii_clk(struct ast2600_scu *scu, int index)
{
	debug("%s not ready\n", __func__);
}
#endif
static u32 ast2600_configure_mac(struct ast2600_scu *scu, int index)
{
	u32 reset_bit;
	u32 clkstop_bit;

	if (index < 3)
		ast2600_configure_mac12_clk(scu);
	else
		ast2600_configure_mac34_clk(scu);

	switch (index) {
	case 1:
		reset_bit = BIT(ASPEED_RESET_MAC1);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC1);
		writel(reset_bit, &scu->sysreset_ctrl1);
		udelay(100);
		writel(clkstop_bit, &scu->clk_stop_clr_ctrl1);
		mdelay(10);
		writel(reset_bit, &scu->sysreset_clr_ctrl1);

		break;
	case 2:
		reset_bit = BIT(ASPEED_RESET_MAC2);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC2);
		writel(reset_bit, &scu->sysreset_ctrl1);
		udelay(100);
		writel(clkstop_bit, &scu->clk_stop_clr_ctrl1);
		mdelay(10);
		writel(reset_bit, &scu->sysreset_clr_ctrl1);
		break;
	case 3:
		reset_bit = BIT(ASPEED_RESET_MAC3 - 32);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC3);
		writel(reset_bit, &scu->sysreset_ctrl2);
		udelay(100);
		writel(clkstop_bit, &scu->clk_stop_clr_ctrl2);
		mdelay(10);
		writel(reset_bit, &scu->sysreset_clr_ctrl2);
		break;
	case 4:
		reset_bit = BIT(ASPEED_RESET_MAC4 - 32);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC4);
		writel(reset_bit, &scu->sysreset_ctrl2);
		udelay(100);
		writel(clkstop_bit, &scu->clk_stop_clr_ctrl2);
		mdelay(10);
		writel(reset_bit, &scu->sysreset_clr_ctrl2);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

#define SCU_CLKSTOP_SDIO 4
static ulong ast2600_enable_sdclk(struct ast2600_scu *scu)
{
	u32 reset_bit;
	u32 clkstop_bit;

	reset_bit = BIT(ASPEED_RESET_SD - 32);
	clkstop_bit = BIT(SCU_CLKSTOP_SDIO);

	writel(reset_bit, &scu->sysreset_clr_ctrl2);
	udelay(100);
	//enable clk 
	writel(clkstop_bit, &scu->clk_stop_clr_ctrl2);
	mdelay(10);
	writel(reset_bit, &scu->sysreset_ctrl2);

	return 0;
}

#define SCU_CLKSTOP_EXTSD 31
#define SCU_CLK_SD_MASK				(0x7 << 28)
#define SCU_CLK_SD_DIV(x)			(x << 28)

static ulong ast2600_enable_extsdclk(struct ast2600_scu *scu)
{
	u32 clk_sel = readl(&scu->clk_sel4);
	u32 enableclk_bit;

	enableclk_bit = BIT(SCU_CLKSTOP_EXTSD);

	clk_sel &= ~SCU_CLK_SD_MASK;
	clk_sel |= SCU_CLK_SD_DIV(0);
	writel(clk_sel, &scu->clk_sel4);
	
	//enable clk 
	setbits_le32(&scu->clk_sel4, enableclk_bit);
	
	return 0;
}

#define SCU_CLKSTOP_EMMC 27
static ulong ast2600_enable_emmcclk(struct ast2600_scu *scu)
{
	u32 reset_bit;
	u32 clkstop_bit;

	reset_bit = BIT(ASPEED_RESET_EMMC);
	clkstop_bit = BIT(SCU_CLKSTOP_EMMC);

	writel(reset_bit, &scu->sysreset_clr_ctrl1);
	udelay(100);
	//enable clk 
	writel(clkstop_bit, &scu->clk_stop_clr_ctrl1);
	mdelay(10);
	writel(reset_bit, &scu->sysreset_ctrl2);

	return 0;
}

#define SCU_CLKSTOP_EXTEMMC 15
#define SCU_CLK_EMMC_MASK			(0x7 << 12)
#define SCU_CLK_EMMC_DIV(x)			(x << 12)

static ulong ast2600_enable_extemmcclk(struct ast2600_scu *scu)
{
	u32 clk_sel = readl(&scu->clk_sel1);
	u32 enableclk_bit;

	enableclk_bit = BIT(SCU_CLKSTOP_EXTSD);

	clk_sel &= ~SCU_CLK_SD_MASK;
	clk_sel |= SCU_CLK_SD_DIV(1);
	writel(clk_sel, &scu->clk_sel1);

	//enable clk 
	setbits_le32(&scu->clk_sel1, enableclk_bit);

	return 0;
}

static int ast2600_clk_enable(struct clk *clk)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
		case ASPEED_CLK_GATE_MAC1CLK:
			ast2600_configure_mac(priv->scu, 1);
			break;
		case ASPEED_CLK_GATE_MAC2CLK:
			ast2600_configure_mac(priv->scu, 2);
			break;
		case ASPEED_CLK_GATE_MAC3CLK:
			ast2600_configure_mac(priv->scu, 3);
			break;
		case ASPEED_CLK_GATE_MAC4CLK:
			ast2600_configure_mac(priv->scu, 4);
			break;
		case ASPEED_CLK_GATE_SDCLK:
			ast2600_enable_sdclk(priv->scu);
			break;
		case ASPEED_CLK_GATE_SDEXTCLK:
			ast2600_enable_extsdclk(priv->scu);
			break;
		case ASPEED_CLK_GATE_EMMCCLK:
			ast2600_enable_emmcclk(priv->scu);
			break;
		case ASPEED_CLK_GATE_EMMCEXTCLK:
			ast2600_enable_extemmcclk(priv->scu);
			break;
		default:
			pr_debug("can't enable clk \n");
			return -ENOENT;
			break;
	}

	return 0;
}

struct clk_ops ast2600_clk_ops = {
	.get_rate = ast2600_clk_get_rate,
	.set_rate = ast2600_clk_set_rate,
	.enable = ast2600_clk_enable,
};

static int ast2600_clk_probe(struct udevice *dev)
{
	struct ast2600_clk_priv *priv = dev_get_priv(dev);

	priv->scu = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->scu))
		return PTR_ERR(priv->scu);

	return 0;
}

static int ast2600_clk_bind(struct udevice *dev)
{
	int ret;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(gd->dm_root, "ast_sysreset", "reset", &dev);
	if (ret)
		debug("Warning: No reset driver: ret=%d\n", ret);

	return 0;
}

#if CONFIG_IS_ENABLED(CMD_CLK)
struct aspeed_clks {
	ulong id;
	const char *name;
};

static struct aspeed_clks aspeed_clk_names[] = {
	{ ASPEED_CLK_HPLL, "hpll" },
	{ ASPEED_CLK_MPLL, "mpll" },
	{ ASPEED_CLK_APLL, "apll" },
	{ ASPEED_CLK_EPLL, "epll" },
	{ ASPEED_CLK_DPLL, "dpll" },
	{ ASPEED_CLK_AHB, "hclk" },
	{ ASPEED_CLK_APB, "pclk" },
};

int soc_clk_dump(void)
{
	struct udevice *dev;
	struct clk clk;
	unsigned long rate;
	int i, ret;

	ret = uclass_get_device_by_driver(UCLASS_CLK,
					  DM_GET_DRIVER(aspeed_scu), &dev);
	if (ret)
		return ret;

	printf("Clk\t\tHz\n");

	for (i = 0; i < ARRAY_SIZE(aspeed_clk_names); i++) {
		clk.id = aspeed_clk_names[i].id;
		ret = clk_request(dev, &clk);
		if (ret < 0) {
			debug("%s clk_request() failed: %d\n", __func__, ret);
			continue;
		}

		ret = clk_get_rate(&clk);
		rate = ret;

		clk_free(&clk);

		if (ret == -ENOTSUPP) {
			printf("clk ID %lu not supported yet\n",
			       aspeed_clk_names[i].id);
			continue;
		}
		if (ret < 0) {
			printf("%s %lu: get_rate err: %d\n",
			       __func__, aspeed_clk_names[i].id, ret);
			continue;
		}

		printf("%s(%3lu):\t%lu\n",
		       aspeed_clk_names[i].name, aspeed_clk_names[i].id, rate);
	}

	return 0;
}
#endif

static const struct udevice_id ast2600_clk_ids[] = {
	{ .compatible = "aspeed,ast2600-scu", },
	{ }
};

U_BOOT_DRIVER(aspeed_scu) = {
	.name		= "aspeed_scu",
	.id		= UCLASS_CLK,
	.of_match	= ast2600_clk_ids,
	.priv_auto_alloc_size = sizeof(struct ast2600_clk_priv),
	.ops		= &ast2600_clk_ops,
	.bind		= ast2600_clk_bind,
	.probe		= ast2600_clk_probe,
};
