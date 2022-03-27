//==========================================================================
//
//	board_misc.c
//
//	HAL misc board support code for the board
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
//
// modification information
// ------------------------
// 2009/07/03 : error_loop() LED fixed.
//		bootVersion[] add.
// 2009/07/10 : Ver 0.003
// 2009/07/13 : Ver 0.004
// 2009/07/21 : Ver 0.008
//		Cover-SW(CSI2_D19)/POWER-SW(EIM_A27) support.
//		BOARD_SPIDER_QA0 support.
//		is_ac_adapter_connect() add.
// 2009/07/27 : Ver 0.009
//		from redboot_200925
//		 spi fifo_sz 64*4 -> 32,
//		CHG_CTRL alway charge add.
//		delete is_ac_adapter_connect().
//		BAT_AC2 check DCin volt.
//		IOMUXC_GPIO3_IPP_IND_G_IN_4_SELECT_INPUT(DI1_D1_CS) fixed.
// 2009/07/28 : BAT_AC2 alway set DCinput
// 2009/07/29 : Ver 0.010
//		WDI(GPIO1_4) support for except ERDOS.
//		stop_normal_boot() add.
// 2009/08/01 : Ver 0.012
// 2009/08/02 : Ver 0.013
// 2009/08/06 : Ver 0.014
//		console restart at stop_normal_boot().
// 2009/08/12 : Ver 0.015 PWM change.
// 2009/08/12 : Ver 0.016 PWM mode fixed.
// 2009/08/13 : Ver 0.017
// 2009/08/17 : Ver 0.018
//              0001-Increase-voltages-and-adjust-the-frequency.patch
//              apply Switch_To_PWM_Atlas20a.patch
// 2009/08/19 : Ver 0.019
//              delete Switch_To_PWM_Atlas20a.patch
// 2009/09/03 : Ver 0.101
//               Regulator setting change reg:33=001209
// 2009/09/04 : Ver 0.102
//               add RTC calibration setting.
// 2009/09/06 : Ver 0.103
//               Regulator setting change reg:32=049608 33=003209
// 2009/09/09 : Ver 1.000
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include <redboot.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h> 	// base types
#include <cyg/infra/cyg_trac.h> 	// tracing macros
#include <cyg/infra/cyg_ass.h>		// assertion macros

#include <cyg/hal/hal_io.h>		// IO macros
#include <cyg/hal/hal_arch.h>		// Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>		// Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_soc.h>	     // Hardware definitions
#include <cyg/hal/fsl_board.h>		   // Platform specifics
#include <cyg/infra/diag.h>		// diag_printf

// All the MM table layout is here:
#include <cyg/hal/hal_mm.h>

externC void* memset(void *, int, size_t);
unsigned int cpld_base_addr;

#ifdef MX51_ERDOS
/*
 * Bootloader Version
 */
