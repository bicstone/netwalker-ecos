//==========================================================================
//
//      board_misc.c
//
//      HAL misc board support code for the board
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include <redboot.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_soc.h>         // Hardware definitions
#include <cyg/hal/fsl_board.h>             // Platform specifics
#include <cyg/infra/diag.h>             // diag_printf

// All the MM table layout is here:
#include <cyg/hal/hal_mm.h>

externC void* memset(void *, int, size_t);
unsigned int cpld_base_addr;
extern char HAL_PLATFORM_EXTRA[40];

void hal_mmu_init(void)
{
    unsigned long ttb_base = RAM_BANK0_BASE + 0x4000;
    unsigned long i;

    /*
     * Set the TTB register
     */
    asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

    /*
     * Set the Domain Access Control Register
     */
    i = ARM_ACCESS_DACR_DEFAULT;
    asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(i) /*:*/);

    /*
     * First clear all TT entries - ie Set them to Faulting
     */
    memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);

    /*              Actual   Virtual  Size   Attributes                                                    Function  */
    /*              Base     Base     MB     cached?           buffered?        access permissions                 */
    /*              xxx00000 xxx00000                                                                                */
    X_ARM_MMU_SECTION(0x000, 0x200,   0x1,   ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* ROM */
    X_ARM_MMU_SECTION(0x1FF, 0x1FF,   0x001, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* IRAM */
    X_ARM_MMU_SECTION(0x300, 0x300,   0x100, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* GPU */
    X_ARM_MMU_SECTION(0x400, 0x400,   0x200, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* IPUv3D */
    X_ARM_MMU_SECTION(0x600, 0x600,   0x300, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* periperals */
    X_ARM_MMU_SECTION(0x900, 0x000,   0x1FF, ARM_CACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0x900, 0x900,   0x200, ARM_CACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0x900, 0xE00,   0x200, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SDRAM 0:128M*/
    X_ARM_MMU_SECTION(0xB80, 0xB80,   0x10,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CS1 EIM control*/
    X_ARM_MMU_SECTION(0xCC0, 0xCC0,   0x040, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CS4/5/NAND Flash buffer */
}

static void mxc_fec_setup(void)
{
    volatile unsigned int reg;
    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) == 0x2) {
        /*FEC_MDIO*/
        writel(0x3, IOMUXC_BASE_ADDR + 0x0D4);
        writel(0x1FD, IOMUXC_BASE_ADDR + 0x0468);
        writel(0x0, IOMUXC_BASE_ADDR + 0x0954);

        /*FEC_MDC*/
        writel(0x2, IOMUXC_BASE_ADDR + 0x13C);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x0524);

        /* FEC RDATA[3] */
        writel(0x3, IOMUXC_BASE_ADDR + 0x0EC);
        writel(0x180, IOMUXC_BASE_ADDR + 0x0480);
        writel(0x0, IOMUXC_BASE_ADDR + 0x0964);

        /* FEC RDATA[2] */
        writel(0x3, IOMUXC_BASE_ADDR + 0x0E8);
        writel(0x180, IOMUXC_BASE_ADDR + 0x047C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x0960);

        /* FEC RDATA[1] */
        writel(0x3, IOMUXC_BASE_ADDR + 0x0d8);
        writel(0x180, IOMUXC_BASE_ADDR + 0x046C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x095C);

        /* FEC RDATA[0] */
        writel(0x2, IOMUXC_BASE_ADDR + 0x016C);
        writel(0x2180, IOMUXC_BASE_ADDR + 0x0554);
        writel(0x0, IOMUXC_BASE_ADDR + 0x0958);

        /* FEC TDATA[3] */
        writel(0x2, IOMUXC_BASE_ADDR + 0x148);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x0530);

        /* FEC TDATA[2] */
        writel(0x2, IOMUXC_BASE_ADDR + 0x144);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x052C);

        /* FEC TDATA[1] */
        writel(0x2, IOMUXC_BASE_ADDR + 0x140);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x0528);

        /* FEC TDATA[0] */
        writel(0x2, IOMUXC_BASE_ADDR + 0x0170);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x0558);

        /* FEC TX_EN */
        writel(0x1, IOMUXC_BASE_ADDR + 0x014C);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x0534);

        /* FEC TX_ER */
        writel(0x2, IOMUXC_BASE_ADDR + 0x0138);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x0520);

        /* FEC TX_CLK */
        writel(0x1, IOMUXC_BASE_ADDR + 0x0150);
        writel(0x2180, IOMUXC_BASE_ADDR + 0x0538);
        writel(0x0, IOMUXC_BASE_ADDR + 0x0974);

        /* FEC COL */
        writel(0x1, IOMUXC_BASE_ADDR + 0x0124);
        writel(0x2180, IOMUXC_BASE_ADDR + 0x0500);
        writel(0x0, IOMUXC_BASE_ADDR + 0x094c);

        /* FEC RX_CLK */
        writel(0x1, IOMUXC_BASE_ADDR + 0x0128);
        writel(0x2180, IOMUXC_BASE_ADDR + 0x0504);
        writel(0x0, IOMUXC_BASE_ADDR + 0x0968);

        /* FEC CRS */
        writel(0x3, IOMUXC_BASE_ADDR + 0x0f4);
        writel(0x180, IOMUXC_BASE_ADDR + 0x0488);
        writel(0x0, IOMUXC_BASE_ADDR + 0x0950);

        /* FEC RX_ER */
        writel(0x3, IOMUXC_BASE_ADDR + 0x0f0);
        writel(0x180, IOMUXC_BASE_ADDR + 0x0484);
        writel(0x0, IOMUXC_BASE_ADDR + 0x0970);

        /* FEC RX_DV */
        writel(0x2, IOMUXC_BASE_ADDR + 0x164);
        writel(0x2180, IOMUXC_BASE_ADDR + 0x054C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x096C);
    } else {
        /*FEC_MDIO*/
        writel(0x3, IOMUXC_BASE_ADDR + 0x0D4);
        writel(0x1FD, IOMUXC_BASE_ADDR + 0x0470);
        writel(0x0, IOMUXC_BASE_ADDR + 0x09B0);

        /*FEC_RDATA1*/
        writel(0x3, IOMUXC_BASE_ADDR + 0x0D8);
        writel(0x180, IOMUXC_BASE_ADDR + 0x0474);
        writel(0x0, IOMUXC_BASE_ADDR + 0x09B8);

        /*FEC_RDATA2*/
        writel(0x3, IOMUXC_BASE_ADDR + 0x0E8);
        writel(0x180, IOMUXC_BASE_ADDR + 0x0484);
        writel(0x0, IOMUXC_BASE_ADDR + 0x09BC);

        /*FEC_RDATA3*/
        writel(0x3, IOMUXC_BASE_ADDR + 0x0EC);
        writel(0x180, IOMUXC_BASE_ADDR + 0x0488);
        writel(0x0, IOMUXC_BASE_ADDR + 0x09C0);

        /*FEC_RX_ERR*/
        writel(0x3, IOMUXC_BASE_ADDR + 0x0F0);
        writel(0x180, IOMUXC_BASE_ADDR + 0x048C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x09CC);

        /*FEC_CRS*/
        writel(0x3, IOMUXC_BASE_ADDR + 0x0F4);
        writel(0x180, IOMUXC_BASE_ADDR + 0x0490);
        writel(0x0, IOMUXC_BASE_ADDR + 0x09AC);

        /*FEC_COL*/
        writel(0x1, IOMUXC_BASE_ADDR + 0x0124);
        writel(0x180, IOMUXC_BASE_ADDR + 0x05CC);
        writel(0x0, IOMUXC_BASE_ADDR + 0x9A8);

        /*FEC_RX_CLK*/
        writel(0x1, IOMUXC_BASE_ADDR + 0x0128);
        writel(0x180, IOMUXC_BASE_ADDR + 0x05D0);
        writel(0x0, IOMUXC_BASE_ADDR + 0x09C4);

        /*FEC_RX_DV*/
        writel(0x1, IOMUXC_BASE_ADDR + 0x012C);
        writel(0x180, IOMUXC_BASE_ADDR + 0x05D4);
        writel(0x0, IOMUXC_BASE_ADDR + 0x09C8);

        /*FEC_RDATA0*/
        writel(0x1, IOMUXC_BASE_ADDR + 0x0134);
        writel(0x2180, IOMUXC_BASE_ADDR + 0x05DC);
        writel(0x0, IOMUXC_BASE_ADDR + 0x09B4);

        /*FEC_TDATA0*/
        writel(0x1, IOMUXC_BASE_ADDR + 0x0138);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x5E0);

        /*FEC_TX_ERR*/
        writel(0x2, IOMUXC_BASE_ADDR + 0x0144);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x05EC);

        /*FEC_MDC*/
        writel(0x2, IOMUXC_BASE_ADDR + 0x0148);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x05F0);

        /*FEC_TDATA1*/
        writel(0x2, IOMUXC_BASE_ADDR + 0x014C);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x05F4);

        /*FEC_TDATA2*/
        writel(0x2, IOMUXC_BASE_ADDR + 0x0150);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x05F8);

        /*FEC_TDATA3*/
        writel(0x2, IOMUXC_BASE_ADDR + 0x0154);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x05FC);

        /*FEC_TX_EN*/
        writel(0x1, IOMUXC_BASE_ADDR + 0x0158);
        writel(0x2004, IOMUXC_BASE_ADDR + 0x0600);

        /*FEC_TX_CLK*/
        writel(0x1, IOMUXC_BASE_ADDR + 0x015C);
        writel(0x2180, IOMUXC_BASE_ADDR + 0x0604);
        writel(0x0, IOMUXC_BASE_ADDR + 0x09D0);
    }
}

