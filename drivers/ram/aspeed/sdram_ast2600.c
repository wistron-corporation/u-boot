// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 *
 * Copyright 2016 Google, Inc
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <ram.h>
#include <regmap.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/arch/scu_ast2600.h>
#include <asm/arch/sdram_ast2600.h>
#include <asm/arch/wdt.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <dt-bindings/clock/ast2600-clock.h>

#define AST2600_SDRAMMC_FPGA
#ifdef AST2600_SDRAMMC_FPGA
#define DDR4_MR01_MODE		0x03010100
#define DDR4_MR23_MODE		0x00000000
#define DDR4_MR45_MODE		0x04C00000
#define DDR4_MR6_MODE		0x00000050
#define DDR4_TRFC		0x17263434
#else
/* derived from model GDDR4-1600 */
#define DDR4_MR01_MODE		0x03010510
#define DDR4_MR23_MODE		0x00000000
#define DDR4_MR45_MODE		0x04000000
#define DDR4_MR6_MODE           0x00000400
#define DDR4_TRFC               0x467299f1
#endif

#ifdef AST2600_SDRAMMC_FPGA
static const u32 ddr4_ac_timing[4] = {0x030C0207, 0x04451133, 0x0E010200,
                                      0x00000140};
#else
static const u32 ddr4_ac_timing[4] = {0x040e0307, 0x0f4711f1, 0x0e060304,
                                      0x00001240};
#endif

#if 1
#include "sdram_phy_ast2600.h"
#else
#define PHY_CFG_SIZE		15
static const struct {
	u32 index[PHY_CFG_SIZE];
	u32 value[PHY_CFG_SIZE];
} ddr4_phy_config = {
	.index = {0, 1, 3, 4, 5, 56, 57, 58, 59, 60, 61, 62, 36, 49, 50},
	.value = {
		0x42492aae, 0x09002000, 0x55e00b0b, 0x20000000, 0x24,
		0x03002900, 0x0e0000a0, 0x000e001c, 0x35b8c106, 0x08080607,
		0x9b000900, 0x0e400a00, 0x00100008, 0x3c183c3c, 0x00631e0e,
	},
};
#endif

#define SDRAM_SIZE_1KB		(1024U)
#define SDRAM_SIZE_1MB		(SDRAM_SIZE_1KB * SDRAM_SIZE_1KB)
#define SDRAM_MIN_SIZE		(256 * SDRAM_SIZE_1MB)
#define SDRAM_MAX_SIZE		(2048 * SDRAM_SIZE_1MB)

DECLARE_GLOBAL_DATA_PTR;

/*
 * Bandwidth configuration parameters for different SDRAM requests.
 * These are hardcoded settings taken from Aspeed SDK.
 */
static const u32 ddr_max_grant_params[4] = {0x44444444, 0x44444444, 0x44444444,
                                            0x44444444};

struct dram_info {
	struct ram_info info;
	struct clk ddr_clk;
	struct ast2600_sdrammc_regs *regs;
	void __iomem *scu;
	struct ast2600_ddr_phy *phy;
	void __iomem *phy_setting;
	void __iomem *phy_status;
	ulong clock_rate;
};

static void ast2600_sdramphy_kick_training(struct dram_info *info)
{
        struct ast2600_sdrammc_regs *regs = info->regs;
        u32 mask = SDRAM_PHYCTRL0_INIT | SDRAM_PHYCTRL0_PLL_LOCKED;
        u32 data;

        writel(0, &regs->phy_ctrl[0]);
        udelay(2);
        writel(SDRAM_PHYCTRL0_NRST | SDRAM_PHYCTRL0_INIT, &regs->phy_ctrl[0]);

        /* wait for (PLL_LOCKED == 1) and (INIT == 0) */
        do {
                data = readl(&regs->phy_ctrl[0]) & mask;
        } while (SDRAM_PHYCTRL0_PLL_LOCKED != data);
}

