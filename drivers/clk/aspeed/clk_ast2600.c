// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/scu_aspeed.h>
#include <dm/lists.h>
#include <dt-bindings/clock/ast2600-clock.h>

/* register */
#define ASPEED_STRAP		0x70
#define SCU_HWSTRAP_VGAMEM_SHIFT	2
#define SCU_HWSTRAP_VGAMEM_MASK		(3 << SCU_HWSTRAP_VGAMEM_SHIFT)
#define SCU_HWSTRAP_MAC1_RGMII		(1 << 6)
#define SCU_HWSTRAP_MAC2_RGMII		(1 << 7)
#define SCU_HWSTRAP_DDR4		(1 << 24)
#define SCU_HWSTRAP_CLKIN_25MHZ		(1 << 23)

#define ASPEED_CLK_SELECT	0x08
#define SCU_PCLK_DIV_SHIFT		23
#define SCU_PCLK_DIV_MASK		(7 << SCU_PCLK_DIV_SHIFT)

#define ASPEED_MSIC2		0x4C	
#define SCU_MISC2_RGMII_HPLL		(1 << 23)
#define SCU_MISC2_RGMII_CLKDIV_SHIFT	20
#define SCU_MISC2_RGMII_CLKDIV_MASK	(3 << SCU_MISC2_RGMII_CLKDIV_SHIFT)
#define SCU_MISC2_RMII_MPLL		(1 << 19)
#define SCU_MISC2_RMII_CLKDIV_SHIFT	16
#define SCU_MISC2_RMII_CLKDIV_MASK	(3 << SCU_MISC2_RMII_CLKDIV_SHIFT)
#define SCU_MISC2_UARTCLK_SHIFT		24

#define ASPEED_MPLL_PARAMETER	0x20
#define SCU_MPLL_DENUM_SHIFT		0
#define SCU_MPLL_DENUM_MASK		0x1f
#define SCU_MPLL_NUM_SHIFT		5
#define SCU_MPLL_NUM_MASK		(0xff << SCU_MPLL_NUM_SHIFT)
#define SCU_MPLL_POST_SHIFT		13
#define SCU_MPLL_POST_MASK		(0x3f << SCU_MPLL_POST_SHIFT)

#define ASPEED_HPLL_PARAMETER	0x24
#define SCU_HPLL_DENUM_SHIFT		0
#define SCU_HPLL_DENUM_MASK		0x1f
#define SCU_HPLL_NUM_SHIFT		5
#define SCU_HPLL_NUM_MASK		(0xff << SCU_HPLL_NUM_SHIFT)
#define SCU_HPLL_POST_SHIFT		13
#define SCU_HPLL_POST_MASK		(0x3f << SCU_HPLL_POST_SHIFT)


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
struct aspeed_div_config {
	unsigned int num;
	unsigned int denum;
	unsigned int post_div;
};

#define AST2600_CLK_IN	25000000

/*
 * Get the rate of the M-PLL clock from input clock frequency and
 * the value of the M-PLL Parameter Register.
 */
static u32 aspeed_get_mpll_rate(struct aspeed_clk_priv *priv)
{
	u32 clkin = AST2600_CLK_IN;
	u32 mpll_reg = readl(priv->regs + ASPEED_MPLL_PARAMETER);

	const ulong num = (mpll_reg & SCU_MPLL_NUM_MASK) >> SCU_MPLL_NUM_SHIFT;
	const ulong denum = (mpll_reg & SCU_MPLL_DENUM_MASK)
			>> SCU_MPLL_DENUM_SHIFT;
	const ulong post_div = (mpll_reg & SCU_MPLL_POST_MASK)
			>> SCU_MPLL_POST_SHIFT;

	return (clkin * ((num + 1) / (denum + 1))) / (post_div + 1);
}

/*
 * Get the rate of the H-PLL clock from input clock frequency and
 * the value of the H-PLL Parameter Register.
 */
