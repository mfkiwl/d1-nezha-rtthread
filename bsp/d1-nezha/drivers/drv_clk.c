#include "drv_clk.h"
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

static void set_pll_cpux_axi(void)
{
    uint32_t val;

    /* Select cpux clock src to osc24m, axi divide ratio is 3, system apb clk ratio is 4 */
    write32(D1_CCU_BASE + CCU_RISCV_CLK_REG, (0 << 24) | (3 << 8) | (1 << 0));
    sdelay(1);

    /* Disable pll gating */
    val = read32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG);
    val &= ~(1 << 27);
    write32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG, val);

    /* Enable pll ldo */
    val = read32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG);
    val |= (1 << 30);
    write32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG, val);
    sdelay(5);

    /* Set default clk to 1008mhz */
    val = read32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG);
    val &= ~((0x3 << 16) | (0xff << 8) | (0x3 << 0));
    val |= (41 << 8);
    write32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG, val);

    /* Lock enable */
    val = read32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG);
    val |= (1 << 29);
    write32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG, val);

    /* Enable pll */
    val = read32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG);
    val |= (1 << 31);
    write32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG, val);

    /* Wait pll stable */
    while(!(read32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG) & (0x1 << 28)));
    sdelay(20);

    /* Enable pll gating */
    val = read32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG);
    val |= (1 << 27);
    write32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG, val);

    /* Lock disable */
    val = read32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG);
    val &= ~(1 << 29);
    write32(D1_CCU_BASE + CCU_PLL_CPU_CTRL_REG, val);
    sdelay(1);

    /* Set and change cpu clk src */
    val = read32(D1_CCU_BASE + CCU_RISCV_CLK_REG);
    val &= ~(0x07 << 24 | 0x3 << 8 | 0xf << 0);
    val |= (0x05 << 24 | 0x1 << 8);
    write32(D1_CCU_BASE + CCU_RISCV_CLK_REG, val);
    sdelay(1);
}

static void set_pll_periph0(void)
{
    uint32_t val;

    /* Periph0 has been enabled */
    if(read32(D1_CCU_BASE + CCU_PLL_PERI0_CTRL_REG) & (1 << 31))
        return;

    /* Change psi src to osc24m */
    val = read32(D1_CCU_BASE + CCU_PSI_CLK_REG);
    val &= (~(0x3 << 24));
    write32(val, D1_CCU_BASE + CCU_PSI_CLK_REG);

    /* Set default val */
    write32(D1_CCU_BASE + CCU_PLL_PERI0_CTRL_REG, 0x63 << 8);

    /* Lock enable */
    val = read32(D1_CCU_BASE + CCU_PLL_PERI0_CTRL_REG);
    val |= (1 << 29);
    write32(D1_CCU_BASE + CCU_PLL_PERI0_CTRL_REG, val);

    /* Enabe pll 600m(1x) 1200m(2x) */
    val = read32(D1_CCU_BASE + CCU_PLL_PERI0_CTRL_REG);
    val |= (1 << 31);
    write32(D1_CCU_BASE + CCU_PLL_PERI0_CTRL_REG, val);

    /* Wait pll stable */
    while(!(read32(D1_CCU_BASE + CCU_PLL_PERI0_CTRL_REG) & (0x1 << 28)));
    sdelay(20);

    /* Lock disable */
    val = read32(D1_CCU_BASE + CCU_PLL_PERI0_CTRL_REG);
    val &= ~(1 << 29);
    write32(D1_CCU_BASE + CCU_PLL_PERI0_CTRL_REG, val);
}

static void set_ahb(void)
{
    write32(D1_CCU_BASE + CCU_PSI_CLK_REG, (2 << 0) | (0 << 8));
    write32(D1_CCU_BASE + CCU_PSI_CLK_REG, read32(D1_CCU_BASE + CCU_PSI_CLK_REG) | (0x03 << 24));
    sdelay(1);
}

static void set_apb(void)
{
    write32(D1_CCU_BASE + CCU_APB0_CLK_REG, (2 << 0) | (1 << 8));
    write32(D1_CCU_BASE + CCU_APB0_CLK_REG, (0x03 << 24) | read32(D1_CCU_BASE + CCU_APB0_CLK_REG));
    sdelay(1);
}

static void set_dma(void)
{
    /* Dma reset */
    write32(D1_CCU_BASE + CCU_DMA_BGR_REG, read32(D1_CCU_BASE + CCU_DMA_BGR_REG) | (1 << 16));
    sdelay(20);
    /* Enable gating clock for dma */
    write32(D1_CCU_BASE + CCU_DMA_BGR_REG, read32(D1_CCU_BASE + CCU_DMA_BGR_REG) | (1 << 0));
}

static void set_mbus(void)
{
    uint32_t val;

    /* Reset mbus domain */
    val = read32(D1_CCU_BASE + CCU_MBUS_CLK_REG);
    val |= (0x1 << 30);
    write32(D1_CCU_BASE + CCU_MBUS_CLK_REG, val);
    sdelay(1);
}

static void set_module(int addr)
{
    uint32_t val;

    if(!(read32(addr) & (1 << 31)))
    {
        val = read32(addr);
        write32(addr, val | (1 << 31) | (1 << 30));

        /* Lock enable */
        val = read32(addr);
        val |= (1 << 29);
        write32(addr, val);

        /* Wait pll stable */
        while(!(read32(addr) & (0x1 << 28)));
        sdelay(20);

        /* Lock disable */
        val = read32(addr);
        val &= ~(1 << 29);
        write32(addr, val);
    }
}

void clk_enable_module_uart(int addr, uint8_t uart_num)
{
    uint32_t val;
    /* Open the clock gate for uart */
    val = read32(addr);
    val |= 1 << (0 + uart_num);
    write32(addr, val);

    /* Deassert uart reset */
    val = read32(addr);
    val |= 1 << (16 + uart_num);
    write32(addr, val);
}

void sys_clock_init(void)
{
    set_pll_cpux_axi();
    set_pll_periph0();
    set_ahb();
    set_apb();
    set_dma();
    set_mbus();
    set_module(D1_CCU_BASE + CCU_PLL_PERI0_CTRL_REG);
    set_module(D1_CCU_BASE + CCU_PLL_VIDEO0_CTRL_REG);
    set_module(D1_CCU_BASE + CCU_PLL_VIDEO1_CTRL_REG);
    set_module(D1_CCU_BASE + CCU_PLL_VE_CTRL);
    set_module(D1_CCU_BASE + CCU_PLL_AUDIO0_CTRL_REG);
    set_module(D1_CCU_BASE + CCU_PLL_AUDIO1_CTRL_REG);
}