static int ast2600_ddr_cbr_test(struct dram_info *info)
{
	struct ast2600_sdrammc_regs *regs = info->regs;
	int i;
	const u32 test_params = SDRAM_TEST_EN
			| SDRAM_TEST_ERRSTOP
			| SDRAM_TEST_TWO_MODES;
	int ret = 0;

#if 0
	writel((1 << SDRAM_REFRESH_CYCLES_SHIFT) |
	       (0x5c << SDRAM_REFRESH_PERIOD_SHIFT), &regs->refresh_timing);
#endif	       
	writel((0xfff << SDRAM_TEST_LEN_SHIFT), &regs->test_addr);
	writel(0xff00ff00, &regs->test_init_val);
	writel(SDRAM_TEST_EN | (SDRAM_TEST_MODE_RW << SDRAM_TEST_MODE_SHIFT) |
	       SDRAM_TEST_ERRSTOP, &regs->ecc_test_ctrl);

	while (!(readl(&regs->ecc_test_ctrl) & SDRAM_TEST_DONE))
		;

	if (readl(&regs->ecc_test_ctrl) & SDRAM_TEST_FAIL) {
		ret = -EIO;
	} else {
		for (i = 0; i <= SDRAM_TEST_GEN_MODE_MASK; ++i) {
			writel((i << SDRAM_TEST_GEN_MODE_SHIFT) | test_params,
			       &regs->ecc_test_ctrl);
			while (!(readl(&regs->ecc_test_ctrl) & SDRAM_TEST_DONE))
				;
			if (readl(&regs->ecc_test_ctrl) & SDRAM_TEST_FAIL) {
				ret = -EIO;
				break;
			}
		}
	}

	writel(0, &regs->refresh_timing);
	writel(0, &regs->ecc_test_ctrl);

	return ret;
}

static size_t ast2600_sdrammc_get_vga_mem_size(struct dram_info *info)
{
        u32 vga_hwconf;
        size_t vga_mem_size_base = 8 * 1024 * 1024;

        vga_hwconf = readl(info->scu + AST_SCU_HW_STRAP) &
                     SCU_HWSTRAP_VGAMEM_MASK >> SCU_HWSTRAP_VGAMEM_SHIFT;

        return vga_mem_size_base << vga_hwconf;
}

/*
 * Find out RAM size and save it in dram_info
 *
 * The procedure is taken from Aspeed SDK
 */
static void ast2600_sdrammc_calc_size(struct dram_info *info)
{
	/* The controller supports 256/512/1024/2048 MB ram */
	size_t ram_size = SDRAM_MIN_SIZE;
	const int write_test_offset = 0x100000;
	u32 test_pattern = 0xdeadbeef;
	u32 cap_param = SDRAM_CONF_CAP_2048M;
	u32 refresh_timing_param = DDR4_TRFC;
	const u32 write_addr_base = CONFIG_SYS_SDRAM_BASE + write_test_offset;

	for (ram_size = SDRAM_MAX_SIZE; ram_size > SDRAM_MIN_SIZE;
	     ram_size >>= 1) {
		writel(test_pattern, write_addr_base + (ram_size >> 1));
		test_pattern = (test_pattern >> 4) | (test_pattern << 28);
	}

	/* One last write to overwrite all wrapped values */
	writel(test_pattern, write_addr_base);

	/* Reset the pattern and see which value was really written */
	test_pattern = 0xdeadbeef;
	for (ram_size = SDRAM_MAX_SIZE; ram_size > SDRAM_MIN_SIZE;
	     ram_size >>= 1) {
		if (readl(write_addr_base + (ram_size >> 1)) == test_pattern)
			break;

		--cap_param;
		refresh_timing_param >>= 8;
		test_pattern = (test_pattern >> 4) | (test_pattern << 28);
	}

	clrsetbits_le32(&info->regs->ac_timing[1],
			(SDRAM_AC_TRFC_MASK << SDRAM_AC_TRFC_SHIFT),
			((refresh_timing_param & SDRAM_AC_TRFC_MASK)
			 << SDRAM_AC_TRFC_SHIFT));

	info->info.base = CONFIG_SYS_SDRAM_BASE;
	info->info.size = ram_size - ast2600_sdrammc_get_vga_mem_size(info);
	clrsetbits_le32(&info->regs->config,
			(SDRAM_CONF_CAP_MASK << SDRAM_CONF_CAP_SHIFT),
			((cap_param & SDRAM_CONF_CAP_MASK)
			 << SDRAM_CONF_CAP_SHIFT));
}
/**
 * @brief	load DDR-PHY configurations table to the PHY registers
 * @param[in]	p_tbl - pointer to the configuration table
 * @param[in]	info - pointer to the DRAM info struct
*/
static void ast2600_sdramphy_init(u32 *p_tbl, struct dram_info *info)
{
	u32 reg_base = (u32)&info->phy_setting;
	u32 addr = p_tbl[0];
        u32 data;
        int i = 1;

	debug("%s:phy_base=0x%08x addr=0x%08x\n", __func__, reg_base, addr);	

        /* load PHY configuration table into PHY-setting registers */
        while (1) {
                if (addr < reg_base) {
                        debug("invalid DDR-PHY addr: 0x%08x\n", addr);
                        break;
                }
                data = p_tbl[i++];

                if (DDR_PHY_TBL_END == data) {
                        break;
                } else if (DDR_PHY_TBL_CHG_ADDR == data) {
                        addr = p_tbl[i++];
                } else {
                        writel(data, addr);
                        addr += 4;
                }
        }
}

