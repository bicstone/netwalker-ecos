//==========================================================================
//
//      soc_misc.c
//
//      HAL misc board support code
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
// 2009/07/27 : from redboot_200925
//              serial silent mode on Flash boot.
//              time measure support on EPIT2.
//
//========================================================================*/

#include <redboot.h>
#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_misc.h>           // Size constants
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>          // Cache control
#include <cyg/hal/hal_soc.h>            // Hardware definitions
#include <cyg/hal/hal_mm.h>             // MMap table definitions
#include <cyg/io/mxc_mmc.h>

#include <cyg/infra/diag.h>             // diag_printf
#ifdef CYGPKG_DEVS_ETH_FEC
#include <cyg/io/fec.h>
#endif

// Most initialization has already been done before we get here.
// All we do here is set up the interrupt environment.
// FIXME: some of the stuff in hal_platform_setup could be moved here.

externC void plf_hardware_init(void);
int _mxc_fis;

/*
 * System_rev will have the following format
 * 31-12 = part # (0x31, 0x32, 0x27, 0x91131, 0x91321, etc)
 * 11-8 = unused
 * 7-4 = major (1.y)
 * 3-0 = minor (x.0)
 */
unsigned int system_rev = CHIP_REV_1_0;
static int find_correct_chip;
extern char HAL_PLATFORM_EXTRA[20];
static int _reset_reason;

struct soc_sbmr {
	unsigned int bt_mem_ctl:2,
        bt_bus_width:1,
        bt_page_size:2,
        rsv2:1,
        bt_spare_size:1,
        bt_mem_type:2,
        rsv1:1,
        bt_ecc_sel:1,
        bt_usb_src_0:1,
        bt_eeprom_cfg:1,
        dir_bt_dis:1,
        bmod:2,
        bt_weim_muxed:2,
        bt_spare:1,
        bt_sdmmc_src:2,
        bt_chih_freq_sel:2,
        bt_i2c_src:2,
        bt_uart_src:2,
        bt_cspi_src:2,
        rsv0:3;
} __attribute__ ((packed));
struct soc_sbmr *soc_sbmr = (struct soc_sbmr *) (SRC_BASE_ADDR + 0x4);
/*
 * This functions reads the IIM module and returns the system revision number.
 * It returns the IIM silicon revision reg value if valid product rev is found.
 . Otherwise, it returns -1.
 */
static int read_system_rev(void)
{
    int val;
    int *rom_id_address;

    rom_id_address = ROM_BASE_ADDRESS_VIRT + ROM_SI_REV_OFFSET;

    val = readl(IIM_BASE_ADDR + IIM_PREV_OFF);

    system_rev = 0x51 << PART_NUMBER_OFFSET; /* For MX51 Platform*/

    /* Now trying to retrieve the silicon rev from IIM's SREV register */
    return *rom_id_address;
}

#ifdef MXCFLASH_SELECT_NAND
extern nfc_setup_func_t *nfc_setup;
#endif
unsigned int mxc_nfc_soc_setup(unsigned int pg_sz, unsigned int io_sz,
                                      unsigned int is_mlc, unsigned int num_of_chips);

#ifdef CYGPKG_DEVS_ETH_FEC
extern int read_mac_addr_from_fuse(unsigned char* data);
extern mxc_fec_read_mac_from_fuse *get_mac_addr;
#endif

#ifdef MXCFLASH_SELECT_MMC
extern mxc_mmc_check_sdhc_boot_slot *check_sdhc_slot;
#endif

int mxc_check_sdhc_boot_slot(unsigned int port, unsigned int *sdhc_addr);

#ifdef MX51_ERDOS
#undef TIME_MEASURE	//GGGGGG
#ifdef TIME_MEASURE
/*
 * tick 0x8000 count/1sec
 */
unsigned long tickGet (void)
{
    return readl(EPIT2_BASE_ADDR + 0x10);
}
#endif /* TIME_MEASURE */
#endif /* MX51_ERDOS */

