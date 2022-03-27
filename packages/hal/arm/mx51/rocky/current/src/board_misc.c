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
    X_ARM_MMU_SECTION(0xB00, 0xB00,   0x10,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CS0 EIM control*/
    X_ARM_MMU_SECTION(0xCC0, 0xCC0,   0x040, ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CS4/5/NAND Flash buffer */
}

#include <cyg/io/imx_spi.h>
struct spi_v2_3_reg spi_pmic_reg;

struct imx_spi_dev imx_spi_pmic = {
    base : CSPI1_BASE_ADDR,
    freq : 25000000,
    ss_pol : IMX_SPI_ACTIVE_HIGH,
    ss : 0,                     // slave select 0
    fifo_sz : 64 * 4,
    reg : &spi_pmic_reg,
};

struct spi_v2_3_reg spi_nor_reg;

struct imx_spi_dev imx_spi_nor = {
    base : CSPI1_BASE_ADDR,
    freq : 25000000,
    ss_pol : IMX_SPI_ACTIVE_LOW,
    ss : 1,                     // slave select 1
    fifo_sz : 64 * 4,
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

void plf_hardware_init(void)
{
    unsigned int reg;
    unsigned long weim_base;

    // CS0 setup
    weim_base = WEIM_BASE_ADDR;
    writel(0x00410089, weim_base + CSGCR1);
    writel(0x00000002, weim_base + CSGCR2);
    // RWSC=50, RADVA=2, RADVN=6, OEA=0, OEN=0, RCSA=0, RCSN=0
    writel(0x32260000, weim_base + CSRCR1);
    // APR=0
    writel(0x00000000, weim_base + CSRCR2);
    // WAL=0, WBED=1, WWSC=50, WADVA=2, WADVN=6, WEA=0, WEN=0, WCSA=0, WCSN=0
    writel(0x72080F00, weim_base + CSWCR1);

    /* Disable IPU and HSC dividers */
    writel(0x60000, CCM_BASE_ADDR + CLKCTL_CCDR);
    /* Change the DDR divider to run at 166MHz on CPU 2 */
    reg = readl(CCM_BASE_ADDR + CLKCTL_CBCDR);
    reg = (reg & (~0x70000)) | 0x30000;
    writel(reg, CCM_BASE_ADDR + CLKCTL_CBCDR);
     /* make sure divider effective */
    while (readl(CCM_BASE_ADDR + CLKCTL_CDHIPR) != 0);
    writel(0x0, CCM_BASE_ADDR + CLKCTL_CCDR);

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
    // enable ARM clock div by 8
    writel(0x010900F0, CCM_BASE_ADDR + CLKCTL_CCOSR);

    // now turn on the LCD backlight
    reg = readl(0x73f84004);
    writel(reg | 0x4, 0x73f84004);
    reg = readl(0x73f84000);
    writel(reg | 0x4, 0x73f84000);

    // now turn on the LCD
    // Set NANDF_CS7 pin to be GPIO output and set it to 1
    writel(0x3, 0x73fa8158);
    reg = readl(0x73f8c004);
    writel(reg | 0x800000, 0x73f8c004);
    reg = readl(0x73f8c000);
    writel(reg | 0x800000, 0x73f8c000);

    spi_nor_init = (imx_spi_init_func_t *)imx_spi_init_v2_3;
    spi_nor_xfer = (imx_spi_xfer_func_t *)imx_spi_xfer_v2_3;

    spi_pmic_init = (imx_spi_init_func_t *)imx_spi_init_v2_3;
    spi_pmic_xfer = (imx_spi_xfer_func_t *)imx_spi_xfer_v2_3;
}

static void rocky_lan_reset(void)
{
    unsigned int reg;

    /* Issue a reset to the LAN chip */
    reg = readl(GPIO2_BASE_ADDR + 0x0);
    reg |= 0x20000000 ; // write a 1 on the reset line
    writel(reg, GPIO2_BASE_ADDR + 0x0);

    reg = readl(GPIO2_BASE_ADDR + 0x4);
    reg |= 0x20000000;  // configure GPIO lines as output
    writel(reg, GPIO2_BASE_ADDR + 0x4);

    hal_delay_us(300);

    reg = readl(GPIO2_BASE_ADDR + 0x0);
    reg &= ~0x20000000;  // write a 0 on the reset line
    writel(reg, GPIO2_BASE_ADDR + 0x0);

    hal_delay_us(30000);

    reg = readl(GPIO2_BASE_ADDR + 0x0);
    reg |= 0x20000000 ; // write a 1 on the reset line
    writel(reg, GPIO2_BASE_ADDR + 0x0);
}

void mxc_ata_iomux_setup(void)
{
    // config NANDF_WE_B pad for pata instance DIOW port
    // config_pad_mode(NANDF_WE_B, ALT1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_WE_B);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull / Keep Select to Pull (Different from Module Level value: NA)
    // Pull Up / Down Config. to NA (CFG in SoC Level however NA in Module Level)
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to Disabled
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_WE_B, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_WE_B);

    // config NANDF_RE_B pad for pata instance DIOR port
    // config_pad_mode(NANDF_RE_B, ALT1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_RE_B);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull / Keep Select to Pull (Different from Module Level value: NA)
    // Pull Up / Down Config. to NA (CFG in SoC Level however NA in Module Level)
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to Disabled
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_RE_B, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_RE_B);

    // config NANDF_CLE pad for pata instance PATA_RESET_B port
    // config_pad_mode(NANDF_CLE, ALT1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CLE);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Hyst. Enable to Disabled
    // Pull / Keep Select to Keep (Different from Module Level value: NA)
    // Pull Up / Down Config. to 100Kohm PU (Different from Module Level value: NA)
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Pull / Keep Enable to Disabled
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_CLE, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CLE);

    // config NANDF_WP_B pad for pata instance DMACK port
    // config_pad_mode(NANDF_WP_B, ALT1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_WP_B);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull / Keep Select to Pull (Different from Module Level value: NA)
    // Pull Up / Down Config. to NA (CFG in SoC Level however NA in Module Level)
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to Disabled
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_WP_B, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_WP_B);

    // config NANDF_RB0 pad for pata instance DMARQ port
    // config_pad_mode(NANDF_RB0, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_RB0);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled (Different from Module Level value: NA)
    // Drive Strength to NA (CFG in SoC Level however NA in Module Level)
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to CFG(360Kohm PD)
    // config_pad_settings(NANDF_RB0, 0x20c0);
    writel(0xC0, IOMUXC_SW_PAD_CTL_PAD_NANDF_RB0);

    // config NANDF_RB1 pad for pata instance IORDY port
    // config_pad_mode(NANDF_RB1, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_RB1);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to NA (CFG in SoC Level however NA in Module Level)
    // Drive Strength to NA (CFG in SoC Level however NA in Module Level)
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // config_pad_settings(NANDF_RB1, 0x20e0);
    writel(0xD0, IOMUXC_SW_PAD_CTL_PAD_NANDF_RB1);

    // config NANDF_RB5 pad for pata instance INTRQ port
    // config_pad_mode(NANDF_RB5, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_RB5);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull Up / Down Config. to 100Kohm PU
    // Open Drain Enable to Disabled (Different from Module Level value: NA)
    // Drive Strength to NA (CFG in SoC Level however NA in Module Level)
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // config_pad_settings(NANDF_RB5, 0x20c0);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_RB5);

    // config NANDF_CS2 pad for pata instance CS_0 port
    // config_pad_mode(NANDF_CS2, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CS2);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull / Keep Select to NA (CFG in SoC Level however NA in Module Level)
    // Pull Up / Down Config. to NA (CFG in SoC Level however NA in Module Level)
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to Disabled
    // Open Drain Enable to Disabled
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_CS2, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CS2);

    // config NANDF_CS3 pad for pata instance CS_1 port
    // config_pad_mode(NANDF_CS3, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CS3);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull / Keep Select to NA (CFG in SoC Level however NA in Module Level)
    // Pull Up / Down Config. to NA (CFG in SoC Level however NA in Module Level)
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to Disabled
    // Open Drain Enable to Disabled
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_CS3, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CS3);

    // config NANDF_CS4 pad for pata instance DA_0 port
    // config_pad_mode(NANDF_CS4, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CS4);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull / Keep Select to NA (CFG in SoC Level however NA in Module Level)
    // Pull Up / Down Config. to NA (CFG in SoC Level however NA in Module Level)
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to Disabled
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_CS4, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CS4);

    // config NANDF_CS5 pad for pata instance DA_1 port
    // config_pad_mode(NANDF_CS5, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CS5);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull / Keep Select to NA (CFG in SoC Level however NA in Module Level)
    // Pull Up / Down Config. to NA (CFG in SoC Level however NA in Module Level)
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to Disabled
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_CS5, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CS5);

    // config NANDF_CS6 pad for pata instance DA_2 port
    // config_pad_mode(NANDF_CS6, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_CS6);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull / Keep Select to Pull (Different from Module Level value: NA)
    // Pull Up / Down Config. to NA (CFG in SoC Level however NA in Module Level)
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to Disabled
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_CS6, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_CS6);

    // config NANDF_D15 pad for pata instance PATA_DATA[15] port
    // config_pad_mode(NANDF_D15, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D15);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D15, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D15);

    // config NANDF_D14 pad for pata instance PATA_DATA[14] port
    // config_pad_mode(NANDF_D14, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D14);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D14, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D14);

    // config NANDF_D13 pad for pata instance PATA_DATA[13] port
    // config_pad_mode(NANDF_D13, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D13);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D13, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D13);

    // config NANDF_D12 pad for pata instance PATA_DATA[12] port
    // config_pad_mode(NANDF_D12, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D12);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D12, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D12);

    // config NANDF_D11 pad for pata instance PATA_DATA[11] port
    // config_pad_mode(NANDF_D11, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D11);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D11, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D11);

    // config NANDF_D10 pad for pata instance PATA_DATA[10] port
    // config_pad_mode(NANDF_D10, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D10);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D10, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D10);

    // config NANDF_D9 pad for pata instance PATA_DATA[9] port
    // config_pad_mode(NANDF_D9, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D9);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D9, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D9);

    // config NANDF_D8 pad for pata instance PATA_DATA[8] port
    // config_pad_mode(NANDF_D8, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D8);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D8, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D8);

    // config NANDF_D7 pad for pata instance PATA_DATA[7] port
    // config_pad_mode(NANDF_D7, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D7);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull Up / Down Config. to 100Kohm PU
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D7, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D7);

    // config NANDF_D6 pad for pata instance PATA_DATA[6] port
    // config_pad_mode(NANDF_D6, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D6);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull Up / Down Config. to 100Kohm PU
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Open Drain Enable to Disabled
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D6, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D6);

    // config NANDF_D5 pad for pata instance PATA_DATA[5] port
    // config_pad_mode(NANDF_D5, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D5);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull Up / Down Config. to 100Kohm PU
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D5, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D5);

    // config NANDF_D4 pad for pata instance PATA_DATA[4] port
    // config_pad_mode(NANDF_D4, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D4);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Pull Up / Down Config. to 100Kohm PU
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D4, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D4);

    // config NANDF_D3 pad for pata instance PATA_DATA[3] port
    // config_pad_mode(NANDF_D3, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D3);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D3, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D3);

    // config NANDF_D2 pad for pata instance PATA_DATA[2] port
    // config_pad_mode(NANDF_D2, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D2);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D2, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D2);

    // config NANDF_D1 pad for pata instance PATA_DATA[1] port
    // config_pad_mode(NANDF_D1, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D1);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D1, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D1);

    // config NANDF_D0 pad for pata instance PATA_DATA[0] port
    // config_pad_mode(NANDF_D0, 0x1);
    writel(0x1, IOMUXC_SW_MUX_CTL_PAD_NANDF_D0);
    // CONSTANT SETTINGS:
    // test_ts to Disabled
    // dse test to regular
    // strength mode to NA (Different from Module Level value: 4_level)
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // Slew Rate to NA
    // CONFIGURED SETTINGS:
    // low/high output voltage to CFG(High)
    // Hyst. Enable to Disabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to 100Kohm PU
    // Drive Strength to CFG(High)
    // config_pad_settings(NANDF_D0, 0x2004);
    writel(0x2004, IOMUXC_SW_PAD_CTL_PAD_NANDF_D0);
}

