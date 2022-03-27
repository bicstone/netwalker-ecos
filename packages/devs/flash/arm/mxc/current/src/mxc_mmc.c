// ==========================================================================
//
//   mxc_mmc.c
//   (c) 2008, Freescale
//
//   MMC card driver for MXC platform
//
// ==========================================================================
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Lewis Liu <weizhi.liu@freescale.com>
// Contributors: Lewis Liu <weizhi.liu@freescale.com>
// Date:         2008-05-13 Initial version
// Purpose:
// Description:
//     Support SD/MMC cards based on eSDHC controller.
//     only base functionality is implemented: Card init, read and write.
//     Erase and write protection are not supported so far.
//
//####DESCRIPTIONEND####
//
// modification information
// ------------------------
// 2009/07/13 : mmc_data_write/read() offset 64bit.
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <redboot.h>
#include <stdlib.h>
#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>
#include <cyg/io/mxc_mmc.h>

#if defined(CYGPKG_HAL_ARM_MX31ADS) || defined(CYGPKG_HAL_ARM_MX31_3STACK)
    #include <cyg/io/card_mx32.h>
#endif

#if defined(CYGPKG_HAL_ARM_MX25_3STACK) || defined(CYGPKG_HAL_ARM_MX35_3STACK) || defined(CYGPKG_HAL_ARM_MX37_3STACK) || defined(CYGPKG_HAL_ARM_MX51)
    #include <cyg/io/mxcmci_core.h>
#endif

mxc_mmc_check_sdhc_boot_slot *check_sdhc_slot = NULL;

#ifdef MX51_ERDOS
#include <redboot.h>
#include <pkgconf/redboot.h>
#include <fs/disk.h>
/*
 * for fs(EXT filesystem)
 */
static int sd_read_sector (struct disk *d, cyg_uint32 start_sector,
                           cyg_uint32 *buf, cyg_uint8  nr_sectors)
{
    int rc = mmc_data_read (buf, nr_sectors * SECTOR_SIZE, (cyg_uint64)start_sector * SECTOR_SIZE);
    if (rc == 0) {
        rc = nr_sectors * SECTOR_SIZE;
    } else {
        rc = 0;		/* error */
    }
    return rc;
}

static disk_funs_t   sd_funs = { sd_read_sector };
static unsigned long sd_capacity [2];
static int           sd_registered [2];
extern int mmcflash_hwr_slot_init(int slot);

int sd_fs_register (int slot)
{
    unsigned long capacity;
    disk_t        disk;
    int           rc;

    /*
     * check slot number
     */
    if ((slot == 1) || (slot == 2)) {
        if (sd_capacity [slot - 1] == 0) {
            if (mmcflash_hwr_slot_init (slot) != SUCCESS) {
                diag_printf ("sd_fs_register: failed hwr_init slot:%d\n", slot);
                return -1;
            }
        }
    } else {
        return -1;
    }

    /*
     * check already registered
     */
    if (sd_registered [slot - 1] != 0) {
        return 0;
    }

    /*
     * check capacity
     */
    capacity = sd_capacity [slot - 1];
    if (capacity == 0) {
        diag_printf ("sd_fs_register: unknown capacity slot:%d\n", slot);
        return -1;
    }

    /*
     * register DISK
     */
    memset (&disk, 0, sizeof(disk));
    disk.funs       = &sd_funs;
    disk.private    = 0;
    disk.kind       = DISK_SD;
    disk.nr_sectors = capacity / SECTOR_SIZE;
    rc = disk_register (&disk);
    if (rc != 1) {
        diag_printf ("sd_fs_register: failed %d\n", rc);
        return -1;
    }
    sd_registered [slot - 1] = 1;
    return 0;
}
#endif /* MX51_ERDOS */

