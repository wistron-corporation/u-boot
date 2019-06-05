// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <reset.h>
#include <reset-uclass.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/wdt.h>
#include <asm/arch/scu_ast2600.h>

struct ast2600_reset_priv {
	/* WDT used to perform resets. */
	struct udevice *wdt;
	struct ast2600_scu *scu;
};

static int ast2600_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct ast2600_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2600_scu *scu = priv->scu;

	printf("ast2600_reset_assert reset_ctl->id %ld \n", reset_ctl->id);

	if(reset_ctl->id >= 32)
		writel(scu->sysreset_clr_ctrl2 , BIT(reset_ctl->id - 32));
	else
		writel(scu->sysreset_clr_ctrl1 , BIT(reset_ctl->id));

	return 0;
}

static int ast2600_reset_assert(struct reset_ctl *reset_ctl)
{
	struct ast2600_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2600_scu *scu = priv->scu;	

	printf("ast2600_reset_assert reset_ctl->id %ld \n", reset_ctl->id);

	if(reset_ctl->id >= 32)
		writel(scu->sysreset_ctrl2 , BIT(reset_ctl->id - 32));
	else
		writel(scu->sysreset_ctrl1 , BIT(reset_ctl->id));

	return 0;
}

static int ast2600_reset_request(struct reset_ctl *reset_ctl)
{
	printf("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	return 0;
}

static int ast2600_reset_probe(struct udevice *dev)
{
	struct ast2600_reset_priv *priv = dev_get_priv(dev);

	priv->scu = ast_get_scu();

	return 0;
}

static int aspeed_ofdata_to_platdata(struct udevice *dev)
{
	struct ast2600_reset_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_WDT, dev, "aspeed,wdt",
					   &priv->wdt);
	if (ret) {
		debug("%s: can't find WDT for reset controller", __func__);
		return ret;
	}

	return 0;
}


static const struct udevice_id aspeed_reset_ids[] = {
	{ .compatible = "aspeed,ast2600-reset" },
	{ }
};

struct reset_ops aspeed_reset_ops = {
	.rst_assert = ast2600_reset_assert,
	.rst_deassert = ast2600_reset_deassert,
	.request = ast2600_reset_request,
};

U_BOOT_DRIVER(aspeed_reset) = {
	.name		= "aspeed_reset",
	.id		= UCLASS_RESET,
	.of_match = aspeed_reset_ids,
	.probe = ast2600_reset_probe,
	.ops = &aspeed_reset_ops,
	.ofdata_to_platdata = aspeed_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct ast2600_reset_priv),
};
