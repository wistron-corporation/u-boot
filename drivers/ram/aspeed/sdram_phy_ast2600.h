#define DDR_PHY_TBL_CHG_ADDR            0xaeeddeea
#define DDR_PHY_TBL_END                 0xaeededed

#define DDR_PHY_TBL_POST_SIM_1600	0
#define DDR_PHY_TBL_POST_SIM_800	1
#define DDR_PHY_TBL_GUC_1600		2
#define DDR_PHY_TBL_GUC_800		3
#define DDR_PHY_TBL_SEL			DDR_PHY_TBL_GUC_1600		
/* DDR4-1600 PHY configuration table */
#if (DDR_PHY_TBL_SEL == DDR_PHY_TBL_GUC_800)
u32 ast2600_sdramphy_config[165] = {
	0x1e6e0100,	// start address
	0x00000000,	// phyr000
	0x0c002062,	// phyr004
	0x1a7a0063,	// phyr008
	0x5a7a0063,	// phyr00c
	0x1a7a0063,	// phyr010
	0x1a7a0063,	// phyr014
	0x20000000,	// phyr018
	0x20000000,	// phyr01c
	0x20000000,	// phyr020
	0x20000000,	// phyr024
	0x00000008,	// phyr028
	0x00000000,	// phyr02c
	0x00077600,	// phyr030
	0x00000000,	// phyr034
	0x00000000,	// phyr038
	0x20000000,	// phyr03c
	0x50506000,	// phyr040
	0x50505050,	// phyr044
	0x00002f07,	// phyr048
	0x00003080,	// phyr04c
	0x04000000,	// phyr050
	0x00000200,	// phyr054
	0x03140201,	// phyr058
	0x04800000,	// phyr05c
	0x0800044e,	// phyr060
	0x00000000,	// phyr064
	0x00180008,	// phyr068
	0x00e00400,	// phyr06c
	0x00140206,	// phyr070
	0x1d4c0000,	// phyr074
	0x493e0107,	// phyr078
	0x08060404,	// phyr07c
	0x90000a00,	// phyr080
	0x06420618,	// phyr084
	0x00001002,	// phyr088
	0x05701016,	// phyr08c
	0x10000000,	// phyr090
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x00002020,	// phyr0a8
	0x00000001,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e01cc,	// new address
	0x01010101,	// phyr0cc
	0x01010101,	// phyr0d0
	0x80808080,	// phyr0d4
	0x80808080,	// phyr0d8
	0xaeeddeea,	// change address
	0x1e6e0288,	// new address
	0x80808080,	// phyr188
	0x80808080,	// phyr18c
	0x80808080,	// phyr190
	0x80808080,	// phyr194
	0xaeeddeea,	// change address
	0x1e6e02f8,	// new address
	0x90909090,	// phyr1f8
	0x88888888,	// phyr1fc
	0xaeeddeea,	// change address
	0x1e6e0300,	// new address
	0x00000000,	// phyr200
	0xaeeddeea,	// change address
	0x1e6e0194,	// new address
	0x80118260,	// phyr094
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x80000000,	// phyr0a8
	0x00000001,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e0318,	// new address
	0x09222719,	// phyr218
	0x00aa4403,	// phyr21c
	0xaeeddeea,	// change address
	0x1e6e0198,	// new address
	0x08060000,	// phyr098
	0xaeeddeea,	// change address
	0x1e6e01b0,	// new address
	0x00000000,	// phyr0b0
	0x00000000,	// phyr0b4
	0x00000000,	// phyr0b8
	0x00000000,	// phyr0bc
	0x00000000,	// phyr0c0
	0x00000000,	// phyr0c4
	0x000aff2c,	// phyr0c8
	0xaeeddeea,	// change address
	0x1e6e01dc,	// new address
	0x00080000,	// phyr0dc
	0x00000000,	// phyr0e0
	0xaa55aa55,	// phyr0e4
	0x55aa55aa,	// phyr0e8
	0xaaaa5555,	// phyr0ec
	0x5555aaaa,	// phyr0f0
	0xaa55aa55,	// phyr0f4
	0x55aa55aa,	// phyr0f8
	0xaaaa5555,	// phyr0fc
	0x5555aaaa,	// phyr100
	0xaa55aa55,	// phyr104
	0x55aa55aa,	// phyr108
	0xaaaa5555,	// phyr10c
	0x5555aaaa,	// phyr110
	0xaa55aa55,	// phyr114
	0x55aa55aa,	// phyr118
	0xaaaa5555,	// phyr11c
	0x5555aaaa,	// phyr120
	0x20202020,	// phyr124
	0x20202020,	// phyr128
	0x20202020,	// phyr12c
	0x20202020,	// phyr130
	0x20202020,	// phyr134
	0x20202020,	// phyr138
	0x20202020,	// phyr13c
	0x20202020,	// phyr140
	0x20202020,	// phyr144
	0x20202020,	// phyr148
	0x20202020,	// phyr14c
	0x20202020,	// phyr150
	0x20202020,	// phyr154
	0x20202020,	// phyr158
	0x20202020,	// phyr15c
	0x20202020,	// phyr160
	0x20202020,	// phyr164
	0x20202020,	// phyr168
	0x20202020,	// phyr16c
	0x20202020,	// phyr170
	0xaeeddeea,	// change address
	0x1e6e0298,	// new address
	0x20200800,	// phyr198
	0x20202020,	// phyr19c
	0x20202020,	// phyr1a0
	0x20202020,	// phyr1a4
	0x20202020,	// phyr1a8
	0x20202020,	// phyr1ac
	0x20202020,	// phyr1b0
	0x20202020,	// phyr1b4
	0x20202020,	// phyr1b8
	0x20202020,	// phyr1bc
	0x20202020,	// phyr1c0
	0x20202020,	// phyr1c4
	0x20202020,	// phyr1c8
	0x20202020,	// phyr1cc
	0x20202020,	// phyr1d0
	0x20202020,	// phyr1d4
	0x20202020,	// phyr1d8
	0x20202020,	// phyr1dc
	0x20202020,	// phyr1e0
	0x20202020,	// phyr1e4
	0x00002020,	// phyr1e8
	0xaeeddeea,	// change address
	0x1e6e0304,	// new address
	0x00000800,	// phyr204
	0xaeeddeea,	// change address
	0x1e6e027c,	// new address
	0x4e400000,	// phyr17c
	0x40404040,	// phyr180
	0x40404040,	// phyr184
	0xaeeddeea,	// change address
	0x1e6e02f4,	// new address
	0x00000059,	// phyr1f4
	0xaeededed,	// end
};
#elif (DDR_PHY_TBL_SEL == DDR_PHY_TBL_GUC_1600)
u32 ast2600_sdramphy_config[165] = {
	0x1e6e0100,	// start address
	0x00000000,	// phyr000
	0x0c002062,	// phyr004
	0x1a7a0063,	// phyr008
	0x5a7a0063,	// phyr00c
	0x1a7a0063,	// phyr010
	0x1a7a0063,	// phyr014
	0x20000000,	// phyr018
	0x20000000,	// phyr01c
	0x20000000,	// phyr020
	0x20000000,	// phyr024
	0x00000008,	// phyr028
	0x00000000,	// phyr02c
	0x00077600,	// phyr030
	0x00000000,	// phyr034
	0x00000000,	// phyr038
	0x20000000,	// phyr03c
	0x50506000,	// phyr040
	0x50505050,	// phyr044
	0x00002f07,	// phyr048
	0x00003080,	// phyr04c
	0x04000000,	// phyr050
	0x00000200,	// phyr054
	0x03140201,	// phyr058
	0x04800000,	// phyr05c
	0x0800044e,	// phyr060
	0x00000000,	// phyr064
	0x00180008,	// phyr068
	0x00e00400,	// phyr06c
	0x00140206,	// phyr070
	0x1d4c0000,	// phyr074
	0x493e0107,	// phyr078
	0x08060404,	// phyr07c
	0x90000a00,	// phyr080
	0x06420c30,	// phyr084
	0x00001002,	// phyr088
	0x05701016,	// phyr08c
	0x10000000,	// phyr090
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x00002020,	// phyr0a8
	0x00000001,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e01cc,	// new address
	0x01010101,	// phyr0cc
	0x01010101,	// phyr0d0
	0x80808080,	// phyr0d4
	0x80808080,	// phyr0d8
	0xaeeddeea,	// change address
	0x1e6e0288,	// new address
	0x80808080,	// phyr188
	0x80808080,	// phyr18c
	0x80808080,	// phyr190
	0x80808080,	// phyr194
	0xaeeddeea,	// change address
	0x1e6e02f8,	// new address
	0x90909090,	// phyr1f8
	0x88888888,	// phyr1fc
	0xaeeddeea,	// change address
	0x1e6e0300,	// new address
	0x00000000,	// phyr200
	0xaeeddeea,	// change address
	0x1e6e0194,	// new address
	0x80118260,	// phyr094
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x80000000,	// phyr0a8
	0x00000001,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e0318,	// new address
	0x09222719,	// phyr218
	0x00aa4403,	// phyr21c
	0xaeeddeea,	// change address
	0x1e6e0198,	// new address
	0x08060000,	// phyr098
	0xaeeddeea,	// change address
	0x1e6e01b0,	// new address
	0x00000000,	// phyr0b0
	0x00000000,	// phyr0b4
	0x00000000,	// phyr0b8
	0x00000000,	// phyr0bc
	0x00000000,	// phyr0c0
	0x00000000,	// phyr0c4
	0x000aff2c,	// phyr0c8
	0xaeeddeea,	// change address
	0x1e6e01dc,	// new address
	0x00080000,	// phyr0dc
	0x00000000,	// phyr0e0
	0xaa55aa55,	// phyr0e4
	0x55aa55aa,	// phyr0e8
	0xaaaa5555,	// phyr0ec
	0x5555aaaa,	// phyr0f0
	0xaa55aa55,	// phyr0f4
	0x55aa55aa,	// phyr0f8
	0xaaaa5555,	// phyr0fc
	0x5555aaaa,	// phyr100
	0xaa55aa55,	// phyr104
	0x55aa55aa,	// phyr108
	0xaaaa5555,	// phyr10c
	0x5555aaaa,	// phyr110
	0xaa55aa55,	// phyr114
	0x55aa55aa,	// phyr118
	0xaaaa5555,	// phyr11c
	0x5555aaaa,	// phyr120
	0x20202020,	// phyr124
	0x20202020,	// phyr128
	0x20202020,	// phyr12c
	0x20202020,	// phyr130
	0x20202020,	// phyr134
	0x20202020,	// phyr138
	0x20202020,	// phyr13c
	0x20202020,	// phyr140
	0x20202020,	// phyr144
	0x20202020,	// phyr148
	0x20202020,	// phyr14c
	0x20202020,	// phyr150
	0x20202020,	// phyr154
	0x20202020,	// phyr158
	0x20202020,	// phyr15c
	0x20202020,	// phyr160
	0x20202020,	// phyr164
	0x20202020,	// phyr168
	0x20202020,	// phyr16c
	0x20202020,	// phyr170
	0xaeeddeea,	// change address
	0x1e6e0298,	// new address
	0x20200800,	// phyr198
	0x20202020,	// phyr19c
	0x20202020,	// phyr1a0
	0x20202020,	// phyr1a4
	0x20202020,	// phyr1a8
	0x20202020,	// phyr1ac
	0x20202020,	// phyr1b0
	0x20202020,	// phyr1b4
	0x20202020,	// phyr1b8
	0x20202020,	// phyr1bc
	0x20202020,	// phyr1c0
	0x20202020,	// phyr1c4
	0x20202020,	// phyr1c8
	0x20202020,	// phyr1cc
	0x20202020,	// phyr1d0
	0x20202020,	// phyr1d4
	0x20202020,	// phyr1d8
	0x20202020,	// phyr1dc
	0x20202020,	// phyr1e0
	0x20202020,	// phyr1e4
	0x00002020,	// phyr1e8
	0xaeeddeea,	// change address
	0x1e6e0304,	// new address
	0x00000800,	// phyr204
	0xaeeddeea,	// change address
	0x1e6e027c,	// new address
	0x4e400000,	// phyr17c
	0x40404040,	// phyr180
	0x40404040,	// phyr184
	0xaeeddeea,	// change address
	0x1e6e02f4,	// new address
	0x00000059,	// phyr1f4
	0xaeededed,	// end
};
#elif (DDR_PHY_TBL_SEL == DDR_PHY_TBL_POST_SIM_1600)
u32 ast2600_sdramphy_config[163] = {
	0x1e6e0100,	// start address
	0x022a0092,	// phyr000
	0x00000042,	// phyr004
	0x1a7a0063,	// phyr008
	0x1a7a0063,	// phyr00C
	0x1a7a0063,	// phyr010
	0x1a7a0063,	// phyr014
	0x20000000,	// phyr018
	0x20000000,	// phyr01C
	0x20000000,	// phyr020
	0x20000000,	// phyr024
	0x00000008,	// phyr028
	0x00000000,	// phyr02C
	0x00077600,	// phyr030
	0x00000000,	// phyr034
	0x00000000,	// phyr038
	0x20000000,	// phyr03C
	0x00000000,	// phyr040
	0x00000000,	// phyr044
	0x00000000,	// phyr048
	0x00003080,	// phyr04C
	0xf400f000,	// phyr050
	0x00000000,	// phyr054
	0x07100001,	// phyr058
	0x04000000,	// phyr05c
	0x00000400,	// phyr060
	0x00000000,	// phyr064
	0x000c0004,	// phyr068
	0x00800200,	// phyr06c
	0x000c0006,	// phyr070
	0x000f0000,	// phyr074
	0x0012c107,	// phyr078
	0x06080606,	// phyr07c
	0x40000900,	// phyr080
	0x0c400c30,	// phyr084
	0x00001002,	// phyr088
	0x05701016,	// phyr08c
	0x00000000,	// phyr090
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x80002020,	// phyr0a8
	0x00000001,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e01cc,	// new address
	0x01010101,	// phyr0cc
	0x01010101,	// phyr0d0
	0x80808080,	// phyr0d4
	0x80808080,	// phyr0d8
	0xaeeddeea,	// change address
	0x1e6e0288,	// new address
	0x78787878,	// phyr188
	0x78787878,	// phyr18c
	0x78787878,	// phyr190
	0x78787878,	// phyr194
	0xaeeddeea,	// change address
	0x1e6e02f8,	// new address
	0x88888888,	// phyr1f8
	0x88888888,	// phyr1fc
	0x00000000,	// phyr200
	0xaeeddeea,	// change address
	0x1e6e0194,	// new address
	0xff2100e0,	// phyr094
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x80000000,	// phyr0a8
	0x00000000,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e027c,	// new address
	0x00080000,	// phyr17c
	0xaeeddeea,	// change address
	0x1e6e0140,	// new address
	0x00000f00,	// phyr040
	0x00000000,	// phyr044
	0x00000704,	// phyr048
	0xaeeddeea,	// change address
	0x1e6e0198,	// new address
	0x08060000,	// phyr098
	0xaeeddeea,	// change address
	0x1e6e01b0,	// new address
	0x00000000,	// phyr0b0
	0x00000000,	// phyr0b4
	0x00000000,	// phyr0b8
	0x00000000,	// phyr0bc
	0x00000000,	// phyr0c0
	0x00000000,	// phyr0c4
	0x000aff2c,	// phyr0c8
	0xaeeddeea,	// change address
	0x1e6e01dc,	// new address
	0x00080000,	// phyr0dc
	0x00000000,	// phyr0e0
	0xaa55aa55,	// phyr0e4
	0x55aa55aa,	// phyr0e8
	0xaaaa5555,	// phyr0ec
	0x5555aaaa,	// phyr0f0
	0xaa55aa55,	// phyr0f4
	0x55aa55aa,	// phyr0f8
	0xaaaa5555,	// phyr0fc
	0x5555aaaa,	// phyr100
	0xaa55aa55,	// phyr104
	0x55aa55aa,	// phyr108
	0xaaaa5555,	// phyr10c
	0x5555aaaa,	// phyr110
	0xaa55aa55,	// phyr114
	0x55aa55aa,	// phyr118
	0xaaaa5555,	// phyr11c
	0x5555aaaa,	// phyr120
	0x1f1f1f1f,	// phyr124
	0x1f1f1f1f,	// phyr128
	0x1f1f1f1f,	// phyr12c
	0x1f1f1f1f,	// phyr130
	0x1f1f1f1f,	// phyr134
	0x1f1f1f1f,	// phyr138
	0x1f1f1f1f,	// phyr13c
	0x1f1f1f1f,	// phyr140
	0x1f1f1f1f,	// phyr144
	0x1f1f1f1f,	// phyr148
	0x1f1f1f1f,	// phyr14c
	0x1f1f1f1f,	// phyr150
	0x1f1f1f1f,	// phyr154
	0x1f1f1f1f,	// phyr158
	0x1f1f1f1f,	// phyr15c
	0x1f1f1f1f,	// phyr160
	0x1f1f1f1f,	// phyr164
	0x1f1f1f1f,	// phyr168
	0x1f1f1f1f,	// phyr16c
	0x1f1f1f1f,	// phyr170
	0xaeeddeea,	// change address
	0x1e6e0198,	// new address
	0x1f1f0800,	// phyr198
	0x1f1f1f1f,	// phyr19c
	0x1f1f1f1f,	// phyr1a0
	0x1f1f1f1f,	// phyr1a4
	0x1f1f1f1f,	// phyr1a8
	0x1f1f1f1f,	// phyr1ac
	0x1f1f1f1f,	// phyr1b0
	0x1f1f1f1f,	// phyr1b4
	0x1f1f1f1f,	// phyr1b8
	0x1f1f1f1f,	// phyr1bc
	0x1f1f1f1f,	// phyr1c0
	0x1f1f1f1f,	// phyr1c4
	0x1f1f1f1f,	// phyr1c8
	0x1f1f1f1f,	// phyr1cc
	0x1f1f1f1f,	// phyr1d0
	0x1f1f1f1f,	// phyr1d4
	0x1f1f1f1f,	// phyr1d8
	0x1f1f1f1f,	// phyr1dc
	0x1f1f1f1f,	// phyr1e0
	0x1f1f1f1f,	// phyr1e4
	0x00001f1f,	// phyr1e8
	0xaeeddeea,	// change address
	0x1e6e0304,	// new address
	0x00000800,	// phyr204
	0xaeeddeea,	// change address
	0x1e6e0310,	// new address
	0xc0300c03,	// phyr210
	0xa0500a05,	// phyr214
	0xaeededed,	// end
};
#elif (DDR_PHY_TBL_SEL == DDR_PHY_TBL_POST_SIM_800)
u32 ast2600_sdramphy_config[163] = {
	0x1e6e0100,	// start address
	0x022a0092,	// phyr000
	0x00000042,	// phyr004
	0x1a7a0063,	// phyr008
	0x1a7a0063,	// phyr00C
	0x1a7a0063,	// phyr010
	0x1a7a0063,	// phyr014
	0x20000000,	// phyr018
	0x20000000,	// phyr01C
	0x20000000,	// phyr020
	0x20000000,	// phyr024
	0x00000008,	// phyr028
	0x00000000,	// phyr02C
	0x00077600,	// phyr030
	0x00000000,	// phyr034
	0x00000000,	// phyr038
	0x20000000,	// phyr03C
	0x00000000,	// phyr040
	0x00000000,	// phyr044
	0x00000000,	// phyr048
	0x00003080,	// phyr04C
	0xf400f000,	// phyr050
	0x00000000,	// phyr054
	0x07100001,	// phyr058
	0x04000000,	// phyr05c
	0x00000400,	// phyr060
	0x00000000,	// phyr064
	0x000c0004,	// phyr068
	0x00800200,	// phyr06c
	0x000c0006,	// phyr070
	0x000f0000,	// phyr074
	0x0012c107,	// phyr078
	0x06080606,	// phyr07c
	0x40000900,	// phyr080
	0x0c400618,	// phyr084
	0x00001002,	// phyr088
	0x05701016,	// phyr08c
	0x00000000,	// phyr090
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x80002020,	// phyr0a8
	0x00000001,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e01cc,	// new address
	0x01010101,	// phyr0cc
	0x01010101,	// phyr0d0
	0x80808080,	// phyr0d4
	0x80808080,	// phyr0d8
	0xaeeddeea,	// change address
	0x1e6e0288,	// new address
	0x78787878,	// phyr188
	0x78787878,	// phyr18c
	0x78787878,	// phyr190
	0x78787878,	// phyr194
	0xaeeddeea,	// change address
	0x1e6e02f8,	// new address
	0x88888888,	// phyr1f8
	0x88888888,	// phyr1fc
	0x00000000,	// phyr200
	0xaeeddeea,	// change address
	0x1e6e0194,	// new address
	0xff2100e0,	// phyr094
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x80000000,	// phyr0a8
	0x00000000,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e027c,	// new address
	0x00080000,	// phyr17c
	0xaeeddeea,	// change address
	0x1e6e0140,	// new address
	0x00000f00,	// phyr040
	0x00000000,	// phyr044
	0x00000704,	// phyr048
	0xaeeddeea,	// change address
	0x1e6e0198,	// new address
	0x08060000,	// phyr098
	0xaeeddeea,	// change address
	0x1e6e01b0,	// new address
	0x00000000,	// phyr0b0
	0x00000000,	// phyr0b4
	0x00000000,	// phyr0b8
	0x00000000,	// phyr0bc
	0x00000000,	// phyr0c0
	0x00000000,	// phyr0c4
	0x000aff2c,	// phyr0c8
	0xaeeddeea,	// change address
	0x1e6e01dc,	// new address
	0x00080000,	// phyr0dc
	0x00000000,	// phyr0e0
	0xaa55aa55,	// phyr0e4
	0x55aa55aa,	// phyr0e8
	0xaaaa5555,	// phyr0ec
	0x5555aaaa,	// phyr0f0
	0xaa55aa55,	// phyr0f4
	0x55aa55aa,	// phyr0f8
	0xaaaa5555,	// phyr0fc
	0x5555aaaa,	// phyr100
	0xaa55aa55,	// phyr104
	0x55aa55aa,	// phyr108
	0xaaaa5555,	// phyr10c
	0x5555aaaa,	// phyr110
	0xaa55aa55,	// phyr114
	0x55aa55aa,	// phyr118
	0xaaaa5555,	// phyr11c
	0x5555aaaa,	// phyr120
	0x1f1f1f1f,	// phyr124
	0x1f1f1f1f,	// phyr128
	0x1f1f1f1f,	// phyr12c
	0x1f1f1f1f,	// phyr130
	0x1f1f1f1f,	// phyr134
	0x1f1f1f1f,	// phyr138
	0x1f1f1f1f,	// phyr13c
	0x1f1f1f1f,	// phyr140
	0x1f1f1f1f,	// phyr144
	0x1f1f1f1f,	// phyr148
	0x1f1f1f1f,	// phyr14c
	0x1f1f1f1f,	// phyr150
	0x1f1f1f1f,	// phyr154
	0x1f1f1f1f,	// phyr158
	0x1f1f1f1f,	// phyr15c
	0x1f1f1f1f,	// phyr160
	0x1f1f1f1f,	// phyr164
	0x1f1f1f1f,	// phyr168
	0x1f1f1f1f,	// phyr16c
	0x1f1f1f1f,	// phyr170
	0xaeeddeea,	// change address
	0x1e6e0198,	// new address
	0x1f1f0800,	// phyr198
	0x1f1f1f1f,	// phyr19c
	0x1f1f1f1f,	// phyr1a0
	0x1f1f1f1f,	// phyr1a4
	0x1f1f1f1f,	// phyr1a8
	0x1f1f1f1f,	// phyr1ac
	0x1f1f1f1f,	// phyr1b0
	0x1f1f1f1f,	// phyr1b4
	0x1f1f1f1f,	// phyr1b8
	0x1f1f1f1f,	// phyr1bc
	0x1f1f1f1f,	// phyr1c0
	0x1f1f1f1f,	// phyr1c4
	0x1f1f1f1f,	// phyr1c8
	0x1f1f1f1f,	// phyr1cc
	0x1f1f1f1f,	// phyr1d0
	0x1f1f1f1f,	// phyr1d4
	0x1f1f1f1f,	// phyr1d8
	0x1f1f1f1f,	// phyr1dc
	0x1f1f1f1f,	// phyr1e0
	0x1f1f1f1f,	// phyr1e4
	0x00001f1f,	// phyr1e8
	0xaeeddeea,	// change address
	0x1e6e0304,	// new address
	0x00000800,	// phyr204
	0xaeeddeea,	// change address
	0x1e6e0310,	// new address
	0xc0300c03,	// phyr210
	0xa0500a05,	// phyr214
	0xaeededed,	// end
};
#else
#error "PHY table not set\n"
#endif