char *bootVersion = "1.000";
#endif /* MX51_ERDOS */

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

    /*		    Actual   Virtual  Size   Attributes 						   Function  */
    /*		    Base     Base     MB     cached?	       buffered?	access permissions		   */
    /*		    xxx00000 xxx00000										     */
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
    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) >= 0x2) {
	/*FEC_MDIO*/
	writel(0x3, IOMUXC_BASE_ADDR + 0x0D4);
	writel(0x1FD, IOMUXC_BASE_ADDR + 0x0468);
	writel(0x0, IOMUXC_BASE_ADDR + 0x0954);

	/*FEC_MDC*/
	writel(0x2, IOMUXC_BASE_ADDR + 0x13C);
#if 1
	writel(0x0004, IOMUXC_BASE_ADDR + 0x0524);
#else
	writel(0x2004, IOMUXC_BASE_ADDR + 0x0524);
#endif

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
#if 1
	writel(0x0180, IOMUXC_BASE_ADDR + 0x0554);
#else
	writel(0x2180, IOMUXC_BASE_ADDR + 0x0554);
#endif
	writel(0x0, IOMUXC_BASE_ADDR + 0x0958);

	/* FEC TDATA[3] */
	writel(0x2, IOMUXC_BASE_ADDR + 0x148);
#if 1
	writel(0x0004, IOMUXC_BASE_ADDR + 0x0530);
#else
	writel(0x2004, IOMUXC_BASE_ADDR + 0x0530);
#endif

	/* FEC TDATA[2] */
	writel(0x2, IOMUXC_BASE_ADDR + 0x144);
#if 1
	writel(0x0004, IOMUXC_BASE_ADDR + 0x052C);
#else
	writel(0x2004, IOMUXC_BASE_ADDR + 0x052C);
#endif

	/* FEC TDATA[1] */
	writel(0x2, IOMUXC_BASE_ADDR + 0x140);
#if 1
	writel(0x0004, IOMUXC_BASE_ADDR + 0x0528);
#else
	writel(0x2004, IOMUXC_BASE_ADDR + 0x0528);
#endif

	/* FEC TDATA[0] */
	writel(0x2, IOMUXC_BASE_ADDR + 0x0170);
#if 1
	writel(0x0004, IOMUXC_BASE_ADDR + 0x0558);
#else
	writel(0x2004, IOMUXC_BASE_ADDR + 0x0558);
#endif

	/* FEC TX_EN */
	writel(0x1, IOMUXC_BASE_ADDR + 0x014C);
#if 1
	writel(0x0004, IOMUXC_BASE_ADDR + 0x0534);
#else
	writel(0x2004, IOMUXC_BASE_ADDR + 0x0534);
#endif

#if 1
	/* FEC TX_ER(GPIO3 18 INPUT) */
	writel(0x3, IOMUXC_BASE_ADDR + 0x0138);
	writel(0x0004, IOMUXC_BASE_ADDR + 0x0520);
#else
	/* FEC TX_ER */
	writel(0x2, IOMUXC_BASE_ADDR + 0x0138);
	writel(0x2004, IOMUXC_BASE_ADDR + 0x0520);
#endif

	/* FEC TX_CLK */
	writel(0x1, IOMUXC_BASE_ADDR + 0x0150);
#if 1
	writel(0x0180, IOMUXC_BASE_ADDR + 0x0538);
#else
	writel(0x2180, IOMUXC_BASE_ADDR + 0x0538);
#endif
	writel(0x0, IOMUXC_BASE_ADDR + 0x0974);

	/* FEC COL */
	writel(0x1, IOMUXC_BASE_ADDR + 0x0124);
#if 1
	writel(0x0180, IOMUXC_BASE_ADDR + 0x0500);
#else
	writel(0x2180, IOMUXC_BASE_ADDR + 0x0500);
#endif
	writel(0x0, IOMUXC_BASE_ADDR + 0x094c);

	/* FEC RX_CLK */
	writel(0x1, IOMUXC_BASE_ADDR + 0x0128);
#if 1
	writel(0x0180, IOMUXC_BASE_ADDR + 0x0504);
#else
	writel(0x2180, IOMUXC_BASE_ADDR + 0x0504);
#endif
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
#if 1
	writel(0x0180, IOMUXC_BASE_ADDR + 0x054C);
#else
	writel(0x2180, IOMUXC_BASE_ADDR + 0x054C);
#endif
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

static void mxc_nand_setup(void)
{
    volatile unsigned int reg;
    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) == 0x2) {
	writel(0x0094, IOMUXC_BASE_ADDR + 0x04E4);	/* NANDF_WE */
	writel(0x0094, IOMUXC_BASE_ADDR + 0x04E8);	/* NANDF_RE */
	writel(0x0084, IOMUXC_BASE_ADDR + 0x04EC);	/* NANDF_ALE */
	writel(0x0084, IOMUXC_BASE_ADDR + 0x04F0);	/* NANDF_CLE */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x04F4);	/* NANDF_WP */
	writel(0x00E0, IOMUXC_BASE_ADDR + 0x04F8);	/* NANDF_RB0 */
	writel(0x00E0, IOMUXC_BASE_ADDR + 0x04FC);	/* NANDF_RB1 */
	writel(0x0080, IOMUXC_BASE_ADDR + 0x0514);	/* GPIO_NAND */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x053C);	/* NAND_D15 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x0540);	/* NAND_D14 */
	writel(0x0024, IOMUXC_BASE_ADDR + 0x0544);	/* NAND_D13 */
	writel(0x0024, IOMUXC_BASE_ADDR + 0x0548);	/* NAND_D12 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x054C);	/* NAND_D11 */
	writel(0x0024, IOMUXC_BASE_ADDR + 0x0550);	/* NAND_D10 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x0554);	/* NAND_D9 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x0558);	/* NAND_D8 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x055C);	/* NAND_D7 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x0560);	/* NAND_D6 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x0564);	/* NAND_D5 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x0568);	/* NAND_D4 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x056C);	/* NAND_D3 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x0570);	/* NAND_D2 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x0574);	/* NAND_D1 */
	writel(0x00A4, IOMUXC_BASE_ADDR + 0x0578);	/* NAND_D0 */
    }
    /*
     * 8bit-NAND
     */
    reg = readl (NFC_FLASH_CONFIG3_REG);
    reg = (reg & ~0x7003) | 0x00000008;
    writel (reg, NFC_FLASH_CONFIG3_REG);
}