void hal_hardware_init(void)
{
    int ver = read_system_rev();
    unsigned int i;
    unsigned int *fis_addr = (unsigned int *)IRAM_BASE_ADDR;

     _reset_reason = readl(SRC_BASE_ADDR + 0x8);
    switch (*fis_addr) {
    case FROM_MMC_FLASH:
        _mxc_fis = FROM_MMC_FLASH;
        break;
    case FROM_NAND_FLASH:
        _mxc_fis = FROM_NAND_FLASH;
        break;
    case FROM_SPI_NOR_FLASH:
        _mxc_fis = FROM_SPI_NOR_FLASH;
        break;
    default:
        if (soc_sbmr->bt_mem_ctl == 0x3) {
            if (soc_sbmr->bt_mem_type == 0) {
                _mxc_fis = MMC_FLASH_BOOT;
                *fis_addr = FROM_MMC_FLASH;
            } else if (soc_sbmr->bt_mem_type == 3) {
                _mxc_fis = SPI_NOR_FLASH_BOOT;
                *fis_addr = FROM_SPI_NOR_FLASH;
            }
        } else if (soc_sbmr->bt_mem_ctl == 1) {
            _mxc_fis = NAND_FLASH_BOOT;
            *fis_addr = FROM_NAND_FLASH;
        }
    }

#ifdef MX51_ERDOS
    if ((_mxc_fis == FROM_SPI_NOR_FLASH) || (_mxc_fis == SPI_NOR_FLASH_BOOT)) {
        /*
         * serial silent mode
         */
        extern void cyg_hal_plf_serial_silent (int mode);
        cyg_hal_plf_serial_silent (1);
    }
#endif /* MX51_ERDOS */

    find_correct_chip = ver;

    if (ver != CHIP_VERSION_NONE) {
        /* Valid product revision found. Check actual silicon rev from the ROM code. */
        if (ver == 0x1) {
            HAL_PLATFORM_EXTRA[5] = '1';
            HAL_PLATFORM_EXTRA[7] = '0';
            system_rev |= 1 << MAJOR_NUMBER_OFFSET; /*Major Number*/
            system_rev |= 0 << MINOR_NUMBER_OFFSET; /*Minor Number*/
        } else if (ver == 0x2) {
            HAL_PLATFORM_EXTRA[5] = '1';
            HAL_PLATFORM_EXTRA[7] = '1';
            system_rev |= 1 << MAJOR_NUMBER_OFFSET; /*Major Number*/
            system_rev |= 1 << MINOR_NUMBER_OFFSET; /*Minor Number*/
        } else if (ver == 0x10) {
            HAL_PLATFORM_EXTRA[5] = '2';
            HAL_PLATFORM_EXTRA[7] = '0';
            system_rev |= 2 << MAJOR_NUMBER_OFFSET; /*Major Number*/
            system_rev |= 0 << MINOR_NUMBER_OFFSET; /*Minor Number*/
        } else if (ver == 0x20) {
            HAL_PLATFORM_EXTRA[5] = '3';
            HAL_PLATFORM_EXTRA[7] = '0';
            system_rev |= 3 << MAJOR_NUMBER_OFFSET; /*Major Number*/
            system_rev |= 0 << MINOR_NUMBER_OFFSET; /*Minor Number*/
        } else {
            HAL_PLATFORM_EXTRA[5] = 'x';
            HAL_PLATFORM_EXTRA[7] = 'x';
            system_rev |= 3 << MAJOR_NUMBER_OFFSET; /*Major Number*/
            system_rev |= 0 << MINOR_NUMBER_OFFSET; /*Minor Number*/
            find_correct_chip = CHIP_VERSION_UNKNOWN;
        }

    }
    // Enable caches
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_ENABLE();

#ifdef MX51_ERDOS
#ifdef TIME_MEASURE
    /* EPIT2 free counter  32768 = 1sec */
    writel(0x00010000, EPIT2_BASE_ADDR + EPITCR);
    while ((readl(EPIT2_BASE_ADDR + EPITCR) & 0x10000) != 0) {
    }
    writel(0x030E0002, EPIT2_BASE_ADDR + EPITCR);
    writel(0x030E0003, EPIT2_BASE_ADDR + EPITCR);
#endif /* TIME_MEASURE */
#endif /* MX51_ERDOS */

    // enable EPIT and start it with 32KHz input clock
    writel(0x00010000, EPIT_BASE_ADDR + EPITCR);

    // make sure reset is complete
    while ((readl(EPIT_BASE_ADDR + EPITCR) & 0x10000) != 0) {
    }

    writel(0x030E0002, EPIT_BASE_ADDR + EPITCR);
    writel(0x030E0003, EPIT_BASE_ADDR + EPITCR);

    writel(0, EPIT_BASE_ADDR + EPITCMPR);  // always compare with 0

    if ((readw(WDOG_BASE_ADDR) & 4) != 0) {
        // increase the WDOG timeout value to the max
        writew(readw(WDOG_BASE_ADDR) | 0xFF00, WDOG_BASE_ADDR);
    }

    // Perform any platform specific initializations
    plf_hardware_init();

    // Set up eCos/ROM interfaces
    hal_if_init();

    // initial NAND setup
    writel(0xFFFF0000, UNLOCK_BLK_ADD0_REG);
    writel(0xFFFF0000, UNLOCK_BLK_ADD1_REG);
    writel(0xFFFF0000, UNLOCK_BLK_ADD2_REG);
    writel(0xFFFF0000, UNLOCK_BLK_ADD3_REG);
    writel(0xFFFF0000, UNLOCK_BLK_ADD4_REG);
    writel(0xFFFF0000, UNLOCK_BLK_ADD5_REG);
    writel(0xFFFF0000, UNLOCK_BLK_ADD6_REG);
    writel(0xFFFF0000, UNLOCK_BLK_ADD7_REG);

    // unlock all the CS's
    for (i = 0; i < 8; i++) {
        writel(0x84 | (i << 3), NFC_WR_PROT_REG);
    }
    writel(0, NFC_IPC_REG);
#ifdef MXCFLASH_SELECT_NAND
    nfc_setup = (nfc_setup_func_t*)mxc_nfc_soc_setup;
#endif

#ifdef CYGPKG_DEVS_ETH_FEC
    get_mac_addr = (mxc_fec_read_mac_from_fuse *)read_mac_addr_from_fuse;
#endif

#ifdef MXCFLASH_SELECT_MMC
    check_sdhc_slot = (mxc_mmc_check_sdhc_boot_slot *)mxc_check_sdhc_boot_slot;
#endif
}

