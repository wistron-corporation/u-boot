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

/*
 * Get the rate of the M-PLL clock from input clock frequency and
 * the value of the M-PLL Parameter Register.
 */
extern u32 ast2600_get_mpll_rate(struct ast2600_scu *scu)
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
extern u32 ast2600_get_hpll_rate(struct ast2600_scu *scu)
{
	u32 clkin = AST2600_CLK_IN;
	u32 hpll_reg = readl(&scu->h_pll_param);
	unsigned int mult, div = 1;

	if (hpll_reg & BIT(24)) {
		/* Pass through mode */
		mult = div = 1;
	} else {
		/* F = 25Mhz * [(M + 1) / (n + 1)] / (p + 1) */		
		u32 m = (hpll_reg & 0x1fff);
		u32 n = (hpll_reg >> 13) & 0x3f;
		u32 p = (hpll_reg >> 19) & 0xf;
		mult = (m + 1) / (n + 1);
		div = (p + 1);
	}
	return ((clkin * mult)/div);
}

extern u32 ast2600_get_dpll_rate(struct ast2600_scu *scu)
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
	
	if((hwstrap1 >> 16) & 0x1)
		axi_div = 1;
	else
		axi_div = 2;
	
	if (hw_rev & BIT(16))
		ahb_div = ast2600_a1_axi_ahb_div_table[(hwstrap1 >> 11) & 0x3];
	else
		ahb_div = ast2600_a0_axi_ahb_div_table[(hwstrap1 >> 11) & 0x3];
	
	rate = ast2600_get_hpll_rate(scu);
	rate = rate / axi_div / ahb_div;

	return rate;
}

extern u32 ast2600_get_apll_rate(struct ast2600_scu *scu)
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

extern u32 ast2600_get_epll_rate(struct ast2600_scu *scu)
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

static ulong ast2600_get_uart_clk_rate(struct ast2600_scu *scu, int uart_idx)
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

static u32 ast2600_hpll_pclk_div_table[] = {
	4, 8, 12, 16, 20, 24, 28, 32,
};
static ulong ast2600_clk_get_rate(struct clk *clk)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	switch (clk->id) {
	//HPLL
	case ASPEED_CLK_HPLL:
		rate = ast2600_get_hpll_rate(priv->scu);
		break;
	//HCLK
	case ASPEED_CLK_AHB:
		rate = ast2600_get_hclk(priv->scu);
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
	default:
		return -ENOENT;
	}

	return rate;
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

static u32 ast2600_configure_ddr(struct ast2600_scu *scu, ulong rate)
{
	u32 clkin = AST2600_CLK_IN;
	u32 mpll_reg;
	struct ast2600_div_config div_cfg = {
		.num = (SCU_MPLL_NUM_MASK >> SCU_MPLL_NUM_SHIFT),
		.denum = (SCU_MPLL_DENUM_MASK >> SCU_MPLL_DENUM_SHIFT),
		.post_div = (SCU_MPLL_POST_MASK >> SCU_MPLL_POST_SHIFT),
	};

	aspeed_calc_clock_config(clkin, rate, &div_cfg);

	mpll_reg = readl(&scu->m_pll_param);
	mpll_reg &= ~(SCU_MPLL_POST_MASK | SCU_MPLL_NUM_MASK
		      | SCU_MPLL_DENUM_MASK);
	mpll_reg |= (div_cfg.post_div << SCU_MPLL_POST_SHIFT)
	    | (div_cfg.num << SCU_MPLL_NUM_SHIFT)
	    | (div_cfg.denum << SCU_MPLL_DENUM_SHIFT);

	writel(mpll_reg, &scu->m_pll_param);

	return ast2600_get_mpll_rate(scu);
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

static u32 ast2600_configure_mac(struct ast2600_scu *scu, int index)
{
	u32 reset_bit;
	u32 clkstop_bit;


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