#include <cyg/io/imx_spi.h>
struct spi_v2_3_reg spi_pmic_reg;

struct imx_spi_dev imx_spi_pmic = {
    base : CSPI1_BASE_ADDR,
    freq : 25000000,
    ss_pol : IMX_SPI_ACTIVE_HIGH,
    ss : 0,			// slave select 0
    fifo_sz : 32,
    reg : &spi_pmic_reg,
};

struct spi_v2_3_reg spi_nor_reg;

struct imx_spi_dev imx_spi_nor = {
    base : CSPI1_BASE_ADDR,
    freq : 25000000,
    ss_pol : IMX_SPI_ACTIVE_LOW,
    ss : 1,			// slave select 1
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

extern void (*fec_phy_callback)(int phy_found);
static unsigned int sample_initial_gpio_mask;	/* sample initailize GPIO MASK */
static unsigned int sample_initial_gpio;	/* sample initailize GPIO */
static int	    flag_phy_found = -1;	/* Ethernet phy found flag */

/*
 * stop_normal_boot - except normal booting. ex) boot stop, SD start.
 */
void stop_normal_boot (void)
{
    void cyg_hal_plf_serial_silent (int mode);
    /*
     * WDOG stop PowerDown Counter (16sec)
     */
    writew (0x0000, WDOG1_BASE_ADDR + 0x0008);	// WMCR
    /*
     * console restart
     */
    cyg_hal_plf_serial_silent ( 0 );
}

/*
 * Ethernet PHY found check
 */
int is_phy_found (void)
{
    return flag_phy_found;
}

/*
 * callback from LAN PHY checked
 */
static void gpio_sw_initial (int phy_found)
{
    unsigned int reg;

    flag_phy_found = phy_found;
    if (phy_found == 0) {
	/*
	 * SW[LEFT] GPIO2_22
	 */
	writel(0x0001, IOMUXC_BASE_ADDR + 0x00D4);	// GPIO2[22]
	writel(0x01E0, IOMUXC_BASE_ADDR + 0x0468);	// enable PULL
	reg  = readl(GPIO2_BASE_ADDR + 0x4);
	reg &= ~(1 << 22);
	writel(reg, GPIO2_BASE_ADDR + 0x4);
	sample_initial_gpio_mask |= 0x2;
	/*
	 * SW[RIGHT] GPIO2_23
	 */
	writel(0x0001, IOMUXC_BASE_ADDR + 0x00D8);	// GPIO2[23]
	writel(0x01C0, IOMUXC_BASE_ADDR + 0x046C);	// enable PULL
	reg  = readl(GPIO2_BASE_ADDR + 0x4);
	reg &= ~(1 << 23);
	writel(reg, GPIO2_BASE_ADDR + 0x4);
	sample_initial_gpio_mask |= 0x1;
    } else {
	/*
	 * don't have LEFT/RIGHT-SW
	 */
	sample_initial_gpio_mask = 0;
    }
    /*
     * try 3times
     */
    sample_initial_gpio = readl(GPIO2_BASE_ADDR + 0x0);
    sample_initial_gpio = readl(GPIO2_BASE_ADDR + 0x0);
    sample_initial_gpio = readl(GPIO2_BASE_ADDR + 0x0);
}

/*
 * pickup SW[LEFT/RIGHT] current status
 *  b31  b30   ...... b01  b00
 *		      LEFT RIGHT
 *		       1:ON 0:OFF
 */
static unsigned int gpio_sw_rl (unsigned int sw)
{
    unsigned int rc = 0x00000000;

    /*
     * LEFT
     */
    if ((sw & (1 << 22)) == 0x00000000) {
	rc |= 0x00000002;
    }
    /*
     * RIGHT
     */
    if ((sw & (1 << 23)) == 0x00000000) {
	rc |= 0x00000001;
    }
    rc = rc & sample_initial_gpio_mask;
    return rc;
}

/*
 * pickup SW[LEFT/RIGHT] current status
 */
unsigned int get_gpio_sw (void)
{
    return gpio_sw_rl ( readl(GPIO2_BASE_ADDR + 0x0) );
}

/*
 * pickup SW[LEFT/RIGHT] init status
 */
unsigned int get_gpio_sw_init (void)
{
    return gpio_sw_rl ( sample_initial_gpio );
}

/*
 * select BATT Volt measure
 */
void select_batt_volt (int onoff)
{
    unsigned int reg;
    /*
     * REFON
     */
    reg = readl(GPIO3_BASE_ADDR + 0x0);
    if (onoff == 0) {
	reg &= ~0x00000040;
    } else {
	reg |= 0x00000040;
    }
    writel(reg, GPIO3_BASE_ADDR + 0x0);
}

volatile static int dcin_startup;
void plf_hardware_init(void)
{
    unsigned int reg;

    /*
     * WDOG_B/WDI (GPIO1_4) Watchdog MX51 -> PMIC
     */
    writel(0x0002, IOMUXC_BASE_ADDR + 0x03D8);	// GPIO1[04]
    writel(0x0004, IOMUXC_BASE_ADDR + 0x0804);	// disable PULL

    /*
     * BAT_AC = AC (DI1_D1_CS/GPIO3[4]) GPIO Output=L
     *	IOMUXC_GPIO3_IPP_IND_G_IN_4_SELECT_INPUT(DI1_D1_CS)
     */
    writel(0x0004, IOMUXC_BASE_ADDR + 0x02B8);	// GPIO3[4]
    writel(0x0004, IOMUXC_BASE_ADDR + 0x06B8);	// disable PULL/KEEPER
    reg  = readl(GPIO3_BASE_ADDR + 0x0);
    reg &= ~0x00000010;
    writel(reg, GPIO3_BASE_ADDR + 0x0);
    reg  = readl(GPIO3_BASE_ADDR + 0x4);
    reg |= 0x00000010;
    writel(reg, GPIO3_BASE_ADDR + 0x4);
    writel(0x0001, IOMUXC_BASE_ADDR + 0x0984);	//IOMUXC_GPIO3_IPP_IND_G_IN_4_SELECT_INPUT

    /*
     * LOW_CHG = OFF (DISPB2_SER_DIN/GPIO3[5]) GPIO Output=L
     *	IOMUXC_GPIO3_IPP_IND_G_IN_5_SELECT_INPUT(DISPB2_SER_DIN)
     */
    writel(0x0004, IOMUXC_BASE_ADDR + 0x02BC);	// GPIO3[5]
    writel(0x0004, IOMUXC_BASE_ADDR + 0x06BC);	// disable PULL/KEEPER
    reg  = readl(GPIO3_BASE_ADDR + 0x0);
    reg &= ~0x00000020;
    writel(reg, GPIO3_BASE_ADDR + 0x0);
    reg  = readl(GPIO3_BASE_ADDR + 0x4);
    reg |= 0x00000020;
    writel(reg, GPIO3_BASE_ADDR + 0x4);
    writel(0x0001, IOMUXC_BASE_ADDR + 0x0988);	//IOMUXC_GPIO3_IPP_IND_G_IN_5_SELECT_INPUT

    /*
     * REFON = OFF (DISPB2_SER_DIO/GPIO3[6]) GPIO Output=L
     *	IOMUXC_GPIO3_IPP_IND_G_IN_6_SELECT_INPUT(DISPB2_SER_DIO)
     */
    writel(0x0004, IOMUXC_BASE_ADDR + 0x02C0);	// GPIO3[6]
    writel(0x0004, IOMUXC_BASE_ADDR + 0x06C0);	// disable PULL/KEEPER
    reg  = readl(GPIO3_BASE_ADDR + 0x0);
    reg &= ~0x00000040;
    writel(reg, GPIO3_BASE_ADDR + 0x0);
    reg  = readl(GPIO3_BASE_ADDR + 0x4);
    reg |= 0x00000040;
    writel(reg, GPIO3_BASE_ADDR + 0x4);
    writel(0x0001, IOMUXC_BASE_ADDR + 0x098C);	//IOMUXC_GPIO3_IPP_IND_G_IN_6_SELECT_INPUT

    /*
     * CHG_STAT2 (NANDF_D10/GPIO3[30]) GPIO Input
     */
    writel(0x0003, IOMUXC_BASE_ADDR + 0x0168);	// GPIO3[30]
    writel(0x0024, IOMUXC_BASE_ADDR + 0x0550);	// disable PULL/KEEPER
    reg  = readl(GPIO3_BASE_ADDR + 0x4);
    reg &= ~0x40000000;
    writel(reg, GPIO3_BASE_ADDR + 0x4);

    /*
     * CHG_STAT1 (NANDF_D12/GPIO3[28]) GPIO Input
     */
    writel(0x0003, IOMUXC_BASE_ADDR + 0x0160);	// GPIO3[28]
    writel(0x0024, IOMUXC_BASE_ADDR + 0x0548);	// disable PULL/KEEPER
    reg  = readl(GPIO3_BASE_ADDR + 0x4);
    reg &= ~0x10000000;
    writel(reg, GPIO3_BASE_ADDR + 0x4);

    /*
     * CHG_PG (NANDF_D13/GPIO3[27]) GPIO Input
     */
    writel(0x0003, IOMUXC_BASE_ADDR + 0x015C);	// GPIO3[27]
    writel(0x0024, IOMUXC_BASE_ADDR + 0x0544);	// disable PULL/KEEPER
    reg  = readl(GPIO3_BASE_ADDR + 0x4);
    reg &= ~0x08000000;
    writel(reg, GPIO3_BASE_ADDR + 0x4);

    /*
     * CHG_CTRL (DISPB2_SER_CLK/GPIO3[7]) GPIO Output=Low(alway charge)
     *	GPIO3_IPP_IND_G_IN_7_SELECT_INPUT(DISPB2_SER_CLK)
     */
    writel(0x0004, IOMUXC_BASE_ADDR + 0x02C4); // GPIO3[7]
    writel(0x0004, IOMUXC_BASE_ADDR + 0x06C4); // disable PULL/KEEPER
    reg  = readl(GPIO3_BASE_ADDR + 0x0);
    reg &= ~(1 << 7);	/* charge */
    writel(reg, GPIO3_BASE_ADDR + 0x0);
    reg  = readl(GPIO3_BASE_ADDR + 0x4);
    reg |= (1 << 7);
    writel(reg, GPIO3_BASE_ADDR + 0x4);
    writel(0x0001, IOMUXC_BASE_ADDR + 0x0990); // GPIO3_IPP_IND_G_IN_7_SELECT_INPUT

    /*
     * USB PHY RESET (EIM_D21/GPIO2[5]) GPIO Output=L
     */
    writel(0x0001, IOMUXC_BASE_ADDR + 0x0070);	// GPIO2[5]
    writel(0x0004, IOMUXC_BASE_ADDR + 0x0404);	// disable PULL/KEEPER
    reg  = readl(GPIO2_BASE_ADDR + 0x0);
    reg &= ~0x00000020;
    writel(reg, GPIO2_BASE_ADDR + 0x0);
    reg  = readl(GPIO2_BASE_ADDR + 0x4);
    reg |= 0x00000020;
    writel(reg, GPIO2_BASE_ADDR + 0x4);

    /*
     * USB PHY CLK ENABLE (EIM_D17/GPIO2[1]) GPIO Output=L
     */
    writel(0x0001, IOMUXC_BASE_ADDR + 0x0060);	// GPIO2[1]
    writel(0x0004, IOMUXC_BASE_ADDR + 0x03F4);	// disable PULL/KEEPER
    reg  = readl(GPIO2_BASE_ADDR + 0x0);
    reg &= ~0x00000002;
    writel(reg, GPIO2_BASE_ADDR + 0x0);
    reg  = readl(GPIO2_BASE_ADDR + 0x4);
    reg |= 0x00000002;
    writel(reg, GPIO2_BASE_ADDR + 0x4);

    /*
     * 26MHz OSC ENABLE (DI1_PIN12/GPIO3[1]) GPIO Output=H
     */
    writel(0x0001, IOMUXC_BASE_ADDR + 0x02AC);	// GPIO3[1]
    writel(0x0004, IOMUXC_BASE_ADDR + 0x06AC);	// disable PULL/KEEPER
    reg  = readl(GPIO3_BASE_ADDR + 0x0);
    reg |= 0x00000002;
    writel(reg, GPIO3_BASE_ADDR + 0x0);
    reg  = readl(GPIO3_BASE_ADDR + 0x4);
    reg |= 0x00000002;
    writel(reg, GPIO3_BASE_ADDR + 0x4);

    /*
     * USB PHY RESET (EIM_D21/GPIO2[5]) GPIO Output=H
     */
    reg  = readl(GPIO2_BASE_ADDR + 0x0);
    reg |= 0x00000020;
    writel(reg, GPIO2_BASE_ADDR + 0x0);

    /*
     * COVER-SW (CSI2_D19/GPIO4[12]) GPIO Input
     */
    writel(0x0003, IOMUXC_BASE_ADDR + 0x01E8);	// GPIO4[12]
    writel(0x0100, IOMUXC_BASE_ADDR + 0x05D8);	// HYS, disable PULL/KEEPER
    reg  = readl(GPIO4_BASE_ADDR + 0x4);
    reg &= ~(1 << 12);
    writel(reg, GPIO4_BASE_ADDR + 0x4);

    /*
     * POWER-SW (EIM_A27/GPIO2[21]) GPIO Input
     */
    writel(0x0001, IOMUXC_BASE_ADDR + 0x00C8);	// GPIO2[21]
    writel(0x0100, IOMUXC_BASE_ADDR + 0x045C);	// HYS, disable PULL/KEEPER
    reg  = readl(GPIO2_BASE_ADDR + 0x4);
    reg &= ~(1 << 21);
    writel(reg, GPIO2_BASE_ADDR + 0x4);

#if 1 // 2009/08/17
    spi_nor_init = (imx_spi_init_func_t *)imx_spi_init_v2_3;
    spi_nor_xfer = (imx_spi_xfer_func_t *)imx_spi_xfer_v2_3;

    spi_pmic_init = (imx_spi_init_func_t *)imx_spi_init_v2_3;
    spi_pmic_xfer = (imx_spi_xfer_func_t *)imx_spi_xfer_v2_3;
    spi_pmic_init(&imx_spi_pmic);

    babbage_power_init();
#endif

    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) < 0x2) {
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
    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) >= 0x2) {
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
#if 1
	// enable GPIO1_8 for CLKO2
	writel(0x00000004, 0x73fa83E8);
#else
	// enable GPIO1_9 for CLKO and GPIO1_8 for CLKO2
	writel(0x00000004, 0x73fa83E8);
	writel(0x00000004, 0x73fa83EC);
#endif
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

#if 0 // 2009/08/17
    spi_nor_init = (imx_spi_init_func_t *)imx_spi_init_v2_3;
    spi_nor_xfer = (imx_spi_xfer_func_t *)imx_spi_xfer_v2_3;

    spi_pmic_init = (imx_spi_init_func_t *)imx_spi_init_v2_3;
    spi_pmic_xfer = (imx_spi_xfer_func_t *)imx_spi_xfer_v2_3;
    spi_pmic_init(&imx_spi_pmic);

    /*
     * Force Switcher Mode Control NORMAL=PWM STANDBY=PWM
     *  PMIC default 2.0=PWM 2.0a=PWMPS -> alway PWM.
     */
    pmic_reg(28, 0x00211445, 1);
    pmic_reg(29, 0x505, 1);

    babbage_power_init();
#endif

    /*
     * BAT_AC2 = AC (CLKO/GPIO1[9]) GPIO Output=L(DCin OK)
     */
    writel(0x0000, IOMUXC_BASE_ADDR + 0x03EC);	// GPIO1[9]
    writel(0x0004, IOMUXC_BASE_ADDR + 0x0818);	// disable PULL/KEEPER
    reg  = readl(GPIO1_BASE_ADDR + 0x0);
    reg &= ~0x00000200;	// DCinput Power
    writel(reg, GPIO1_BASE_ADDR + 0x0);
    reg  = readl(GPIO1_BASE_ADDR + 0x4);
    reg |= 0x00000200;
    writel(reg, GPIO1_BASE_ADDR + 0x4);

    fec_phy_callback = gpio_sw_initial;
}