// -------------------------------------------------------------------------
void hal_clock_initialize(cyg_uint32 period)
{
}

// This routine is called during a clock interrupt.

// Define this if you want to ensure that the clock is perfect (i.e. does
// not drift).  One reason to leave it turned off is that it costs some
// us per system clock interrupt for this maintenance.
#undef COMPENSATE_FOR_CLOCK_DRIFT

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)

// Note: The "contract" for this function is that the value is the number
// of hardware clocks that have happened since the last interrupt (i.e.
// when it was reset).  This value is used to measure interrupt latencies.
// However, since the hardware counter runs freely, this routine computes
// the difference between the current clock period and the number of hardware
// ticks left before the next timer interrupt.
void hal_clock_read(cyg_uint32 *pvalue)
{
}

// This is to cope with the test read used by tm_basic with
// CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY defined; we read the count ASAP
// in the ISR, *before* resetting the clock.  Which returns 1tick +
// latency if we just use plain hal_clock_read().
void hal_clock_latency(cyg_uint32 *pvalue)
{
}

unsigned int hal_timer_count(void)
{
    return (0xFFFFFFFF - readl(EPIT_BASE_ADDR + EPITCNR));
}

#define WDT_MAGIC_1             0x5555
#define WDT_MAGIC_2             0xAAAA
#define MXC_WDT_WSR             0x2