static ast2600_sdramphy_show_status(struct dram_info *info)
{
        u32 value;
        u32 reg_base = (u32)&info->phy_status;

        debug("%s\n", __func__);

        value = readl(reg_base + 0x00);
        if (value & BIT(3)) {
                debug("initial PVT calibration fail\n");
        }
        if (value & BIT(5)) {
                debug("runtime calibration fail\n");
        }

        value = readl(reg_base + 0x30);
        debug("IO PU = 0x%02x\n", value & 0xff);
        debug("IO PD = 0x%02x\n", (value >> 16) & 0xff);

        value = readl(reg_base + 0x88);
        debug("PHY vref: 0x%02x_%02x\n", value & 0xff, (value >> 8) & 0xff);
        value = readl(reg_base + 0x90);
        debug("DDR vref: 0x%02x\n", value & 0x3f);

        value = readl(reg_base + 0x40);
        debug("MLB Gate training result: 0x%04x_%04x\n", value & 0xffff,
              (value >> 16) & 0xffff);
        value = readl(reg_base + 0x50);
        debug("MLB Gate pass window: 0x%04x_%04x\n", value & 0xffff,
              (value >> 16) & 0xffff);
        value = readl(reg_base + 0x60);
        debug("Rising  edge Read Data Eye Training Result      = 0x%x_%x\n",
              value & 0xff, (value >> 8) & 0xff);

        value = readl(reg_base + 0x68);
        debug("Rising  edge Read Data Eye Training Pass Window = 0x%x_%x\n",
              value & 0xff, (value >> 8) & 0xff);

        value = readl(reg_base + 0xC0);
        debug("Falling edge Read Data Eye Training Result      = 0x%x_%x\n",
              value & 0xff, (value >> 8) & 0xff);

        value = readl(reg_base + 0xC8);
        debug("Falling edge Read Data Eye Training Pass Window = 0x%x_%x\n",
              value & 0xff, (value >> 8) & 0xff);

        value = readl(reg_base + 0x74);
        debug("Write Data Eye fine Training Result             = %X_%X\n",
              value & 0xff, (value >> 8) & 0xff);

        value = readl(reg_base + 0x7C);
        debug("Write Data Eye Training Pass Window             = 0x%x_%x\n",
              value & 0xff, (value >> 8) & 0xff);
}

