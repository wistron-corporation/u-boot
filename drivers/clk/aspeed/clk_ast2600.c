// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
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
#define RGMII_TXCLK_ODLY		8
#define RMII_RXCLK_IDLY		2

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
struct ast2600_div_config {
	unsigned int num;
	unsigned int denum;
	unsigned int post_div;
};

#define AST2600_CLK_IN	25000000

/*
 * Get the rate of the M-PLL clock from input clock frequency and
 * the value of the M-PLL Parameter Register.
 */
static u32 ast2600_get_mpll_rate(struct ast2600_scu *scu)
{
	u32 clkin = AST2600_CLK_IN;
	u32 mpll_reg = readl(&scu->m_pll_param);
	unsigned int mult, div = 1;

	if (mpll_reg & BIT(24)) {
		/* Pass through mode */
		mult = div = 1;
	} else {
		/* F = 25Mhz * [(M + 2) / (n + 1)] / (p + 1) */
		u32 m = mpll_reg  & 0x1fff;
		u32 n = (mpll_reg >> 13) & 0x3f;
		u32 p = (mpll_reg >> 19) & 0xf;
		mult = (m + 1) / (n + 1);
		div = (p + 1);
	}
	return ((clkin * mult)/div);

}

/*
 * Get the rate of the H-PLL clock from input clock frequency and
 * the value of the H-PLL Parameter Register.
 */
static ulong ast2600_get_hpll_rate(struct ast2600_scu *scu)
{
	ulong clkin = AST2600_CLK_IN;
	u32 hpll_reg = readl(&scu->h_pll_param);

	const ulong num = (hpll_reg & 0x1fff);
	const ulong denum = (hpll_reg >> 13) & 0x3f;
	const ulong post_div = (hpll_reg >> 19) & 0xf;

	return (clkin * ((num + 1) / (denum + 1))) / (post_div + 1);
}

static ulong ast2600_get_apll_rate(struct ast2600_scu *scu)
{
	u32 clk_in = AST2600_CLK_IN;
	u32 apll_reg = readl(&scu->a_pll_param);
	unsigned int mult, div = 1;

	if (apll_reg & BIT(20)) {
		/* Pass through mode */
		mult = div = 1;
	} else {
		/* F = 25Mhz * (2-OD) * [(M + 2) / (n + 1)] */
		u32 m = (apll_reg >> 5) & 0x3f;
		u32 od = (apll_reg >> 4) & 0x1;
		u32 n = apll_reg & 0xf;

		mult = (2 - od) * ((m + 2) / (n + 1));
	}
	return (clk_in * mult)/div;
}

static ulong ast2600_get_epll_rate(struct ast2600_scu *scu)
{
	u32 clk_in = AST2600_CLK_IN;
	u32 epll_reg = readl(&scu->e_pll_param);
	unsigned int mult, div = 1;

	if (epll_reg & BIT(24)) {
		/* Pass through mode */
		mult = div = 1;
	} else {
		/* F = 25Mhz * [(M + 2) / (n + 1)] / (p + 1)*/
		u32 m = epll_reg  & 0x1fff;
		u32 n = (epll_reg >> 13) & 0x3f;		
		u32 p = (epll_reg >> 19) & 0x7;

		mult = ((m + 1) / (n + 1));
		div = (p + 1);
	}
	return (clk_in * mult)/div;
}

static ulong ast2600_get_dpll_rate(struct ast2600_scu *scu)
{
	u32 clk_in = AST2600_CLK_IN;
	u32 dpll_reg = readl(&scu->d_pll_param);
	unsigned int mult, div = 1;

	if (dpll_reg & BIT(24)) {
		/* Pass through mode */
		mult = div = 1;
	} else {
		/* F = 25Mhz * [(M + 2) / (n + 1)] / (p + 1)*/
		u32 m = dpll_reg  & 0x1fff;
		u32 n = (dpll_reg >> 13) & 0x3f;		
		u32 p = (dpll_reg >> 19) & 0x7;
		mult = ((m + 1) / (n + 1));
		div = (p + 1);
	}
	return (clk_in * mult)/div;
}