#include <cyg/io/imx_spi.h>
struct spi_v2_3_reg spi_pmic_reg;

struct imx_spi_dev imx_spi_pmic = {
    base : CSPI1_BASE_ADDR,
    freq : 25000000,
    ss_pol : IMX_SPI_ACTIVE_HIGH,
    ss : 0,                     // slave select 0
    fifo_sz : 32,
    reg : &spi_pmic_reg,
};

struct spi_v2_3_reg spi_nor_reg;

struct imx_spi_dev imx_spi_nor = {
    base : CSPI1_BASE_ADDR,
    freq : 25000000,
    ss_pol : IMX_SPI_ACTIVE_LOW,
    ss : 1,                     // slave select 1
    fifo_sz : 32,
    us_delay: 0,
    reg : &spi_nor_reg,
};

imx_spi_init_func_t *spi_nor_init;
imx_spi_xfer_func_t *spi_nor_xfer;

imx_spi_init_func_t *spi_pmic_init;
imx_spi_xfer_func_t *spi_pmic_xfer;

//
// Platform specific initialization
//
static void babbage_power_init(void);

void plf_hardware_init(void)
{
    unsigned int reg;

    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) != 0x2) {
        /* Disable IPU and HSC dividers */
        writel(0x60000, CCM_BASE_ADDR + CLKCTL_CCDR);
        /* Change the DDR divider to run at 133MHz */
        reg = readl(CCM_BASE_ADDR + CLKCTL_CBCDR);
        reg = (reg & (~0x70000)) | 0x40000;
        writel(reg, CCM_BASE_ADDR + CLKCTL_CBCDR);
         /* make sure divider effective */
        while (readl(CCM_BASE_ADDR + CLKCTL_CDHIPR) != 0);
        writel(0x0, CCM_BASE_ADDR + CLKCTL_CCDR);
    }
    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) == 0x2) {
        // UART1
        //RXD
        writel(0x0, IOMUXC_BASE_ADDR + 0x228);
        writel(0x1C5, IOMUXC_BASE_ADDR + 0x618);
        //TXD
        writel(0x0, IOMUXC_BASE_ADDR + 0x22c);
        writel(0x1C5, IOMUXC_BASE_ADDR + 0x61c);
        //RTS
        writel(0x0, IOMUXC_BASE_ADDR + 0x230);
        writel(0x1C4, IOMUXC_BASE_ADDR + 0x620);
        //CTS
        writel(0x0, IOMUXC_BASE_ADDR + 0x234);
        writel(0x1C4, IOMUXC_BASE_ADDR + 0x624);
        // enable GPIO1_9 for CLKO and GPIO1_8 for CLKO2
        writel(0x00000004, 0x73fa83E8);
        writel(0x00000004, 0x73fa83Ec);
    } else {
        // UART1
        //RXD
        writel(0x0, IOMUXC_BASE_ADDR + 0x234);
        writel(0x1C5, IOMUXC_BASE_ADDR + 0x6E4);
        //TXD
        writel(0x0, IOMUXC_BASE_ADDR + 0x238);
        writel(0x1C5, IOMUXC_BASE_ADDR + 0x6E8);
        //RTS
        writel(0x0, IOMUXC_BASE_ADDR + 0x23C);
        writel(0x1C4, IOMUXC_BASE_ADDR + 0x6EC);
        //CTS
        writel(0x0, IOMUXC_BASE_ADDR + 0x240);
        writel(0x1C4, IOMUXC_BASE_ADDR + 0x6F0);
        // enable GPIO1_9 for CLKO and GPIO1_8 for CLKO2
        writel(0x00000004, 0x73fa83F4);
        writel(0x00000004, 0x73fa83F0);
    }

    // enable ARM clock div by 8
    writel(0x010900F0, CCM_BASE_ADDR + CLKCTL_CCOSR);

    spi_nor_init = (imx_spi_init_func_t *)imx_spi_init_v2_3;
    spi_nor_xfer = (imx_spi_xfer_func_t *)imx_spi_xfer_v2_3;

    spi_pmic_init = (imx_spi_init_func_t *)imx_spi_init_v2_3;
    spi_pmic_xfer = (imx_spi_xfer_func_t *)imx_spi_xfer_v2_3;
    spi_pmic_init(&imx_spi_pmic);

    /* Configure UART3_RXD pin for GPIO */
    writel(0x3, IOMUXC_BASE_ADDR + 0x240);
    reg = readl(GPIO1_BASE_ADDR + 0x4);
    reg &= ~0x400000;  // configure GPIO lines as input
    writel(reg, GPIO1_BASE_ADDR + 0x4);

    if ((readl(GPIO1_BASE_ADDR + 0x0) & (0x1 << 22)) == 0) {
        /* Babbage 2.5 */
        system_rev |= 0x1 << BOARD_VER_OFFSET;
        HAL_PLATFORM_EXTRA[32] = '5';
    }

    babbage_power_init();
}