unsigned int i2c_base_addr[] = {
    I2C_BASE_ADDR,
    I2C2_BASE_ADDR,
};
unsigned int i2c_num = 2;

//
// Delay for some number of micro-seconds
//
void hal_delay_us(unsigned int usecs)
{
    /*
     * This causes overflow.
     * unsigned int delayCount = (usecs * 32768) / 1000000;
     * So use the following one instead
     */
    unsigned int delayCount = (usecs * 512) / 15625;

    if (delayCount == 0) {
        return;
    }

    // issue the service sequence instructions
    if ((readw(WDOG_BASE_ADDR) & 4) != 0) {
        writew(WDT_MAGIC_1, WDOG_BASE_ADDR + MXC_WDT_WSR);
        writew(WDT_MAGIC_2, WDOG_BASE_ADDR + MXC_WDT_WSR);
    }

    writel(0x01, EPIT_BASE_ADDR + EPITSR); // clear the compare status bit

    writel(delayCount, EPIT_BASE_ADDR + EPITLR);

    while ((0x1 & readl(EPIT_BASE_ADDR + EPITSR)) == 0); // return until compare bit is set
}

// -------------------------------------------------------------------------

// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.
int hal_IRQ_handler(void)
{
#ifdef HAL_EXTENDED_IRQ_HANDLER
    cyg_uint32 index;

    // Use platform specific IRQ handler, if defined
    // Note: this macro should do a 'return' with the appropriate
    // interrupt number if such an extended interrupt exists.  The
    // assumption is that the line after the macro starts 'normal' processing.
    HAL_EXTENDED_IRQ_HANDLER(index);
#endif

    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
//    diag_printf("6hal_interrupt_mask(vector=%d) \n", vector);
#ifdef HAL_EXTENDED_INTERRUPT_MASK
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_MASK(vector);
#endif
}

void hal_interrupt_unmask(int vector)
{
//    diag_printf("7hal_interrupt_unmask(vector=%d) \n", vector);

#ifdef HAL_EXTENDED_INTERRUPT_UNMASK
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_UNMASK(vector);
#endif
}

void hal_interrupt_acknowledge(int vector)
{

//    diag_printf("8hal_interrupt_acknowledge(vector=%d) \n", vector);
#ifdef HAL_EXTENDED_INTERRUPT_UNMASK
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_ACKNOWLEDGE(vector);
#endif
}

void hal_interrupt_configure(int vector, int level, int up)
{

#ifdef HAL_EXTENDED_INTERRUPT_CONFIGURE
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_CONFIGURE(vector, level, up);
#endif
}

void hal_interrupt_set_level(int vector, int level)
{

#ifdef HAL_EXTENDED_INTERRUPT_SET_LEVEL
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_SET_LEVEL(vector, level);
#endif

    // Interrupt priorities are not configurable.
}