static ulong ast2600_get_uart_clk_rate(struct ast2600_clk_priv *priv, int uart_index)
{
	ulong uart_clkin;

	printf("ast2600_get_uart_clk_rate source %ld \n\n", ast2600_get_apll_rate(priv->scu));
	return (24000000/13);
	
	if (readl(&priv->scu->misc_ctrl2) &
	    (1 << (uart_index - 1 + SCU_MISC2_UARTCLK_SHIFT)))
		uart_clkin = 192 * 1000 * 1000;
	else
		uart_clkin = 24 * 1000 * 1000;

	if (readl(&priv->scu->misc_ctrl2) & SCU_MISC_UARTCLK_DIV13)
		uart_clkin /= 13;

	return uart_clkin;
}

struct aspeed_clock_config {
	ulong input_rate;
	ulong rate;
	struct ast2600_div_config cfg;
};

static const struct aspeed_clock_config aspeed_clock_config_defaults[] = {
	{ 25000000, 400000000, { .num = 95, .denum = 2, .post_div = 1 } },
};

static bool aspeed_get_clock_config_default(ulong input_rate,
					     ulong requested_rate,
					     struct ast2600_div_config *cfg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(aspeed_clock_config_defaults); i++) {
		const struct aspeed_clock_config *default_cfg =
			&aspeed_clock_config_defaults[i];
		if (default_cfg->input_rate == input_rate &&
		    default_cfg->rate == requested_rate) {
			*cfg = default_cfg->cfg;
			return true;
		}
	}

	return false;
}

/*
 * @input_rate - the rate of input clock in Hz
 * @requested_rate - desired output rate in Hz
 * @div - this is an IN/OUT parameter, at input all fields of the config
 * need to be set to their maximum allowed values.
 * The result (the best config we could find), would also be returned
 * in this structure.
 *
 * @return The clock rate, when the resulting div_config is used.
 */
static ulong aspeed_calc_clock_config(ulong input_rate, ulong requested_rate,
				       struct ast2600_div_config *cfg)
{
	/*
	 * The assumption is that kHz precision is good enough and
	 * also enough to avoid overflow when multiplying.
	 */
	const ulong input_rate_khz = input_rate / 1000;
	const ulong rate_khz = requested_rate / 1000;
	const struct ast2600_div_config max_vals = *cfg;
	struct ast2600_div_config it = { 0, 0, 0 };
	ulong delta = rate_khz;
	ulong new_rate_khz = 0;

	/*
	 * Look for a well known frequency first.
	 */
	if (aspeed_get_clock_config_default(input_rate, requested_rate, cfg))
		return requested_rate;

	for (; it.denum <= max_vals.denum; ++it.denum) {
		for (it.post_div = 0; it.post_div <= max_vals.post_div;
		     ++it.post_div) {
			it.num = (rate_khz * (it.post_div + 1) / input_rate_khz)
			    * (it.denum + 1);
			if (it.num > max_vals.num)
				continue;

			new_rate_khz = (input_rate_khz
					* ((it.num + 1) / (it.denum + 1)))
			    / (it.post_div + 1);

			/* Keep the rate below requested one. */
			if (new_rate_khz > rate_khz)
				continue;

			if (new_rate_khz - rate_khz < delta) {
				delta = new_rate_khz - rate_khz;
				*cfg = it;
				if (delta == 0)
					return new_rate_khz * 1000;
			}
		}
	}

	return new_rate_khz * 1000;
}

