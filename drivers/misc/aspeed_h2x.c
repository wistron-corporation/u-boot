// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/h2x_ast2600.h>

/* reg 0x80, 0xC0 */
#define PCIE_RX_TAG_MASK		GENMASK(23, 16)
#define PCIE_RX_LINEAR			BIT(8)
#define PCIE_RX_MSI_SEL			BIT(7)
#define PCIE_RX_MSI_EN			BIT(6)
#define PCIE_1M_ADDRESS_EN		BIT(5)
#define PCIE_UNLOCK_RX_BUFF		BIT(4)
#define PCIE_RX_TLP_TAG_MATCH	BIT(3)
#define PCIE_Wait_RX_TLP_CLR	BIT(2)
#define PCIE_RC_L_RX			BIT(1)
#define PCIE_RC_L				BIT(0)

struct aspeed_h2x_priv {
	struct aspeed_h2x_reg *h2x;
};

static u8 txTag = 0;

extern void aspeed_pcie_cfg_read(struct aspeed_h2x_reg *h2x, u8 type, u32 bdf_offset, ulong *value)
{
	u32 timeout = 0;

	txTag %= 0x7;
	printf("type = %d, busfunc = %x \n",type, bdf_offset);
	
	writel(0x04000001 | (type << 24), &h2x->h2x_tx_desc3);
	writel(0x00002000 | (txTag << 8), &h2x->h2x_tx_desc2);
	writel(bdf_offset, &h2x->h2x_tx_desc1);
	writel(0x00000000, &h2x->h2x_tx_desc0);

	//trigger tx
	writel(1, &h2x->h2x_reg24);	

	//wait tx idle
	while(!(readl(&h2x->h2x_reg24) & BIT(31))) {
		timeout++;
		if(timeout > 10000) {
			printf("time out \n");
			*value = 0xffffffff;
			goto out;
		}
	};

	//write clr tx idle
	writel(1, &h2x->h2x_reg08);
	
	//read 
	*value = readl(&h2x->h2x_rdata);

out:
	txTag++;
}

extern void aspeed_pcie_cfg_write(struct aspeed_h2x_reg *h2x, u8 type, u8 byte_en, u32 bdf_offset, u32 data)
{
	u32 timeout = 0;
	txTag %= 0x7;

	//printf("byte_en : %x, offset: %x, value = %x \n",byte_en , bdf_offset, data);

	writel(0x44000001 | (type << 24), &h2x->h2x_tx_desc3);
	writel(0x00002000 | (txTag << 8) | byte_en, &h2x->h2x_tx_desc2);
	writel(bdf_offset, &h2x->h2x_tx_desc1);
	writel(0x00000000, &h2x->h2x_tx_desc0);

	writel(data, &h2x->h2x_tx_data);

	//trigger tx
	writel(1, &h2x->h2x_reg24);	

	//wait tx idle
	while(!(readl(&h2x->h2x_reg24) & BIT(31))) {
		timeout++;
		if(timeout > 10000) {
			printf("time out \n");
			goto out;
		}
	};

	//write clr tx idle
	writel(1, &h2x->h2x_reg08);

	//clr rx done int
	writel(0x10, &h2x->h2x_reg88);
out:
	txTag++;	
}

static int aspeed_h2x_probe(struct udevice *dev)
{
	struct aspeed_h2x_priv *priv = dev_get_priv(dev);

	debug("%s(dev=%p) \n", __func__, dev);

	priv->h2x = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->h2x))
		return PTR_ERR(priv->h2x);

	//init
	writel(0x1, &priv->h2x->h2x_reg00);

	//ahb to pcie rc 
	writel(0xf0003000, &priv->h2x->h2x_reg60);
	writel(0x0, &priv->h2x->h2x_reg64);
	writel(0xFFFFFFFF, &priv->h2x->h2x_reg68);
	
	writel( PCIE_RX_LINEAR |
	PCIE_RX_MSI_SEL | PCIE_RX_MSI_EN |
	PCIE_Wait_RX_TLP_CLR | PCIE_RC_L_RX | PCIE_RC_L,
	&priv->h2x->h2x_reg80);

	//assign debug tx tag
	writel(0x28, &priv->h2x->h2x_regBC);

	return 0;
}

static const struct udevice_id aspeed_h2x_ids[] = {
	{ .compatible = "aspeed,aspeed-h2x" },
	{ }
};

U_BOOT_DRIVER(aspeed_h2x) = {
	.name		= "aspeed_h2x",
	.id			= UCLASS_MISC,
	.of_match	= aspeed_h2x_ids,
	.probe		= aspeed_h2x_probe,
	.priv_auto_alloc_size = sizeof(struct aspeed_h2x_priv),
};