unsigned int mxc_nfc_soc_setup(unsigned int pg_sz, unsigned int io_sz, unsigned int is_mlc, unsigned int num_of_chips)
{
    unsigned int nfc_config_reg3, src_scr_reg;

    if (pg_sz == 2048 && io_sz == 8) {
        writel(0x70202179, NFC_FLASH_CONFIG2_REG);
        nfc_config_reg3 = 0x00160608 | ((num_of_chips - 1) << 12);
        if (num_of_chips > 1)
            nfc_config_reg3 |= 0x1;
        writel(nfc_config_reg3, NFC_FLASH_CONFIG3_REG);
    } else if (pg_sz == 4096 && io_sz == 8) {
#ifdef MX51_ERDOS
        extern int nandflash_sparesize (void);
        int spare = nandflash_sparesize ();		// spare size
        int reg   = 0x7000213A;
        if (spare == 218) {
            reg |= ((spare / 2) << 16) | 0x00000040;	// 8bit ecc
        } else {
            reg |= ((spare / 2) << 16);			// 4bit ecc
        }
        writel(reg, NFC_FLASH_CONFIG2_REG);
#else
        // This only works for 4KB with 218 spare area size
        writel(0x706D217A, NFC_FLASH_CONFIG2_REG); // default is 0x706D273A
#endif /* MX51_ERDOS */
        nfc_config_reg3 = 0x00120608 | ((num_of_chips - 1) << 12);
        if (num_of_chips > 1)
            nfc_config_reg3 |= 0x1;
        writel(nfc_config_reg3, NFC_FLASH_CONFIG3_REG);
    } else {
        diag_printf("not supported nand flash: pg_sz=%d, io_sz=%d\n",
                    pg_sz, io_sz);
        while (1) {
        }
    }

    if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) <= 0x2) {
        /* This issue is fixed in MX51 TO 3.0 */
    /* Workaround to disable WARM RESET when booting from interleaved NAND devices */
    if ((num_of_chips > 1) && (soc_sbmr->bt_mem_ctl == 1)) {
        src_scr_reg = readl(SRC_BASE_ADDR);
        src_scr_reg &= ~0x1;
        writel(src_scr_reg, SRC_BASE_ADDR);
    }
    }

    return 0x30;
}

int mxc_check_sdhc_boot_slot(unsigned int port, unsigned int *sdhc_addr)
{
    unsigned int bt_sdmmc_src = 0;
    int ret = 1;

    if (port != READ_PORT_FROM_FUSE) {
        bt_sdmmc_src = port;
    } else {
        bt_sdmmc_src = soc_sbmr->bt_sdmmc_src;
    }

    switch(bt_sdmmc_src){
    case 0x0:
        *sdhc_addr = MMC_SDHC1_BASE_ADDR;
        break;
    case 0x1:
        *sdhc_addr = MMC_SDHC2_BASE_ADDR;
        break;
    case 0x2:
        *sdhc_addr = MMC_SDHC3_BASE_ADDR;
        break;
    case 0x3:
        *sdhc_addr = MMC_SDHC4_BASE_ADDR;
        break;
    default:
        ret = 0;
    }

    if ((ret == 1) && (port == READ_PORT_FROM_FUSE)) {
        diag_printf("Booting from SDHC%d\n", bt_sdmmc_src);
    } else if (ret == 0) {
        diag_printf("Invalid SD port number %d\n", port);
    }

    return ret;
}

