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

char HAL_PLATFORM_EXTRA[40] = "PASS x.x [x32 DDR]. Board Rev 2.0";

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
extern cyg_uint32 mmc_data_read (cyg_uint32 *,cyg_uint32 ,cyg_uint32);
extern int spi_nor_erase_block(void* block_addr, unsigned int block_size);
extern int spi_nor_program_buf(void *addr, void *data, int len);
extern void __attribute__((__noinline__)) launchRunImg(unsigned long addr);

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
            if(!emmc_set_boot_partition((cyg_uint32*)(SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000), CYGBLD_REDBOOT_MIN_IMAGE_SIZE)) {
                /* eMMC 4.3 */
                diag_printf("Card supports MMC-4.3, programming for boot operation.\n");
                return;
            } else if(!esd_set_boot_partition((cyg_uint32*)(SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000), CYGBLD_REDBOOT_MIN_IMAGE_SIZE)) {
                /* eSD 2.1 */
                diag_printf("Card supports SD-2.1, programming for boot operation.\n");
                return;
            }
        }
        base_addr = (void*)0;
        /* Read the first 1K from the card */
        mmc_data_read((cyg_uint32*)(SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000),
                      0x400, base_addr);
        diag_printf("Programming Redboot to MMC/SD flash\n");
        mmc_data_write((cyg_uint32*)(SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000),
                       CYGBLD_REDBOOT_MIN_IMAGE_SIZE, (cyg_uint32)base_addr);
        return;
    } else if (IS_BOOTING_FROM_SPI_NOR() || IS_FIS_FROM_SPI_NOR()) {
        diag_printf("Updating ROM in SPI-NOR flash\n");
        base_addr = (void*)0;
    } else {
        diag_printf("romupdate not supported\n");
        diag_printf("Use \"factive [SPI|MMC]\" to select either NAND or MMC flash\n");
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
    if ((stat = flash_program((void *)base_addr,
                              (void *)SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000,
                              CYGBLD_REDBOOT_MIN_IMAGE_SIZE,
                              (void **)&err_addr)) != 0) {
        diag_printf("Can't program region at %p: %s\n",
                    err_addr, flash_errmsg(stat));
    }
}
RedBoot_cmd("factive",
            "Enable one flash media for Redboot",
            "[MMC|SPI]",
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

    if (strcasecmp(argv[1], "MMC") == 0) {
        *fis_addr = FROM_MMC_FLASH;
    } else if (strcasecmp(argv[1], "SPI") == 0) {
        *fis_addr = FROM_SPI_NOR_FLASH;
    }
    else {
        diag_printf("Invalid command: %s\n", argv[1]);
        return;
    }

    //HAL_VIRT_TO_PHYS_ADDRESS(ram_end, phys_addr);
    launchRunImg(reset_vector);
}
#endif //CYGPKG_IO_FLASH

#define POST_SDRAM_START_OFFSET         0x800000
#define POST_MMC_OFFSET                 0x100000
#define POST_SIZE                       0x100000
#define POST_MAGIC_MARKER               0x43

void imx_launch_post(void)
{
    mmc_data_read(0x100000,     // ram location
                  0x40000,      // length
                  0x100000);    // from MMC/SD offset 0x100000
    /* Need this to recognize the SPI-NOR part */
    if (spi_norflash_hwr_init() != 0)
        return;

    spi_nor_erase_block(0, 0x10000);
    spi_nor_erase_block(0x10000, 0x10000);
    spi_nor_erase_block(0x20000, 0x10000);
    spi_nor_erase_block(0x30000, 0x10000);
    // save the redboot to SPI-NOR
    if (spi_nor_program_buf(0, 0x100000, 0x40000) != 0)
        return;

    diag_printf("Reading POST from MMC to SDRAM...\n");
    mmc_data_read(SDRAM_BASE_ADDR + POST_SDRAM_START_OFFSET,    // ram location
                  0x200000,                                     // length
                  0x200000);                                     // from MMC offset
    diag_printf("Launching POST\n");
    launchRunImg(SDRAM_BASE_ADDR + POST_SDRAM_START_OFFSET);
}
//RedBoot_init(imx_launch_post, RedBoot_INIT_BEFORE_NET);

#endif /* CYG_HAL_STARTUP_ROMRAM */