static int ast2600_sdrammc_init_ddr4(struct dram_info *info)
{
        const u32 power_ctrl = MCR34_CKE_EN | MCR34_AUTOPWRDN_EN |
                               MCR34_MREQ_BYPASS_DIS | MCR34_RESETN_DIS |
                               MCR34_ODT_EN | MCR34_ODT_AUTO_ON |
                               (0x1 << MCR34_ODT_EXT_SHIFT);

#ifdef CONFIG_DUALX8_RAM
	setbits_le32(&info->regs->config, SDRAM_CONF_DDR4 | SDRAM_CONF_DUALX8);
#else
	setbits_le32(&info->regs->config, SDRAM_CONF_DDR4);
#endif	

	ast2600_sdramphy_init(ast2600_sdramphy_config, info);
	ast2600_sdramphy_kick_training(info);

	writel(SDRAM_RESET_DLL_ZQCL_EN, &info->regs->refresh_timing);

        writel(MCR30_SET_MR(3), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(6), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(5), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(4), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(2), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(1), &info->regs->mode_setting_control);
        writel(MCR30_SET_MR(0) | MCR30_RESET_DLL_DELAY_EN,
               &info->regs->mode_setting_control);

        writel(SDRAM_REFRESH_EN | SDRAM_RESET_DLL_ZQCL_EN |
                   (0x5f << SDRAM_REFRESH_PERIOD_SHIFT),
               &info->regs->refresh_timing);

	/* wait self-refresh idle */
        while (readl(&info->regs->power_ctrl) & MCR34_SELF_REFRESH_STATUS_MASK)
                ;

        writel(SDRAM_REFRESH_EN | SDRAM_LOW_PRI_REFRESH_EN |
                   SDRAM_REFRESH_ZQCS_EN |
                   (0x5f << SDRAM_REFRESH_PERIOD_SHIFT) |
                   (0x42aa << SDRAM_REFRESH_PERIOD_ZQCS_SHIFT),
               &info->regs->refresh_timing);

	writel(power_ctrl, &info->regs->power_ctrl);

#if 0
	setbits_le32(&info->regs->config, SDRAM_CONF_CACHE_INIT_EN);
	while (!(readl(&info->regs->config) & SDRAM_CONF_CACHE_INIT_DONE))
		;
	setbits_le32(&info->regs->config, SDRAM_CONF_CACHE_EN);

	writel(SDRAM_MISC_DDR4_TREFRESH, &info->regs->misc_control);

	/* Enable all requests except video & display */
	writel(SDRAM_REQ_USB20_EHCI1
	       | SDRAM_REQ_USB20_EHCI2
	       | SDRAM_REQ_CPU
	       | SDRAM_REQ_AHB2
	       | SDRAM_REQ_AHB
	       | SDRAM_REQ_MAC0
	       | SDRAM_REQ_MAC1
	       | SDRAM_REQ_PCIE
	       | SDRAM_REQ_XDMA
	       | SDRAM_REQ_ENCRYPTION
	       | SDRAM_REQ_VIDEO_FLAG
	       | SDRAM_REQ_VIDEO_LOW_PRI_WRITE
	       | SDRAM_REQ_2D_RW
	       | SDRAM_REQ_MEMCHECK, &info->regs->req_limit_mask);
#endif	
	return 0;
}

static void ast2600_sdrammc_unlock(struct dram_info *info)
{
	writel(SDRAM_UNLOCK_KEY, &info->regs->protection_key);
	while (!readl(&info->regs->protection_key))
		;
}

static void ast2600_sdrammc_lock(struct dram_info *info)
{
	writel(~SDRAM_UNLOCK_KEY, &info->regs->protection_key);
	while (readl(&info->regs->protection_key))
		;
}

static void ast2600_sdrammc_common_init(struct ast2600_sdrammc_regs *regs)
{
	int i;

        writel(SDRAM_VIDEO_UNLOCK_KEY, &regs->gm_protection_key);
        writel(MCR34_MREQI_DIS | MCR34_RESETN_DIS, &regs->power_ctrl);
        writel(0x10 << MCR38_RW_MAX_GRANT_CNT_RQ_SHIFT,
               &regs->arbitration_ctrl);
        writel(MCR3C_DEFAULT_MASK, &regs->req_limit_mask);

        for (i = 0; i < ARRAY_SIZE(ddr_max_grant_params); ++i)
                writel(ddr_max_grant_params[i], &regs->max_grant_len[i]);

        setbits_le32(&regs->intr_ctrl, MCR50_RESET_ALL_INTR);

        /* FIXME: the sample code does NOT match the datasheet */
        writel(0x7FFFFFF, &regs->ecc_range_ctrl);

        writel(0, &regs->ecc_test_ctrl);
        writel(0, &regs->test_addr);
        writel(0, &regs->test_fail_dq_bit);
        writel(0, &regs->test_init_val);

        writel(0xFFFFFFFF, &regs->req_input_ctrl);
        writel(0, &regs->req_high_pri_ctrl);
        udelay(500);

	/* set capacity to the max size */
        clrsetbits_le32(&regs->config, SDRAM_CONF_CAP_MASK,
                        SDRAM_CONF_CAP_2048M);

	/* load controller setting */
	for (i = 0; i < ARRAY_SIZE(ddr4_ac_timing); ++i)
		writel(ddr4_ac_timing[i], &regs->ac_timing[i]);

	writel(DDR4_MR01_MODE, &regs->mr01_mode_setting);
	writel(DDR4_MR23_MODE, &regs->mr23_mode_setting);
	writel(DDR4_MR45_MODE, &regs->mr45_mode_setting);
	writel(DDR4_MR6_MODE, &regs->mr6_mode_setting);			
}

