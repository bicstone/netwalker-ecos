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
#include <cyg/io/mxc_i2c.h>
#include <cyg/io/imx_nfc.h>
#include <cyg/infra/diag.h>             // diag_printf

// All the MM table layout is here:
#include <cyg/hal/hal_mm.h>
#include <cyg/io/imx_spi.h>

externC void* memset(void *, int, size_t);
extern nfc_iomuxsetup_func_t *nfc_iomux_setup;

unsigned int cpld_base_addr;

struct spi_v2_3_reg spi_nor_reg;
struct imx_spi_dev imx_spi_nor = {
    base : CSPI2_BASE_ADDR,
    freq : 25000000,
    ss_pol : IMX_SPI_ACTIVE_LOW,
    ss : 1,
    fifo_sz : 64 * 4,
    us_delay: 0,
    reg : &spi_nor_reg,
};

imx_spi_init_func_t *spi_nor_init;
imx_spi_xfer_func_t *spi_nor_xfer;

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
    X_ARM_MMU_SECTION(0x900, 0x000,   0x080, ARM_CACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0x900, 0x900,   0x080, ARM_CACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0x900, 0x980,   0x080, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SDRAM 0:128M*/
    X_ARM_MMU_SECTION(0xB80, 0xB80,   0x10,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CS1 EIM control*/
    X_ARM_MMU_SECTION(0xCC0, 0xCC0,   0x040, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CS4/5/NAND Flash buffer */
}

void mxc_i2c_init(unsigned int module_base)
{
    unsigned int reg;

    switch (module_base) {
    case I2C_BASE_ADDR:
        if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) == 0x2) {
            reg = IOMUXC_BASE_ADDR + 0x210; // i2c SDA
            writel(0x11, reg);
            reg = IOMUXC_BASE_ADDR + 0x600;
            writel(0x1ad, reg);
            reg = IOMUXC_BASE_ADDR + 0x9B4;
            writel(0x1, reg);

            reg = IOMUXC_BASE_ADDR + 0x224; // i2c SCL
            writel(0x11, reg);
            reg = IOMUXC_BASE_ADDR + 0x614;
            writel(0x1ad, reg);
            reg = IOMUXC_BASE_ADDR + 0x9B0;
            writel(0x1, reg);
        } else {
            reg = IOMUXC_BASE_ADDR + 0x230; // i2c SCL
            writel(0x11, reg);
            reg = IOMUXC_BASE_ADDR + 0x6e0;
            writel(0x1ad, reg);
            reg = IOMUXC_BASE_ADDR + 0xA00;
            writel(0x1, reg);

            reg = IOMUXC_BASE_ADDR + 0x21C; // i2c SDA
            writel(0x11, reg);
            reg = IOMUXC_BASE_ADDR + 0x6cc;
            writel(0x1ad, reg);
            reg = IOMUXC_BASE_ADDR + 0xA04;
            writel(0x1, reg);
        }
        break;
    case I2C2_BASE_ADDR:
        if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) == 0x2) {
            /* Workaround for Atlas Lite */
            writel(0x0, IOMUXC_BASE_ADDR + 0x3CC); // i2c SCL
            writel(0x0, IOMUXC_BASE_ADDR + 0x3D0); // i2c SDA
            reg = readl(GPIO1_BASE_ADDR + 0x0);
            reg |= 0xC;  // write a 1 on the SCL and SDA lines
            writel(reg, GPIO1_BASE_ADDR + 0x0);
            reg = readl(GPIO1_BASE_ADDR + 0x4);
            reg |= 0xC;  // configure GPIO lines as output
            writel(reg, GPIO1_BASE_ADDR + 0x4);
            reg = readl(GPIO1_BASE_ADDR + 0x0);
            reg &= ~0x4 ; // set SCL low for a few milliseconds
            writel(reg, GPIO1_BASE_ADDR + 0x0);
            hal_delay_us(20000);
            reg |= 0x4;
            writel(reg, GPIO1_BASE_ADDR + 0x0);
            hal_delay_us(10);
            reg = readl(GPIO1_BASE_ADDR + 0x4);
            reg &= ~0xC;  // configure GPIO lines back as input
            writel(reg, GPIO1_BASE_ADDR + 0x4);

            writel(0x12, IOMUXC_BASE_ADDR + 0x3CC);  // i2c SCL
            writel(0x3, IOMUXC_BASE_ADDR + 0x9B8);
            writel(0x1ed, IOMUXC_BASE_ADDR + 0x7D4);

            writel(0x12, IOMUXC_BASE_ADDR + 0x3D0); // i2c SDA
            writel(0x3, IOMUXC_BASE_ADDR + 0x9BC);
            writel(0x1ed, IOMUXC_BASE_ADDR + 0x7D8);
        } else {
            /* Workaround for Atlas Lite */
            writel(0x0, IOMUXC_BASE_ADDR + 0x3D4); // i2c SCL
            writel(0x0, IOMUXC_BASE_ADDR + 0x3D8); // i2c SDA
            reg = readl(GPIO1_BASE_ADDR + 0x0);
            reg |= 0xC;  // write a 1 on the SCL and SDA lines
            writel(reg, GPIO1_BASE_ADDR + 0x0);
            reg = readl(GPIO1_BASE_ADDR + 0x4);
            reg |= 0xC;  // configure GPIO lines as output
            writel(reg, GPIO1_BASE_ADDR + 0x4);
            reg = readl(GPIO1_BASE_ADDR + 0x0);
            reg &= ~0x4 ; // set SCL low for a few milliseconds
            writel(reg, GPIO1_BASE_ADDR + 0x0);
            hal_delay_us(20000);
            reg |= 0x4;
            writel(reg, GPIO1_BASE_ADDR + 0x0);
            hal_delay_us(10);
            reg = readl(GPIO1_BASE_ADDR + 0x4);
            reg &= ~0xC;  // configure GPIO lines back as input
            writel(reg, GPIO1_BASE_ADDR + 0x4);

            writel(0x12, IOMUXC_BASE_ADDR + 0x3D4);  // i2c SCL
            writel(0x3, IOMUXC_BASE_ADDR + 0xA08);
            writel(0x1ed, IOMUXC_BASE_ADDR + 0x8A0);

            writel(0x12, IOMUXC_BASE_ADDR + 0x3D8); // i2c SDA
            writel(0x3, IOMUXC_BASE_ADDR + 0xA0C);
            writel(0x1ed, IOMUXC_BASE_ADDR + 0x8A4);
        }
        break;
    default:
        diag_printf("Invalid I2C base: 0x%x\n", module_base);
        return;
    }
}