static ulong aspeed_get_hpll_rate(struct aspeed_clk_priv *priv)
{
	ulong clkin = AST2600_CLK_IN;
	u32 hpll_reg = readl(priv->regs + ASPEED_HPLL_PARAMETER);
	const ulong num = (hpll_reg & SCU_HPLL_NUM_MASK) >> SCU_HPLL_NUM_SHIFT;
	const ulong denum = (hpll_reg & SCU_HPLL_DENUM_MASK)
			>> SCU_HPLL_DENUM_SHIFT;
	const ulong post_div = (hpll_reg & SCU_HPLL_POST_MASK)
			>> SCU_HPLL_POST_SHIFT;

	return (clkin * ((num + 1) / (denum + 1))) / (post_div + 1);
}


#define ASPEED_G6_APLL_PARAMETER	0x210
#define APLL_BYPASS_EN	BIT(20)

static ulong ast2600_get_apll_clk_rate(struct aspeed_clk_priv *priv)
{
	u32 clk_in = 25000000;
	u32 val = readl(priv->regs + ASPEED_G6_APLL_PARAMETER);


	unsigned int mult, div;

	if (val & APLL_BYPASS_EN) {
		/* Pass through mode */
		mult = div = 1;
	} else {
		/* F = 25Mhz * (2-OD) * [(M + 2) / (n + 1)] */
		u32 m = (val >> 5) & 0x3f;
		u32 od = (val >> 4) & 0x1;
		u32 n = val & 0xf;

		mult = (2 - od) * (m + 2);
		div = n + 1;
	}
	return (clk_in * mult)/div;
	
	
	
}

#define ASPEED_G6_CLK_SELECT4		0x314

static ulong ast2600_get_uart_clk_rate(struct aspeed_clk_priv *priv, int uart_index)
{
	ulong uart_clkin;

	printf("ast2600_get_uart_clk_rate source %d \n\n", ast2600_get_apll_clk_rate(priv));
	return (24000000/13);
	
	if (readl(priv->regs + ASPEED_MSIC2) &
	    (1 << (uart_index - 1 + SCU_MISC2_UARTCLK_SHIFT)))
		uart_clkin = 192 * 1000 * 1000;
	else
		uart_clkin = 24 * 1000 * 1000;

	if (readl(priv->regs + ASPEED_MSIC2) & SCU_MISC_UARTCLK_DIV13)
		uart_clkin /= 13;

	return uart_clkin;
}

struct aspeed_clock_config {
	ulong input_rate;
	ulong rate;
	struct aspeed_div_config cfg;
};

static const struct aspeed_clock_config aspeed_clock_config_defaults[] = {
	{ 24000000, 250000000, { .num = 124, .denum = 1, .post_div = 5 } },
};