static u32 ast2600_configure_ddr(struct ast2600_clk_priv *priv, ulong rate)
{
	u32 clkin = AST2600_CLK_IN;
	u32 mpll_reg;
	struct ast2600_div_config div_cfg = {
		.num = (SCU_MPLL_NUM_MASK >> SCU_MPLL_NUM_SHIFT),
		.denum = (SCU_MPLL_DENUM_MASK >> SCU_MPLL_DENUM_SHIFT),
		.post_div = (SCU_MPLL_POST_MASK >> SCU_MPLL_POST_SHIFT),
	};

	aspeed_calc_clock_config(clkin, rate, &div_cfg);

	mpll_reg = readl(&priv->scu->m_pll_param);
	mpll_reg &= ~(SCU_MPLL_POST_MASK | SCU_MPLL_NUM_MASK
		      | SCU_MPLL_DENUM_MASK);
	mpll_reg |= (div_cfg.post_div << SCU_MPLL_POST_SHIFT)
	    | (div_cfg.num << SCU_MPLL_NUM_SHIFT)
	    | (div_cfg.denum << SCU_MPLL_DENUM_SHIFT);

	writel(mpll_reg, &priv->scu->m_pll_param);

	return ast2600_get_mpll_rate(priv->scu);
}

#define SCU_CLKSTOP_MAC1		(20)
#define SCU_CLKSTOP_MAC2		(21)
#define SCU_CLKSTOP_MAC3		(20)
#define SCU_CLKSTOP_MAC4		(21)

static u32 ast2600_configure_mac(struct ast2600_scu *scu, int index)
{
	u32 reset_bit;
	u32 clkstop_bit;


	switch (index) {
	case 1:
		reset_bit = BIT(ASPEED_RESET_MAC1);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC1);
		writel(&scu->sysreset_ctrl1, reset_bit);
		udelay(100);
		clrbits_le32(&scu->clk_stop_clr_ctrl1, clkstop_bit);
		mdelay(10);
		writel(&scu->sysreset_clr_ctrl1, reset_bit);
		
	
		break;
	case 2:
		reset_bit = BIT(ASPEED_RESET_MAC2);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC2);
		writel(&scu->sysreset_ctrl1, reset_bit);
		udelay(100);
		writel(&scu->clk_stop_clr_ctrl1, clkstop_bit);
		mdelay(10);
		writel(&scu->sysreset_clr_ctrl1, reset_bit);
		break;
	case 3:
		reset_bit = BIT(ASPEED_RESET_MAC3 - 32);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC3);
		reset_bit = BIT(ASPEED_RESET_MAC2);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC2);
		writel(&scu->sysreset_ctrl2, reset_bit);
		udelay(100);
		writel(&scu->clk_stop_clr_ctrl2, clkstop_bit);
		mdelay(10);
		writel(&scu->sysreset_clr_ctrl2, reset_bit);
	
		break;
	case 4:
		reset_bit = BIT(ASPEED_RESET_MAC4 - 32);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC4);
		reset_bit = BIT(ASPEED_RESET_MAC2);
		clkstop_bit = BIT(SCU_CLKSTOP_MAC2);
		writel(&scu->sysreset_ctrl2, reset_bit);
		udelay(100);
		writel(&scu->clk_stop_clr_ctrl2, clkstop_bit);
		mdelay(10);
		writel(&scu->sysreset_clr_ctrl2, reset_bit);
	
		break;		
	default:
		return -EINVAL;
	}

	return 0;
}

static u32 ast2600_a0_axi_ahb_div_table[] = {
	2, 2, 3, 5,
};

static u32 ast2600_a1_axi_ahb_div_table[] = {
	4, 6, 2, 4,
};

