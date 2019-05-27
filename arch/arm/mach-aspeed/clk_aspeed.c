// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <dm/uclass.h>
#include <asm/io.h>
#ifdef CONFIG_ASPEED_AST2600
#include <asm/arch/scu_aspeed.h>
#else
#include <asm/arch/scu_ast2500.h>
#endif

int ast_get_clk(struct udevice **devp)
{
        return uclass_get_device_by_driver(UCLASS_CLK,
                                           DM_GET_DRIVER(aspeed_scu), devp);
}

#ifdef CONFIG_ASPEED_AST2600
void *ast_get_scu(void)
{
	struct aspeed_clk_priv *priv;
	struct udevice *dev;
	int ret;

	ret = ast_get_clk(&dev);
	if (ret)
		return ERR_PTR(ret);

	priv = dev_get_priv(dev);

	return priv->regs;
}
#else
void *ast_get_scu(void)
{
	struct ast2500_clk_priv *priv;
	struct udevice *dev;
	int ret;

	ret = ast_get_clk(&dev);
	if (ret)
		return ERR_PTR(ret);

	priv = dev_get_priv(dev);

	return priv->scu;
}
#endif