/*
 * error_loop - display LED error code, infini loop
 */
void error_loop (int code)
{
#define LED_ERROR_PMIC_REG  53	/* (LEDG) */
    int  i, abort;
    char tmp [8];

    diag_printf ("error_loop: code:%x   - any keyinput stop display.\n", code);
    /*
     * LED(pmic) display
     */
    while ( 1 ) {
	/*
	 * error code frash
	 */
	abort = 0;
	for ( i = 0 ; i < code ; i++ ) {
	    pmic_reg (LED_ERROR_PMIC_REG, 0x300000, 1);
	    if (_rb_gets (tmp, sizeof(tmp), 1) != _GETS_TIMEOUT) {
		abort = 1;
		break;
	    }
	    hal_delay_us(500*1000);
	    pmic_reg (LED_ERROR_PMIC_REG, 0x000000, 1);
	    if (_rb_gets (tmp, sizeof(tmp), 1) != _GETS_TIMEOUT) {
		abort = 1;
		break;
	    }
	    hal_delay_us(500*1000);
	}
	if (abort == 1) {
	    break;
	}
	if (_rb_gets (tmp, sizeof(tmp), 1) != _GETS_TIMEOUT) {
	    break;
	}
	hal_delay_us(2000*1000);
	if (_rb_gets (tmp, sizeof(tmp), 1) != _GETS_TIMEOUT) {
	    break;
	}
    }
    pmic_reg (LED_ERROR_PMIC_REG, 0x000000, 1);
}