void mxc_ata_iomux_setup(void)
{
    // config NANDF_WE_B pad for pata instance DIOW port
    // config_pad_mode(NANDF_WE_B, ALT1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_WE_B);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_WE_B);

    // config NANDF_RE_B pad for pata instance DIOR port
    // config_pad_mode(NANDF_RE_B, ALT1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_RE_B);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_RE_B);

    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_ALE);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_ALE);

    // config NANDF_CLE pad for pata instance PATA_RESET_B port
    // config_pad_mode(NANDF_CLE, ALT1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CLE);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CLE);

    // config NANDF_WP_B pad for pata instance DMACK port
    // config_pad_mode(NANDF_WP_B, ALT1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_WP_B);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_WP_B);

    // config NANDF_RB0 pad for pata instance DMARQ port
    // config_pad_mode(NANDF_RB0, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_RB0);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_RB0);

    // config NANDF_RB1 pad for pata instance IORDY port
    // config_pad_mode(NANDF_RB1, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_RB1);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_RB1);

    // config NANDF_RB5 pad for pata instance INTRQ port
    // config_pad_mode(NANDF_RB5, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_RB5);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_RB5);

    // config NANDF_CS2 pad for pata instance CS_0 port
    // config_pad_mode(NANDF_CS2, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CS2);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CS2);

    // config NANDF_CS3 pad for pata instance CS_1 port
    // config_pad_mode(NANDF_CS3, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CS3);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CS3);

    // config NANDF_CS4 pad for pata instance DA_0 port
    // config_pad_mode(NANDF_CS4, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CS4);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CS4);

    // config NANDF_CS5 pad for pata instance DA_1 port
    // config_pad_mode(NANDF_CS5, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CS5);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CS5);

    // config NANDF_CS6 pad for pata instance DA_2 port
    // config_pad_mode(NANDF_CS6, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CS6);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CS6);

    // config NANDF_D15 pad for pata instance PATA_DATA[15] port
    // config_pad_mode(NANDF_D15, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D15);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D15);

    // config NANDF_D14 pad for pata instance PATA_DATA[14] port
    // config_pad_mode(NANDF_D14, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D14);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D14);

    // config NANDF_D13 pad for pata instance PATA_DATA[13] port
    // config_pad_mode(NANDF_D13, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D13);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D13);

    // config NANDF_D12 pad for pata instance PATA_DATA[12] port
    // config_pad_mode(NANDF_D12, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D12);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D12);

    // config NANDF_D11 pad for pata instance PATA_DATA[11] port
    // config_pad_mode(NANDF_D11, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D11);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D11);

    // config NANDF_D10 pad for pata instance PATA_DATA[10] port
    // config_pad_mode(NANDF_D10, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D10);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D10);

    // config NANDF_D9 pad for pata instance PATA_DATA[9] port
    // config_pad_mode(NANDF_D9, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D9);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D9);

    // config NANDF_D8 pad for pata instance PATA_DATA[8] port
    // config_pad_mode(NANDF_D8, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D8);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D8);

    // config NANDF_D7 pad for pata instance PATA_DATA[7] port
    // config_pad_mode(NANDF_D7, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D7);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D7);

    // config NANDF_D6 pad for pata instance PATA_DATA[6] port
    // config_pad_mode(NANDF_D6, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D6);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D6);

    // config NANDF_D5 pad for pata instance PATA_DATA[5] port
    // config_pad_mode(NANDF_D5, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D5);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D5);

    // config NANDF_D4 pad for pata instance PATA_DATA[4] port
    // config_pad_mode(NANDF_D4, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D4);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D4);

    // config NANDF_D3 pad for pata instance PATA_DATA[3] port
    // config_pad_mode(NANDF_D3, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D3);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D3);

    // config NANDF_D2 pad for pata instance PATA_DATA[2] port
    // config_pad_mode(NANDF_D2, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D2);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D2);

    // config NANDF_D1 pad for pata instance PATA_DATA[1] port
    // config_pad_mode(NANDF_D1, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D1);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D1);

    // config NANDF_D0 pad for pata instance PATA_DATA[0] port
    // config_pad_mode(NANDF_D0, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D0);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D0);
}