static u32 ast2600_hpll_pclk_div_table[] = {
	4, 8, 12, 16, 20, 24, 28, 32,
};
static ulong ast2600_clk_get_rate(struct clk *clk)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate;

	switch (clk->id) {
	//HPLL
	case ASPEED_CLK_HPLL:
		rate = ast2600_get_hpll_rate(priv->scu);
		printf("hpll %ld \n", rate);
		break;
	//HCLK
	case ASPEED_CLK_AHB:
		{
			u32 hw_rev = readl(&priv->scu->chip_id0);
			u32 hwstrap1 = readl(&priv->scu->hwstrap1);
			u32 axi_div = 1;
			u32 ahb_div = 0;
			if((hwstrap1 >> 16) & 0x1)
				axi_div = 1;
			else
				axi_div = 2;

			if (hw_rev & BIT(16))
				ahb_div = ast2600_a1_axi_ahb_div_table[(hwstrap1 >> 11) & 0x3];
			else
				ahb_div = ast2600_a0_axi_ahb_div_table[(hwstrap1 >> 11) & 0x3];
			
			rate = ast2600_get_hpll_rate(priv->scu);
			rate = rate / axi_div / ahb_div;
			printf("hclk %ld \n", rate);
		}
		break;
	case ASPEED_CLK_MPLL:
		rate = ast2600_get_mpll_rate(priv->scu);
		break;
	//pclk
	case ASPEED_CLK_APB:
		{
			u32 clk_sel1 = readl(&priv->scu->clk_sel1);
			u32 apb_div = ast2600_hpll_pclk_div_table[((clk_sel1 >> 23) & 0x7)];
			rate = ast2600_get_hpll_rate(priv->scu);
			rate = rate / apb_div;
		}
		break;
	case PCLK_UART1:
		rate = ast2600_get_uart_clk_rate(priv, 1);
		break;
	case PCLK_UART2:
		rate = ast2600_get_uart_clk_rate(priv, 2);
		break;
	case PCLK_UART3:
		rate = ast2600_get_uart_clk_rate(priv, 3);
		break;
	case PCLK_UART4:
		rate = ast2600_get_uart_clk_rate(priv, 4);
		break;
	case ASPEED_CLK_GATE_UART5CLK:
		rate = ast2600_get_uart_clk_rate(priv, 5);
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static ulong ast2600_clk_set_rate(struct clk *clk, ulong rate)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);

	ulong new_rate;
	switch (clk->id) {
	case ASPEED_CLK_MPLL:
		new_rate = ast2600_configure_ddr(priv, rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static int ast2600_clk_enable(struct clk *clk)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	/*
	 * For MAC clocks the clock rate is
	 * configured based on whether RGMII or RMII mode has been selected
	 * through hardware strapping.
	 */
	case ASPEED_CLK_GATE_MAC1CLK:
		printf("ast2600_clk_enable mac 1 ~~~\n");
		ast2600_configure_mac(priv->scu, 1);
		break;
	case ASPEED_CLK_GATE_MAC2CLK:
		printf("ast2600_clk_enable mac 2 ~~~\n");
		ast2600_configure_mac(priv->scu, 2);
		break;
	default:
		return -ENOENT;
	}

	return 0;
}

struct clk_ops aspeed_clk_ops = {
	.get_rate = ast2600_clk_get_rate,
	.set_rate = ast2600_clk_set_rate,
	.enable = ast2600_clk_enable,
};

static int ast2600_clk_probe(struct udevice *dev)
{
	char buf[32];
	struct ast2600_clk_priv *priv = dev_get_priv(dev);

	priv->scu = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->scu))
		return PTR_ERR(priv->scu);

	printf("PLL   : %4s MHz\n", strmhz(buf, AST2600_CLK_IN));
	printf("HPLL  : %4s MHz\n", strmhz(buf, ast2600_get_hpll_rate(priv->scu)));
	printf("MPLL  :	%4s Mhz\n", strmhz(buf, ast2600_get_mpll_rate(priv->scu)));
	printf("APLL  :	%4s Mhz\n", strmhz(buf, ast2600_get_apll_rate(priv->scu)));
	printf("EPLL :	%4s Mhz\n", strmhz(buf, ast2600_get_epll_rate(priv->scu)));
	printf("DPLL :	%4s Mhz\n", strmhz(buf, ast2600_get_dpll_rate(priv->scu)));


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

static const struct udevice_id ast2600_clk_ids[] = {
	{ .compatible = "aspeed,ast2600-scu", },
	{ }
};

U_BOOT_DRIVER(aspeed_scu) = {
	.name		= "aspeed_scu",
	.id		= UCLASS_CLK,
	.of_match	= ast2600_clk_ids,
	.priv_auto_alloc_size = sizeof(struct ast2600_clk_priv),
	.ops		= &aspeed_clk_ops,
	.bind		= ast2600_clk_bind,
	.probe		= ast2600_clk_probe,
};