void mxc_mmc_init(unsigned int base_address)
{
    switch(base_address) {
    case MMC_SDHC1_BASE_ADDR:
    case MMC_SDHC2_BASE_ADDR:
	if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) >= 0x2) {
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

	    /* SD2 CMD, SION bit */
	    writel(0x10, IOMUXC_BASE_ADDR + 0x3B4);
	   /* Configure SW PAD */
#ifdef BOARD_SPIDER_QA0
	    /*
	     * QA0 SD2 3.3V
	     */
	    writel(0xd5, IOMUXC_BASE_ADDR + 0x7BC);	/* SD2 CMD */
	    writel(0xd5, IOMUXC_BASE_ADDR + 0x7C0);	/* SD2 CLK */
	    writel(0xd5, IOMUXC_BASE_ADDR + 0x7C4);	/* SD2 DAT0 */
	    writel(0xd5, IOMUXC_BASE_ADDR + 0x7C8);	/* SD2 DAT1 */
	    writel(0xd5, IOMUXC_BASE_ADDR + 0x7CC);	/* SD2 DAT2 */
	    writel(0xd5, IOMUXC_BASE_ADDR + 0x7D0);	/* SD2 DAT3 */
#else
	    /*
	     * QA0 SD2 1.8V
	     */
	    writel(0x20d5, IOMUXC_BASE_ADDR + 0x7BC);	/* SD2 CMD */
	    writel(0x20d5, IOMUXC_BASE_ADDR + 0x7C0);	/* SD2 CLK */
	    writel(0x20d5, IOMUXC_BASE_ADDR + 0x7C4);	/* SD2 DAT0 */
	    writel(0x20d5, IOMUXC_BASE_ADDR + 0x7C8);	/* SD2 DAT1 */
	    writel(0x20d5, IOMUXC_BASE_ADDR + 0x7CC);	/* SD2 DAT2 */
	    writel(0x20d5, IOMUXC_BASE_ADDR + 0x7D0);	/* SD2 DAT3 */
#endif /* BOARD_SPIDER_QA0 */
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

    /* power up the system first */
    pmic_reg(34, 0x00200000, 1);

#if 1 // 2009/08/17
    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) <= 0x2) {
        /* Set core voltage to 1.1V */
        val = pmic_reg(24, 0, 0);
        val = val & (~0x1F) | 0x14;
        pmic_reg(24, val, 1);

        /* Setup VCC (SW2) to 1.25 */
        val = pmic_reg(25, 0, 0);
        val = val & (~0x1F) | 0x1A;
        pmic_reg(25, val, 1);

        /* Setup 1V2_DIG1 (SW3) to 1.25 */
        val = pmic_reg(26, 0, 0);
        val = val & (~0x1F) | 0x1A;
        pmic_reg(26, val, 1);
        hal_delay_us(50);
        /* Raise the core frequency to 800MHz */
        writel(0x0, CCM_BASE_ADDR + CLKCTL_CACRR);
    } else {
        /* TO 3.0 */
        /* Setup VCC (SW2) to 1.225 */
        val = pmic_reg(25, 0, 0);
        val = val & (~0x1F) | 0x19;
        pmic_reg(25, val, 1);

        /* Setup 1V2_DIG1 (SW3) to 1.2 */
        val = pmic_reg(26, 0, 0);
        val = val & (~0x1F) | 0x18;
        pmic_reg(26, val, 1);
    }
    if (((pmic_reg(7, 0, 0) & 0x1F) < REV_ATLAS_LITE_2_0) || (((pmic_reg(7, 0, 0) >> 9) & 0x3) == 0)) {
        /* Set switchers in PWM mode for Atlas 2.0 and lower */
        /* Setup the switcher mode for SW1 & SW2*/
        val = pmic_reg(28, 0, 0);
        val = val & (~0x3C0F) | 0x1405;
        pmic_reg(28, val, 1);

        /* Setup the switcher mode for SW3 & SW4 */
        val = pmic_reg(29, 0, 0);
        val = val & (~0xF0F) | 0x505;
        pmic_reg(29, val, 1);
    } else {
        /* Set switchers in Auto in NORMAL mode & STANDBY mode for Atlas 2.0a */
        /* Setup the switcher mode for SW1 & SW2*/
        val = pmic_reg(28, 0, 0);
        val = val & (~0x3C0F) | 0x2008;
        pmic_reg(28, val, 1);

        /* Setup the switcher mode for SW3 & SW4 */
        val = pmic_reg(29, 0, 0);
        val = val & (~0xF0F) | 0x808;
        pmic_reg(29, val, 1);
    }