static void mxc_fec_setup(void)
{
    volatile unsigned int reg;

    /* No FEC support for TO 2.0 yet */
    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) == 0x2)
        return;
    /*FEC_TX_CLK*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0390);
    writel(0x180, IOMUXC_BASE_ADDR + 0x085C);
    writel(0x1, IOMUXC_BASE_ADDR + 0x09D0);

    /*FEC_RX_CLK*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0388);
    writel(0x180, IOMUXC_BASE_ADDR + 0x0854);
    writel(0x1, IOMUXC_BASE_ADDR + 0x09C4);

    /*FEC_RX_DV*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x038c);
    writel(0x180, IOMUXC_BASE_ADDR + 0x0858);
    writel(0x1, IOMUXC_BASE_ADDR + 0x09C8);

    /*FEC_COL*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0384);
    writel(0x180, IOMUXC_BASE_ADDR + 0x0850);
    writel(0x1, IOMUXC_BASE_ADDR + 0x9A8);

    /*FEC_RDATA0*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0394);
    writel(0x180, IOMUXC_BASE_ADDR + 0x0860);
    writel(0x1, IOMUXC_BASE_ADDR + 0x09B4);

    /*FEC_TDATA0*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0398);
    writel(0x5, IOMUXC_BASE_ADDR + 0x864);

    /*FEC_TX_EN*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0380);
    writel(0x5, IOMUXC_BASE_ADDR + 0x084C);

    /*FEC_MDC*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x034C);
    writel(0x5, IOMUXC_BASE_ADDR + 0x0818);

    /*FEC_MDIO*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0350);
    writel(0x1CD, IOMUXC_BASE_ADDR + 0x081C);
    writel(0x1, IOMUXC_BASE_ADDR + 0x09B0);

    /*FEC_TX_ERR*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0344);
    writel(0x5, IOMUXC_BASE_ADDR + 0x0810);

    /*FEC_RX_ERR*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0360);
    writel(0x180, IOMUXC_BASE_ADDR + 0x082C);
    writel(0x1, IOMUXC_BASE_ADDR + 0x09CC);

    /*FEC_CRS*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0348);
    writel(0x180, IOMUXC_BASE_ADDR + 0x0814);
    writel(0x1, IOMUXC_BASE_ADDR + 0x09AC);

    /*FEC_RDATA1*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0354);
    writel(0x180, IOMUXC_BASE_ADDR + 0x0820);
    writel(0x1, IOMUXC_BASE_ADDR + 0x09B8);

    /*FEC_TDATA1*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0374);
    writel(0x5, IOMUXC_BASE_ADDR + 0x0840);

    /*FEC_RDATA2*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0358);
    writel(0x180, IOMUXC_BASE_ADDR + 0x0824);
    writel(0x1, IOMUXC_BASE_ADDR + 0x09BC);

    /*FEC_TDATA2*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x0378);
    writel(0x5, IOMUXC_BASE_ADDR + 0x0844);

    /*FEC_RDATA3*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x035C);
    writel(0x180, IOMUXC_BASE_ADDR + 0x0828);
    writel(0x1, IOMUXC_BASE_ADDR + 0x09C0);

    /*FEC_TDATA3*/
    writel(0x2, IOMUXC_BASE_ADDR + 0x037C);
    writel(0x5, IOMUXC_BASE_ADDR + 0x0848);

    reg = readl(GPIO3_BASE_ADDR + 0x0);
    reg &= ~0x40;  // Lower reset line
    writel(reg, GPIO3_BASE_ADDR + 0x0);

    reg = readl(GPIO3_BASE_ADDR + 0x4);
    reg |= 0x40;  // configure GPIO lines as output
    writel(reg, GPIO3_BASE_ADDR + 0x4);

    /* Reset the ethernet controller over GPIO */
    writel(0x4, IOMUXC_BASE_ADDR + 0x02CC);
    writel(0xC5, IOMUXC_BASE_ADDR + 0x078C);

    hal_delay_us(200);

    reg = readl(GPIO3_BASE_ADDR + 0x0);
    reg |= 0x40;
    writel(reg, GPIO3_BASE_ADDR + 0x0);
}