static bool aspeed_get_clock_config_default(ulong input_rate,
					     ulong requested_rate,
					     struct aspeed_div_config *cfg)
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
				       struct aspeed_div_config *cfg)
{
	/*
	 * The assumption is that kHz precision is good enough and
	 * also enough to avoid overflow when multiplying.
	 */
	const ulong input_rate_khz = input_rate / 1000;
	const ulong rate_khz = requested_rate / 1000;
	const struct aspeed_div_config max_vals = *cfg;
	struct aspeed_div_config it = { 0, 0, 0 };
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

static u32 aspeed_configure_ddr(struct aspeed_clk_priv *priv, ulong rate)
{
	u32 clkin = AST2600_CLK_IN;
	u32 mpll_reg;
	struct aspeed_div_config div_cfg = {
		.num = (SCU_MPLL_NUM_MASK >> SCU_MPLL_NUM_SHIFT),
		.denum = (SCU_MPLL_DENUM_MASK >> SCU_MPLL_DENUM_SHIFT),
		.post_div = (SCU_MPLL_POST_MASK >> SCU_MPLL_POST_SHIFT),
	};

	aspeed_calc_clock_config(clkin, rate, &div_cfg);

	mpll_reg = readl(priv->regs + ASPEED_MPLL_PARAMETER);
	mpll_reg &= ~(SCU_MPLL_POST_MASK | SCU_MPLL_NUM_MASK
		      | SCU_MPLL_DENUM_MASK);
	mpll_reg |= (div_cfg.post_div << SCU_MPLL_POST_SHIFT)
	    | (div_cfg.num << SCU_MPLL_NUM_SHIFT)
	    | (div_cfg.denum << SCU_MPLL_DENUM_SHIFT);

	writel(mpll_reg, priv->regs + ASPEED_MPLL_PARAMETER);

	return aspeed_get_mpll_rate(priv);
}

static u32 aspeed_configure_mac(struct aspeed_clk_priv *priv, int index)
{
	u32 clkin = AST2600_CLK_IN;
	u32 hpll_rate = aspeed_get_hpll_rate(priv);
	ulong required_rate;
	u32 hwstrap;
	u32 divisor;
	u32 reset_bit;
	u32 clkstop_bit;
#if 0
	/*
	 * According to data sheet, for 10/100 mode the MAC clock frequency
	 * should be at least 25MHz and for 1000 mode at least 100MHz
	 */
	hwstrap = readl(priv->regs + ASPEED_STRAP);
	if (hwstrap & (SCU_HWSTRAP_MAC1_RGMII | SCU_HWSTRAP_MAC2_RGMII))
		required_rate = 100 * 1000 * 1000;
	else
		required_rate = 25 * 1000 * 1000;

	divisor = hpll_rate / required_rate;

	if (divisor < 4) {
		/* Clock can't run fast enough, but let's try anyway */
		debug("MAC clock too slow\n");
		divisor = 4;
	} else if (divisor > 16) {
		/* Can't slow down the clock enough, but let's try anyway */
		debug("MAC clock too fast\n");
		divisor = 16;
	}

	switch (index) {
	case 1:
		reset_bit = SCU_SYSRESET_MAC1;
		clkstop_bit = SCU_CLKSTOP_MAC1;
		break;
	case 2:
		reset_bit = SCU_SYSRESET_MAC2;
		clkstop_bit = SCU_CLKSTOP_MAC2;
		break;
	default:
		return -EINVAL;
	}

	clrsetbits_le32(priv->regs + ASPEED_CLK_SELECT, SCU_MACCLK_MASK,
			((divisor - 2) / 2) << SCU_MACCLK_SHIFT);

	/*
	 * Disable MAC, start its clock and re-enable it.
	 * The procedure and the delays (100us & 10ms) are
	 * specified in the datasheet.
	 */
	setbits_le32(&scu->sysreset_ctrl1, reset_bit);
	udelay(100);
	clrbits_le32(&scu->clk_stop_ctrl1, clkstop_bit);
	mdelay(10);
	clrbits_le32(&scu->sysreset_ctrl1, reset_bit);

	writel((RGMII2_TXCK_DUTY << SCU_CLKDUTY_RGMII2TXCK_SHIFT)
	       | (RGMII1_TXCK_DUTY << SCU_CLKDUTY_RGMII1TXCK_SHIFT),
	       &scu->clk_duty_sel);
#endif
	return required_rate;
}

static ulong aspeed_configure_d2pll(struct aspeed_clk_priv *priv, ulong rate)
{
	/*
	 * The values and the meaning of the next three
	 * parameters are undocumented. Taken from Aspeed SDK.
	 *
	 * TODO(clg@kaod.org): the SIP and SIC values depend on the
	 * Numerator value
	 */
	 
	const u32 d2_pll_ext_param = 0x2c;
	const u32 d2_pll_sip = 0x11;
	const u32 d2_pll_sic = 0x18;
	u32 clk_delay_settings =
	    (RMII_RXCLK_IDLY << SCU_MICDS_MAC1RMII_RDLY_SHIFT)
	    | (RMII_RXCLK_IDLY << SCU_MICDS_MAC2RMII_RDLY_SHIFT)
	    | (RGMII_TXCLK_ODLY << SCU_MICDS_MAC1RGMII_TXDLY_SHIFT)
	    | (RGMII_TXCLK_ODLY << SCU_MICDS_MAC2RGMII_TXDLY_SHIFT);
	struct aspeed_div_config div_cfg = {
		.num = SCU_D2PLL_NUM_MASK >> SCU_D2PLL_NUM_SHIFT,
		.denum = SCU_D2PLL_DENUM_MASK >> SCU_D2PLL_DENUM_SHIFT,
		.post_div = SCU_D2PLL_POST_MASK >> SCU_D2PLL_POST_SHIFT,
	};
	ulong clkin = AST2600_CLK_IN;
	ulong new_rate;
#if 0
	writel((d2_pll_ext_param << SCU_D2PLL_EXT1_PARAM_SHIFT)
	       | SCU_D2PLL_EXT1_OFF
	       | SCU_D2PLL_EXT1_RESET, &scu->d2_pll_ext_param[0]);

	/*
	 * Select USB2.0 port1 PHY clock as a clock source for GCRT.
	 * This would disconnect it from D2-PLL.
	 */
	clrsetbits_le32(&scu->misc_ctrl1, SCU_MISC_D2PLL_OFF,
			SCU_MISC_GCRT_USB20CLK);

	new_rate = aspeed_calc_clock_config(clkin, rate, &div_cfg);
	writel((d2_pll_sip << SCU_D2PLL_SIP_SHIFT)
	       | (d2_pll_sic << SCU_D2PLL_SIC_SHIFT)
	       | (div_cfg.num << SCU_D2PLL_NUM_SHIFT)
	       | (div_cfg.denum << SCU_D2PLL_DENUM_SHIFT)
	       | (div_cfg.post_div << SCU_D2PLL_POST_SHIFT),
	       &scu->d2_pll_param);

	clrbits_le32(&scu->d2_pll_ext_param[0],
		     SCU_D2PLL_EXT1_OFF | SCU_D2PLL_EXT1_RESET);

	clrsetbits_le32(&scu->misc_ctrl2,
			SCU_MISC2_RGMII_HPLL | SCU_MISC2_RMII_MPLL
			| SCU_MISC2_RGMII_CLKDIV_MASK |
			SCU_MISC2_RMII_CLKDIV_MASK,
			(4 << SCU_MISC2_RMII_CLKDIV_SHIFT));

	writel(clk_delay_settings | SCU_MICDS_RGMIIPLL, &scu->mac_clk_delay);
	writel(clk_delay_settings, &scu->mac_clk_delay_100M);
	writel(clk_delay_settings, &scu->mac_clk_delay_10M);
#endif
	return new_rate;
}

static ulong ast2600_clk_get_rate(struct clk *clk)
{
	struct aspeed_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate;

	switch (clk->id) {
	//HPLL
	case ASPEED_CLK_HPLL:
		rate = aspeed_get_hpll_rate(priv);
		break;
	//HCLK
	case ASPEED_CLK_AHB:
		rate = aspeed_get_hpll_rate(priv);
		break;
	
	case MCLK_DDR:
		rate = aspeed_get_mpll_rate(priv);
		break;
	case BCLK_PCLK:
		{
			ulong apb_div = 4 + 4 * ((readl(priv->regs + ASPEED_CLK_SELECT)
						  & SCU_PCLK_DIV_MASK)
						 >> SCU_PCLK_DIV_SHIFT);
			rate = aspeed_get_hpll_rate(priv);
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
	struct aspeed_clk_priv *priv = dev_get_priv(clk->dev);

	ulong new_rate;
	switch (clk->id) {
	case PLL_MPLL:
	case MCLK_DDR:
		new_rate = aspeed_configure_ddr(priv, rate);
		break;
	case PLL_D2PLL:
		new_rate = aspeed_configure_d2pll(priv, rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static int ast2600_clk_enable(struct clk *clk)
{
	struct aspeed_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	/*
	 * For MAC clocks the clock rate is
	 * configured based on whether RGMII or RMII mode has been selected
	 * through hardware strapping.
	 */
	case PCLK_MAC1:
		aspeed_configure_mac(priv, 1);
		break;
	case PCLK_MAC2:
		aspeed_configure_mac(priv, 2);
		break;
	case PLL_D2PLL:
		aspeed_configure_d2pll(priv, D2PLL_DEFAULT_RATE);
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
	struct aspeed_clk_priv *priv = dev_get_priv(dev);

	priv->regs = devfdt_get_addr_ptr(dev);

	if (IS_ERR(priv->regs))
		return PTR_ERR(priv->regs);

	priv->version = dev_get_driver_data(dev);

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
	.priv_auto_alloc_size = sizeof(struct aspeed_clk_priv),
	.ops		= &aspeed_clk_ops,
	.bind		= ast2600_clk_bind,
	.probe		= ast2600_clk_probe,
};