//hardware init for MMC card
#ifndef MXCFLASH_SELECT_MULTI
int flash_hwr_init(void)
#else
int mmcflash_hwr_init(void)
#endif
{
    cyg_uint32 status = FAIL;
    cyg_uint32 capacity = 0;
    int i = 5;
    int SDHCbootslot = 0;
    unsigned int EsdhcRegBase = 0;

    if (check_sdhc_slot) {
        SDHCbootslot = check_sdhc_slot(READ_PORT_FROM_FUSE, &EsdhcRegBase);
        if(!SDHCbootslot){
            return FAIL;
        }
    } else {
        /* Default to MMC 1 */
        EsdhcRegBase = MMC_SDHC1_BASE_ADDR;
    }

    while (status != SUCCESS && i--) {
        hal_delay_us(100000);
        status = mxcmci_init(1, EsdhcRegBase);
    }

    if (FAIL == status) {
        diag_printf("Error: Card initialization failed!\n");
        return status;
    }
    diag_printf("Card initialization successful!\n");
    //set flash_info structure
    externC struct flash_info flash_info;
    flash_dprintf(FLASH_DEBUG_MAX,"%s: status=%d\n", __FUNCTION__, status);
    capacity = card_get_capacity_size(); // in unit of KB
    diag_printf("Actual capacity of the card is %dKB\n", capacity);
    //if the capacity size is larger than 2G or equals zero, force to be 2G
    if (capacity > 0x200000 || capacity == 0) {
        capacity = 0x200000;
    }
    diag_printf("Redboot uses %dKB\n", capacity);

    flash_info.block_size = 0x20000; // =  128KB
    flash_info.blocks = capacity / 128;
    flash_info.start = (void *)MXC_MMC_BASE_DUMMY;
    flash_info.end = (void *)(MXC_MMC_BASE_DUMMY + flash_info.block_size * flash_info.blocks);

#ifdef MX51_ERDOS
    if (status == SUCCESS) {
        sd_capacity [SDHCbootslot - 1] = capacity;
    }
#endif /* MX51_ERDOS */

    return status;
}

#ifdef MX51_ERDOS
int mmcflash_hwr_slot_init(int slot)
{
    cyg_uint32 status = FAIL;
    cyg_uint32 capacity = 0;
    int i = 5;
    unsigned int EsdhcRegBase = 0;

    if (slot == 1) {
        EsdhcRegBase = MMC_SDHC1_BASE_ADDR;
    } else if (slot == 2) {
        EsdhcRegBase = MMC_SDHC2_BASE_ADDR;
    } else {
        return -1;
    }

    while (status != SUCCESS && i--) {
        hal_delay_us(100000);
        status = mxcmci_init(1, EsdhcRegBase);
    }

    if (FAIL == status) {
        diag_printf("Error: Card initialization failed!\n");
        return status;
    }
    diag_printf("Card initialization successful!\n");
    //set flash_info structure
    externC struct flash_info flash_info;
    flash_dprintf(FLASH_DEBUG_MAX,"%s: status=%d\n", __FUNCTION__, status);
    capacity = card_get_capacity_size(); // in unit of KB
    diag_printf("Actual capacity of the card is %dKB\n", capacity);
    //if the capacity size is larger than 2G or equals zero, force to be 2G
    if (capacity > 0x200000 || capacity == 0) {
        capacity = 0x200000;
    }
    diag_printf("Redboot uses %dKB\n", capacity);

    flash_info.block_size = 0x20000; // =  128KB
    flash_info.blocks = capacity / 128;
    flash_info.start = (void *)MXC_MMC_BASE_DUMMY;
    flash_info.end = (void *)(MXC_MMC_BASE_DUMMY + flash_info.block_size * flash_info.blocks);

    if (status == SUCCESS) {
        sd_capacity [slot - 1] = capacity;
    }

    return status;
}
#endif /* MX51_ERDOS */

