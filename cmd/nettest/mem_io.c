#include "mem_io.h"
uint32_t SRAM_RD(uint32_t addr)
{
	return readl(SRAM_BASE + addr);
}

uint32_t DESC_RD(uint32_t addr)
{
	return readl(SRAM_BASE + addr);
}