static void mxc_nfc_iomux_setup(void)
{
    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) == 0x2) {
        writel(0x0, IOMUXC_BASE_ADDR + 0x108);
        writel(0x0, IOMUXC_BASE_ADDR + 0x10C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x110);
        writel(0x0, IOMUXC_BASE_ADDR + 0x114);
        writel(0x0, IOMUXC_BASE_ADDR + 0x118);
        writel(0x0, IOMUXC_BASE_ADDR + 0x11C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x120);
        writel(0x0, IOMUXC_BASE_ADDR + 0x124);
        writel(0x0, IOMUXC_BASE_ADDR + 0x128);
        writel(0x0, IOMUXC_BASE_ADDR + 0x12C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x130);
        writel(0x0, IOMUXC_BASE_ADDR + 0x134);
        writel(0x0, IOMUXC_BASE_ADDR + 0x138);
        writel(0x0, IOMUXC_BASE_ADDR + 0x13C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x140);
        writel(0x0, IOMUXC_BASE_ADDR + 0x144);
        writel(0x0, IOMUXC_BASE_ADDR + 0x148);
        writel(0x0, IOMUXC_BASE_ADDR + 0x14C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x150);
        writel(0x0, IOMUXC_BASE_ADDR + 0x154);
        writel(0x0, IOMUXC_BASE_ADDR + 0x158);
        writel(0x0, IOMUXC_BASE_ADDR + 0x15C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x160);
        writel(0x0, IOMUXC_BASE_ADDR + 0x164);
        writel(0x0, IOMUXC_BASE_ADDR + 0x168);
        writel(0x0, IOMUXC_BASE_ADDR + 0x16C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x170);
        writel(0x0, IOMUXC_BASE_ADDR + 0x174);
        writel(0x0, IOMUXC_BASE_ADDR + 0x178);
        writel(0x0, IOMUXC_BASE_ADDR + 0x17C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x180);
        writel(0x0, IOMUXC_BASE_ADDR + 0x184);
        writel(0x0, IOMUXC_BASE_ADDR + 0x188);
        writel(0x0, IOMUXC_BASE_ADDR + 0x18C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x190);
    } else {
        writel(0x0, IOMUXC_BASE_ADDR + 0x108);
        writel(0x0, IOMUXC_BASE_ADDR + 0x10C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x110);
        writel(0x0, IOMUXC_BASE_ADDR + 0x114);
        writel(0x0, IOMUXC_BASE_ADDR + 0x118);
        writel(0x0, IOMUXC_BASE_ADDR + 0x11C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x120);
        writel(0x0, IOMUXC_BASE_ADDR + 0x124);
        writel(0x0, IOMUXC_BASE_ADDR + 0x128);
        writel(0x0, IOMUXC_BASE_ADDR + 0x12C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x130);
        writel(0x0, IOMUXC_BASE_ADDR + 0x134);
        writel(0x0, IOMUXC_BASE_ADDR + 0x138);
        writel(0x0, IOMUXC_BASE_ADDR + 0x13C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x140);
        writel(0x0, IOMUXC_BASE_ADDR + 0x144);
        writel(0x0, IOMUXC_BASE_ADDR + 0x148);
        writel(0x0, IOMUXC_BASE_ADDR + 0x14C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x150);
        writel(0x0, IOMUXC_BASE_ADDR + 0x154);
        writel(0x0, IOMUXC_BASE_ADDR + 0x158);
        writel(0x0, IOMUXC_BASE_ADDR + 0x15C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x160);
        writel(0x0, IOMUXC_BASE_ADDR + 0x164);
        writel(0x0, IOMUXC_BASE_ADDR + 0x168);
        writel(0x0, IOMUXC_BASE_ADDR + 0x16C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x170);
        writel(0x0, IOMUXC_BASE_ADDR + 0x174);
        writel(0x0, IOMUXC_BASE_ADDR + 0x178);
        writel(0x0, IOMUXC_BASE_ADDR + 0x17C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x180);
        writel(0x0, IOMUXC_BASE_ADDR + 0x184);
        writel(0x0, IOMUXC_BASE_ADDR + 0x188);
        writel(0x0, IOMUXC_BASE_ADDR + 0x18C);
        writel(0x0, IOMUXC_BASE_ADDR + 0x190);
        writel(0x0, IOMUXC_BASE_ADDR + 0x194);
        writel(0x0, IOMUXC_BASE_ADDR + 0x198);
        writel(0x0, IOMUXC_BASE_ADDR + 0x19C);
    }
}