// Read data into buffer
#ifndef MXCFLASH_SELECT_MULTI
int flash_read_buf(void* addr, void* data, int len)
#else
int mmcflash_read_buf(void* addr, void* data, int len)
#endif
{
    flash_dprintf(FLASH_DEBUG_MAX,"%s:Debug:1:addr=%X, data=%X, len=%d\n", __FUNCTION__, (cyg_uint32)addr, (cyg_uint32)data, len);
#ifdef MX51_ERDOS
    return mmc_data_read(data, len, (cyg_uint64)addr);
#else
    return mmc_data_read(data, len, (cyg_uint32)addr);
#endif/* MX51_ERDOS */
}


// Get CID to pointer data (should hold 4*4 byte space)
#ifndef MXCFLASH_SELECT_MULTI
void flash_query(void* data)
#else
void mmcflash_query(void* data)
#endif
{
    return card_flash_query(data);
}

#ifndef MXCFLASH_SELECT_MULTI
int flash_hwr_map_error(int e)
#else
int mmcflash_hwr_map_error(int e)
#endif
{
    return e;
}

#ifndef MXCFLASH_SELECT_MULTI
bool flash_code_overlaps(void *start, void *end)
#else
bool mmcflash_code_overlaps(void *start, void *end)
#endif
{
    extern char _stext[], _etext[];

    bool ret = ((((unsigned long)&_stext >= (unsigned long)start) &&
                 ((unsigned long)&_stext < (unsigned long)end)) ||
                (((unsigned long)&_etext >= (unsigned long)start) &&
                 ((unsigned long)&_etext < (unsigned long)end)));
    flash_dprintf(FLASH_DEBUG_MAX,"%s: flash code overlap::%d\n", __FUNCTION__, ret);
    return ret;
}

#ifndef MXCFLASH_SELECT_MULTI
int flash_erase_block(void* block, unsigned int size)
#else
int mmcflash_erase_block(void* block, unsigned int size)
#endif
{
    flash_dprintf(FLASH_DEBUG_MAX,"%s:Debug:1:block=0x%X, size=%d\n", __FUNCTION__, (cyg_uint32)block, size);
    // No need to erase for MMC/SD. Skipping ...
    return 0;
}

#ifndef MXCFLASH_SELECT_MULTI
int flash_program_buf(void* addr, void* data, int len)
#else
int mmcflash_program_buf(void* addr, void* data, int len)
#endif
{
    flash_dprintf(FLASH_DEBUG_MAX,"%s:Debug:1:addr=0x%X, data=0x%X, len=%d\n", __FUNCTION__, (cyg_uint32)addr, (cyg_uint32)data, len);
#ifdef MX51_ERDOS
    return mmc_data_write((cyg_uint32*)data, len, (cyg_uint64)addr);
#else
    return mmc_data_write((cyg_uint32*)data, len, (cyg_uint32)addr);
#endif /* MX51_ERDOS */
}

#ifndef MXCFLASH_SELECT_MULTI
int flash_lock_block(void* block)
#else
int mmcflash_lock_block(void* block)
#endif
{
    //not support yet
    return 0;
}

#ifndef MXCFLASH_SELECT_MULTI
int flash_unlock_block(void* block, int block_size, int blocks)
#else
int mmcflash_unlock_block(void* block, int block_size, int blocks)
#endif
{
    //not support yet
    return 0;
}