void mxc_mmc_init(unsigned int base_address)
{
    switch(base_address) {
    case MMC_SDHC1_BASE_ADDR:
        if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) == 0x2) {
            /* SD1 CMD, SION bit */
            writel(0x10, IOMUXC_BASE_ADDR + 0x394);
           /* Configure SW PAD */
            /* SD1 CMD */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x79C);
            /* SD1 CLK */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x7A0);
            /* SD1 DAT0 */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x7A4);
            /* SD1 DAT1 */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x7A8);
            /* SD1 DAT2 */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x7AC);
            /* SD1 DAT3 */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x7B0);
        } else {
            /* SD1 CMD, SION bit */
            writel(0x10, IOMUXC_BASE_ADDR + 0x39c);
            /* SD1 CD, as gpio1_0 */
            writel(0x01, IOMUXC_BASE_ADDR + 0x3b4);
            /* Configure SW PAD */
            /* SD1 CMD */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x868);
            /* SD1 CLK */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x86c);
            /* SD1 DAT0 */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x870);
            /* SD1 DAT1 */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x874);
            /* SD1 DAT2 */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x878);
            /* SD1 DAT3 */
            writel(0xd5, IOMUXC_BASE_ADDR + 0x87c);
            /* SD1 CD as gpio1_0 */
            writel(0x1e2, IOMUXC_BASE_ADDR + 0x880);
        }
        break;
    case MMC_SDHC2_BASE_ADDR:
        /* SD2 CMD, SION bit */
        writel(0x10, IOMUXC_BASE_ADDR + 0x3b4);
        /* Configure SW PAD */
        /* SD2 CMD */
        writel(0x20f4, IOMUXC_BASE_ADDR + 0x7bc);
        /* SD2 CLK */
        writel(0x20d4, IOMUXC_BASE_ADDR + 0x7c0);
        /* SD2 DAT0 */
        writel(0x20e4, IOMUXC_BASE_ADDR + 0x7c4);
        /* SD2 DAT1 */
        writel(0x21d4, IOMUXC_BASE_ADDR + 0x7c8);
        /* SD2 DAT2 */
        writel(0x21d4, IOMUXC_BASE_ADDR + 0x7cc);
        /* SD2 DAT3 */
        writel(0x20e4, IOMUXC_BASE_ADDR + 0x7d0);
    default:
        break;
    }
}