//
// Platform specific initialization
//

void plf_hardware_init(void)
{
    unsigned long sw_rest_reg, weim_base;
    unsigned int reg;
    unsigned char buf[4];
    struct mxc_i2c_request rq;

    /* Atlas Workaround needed only for TO 1.0 and 1.1 boards */
    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) != 0x2) {
        if (i2c_init(I2C2_BASE_ADDR, 170000) == 0) {
            rq.dev_addr = 0x8;
            rq.reg_addr = 0x7;
            rq.reg_addr_sz = 1;
            rq.buffer_sz = 3;
            rq.buffer = buf;

            i2c_xfer(1, &rq, 1);
            /* Make sure we implement this workaround only for boards with Atlas-Lite to turn off the charger */
            if ((buf[1] == 0x41) && (buf[2] == 0xc8 || buf[2] == 0xc9)) {
                buf[0] = 0xb4;
                buf[1] = 0x0;
                buf[2] = 0x3;
                rq.dev_addr = 0x8;
                rq.reg_addr = 0x30;
                rq.reg_addr_sz = 1;
                rq.buffer_sz = 3;
                rq.buffer = buf;

                i2c_xfer(1, &rq, 0);
            }
        }
    }
    // CS5 setup
    writel(0, IOMUXC_BASE_ADDR + 0xF4);
    weim_base = WEIM_BASE_ADDR + 0x78;
    writel(0x00410089, weim_base + CSGCR1);
    writel(0x00000002, weim_base + CSGCR2);
    // RWSC=50, RADVA=2, RADVN=6, OEA=0, OEN=0, RCSA=0, RCSN=0
    writel(0x32260000, weim_base + CSRCR1);
    // APR=0
    writel(0x00000000, weim_base + CSRCR2);
    // WAL=0, WBED=1, WWSC=50, WADVA=2, WADVN=6, WEA=0, WEN=0, WCSA=0, WCSN=0
    writel(0x72080F00, weim_base + CSWCR1);
    cpld_base_addr = CS5_BASE_ADDR;

    /* Reset interrupt status reg */
    writew(0x1F, cpld_base_addr + PBC_INT_REST);
    writew(0x00, cpld_base_addr + PBC_INT_REST);
    writew(0xFFFF, cpld_base_addr + PBC_INT_MASK);

    /* Reset the XUART and Ethernet controllers */
    sw_rest_reg = readw(cpld_base_addr + PBC_SW_RESET);
    sw_rest_reg |= 0x9;
    writew(sw_rest_reg, cpld_base_addr + PBC_SW_RESET);
    sw_rest_reg &= ~0x9;
    writew(sw_rest_reg, cpld_base_addr + PBC_SW_RESET);

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
#ifdef MXCFLASH_SELECT_NAND
    nfc_iomux_setup = (nfc_iomuxsetup_func_t*)mxc_nfc_iomux_setup;
