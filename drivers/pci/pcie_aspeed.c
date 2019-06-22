// SPDX-License-Identifier: GPL-2.0+
/*
 *
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/arch/h2x_ast2600.h>
#include <asm/arch/ahbc_aspeed.h>

DECLARE_GLOBAL_DATA_PTR;

/* PCI Host Controller registers */

#define ASPEED_PCIE_CLASS_CODE		0x04	
#define ASPEED_PCIE_GLOBAL			0x30
#define ASPEED_PCIE_CFG_DIN		0x50
#define ASPEED_PCIE_CFG3			0x58
#define ASPEED_PCIE_LOCK			0x7C
	
#define ASPEED_PCIE_LINK			0xC0
#define ASPEED_PCIE_INT			0xC4


/* 	AST_PCIE_CFG2			0x04		*/
#define PCIE_CFG_CLASS_CODE(x)	(x << 8)
#define PCIE_CFG_REV_ID(x)		(x)

/* 	AST_PCIE_GLOBAL			0x30 	*/
#define ROOT_COMPLEX_ID(x)		(x << 4)

/* 	AST_PCIE_LOCK			0x7C	*/
#define PCIE_UNLOCK				0xa8

/*	AST_PCIE_LINK			0xC0	*/
#define PCIE_LINK_STS			BIT(5)

struct pcie_aspeed {
	void *ctrl_base;
	void *h2x_pt;
	void *cfg_base;
	fdt_size_t cfg_size;
	
	int first_busno;

	/* IO and MEM PCI regions */
	struct pci_region io;
	struct pci_region mem;
};

static int pcie_addr_valid(pci_dev_t d, int first_busno)
{
	if ((PCI_BUS(d) == first_busno) && (PCI_DEV(d) > 0))
		return 0;
	if ((PCI_BUS(d) == first_busno + 1) && (PCI_DEV(d) > 0))
		return 0;

	return 1;
}

static int pcie_aspeed_read_config(struct udevice *bus, pci_dev_t bdf,
				     uint offset, ulong *valuep,
				     enum pci_size_t size)
{
	struct pcie_aspeed *pcie = dev_get_priv(bus);

	debug("PCIE CFG read:  (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	if (!pcie_addr_valid(bdf, pcie->first_busno)) {
		printf("- out of range\n");
		*valuep = pci_get_ff(size);
		return 0;
	}

	if(PCI_BUS(bdf) == 0)
		aspeed_pcie_cfg_read(pcie->h2x_pt, 0, 
						(PCI_BUS(bdf) << 24) |
						(PCI_DEV(bdf) << 19) |
						(PCI_FUNC(bdf) << 16) | 
						offset, valuep);
	else	
		aspeed_pcie_cfg_read(pcie->h2x_pt, 0, 
						(PCI_BUS(bdf) << 24) |
						(PCI_DEV(bdf) << 19) |
						(PCI_FUNC(bdf) << 16) | 
						offset, valuep);


	return 0;
}

static int pcie_aspeed_write_config(struct udevice *bus, pci_dev_t bdf,
				      uint offset, ulong value,
				      enum pci_size_t size)
{
	struct pcie_aspeed *pcie = dev_get_priv(bus);

	debug("PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	debug("(addr,val)=(0x%04x, 0x%08lx)\n", offset, value);

	if (!pcie_addr_valid(bdf, pcie->first_busno)) {
		debug("- out of range\n");
		return 0;
	}

	if(PCI_BUS(bdf) == 0)
		aspeed_pcie_cfg_write(pcie->h2x_pt, 0, 0xf,
						(PCI_BUS(bdf) << 24) |
						(PCI_DEV(bdf) << 19) |
						(PCI_FUNC(bdf) << 16) | 
						(offset & ~3), value);
	else	
		aspeed_pcie_cfg_write(pcie->h2x_pt, 1, 0xf,
						(PCI_BUS(bdf) << 24) |
						(PCI_DEV(bdf) << 19) |
						(PCI_FUNC(bdf) << 16) | 
						(offset & ~3), value);

	return 0;
}


static int pcie_aspeed_probe(struct udevice *dev)
{
	struct pcie_aspeed *pcie = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	struct udevice *ahbc_dev, *h2x_dev;
	int ret = 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_GET_DRIVER(aspeed_ahbc),
										  &ahbc_dev);
	if (ret) {
		debug("ahbc device not defined\n");
		return ret;
	}

	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_GET_DRIVER(aspeed_h2x),
										  &h2x_dev);
	if (ret) {
		debug("h2x device not defined\n");
		return ret;
	}

	pcie->h2x_pt = devfdt_get_addr_ptr(h2x_dev);
	
	aspeed_ahbc_remap_enable(devfdt_get_addr_ptr(ahbc_dev));

	//plda enable 
	writel(PCIE_UNLOCK, pcie->ctrl_base + ASPEED_PCIE_LOCK);
	writel(PCIE_CFG_CLASS_CODE(0x60000) | PCIE_CFG_REV_ID(4), pcie->ctrl_base + ASPEED_PCIE_CLASS_CODE);
	writel(ROOT_COMPLEX_ID(0x3), pcie->ctrl_base + ASPEED_PCIE_GLOBAL);
#if 0	
	//fpga 
	writel(0x500460ff, pcie->ctrl_base + 0x2c);
#endif

	pcie->first_busno = dev->seq;

	/* Don't register host if link is down */
	if (readl(pcie->ctrl_base + ASPEED_PCIE_LINK) & PCIE_LINK_STS) {
		printf("PCIE-%d: Link up\n", dev->seq);
	} else {
		printf("PCIE-%d: Link down\n", dev->seq);
	}

	/* Store the IO and MEM windows settings for future use by the ATU */
	pcie->io.phys_start = hose->regions[0].phys_start; /* IO base */
	pcie->io.bus_start  = hose->regions[0].bus_start;  /* IO_bus_addr */
	pcie->io.size	    = hose->regions[0].size;	   /* IO size */

	pcie->mem.phys_start = hose->regions[1].phys_start; /* MEM base */
	pcie->mem.bus_start  = hose->regions[1].bus_start;  /* MEM_bus_addr */
	pcie->mem.size	     = hose->regions[1].size;	    /* MEM size */

	return 0;
}

static int pcie_aspeed_ofdata_to_platdata(struct udevice *dev)
{
	struct pcie_aspeed *pcie = dev_get_priv(dev);

	/* Get the controller base address */
	pcie->ctrl_base = (void *)devfdt_get_addr_index(dev, 0);

	/* Get the config space base address and size */
	pcie->cfg_base = (void *)devfdt_get_addr_size_index(dev, 1,
							 &pcie->cfg_size);

	printf("pcie->ctrl_base %x , pcie->cfg_base  %x \n", (u32)pcie->ctrl_base, (u32)pcie->cfg_base);

	return 0;
}

static const struct dm_pci_ops pcie_aspeed_ops = {
	.read_config	= pcie_aspeed_read_config,
	.write_config	= pcie_aspeed_write_config,
};

static const struct udevice_id pcie_aspeed_ids[] = {
	{ .compatible = "aspeed,aspeed-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_aspeed) = {
	.name			= "pcie_aspeed",
	.id				= UCLASS_PCI,
	.of_match		= pcie_aspeed_ids,
	.ops			= &pcie_aspeed_ops,
	.ofdata_to_platdata	= pcie_aspeed_ofdata_to_platdata,
	.probe			= pcie_aspeed_probe,
	.priv_auto_alloc_size	= sizeof(struct pcie_aspeed),
};