#include CYGHWR_MEMORY_LAYOUT_H

typedef void code_fun(void);

void board_program_new_stack(void *func)
{
    register CYG_ADDRESS stack_ptr asm("sp");
    register CYG_ADDRESS old_stack asm("r4");
    register code_fun *new_func asm("r0");
    old_stack = stack_ptr;
    stack_ptr = CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE - sizeof(CYG_ADDRESS);
    new_func = (code_fun*)func;
    new_func();
    stack_ptr = old_stack;
}

void increase_core_voltage(bool i)
{
    unsigned int val;

    val = pmic_reg(24, 0, 0);

    if (i) {
        /* Set core voltage to 1.175V */
        val = val & (~0x1F) | 0x17;
    } else {
        /* Set core voltage to 1.05V */
        val = val & (~0x1F) | 0x12;
    }

    pmic_reg(24, val, 1);
}

extern unsigned int pmic_reg(unsigned int reg, unsigned int val, unsigned int write);
static void babbage_power_init(void)
{
    unsigned int val;
    volatile unsigned int reg;

    /* Write needed to Power Gate 2 register */
    val = pmic_reg(34, 0, 0);
    val &= ~0x10000;
    pmic_reg(34, val, 1);

    /* Write needed to update Charger 0 */
    pmic_reg(48, 0x0023807F, 1);

    /* power up the system first */
    pmic_reg(34, 0x00200000, 1);

    if (pll_clock(PLL1) > 800000000) {
        /* Set core voltage to 1.175V */
        val = pmic_reg(24, 0, 0);
        val = val & (~0x1F) | 0x17;
        pmic_reg(24, val, 1);
    }

    /* Setup VCC (SW2) to 1.225 */
    val = pmic_reg(25, 0, 0);
    val = val & (~0x1F) | 0x19;
    pmic_reg(25, val, 1);

    /* Setup 1V2_DIG1 (SW3) to 1.2 */
    val = pmic_reg(26, 0, 0);
    val = val & (~0x1F) | 0x18;
    pmic_reg(25, val, 1);

    /* Set VDIG to 1.65V, VGEN3 to 1.8V, VCAM to 2.5V */
    val = pmic_reg(30, 0, 0);
    val &= ~0x34030;
    val |= 0x10020;
    pmic_reg(30, val, 1);

    /* Set VVIDEO to 2.775V, VAUDIO to 3V, VSD to 3.15V */
    val = pmic_reg(31, 0, 0);
    val &= ~0x1FC;
    val |= 0x1F4;
    pmic_reg(31, val, 1);

    /* Configure VGEN3 and VCAM regulators to use external PNP */
    val = 0x208;
    pmic_reg(33, val, 1);
    hal_delay_us(200);

    reg = readl(GPIO2_BASE_ADDR + 0x0);
    reg &= ~0x4000;  // Lower reset line
    writel(reg, GPIO2_BASE_ADDR + 0x0);

    reg = readl(GPIO2_BASE_ADDR + 0x4);
    reg |= 0x4000;  // configure GPIO lines as output
    writel(reg, GPIO2_BASE_ADDR + 0x4);

    /* Reset the ethernet controller over GPIO */
    writel(0x1, IOMUXC_BASE_ADDR + 0x0AC);

    /* Enable VGEN3, VCAM, VAUDIO, VVIDEO, VSD regulators */
    val = 0x49249;
    pmic_reg(33, val, 1);

    hal_delay_us(500);

    reg = readl(GPIO2_BASE_ADDR + 0x0);
    reg |= 0x4000;
    writel(reg, GPIO2_BASE_ADDR + 0x0);

    /* Setup the FEC after enabling the regulators */
    mxc_fec_setup();
}