#else
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
#endif

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

    /*
     * LAN PHY RESET (ON)
     */
    reg = readl(GPIO2_BASE_ADDR + 0x0);
    reg &= ~0x4000;  // Lower reset line
    writel(reg, GPIO2_BASE_ADDR + 0x0);
    reg = readl(GPIO2_BASE_ADDR + 0x4);
    reg |= 0x4000;  // configure GPIO lines as output
    writel(reg, GPIO2_BASE_ADDR + 0x4);
    /* Reset the ethernet controller over GPIO */
    writel(0x1, IOMUXC_BASE_ADDR + 0x0AC);

#if 1 // 2009/09/06
    /* Enable VDIG standby */
    val = 0x49608;
    pmic_reg(32, val, 1);
#endif

#if 1 // 2009/09/03, 09/06
    /* Enable VGEN3, VVIDEO(+standby) regulators */
    val = 0x003209;
#else
    /* Enable VGEN3, VCAM, VAUDIO, VVIDEO, VSD regulators */
    val = 0x49249;
#endif
    pmic_reg(33, val, 1);

    hal_delay_us(500);

    /*
     * RTC calibration
     *  enable/-31ppm
     */
    val  = pmic_reg(20, 0, 0);
    val &= 0x01FFFF;
    val |= (1 << 22) | (0x1F << 17);
    pmic_reg(20, val, 1);

    /*
     * LAN PHY RESET (OFF)
     */
    reg = readl(GPIO2_BASE_ADDR + 0x0);
    reg |= 0x4000;
    writel(reg, GPIO2_BASE_ADDR + 0x0);

    /* Setup the FEC after enabling the regulators */
    mxc_fec_setup();

    /* NAND */
    mxc_nand_setup();
}

void io_cfg_spi(struct imx_spi_dev *dev)
{
    switch (dev->base) {
    case CSPI1_BASE_ADDR:
	if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) >= 0x2) {
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