void mxc_mmc_print_info(void)
{
	extern card_type Card_type;
    cyg_uint32 i = 0;
    cyg_uint8* cmd_class[] = {
        "basic",           //class 0
        "reserved",        //class 1
        "block-read",      //class 2
        "reserved",        //class 3
        "block-write",       //class 4
        "erase",           //class 5
        "write-protect",   //class 6
        "lock",            //class 7
        "app-command",       //class 8
        "IO-mode",           //class 9
        "switch",           //class 10
        "reserved"           //class 11
    };

    switch (Card_type) {
    case SD_CSD_1_0:
        diag_printf("\nBooting from [SD card, CSD Version 1.0]\n");
        break;
    case SD_CSD_2_0:
        diag_printf("\nBooting from [SD card, CSD Version 2.0]\n");
        break;
    case MMC_CSD_1_0:
        diag_printf("\nBooting from [MMC card, CSD Version 1.0]\n");
        break;
    case MMC_CSD_1_1:
        diag_printf("\nBooting from [MMC card, CSD Version 1.1]\n");
        break;
    case MMC_CSD_1_2:
        diag_printf("\nBooting from [MMC card, CSD Version 1.2]\n");
        break;
    case MMC_UNKNOWN:
        diag_printf("\nBooting from [MMC card (?) ]\n");
        break;
    default:
        diag_printf("\nBooting from [unknown version card ]\n");
        break;
    }
    diag_printf("Supporting Card Command Class: ");
    for (;i<12;i++) {
        if (CCC & (1 << i))
            diag_printf("%s, ", cmd_class[i]);
    }

    diag_printf("\n\n");
}

static void do_mmc_op(int argc, char *argv[]);
RedBoot_cmd("sdhc",
            "Read/Write MMC/SD card",
            "<ram-addr> <flash-addr> <len-bytes> <r/w>",
            do_mmc_op
           );

static void do_mmc_op(int argc, char *argv[])
{
    unsigned int ram, port, flash, len;
    unsigned char op;
    int stat = -1, ret;
    int i = 5;
    unsigned int EsdhcRegBase = 0;
    cyg_uint32 status = FAIL;

    if (argc != 6) {
        diag_printf("\tRead: sdhc <port no> <ram-addr> <flash-addr> <len-bytes> <r>\n");
        diag_printf("\tWrite: sdhc <port no> <ram-addr> <flash-addr> <len-bytes> <w>\n");
        return;
    }

    if (!parse_num(*(&argv[1]), (unsigned long *)&port, &argv[1], ":")) {
        diag_printf("Error: Invalid Port number\n");
        return;
    }

    if (!parse_num(*(&argv[2]), (unsigned long *)&ram, &argv[2], ":")) {
        diag_printf("Error: Invalid ram parameter\n");
        return;
    }

    if (!parse_num(*(&argv[3]), (unsigned long *)&flash, &argv[3], ":")) {
        diag_printf("Error: Invalid flash parameter\n");
        return;
    }

    if (!parse_num(*(&argv[4]), (unsigned long *)&len, &argv[4], ":")) {
        diag_printf("Error: Invalid length parameter\n");
        return;
    }

    op = argv[5][0];

    if (check_sdhc_slot) {
        ret = check_sdhc_slot(port, &EsdhcRegBase);
        if(!ret){
            return;
        }
    } else {
        diag_printf("This command is not implemented on this platform!\n");
        return;
    }
    while (status != SUCCESS && i--) {
        hal_delay_us(100000);
        status = mxcmci_init(1, EsdhcRegBase);
    }

    if (FAIL == status) {
        diag_printf("Error: Card initialization failed!\n");
        return;
    }

    switch (op) {
    case 'r':
    case 'R':
        diag_printf("Reading SDHC%d 0x%x [0x%x bytes] -> ram 0x%x\n", port, flash, len, ram);
        //stat = spi_nor_read((void *)flash, (void *)ram, len);
        stat = mmcflash_read_buf((void *)flash, (void *)ram, len);
        break;
    case 'w':
    case 'W':
        diag_printf("Writing SDHC%d 0x%x [0x%x bytes] <- ram 0x%x\n", port, flash, len, ram);
        //stat = spi_nor_program_buf((void *)flash, (void *)ram, len);
        stat = mmcflash_program_buf((void *)flash, (void *)ram, len);
        break;
    default:
        diag_printf("Error: unknown operation: 0x%02x\n", op);
    }
    diag_printf("%s\n\n", (stat == 0)? "SUCCESS": "FAILED");
    return;
}