static void show_sys_info(void)
{
    if (find_correct_chip == CHIP_VERSION_UNKNOWN) {
        diag_printf("Unrecognized chip version: 0x%x!!!\n", read_system_rev());
        diag_printf("Assuming chip version=0x%x\n", system_rev);
    } else if (find_correct_chip == CHIP_VERSION_NONE) {
        diag_printf("Unrecognized chip: 0x%x!!!\n", readl(IIM_BASE_ADDR + IIM_PREV_OFF));
    }

    diag_printf("Reset reason: ");
    switch (_reset_reason) {
    case 0x09:
        diag_printf("User reset\n");
        break;
    case 0x01:
        diag_printf("Power-on reset\n");
        break;
    case 0x10:
    case 0x11:
        diag_printf("WDOG reset\n");
        break;
    default:
        diag_printf("Unknown: 0x%x\n", _reset_reason);
    }

    if (_mxc_fis == MMC_FLASH_BOOT) {
        diag_printf("fis/fconfig from MMC\n");
    } else if (_mxc_fis == SPI_NOR_FLASH_BOOT) {
        diag_printf("fis/fconfig from SPI-NOR\n");
    } else if (_mxc_fis == NAND_FLASH_BOOT) {
        diag_printf("fis/fconfig from NAND\n");
    } else {
        diag_printf("Use \"factive [MMC|SPI|NAND]\" to choose fis/fconfig storage\n");
    }

    //diag_printf("SBMR = 0x%x\n", readl(SRC_BASE_ADDR + 0x4));
    diag_printf("Boot switch: ");
    if (soc_sbmr->bmod == 0) {
        diag_printf("INTERNAL\n");
    } else if (soc_sbmr->bmod == 3){
        diag_printf("BOOTSTRAP\n");
    } else if (soc_sbmr->bmod == 0x1 && soc_sbmr->dir_bt_dis == 0) {
            diag_printf("TEST EXEC\n");
    } else {
        diag_printf("UNKNOWN\n");
    }
    diag_printf("\t");
    if (soc_sbmr->bt_mem_ctl == 0) {
        diag_printf("WEIM: ");
        if (soc_sbmr->bt_mem_type == 0) {
            diag_printf("NOR");
        } else if (soc_sbmr->bt_mem_type == 2) {
            diag_printf("ONE NAND");
        } else {
            diag_printf("UNKNOWN");
        }
    } else if (soc_sbmr->bt_mem_ctl == 1) {
        diag_printf("NAND: ADDR CYCLES:");
        if (soc_sbmr->bt_mem_type == 0) {
            diag_printf("3: ");
        } else if (soc_sbmr->bt_mem_type == 1) {
            diag_printf("4: ");
        } else if (soc_sbmr->bt_mem_type == 2) {
            diag_printf("5: ");
        } else {
            diag_printf("UNKNOWN: ");
        }
        if (soc_sbmr->bt_ecc_sel == 0) {
            diag_printf("SLC: ");
        } else {
            diag_printf("MLC: ");
        }
        if (soc_sbmr->bt_spare_size == 0) {
            diag_printf("128B spare (4-bit ECC): ");
        } else {
            diag_printf("218B spare (8-bit ECC): ");
        }
        diag_printf("PAGE SIZE: ");
        if (soc_sbmr->bt_page_size == 0) {
            diag_printf("512: ");
        } else if (soc_sbmr->bt_page_size == 1) {
            diag_printf("2K: ");
        } else if (soc_sbmr->bt_page_size == 2) {
            diag_printf("4K: ");
        } else {
            diag_printf("UNKNOWN: ");
        }
        diag_printf("BUS WIDTH: ");
        if (soc_sbmr->bt_bus_width == 0) {
            diag_printf("8");
        } else {
            diag_printf("16");
        }
    } else if (soc_sbmr->bt_mem_ctl == 3) {
        diag_printf("EXPANSION: ");
        if (soc_sbmr->bt_mem_type == 0) {
            diag_printf("SD/MMC-%d", soc_sbmr->bt_sdmmc_src);
        } else if (soc_sbmr->bt_mem_type == 2) {
            diag_printf("I2C-NOR: ");
            if (soc_sbmr->bt_sdmmc_src == 0) {
                diag_printf("I2C-1");
            } else if (soc_sbmr->bt_sdmmc_src == 1) {
                diag_printf("I2C-2");
            } else if (soc_sbmr->bt_sdmmc_src == 2) {
                diag_printf("HS-I2C");
            } else {
                diag_printf("UNKNOWN");
            }
        } else if (soc_sbmr->bt_mem_type == 3) {
            diag_printf("SPI-NOR: ");
            if (soc_sbmr->bt_sdmmc_src == 0) {
                diag_printf("eCSPI1");
            } else if (soc_sbmr->bt_sdmmc_src == 1) {
                diag_printf("eCSPI2");
            } else if (soc_sbmr->bt_sdmmc_src == 2) {
                diag_printf("CSPI");
            } else {
                diag_printf("UNKNOWN");
            }
        } else {
            diag_printf("UNKNOWN");
        }
    } else {
        diag_printf("UNKNOWN");
    }
    diag_printf("\n");
}

RedBoot_init(show_sys_info, RedBoot_INIT_LAST);
