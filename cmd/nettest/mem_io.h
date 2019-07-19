#include "io.h"

#define MAC1_BASE	0x1e660000
#define MAC2_BASE	0x1e680000
#define SCU_BASE        0x1e6e2000


#ifdef CONFIG_ASPEED_AST2600
#define MAC3_BASE	0x1e670000
#define MAC4_BASE	0x1e690000

#endif

/* macros for register access */
#define SCU_RD(offset)          readl(SCU_BASE + offset)
#define SCU_WR(value, offset)   writel(value, SCU_BASE + offset)

/* typedef for register access */
typedef union {
	uint32_t w;
	struct {
		uint32_t reserved_0 : 6;	/* bit[5:0] */
		uint32_t mac1_interface : 1;	/* bit[6] */
		uint32_t mac2_interface : 1;	/* bit[7] */
		uint32_t reserved_1 : 24;	/* bit[31:8] */
	}b;
} hw_strap1_t;

typedef union {
	uint32_t w;
	struct {
		uint32_t mac3_interface : 1;	/* bit[0] */
		uint32_t mac4_interface : 1;	/* bit[1] */		
		uint32_t reserved_0 : 30;	/* bit[31:2] */
	}b;
} hw_strap2_t;