static int ast2600_sdrammc_probe(struct udevice *dev)
{
	struct reset_ctl reset_ctl;
	struct dram_info *priv = (struct dram_info *)dev_get_priv(dev);
	struct ast2600_sdrammc_regs *regs = priv->regs;
	struct udevice *clk_dev;
	int ret = clk_get_by_index(dev, 0, &priv->ddr_clk);

	if (ret) {
		debug("DDR:No CLK\n");
		return ret;
	}

	/* find SCU base address from clock device */
	ret = uclass_get_device_by_driver(UCLASS_CLK, DM_GET_DRIVER(aspeed_scu),
                                          &clk_dev);
	if (ret) {
		debug("clock device not defined\n");
		return ret;
	}

	priv->scu = devfdt_get_addr_ptr(clk_dev);
	if (IS_ERR(priv->scu)) {
		debug("%s(): can't get SCU\n", __func__);
		return PTR_ERR(priv->scu);
	}

	clk_set_rate(&priv->ddr_clk, priv->clock_rate);
	ret = reset_get_by_index(dev, 0, &reset_ctl);
	if (ret) {
		debug("%s(): Failed to get reset signal\n", __func__);
		return ret;
	}

	ret = reset_assert(&reset_ctl);
	if (ret) {
		debug("%s(): SDRAM reset failed: %u\n", __func__, ret);
		return ret;
	}

	ast2600_sdrammc_unlock(priv);
	ast2600_sdrammc_common_init(regs);	

	if (readl(priv->scu + AST_SCU_HW_STRAP) & SCU_HWSTRAP_DDR3) {
		debug("Unsupported DRAM3\n");
		return -EINVAL;
	} else {
		ast2600_sdrammc_init_ddr4(priv);
	}	

	ast2600_sdramphy_show_status(priv);
	ast2600_sdrammc_calc_size(priv);

	clrbits_le32(&regs->intr_ctrl, MCR50_RESET_ALL_INTR);
	ast2600_sdrammc_lock(priv);

	return 0;
}

static int ast2600_sdrammc_ofdata_to_platdata(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);
	struct regmap *map;
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &map);
	if (ret)
		return ret;

	priv->regs = regmap_get_range(map, 0);
	priv->phy_setting = regmap_get_range(map, 1);
	priv->phy_status = regmap_get_range(map, 2);

	priv->clock_rate = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					  "clock-frequency", 0);

	if (!priv->clock_rate) {
		debug("DDR Clock Rate not defined\n");
		return -EINVAL;
	}

	return 0;
}

static int ast2600_sdrammc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops ast2600_sdrammc_ops = {
	.get_info = ast2600_sdrammc_get_info,
};

static const struct udevice_id ast2600_sdrammc_ids[] = {
	{ .compatible = "aspeed,ast2600-sdrammc" },
	{ }
};

U_BOOT_DRIVER(sdrammc_ast2600) = {
	.name = "aspeed_ast2600_sdrammc",
	.id = UCLASS_RAM,
	.of_match = ast2600_sdrammc_ids,
	.ops = &ast2600_sdrammc_ops,
	.ofdata_to_platdata = ast2600_sdrammc_ofdata_to_platdata,
	.probe = ast2600_sdrammc_probe,
	.priv_auto_alloc_size = sizeof(struct dram_info),
};
