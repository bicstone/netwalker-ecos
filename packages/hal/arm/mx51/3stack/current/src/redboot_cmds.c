//==========================================================================
//
//      redboot_cmds.c
//
//      Board [platform] specific RedBoot commands
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
//==========================================================================
#include <redboot.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/plf_mmap.h>
#include <cyg/hal/fsl_board.h>          // Platform specific hardware definitions

#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <flash_config.h>

#if (REDBOOT_IMAGE_SIZE != CYGBLD_REDBOOT_MIN_IMAGE_SIZE)
#error REDBOOT_IMAGE_SIZE != CYGBLD_REDBOOT_MIN_IMAGE_SIZE
#endif

RedBoot_config_option("Board specifics",
                      brd_specs,
                      ALWAYS_ENABLED,
                      true,
                      CONFIG_INT,
                      0
                     );
#endif  //CYGSEM_REDBOOT_FLASH_CONFIG

char HAL_PLATFORM_EXTRA[20] = "PASS x.x [x32 DDR]";

#if defined(CYGSEM_REDBOOT_FLASH_CONFIG) && defined(CYG_HAL_STARTUP_ROMRAM)

RedBoot_cmd("romupdate",
            "Update Redboot with currently running image",
            "",
            romupdate
           );

extern int flash_program(void *_addr, void *_data, int len, void **err_addr);
extern int flash_erase(void *addr, int len, void **err_addr);
extern char *flash_errmsg(int err);
extern unsigned char *ram_end; //ram end is where the redboot starts FIXME: use PC value

#ifdef CYGPKG_IO_FLASH
void romupdate(int argc, char *argv[])
{
    void *err_addr, *base_addr;
    int stat;
    unsigned int nfc_config3_reg, temp;

    if (IS_FIS_FROM_MMC() || IS_BOOTING_FROM_MMC()) {
        diag_printf("Updating ROM in MMC/SD flash\n");
        /* eMMC 4.3 and eSD 2.1 supported only on TO 2.0 */
        if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) == 0x2) {
            if(!emmc_set_boot_partition((cyg_uint32*)ram_end, CYGBLD_REDBOOT_MIN_IMAGE_SIZE)) {
                /* eMMC 4.3 */
                diag_printf("Card supports MMC-4.3, programming for boot operation.\n");
                return;
            } else if(!esd_set_boot_partition((cyg_uint32*)ram_end, CYGBLD_REDBOOT_MIN_IMAGE_SIZE)) {
                /* eSD 2.1 */
                diag_printf("Card supports SD-2.1, programming for boot operation.\n");
                return;
            }
        }
        base_addr = (void*)0;
        /* Read the first 1K from the card */
        mmc_data_read((cyg_uint32*)ram_end, 0x400, base_addr);
        diag_printf("Programming Redboot to MMC/SD flash\n");
        mmc_data_write((cyg_uint32*)ram_end, CYGBLD_REDBOOT_MIN_IMAGE_SIZE, (cyg_uint32)base_addr);

        return;
    } else if (IS_FIS_FROM_NAND() || IS_BOOTING_FROM_NAND()) {
        diag_printf("Updating ROM in NAND flash\n");
        base_addr = (void*)0;
        nfc_config3_reg = readl(NFC_FLASH_CONFIG3_REG);
        temp = nfc_config3_reg & (~ 0x7003);
        writel(temp, NFC_FLASH_CONFIG3_REG);
    } else if (IS_BOOTING_FROM_SPI_NOR() || IS_FIS_FROM_SPI_NOR()) {
        diag_printf("Updating ROM in SPI-NOR flash\n");
        base_addr = (void*)0;
    } else {
        diag_printf("romupdate not supported\n");
        diag_printf("Use \"factive [NAND|MMC|SPI]\" to select either NAND, MMC or SPI flash\n");
    }

    // Erase area to be programmed
    if ((stat = flash_erase((void *)base_addr,
                            CYGBLD_REDBOOT_MIN_IMAGE_SIZE,
                            (void **)&err_addr)) != 0) {
        diag_printf("Can't erase region at %p: %s\n",
                    err_addr, flash_errmsg(stat));
        return;
    }
    // Now program it
    if ((stat = flash_program((void *)base_addr, (void *)ram_end,
                              CYGBLD_REDBOOT_MIN_IMAGE_SIZE,
                              (void **)&err_addr)) != 0) {
        diag_printf("Can't program region at %p: %s\n",
                    err_addr, flash_errmsg(stat));
    }
    if (IS_FIS_FROM_NAND() || IS_BOOTING_FROM_NAND())
    writel(nfc_config3_reg, NFC_FLASH_CONFIG3_REG);
}
RedBoot_cmd("factive",
            "Enable one flash media for Redboot",
            "[NAND | MMC | SPI]",
            factive
           );

typedef void reset_func_t(void);

extern reset_func_t reset_vector;

void factive(int argc, char *argv[])
{
    unsigned int *fis_addr = IRAM_BASE_ADDR;

    if (argc != 2) {
        diag_printf("Invalid factive cmd\n");
        return;
    }

    if (strcasecmp(argv[1], "NOR") == 0) {
        diag_printf("Not supported\n");
        return;
    } else if (strcasecmp(argv[1], "NAND") == 0) {
#ifndef MXCFLASH_SELECT_NAND
        diag_printf("Not supported\n");
        return;
#endif
        *fis_addr = FROM_NAND_FLASH;
    } else if (strcasecmp(argv[1], "MMC") == 0) {
#ifndef MXCFLASH_SELECT_MMC
        diag_printf("Not supported\n");
        return;
#else
        *fis_addr = FROM_MMC_FLASH;
#endif
    } else if (strcasecmp(argv[1], "SPI") == 0) {
#ifndef IMXFLASH_SELECT_SPI_NOR
        diag_printf("Not supported\n");
        return;
#else
        *fis_addr = FROM_SPI_NOR_FLASH;
#endif
    } else {
        diag_printf("Invalid command: %s\n", argv[1]);
        return;
    }

    //HAL_VIRT_TO_PHYS_ADDRESS(ram_end, phys_addr);
    launchRunImg(reset_vector);
}
#endif //CYGPKG_IO_FLASH
#endif /* CYG_HAL_STARTUP_ROMRAM */