void io_cfg_spi(struct imx_spi_dev *dev)
{
    switch (dev->base) {
    case CSPI1_BASE_ADDR:
        if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) == 0x2) {
            // 000: Select mux mode: ALT0 mux port: MOSI of instance: ecspi1
            writel(0x0, IOMUXC_BASE_ADDR + 0x210);
            writel(0x105, IOMUXC_BASE_ADDR + 0x600);

            // 000: Select mux mode: ALT0 mux port: MISO of instance: ecspi1.
            writel(0x0, IOMUXC_BASE_ADDR + 0x214);
            writel(0x105, IOMUXC_BASE_ADDR + 0x604);
            if (dev->ss == 0) {
                // de-select SS1 of instance: ecspi1.
                writel(0x3, IOMUXC_BASE_ADDR + 0x21C);
                writel(0x85, IOMUXC_BASE_ADDR + 0x60C);
                // 000: Select mux mode: ALT0 mux port: SS0 of instance: ecspi1.
                writel(0x0, IOMUXC_BASE_ADDR + 0x218);
                writel(0x185, IOMUXC_BASE_ADDR + 0x608);
            } else if (dev->ss == 1) {
                // de-select SS0 of instance: ecspi1.
                writel(0x3, IOMUXC_BASE_ADDR + 0x218);
                writel(0x85, IOMUXC_BASE_ADDR + 0x608);
                // 000: Select mux mode: ALT0 mux port: SS1 of instance: ecspi1.
                writel(0x0, IOMUXC_BASE_ADDR + 0x21C);
                writel(0x105, IOMUXC_BASE_ADDR + 0x60C);
            }
            // 000: Select mux mode: ALT0 mux port: RDY of instance: ecspi1.
            writel(0x0, IOMUXC_BASE_ADDR + 0x220);
            writel(0x180, IOMUXC_BASE_ADDR + 0x610);

            // 000: Select mux mode: ALT0 mux port: SCLK of instance: ecspi1.
            writel(0x0, IOMUXC_BASE_ADDR + 0x224);
            writel(0x105, IOMUXC_BASE_ADDR + 0x614);
        } else {
            // 000: Select mux mode: ALT0 mux port: MOSI of instance: ecspi1
            writel(0x0, IOMUXC_BASE_ADDR + 0x21C);
            writel(0x105, IOMUXC_BASE_ADDR + 0x6CC);

            // 000: Select mux mode: ALT0 mux port: MISO of instance: ecspi1.
            writel(0x0, IOMUXC_BASE_ADDR + 0x220);
            writel(0x105, IOMUXC_BASE_ADDR + 0x6D0);
            if (dev->ss == 0) {
                // de-select SS1 of instance: ecspi1.
                writel(0x3, IOMUXC_BASE_ADDR + 0x228);
                writel(0x85, IOMUXC_BASE_ADDR + 0x6D8);
                // 000: Select mux mode: ALT0 mux port: SS0 of instance: ecspi1.
                writel(0x0, IOMUXC_BASE_ADDR + 0x224);
                writel(0x185, IOMUXC_BASE_ADDR + 0x6D4);
            } else if (dev->ss == 1) {
                // de-select SS0 of instance: ecspi1.
                writel(0x3, IOMUXC_BASE_ADDR + 0x224);
                writel(0x85, IOMUXC_BASE_ADDR + 0x6D4);
                // 000: Select mux mode: ALT0 mux port: SS1 of instance: ecspi1.
                writel(0x0, IOMUXC_BASE_ADDR + 0x228);
                writel(0x105, IOMUXC_BASE_ADDR + 0x6D8);
            }
            // 000: Select mux mode: ALT0 mux port: RDY of instance: ecspi1.
            writel(0x0, IOMUXC_BASE_ADDR + 0x22C);
            writel(0x180, IOMUXC_BASE_ADDR + 0x6DC);

            // 000: Select mux mode: ALT0 mux port: SCLK of instance: ecspi1.
            writel(0x0, IOMUXC_BASE_ADDR + 0x230);
            writel(0x105, IOMUXC_BASE_ADDR + 0x6E0);
        }
        break;
    case CSPI2_BASE_ADDR:
    default:
        break;
    }
}
