// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <dm/uclass.h>
#include <asm/io.h>
#ifdef CONFIG_ASPEED_AST2600
#include <asm/arch/scu_ast2600.h>
#else
#include <asm/arch/scu_ast2500.h>
#endif

int ast_get_clk(struct udevice **devp)
{
        return uclass_get_device_by_driver(UCLASS_CLK,
                                           DM_GET_DRIVER(aspeed_scu), devp);
}

void *ast_get_scu(void)
{
#ifdef CONFIG_ASPEED_AST2600
	struct ast2600_clk_priv *priv;
#endif

#ifdef CONFIG_ASPEED_AST2500
	struct ast2500_clk_priv *priv;
#endif

	struct udevice *dev;
	int ret;

	ret = ast_get_clk(&dev);
	if (ret)
		return ERR_PTR(ret);

	priv = dev_get_priv(dev);

	return priv->scu;
}