#endif
    mxc_fec_setup();

    spi_nor_init = (imx_spi_init_func_t *)imx_spi_init_v2_3;
    spi_nor_xfer = (imx_spi_xfer_func_t *)imx_spi_xfer_v2_3;
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
    default:
        break;
    }
}

void increase_core_voltage(bool i)
{
    unsigned char buf[4];
    struct mxc_i2c_request rq;

    if (i2c_init(I2C2_BASE_ADDR, 170000) == 0) {
        rq.dev_addr = 0x8;
        rq.reg_addr = 24;
        rq.reg_addr_sz = 1;
        rq.buffer_sz = 3;
        rq.buffer = buf;

        i2c_xfer(1, &rq, 1);

        if (i) {
            buf[2] = (buf[2] & (~0x1F)) | 0x17; //1.175
            //buf[2] = (buf[2] & (~0x1F)) | 0x18; //1.2
        } else {
            buf[2] = (buf[2] & (~0x1F)) | 0x12;
        }
        i2c_xfer(1, &rq, 0);
    } else {
        diag_printf("Cannot increase core voltage, failed to initialize I2C2\n");
    }
}

void io_cfg_spi(struct imx_spi_dev *dev)
{
    unsigned int reg;

    switch (dev->base) {
    case CSPI1_BASE_ADDR:
        break;
    case CSPI2_BASE_ADDR:
        // Select mux mode: ALT2 mux port: MOSI of instance: ecspi2
        writel(0x2, IOMUXC_BASE_ADDR + 0x154);
        writel(0x105, IOMUXC_BASE_ADDR + 0x53C);

        // Select mux mode: ALT2 mux port: MISO of instance: ecspi2.
        writel(0x2, IOMUXC_BASE_ADDR + 0x128);
        writel(0x105, IOMUXC_BASE_ADDR + 0x504);

       // de-select SS0 of instance: ecspi1.
       writel(0x2, IOMUXC_BASE_ADDR + 0x298);
       writel(0x85, IOMUXC_BASE_ADDR + 0x698);
       // Select mux mode: ALT2 mux port: SS1 of instance: ecspi2.
       writel(0x2, IOMUXC_BASE_ADDR + 0x160);
       writel(0x105, IOMUXC_BASE_ADDR + 0x548);

        // Select mux mode: ALT3 mux port: GPIO mode
        writel(0x3, IOMUXC_BASE_ADDR + 0x150);
        writel(0xE0, IOMUXC_BASE_ADDR + 0x538);
        reg = readl(GPIO3_BASE_ADDR + 0x0);
        reg |= 0x1000000;  // write a 1
        writel(reg, GPIO3_BASE_ADDR + 0x0);
        reg = readl(GPIO3_BASE_ADDR + 0x4);
        reg |= 0x1000000;  // configure GPIO lines as output
        writel(reg, GPIO3_BASE_ADDR + 0x4);

        // Select mux mode: ALT2 mux port: SCLK of instance: ecspi2.
        writel(0x2, IOMUXC_BASE_ADDR + 0x124);
        writel(0x105, IOMUXC_BASE_ADDR + 0x500);
        break;
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