void mxc_i2c_init(unsigned int module_base)
{
    unsigned int reg;

    switch (module_base) {
    case I2C_BASE_ADDR:
        break;
    case I2C2_BASE_ADDR:
        writel(0x13, IOMUXC_BASE_ADDR + 0x0278);
        writel(0x13, IOMUXC_BASE_ADDR + 0x027C);

        writel(0x1, IOMUXC_BASE_ADDR + 0x0A08);
        writel(0x1, IOMUXC_BASE_ADDR + 0x0A0C);

        writel(0x1ED, IOMUXC_BASE_ADDR + 0x0728);
        writel(0x1ED, IOMUXC_BASE_ADDR + 0x072C);

        break;
    default:
        diag_printf("Invalid I2C base: 0x%x\n", module_base);
        return;
    }
}


void mxc_mmc_init(unsigned int base_address)
{
    switch(base_address) {
    case MMC_SDHC1_BASE_ADDR:
        //diag_printf("Configure IOMUX of ESDHC1 on i.MX51\n");
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
static void rocky_power_init(void)
{
    unsigned int val;

    /* power up the system first */
    pmic_reg(34, 0x00200000, 1);

    if (pll_clock(PLL1) > 800000000) {
        /* Set core voltage to 1.175V */
        val = pmic_reg(24, 0, 0);
        val = val & (~0x1F) | 0x17;
        pmic_reg(24, val, 1);
    }

    /* Set VDIG to 1.65V, VGEN3 to 1.8V, VCAM to 2.5V */
    val = pmic_reg(30, 0, 0);
    val &= ~0x34030;
    val |= 0x0020;
    pmic_reg(30, val, 1);

    /* Set VVIDEO to 2.775V, VAUDIO to 2.775V, VSD to 3.15V */
    val = pmic_reg(31, 0, 0);
    val &= ~0x1FC;
    val |= 0x1F4;
    pmic_reg(31, val, 1);

    /* Configure VGEN3 and VCAM regulators to use external PNP */
    val = 0x208;
    pmic_reg(33, val, 1);
    hal_delay_us(200);

    /* Enable VGEN1 regulator */
    val = pmic_reg(32, val, 0);
    val |= 0x1;
    pmic_reg(32, val, 1);

    /* Enable VGEN3, VCAM, VAUDIO, VVIDEO, VSD regulators */
    val = 0x49249;
    pmic_reg(33, val, 1);

    /* Enable SWBSTEN */
    val = pmic_reg(29, val, 0);
    val |= 0x100000;
    pmic_reg(29, val, 1);

    /* SW2 to 1.25V (VCC - MX51 Peripheral core) */
    val = pmic_reg(25, val, 0);
    val &= ~0x1F;
    val |= 0x19;
    pmic_reg(25, val, 1);

    hal_delay_us(300);

    rocky_lan_reset();
}

RedBoot_init(rocky_power_init, RedBoot_INIT_PRIO(900));

void io_cfg_spi(struct imx_spi_dev *dev)
{
    switch (dev->base) {
    case CSPI1_BASE_ADDR:
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
        break;
    case CSPI2_BASE_ADDR:
    default:
        break;
    }
}
