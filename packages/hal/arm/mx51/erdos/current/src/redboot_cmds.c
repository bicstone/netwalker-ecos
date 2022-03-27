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
//
// modification information
// ------------------------
// 2009/07/03 : /boot/boot.conf support.
//              sdstart <-f conf> support.
// 2009/07/13 : do_poweroff add. do_adin() change do_reset() -> do_poweroff().
//              BATT Volt convert scale fixed.
//              mmc_data_write/read() offset 64bit.
//              sdwrite()/sdread() <-o sector> support.
// 2009/07/20 : gpio4 support.
//              adin() check AC-adapter add.
// 2009/07/27 : from redboot_200925
//               change check_sdhc_slot.
//              adin include scale convert(ADIN5/ADIN7)
//              delete AC-adapter, check DCin.
// 2009/07/29 : adin low check except BATT/DC under 1V, this case trouble in A/D.
// 2009/08/06 : adin BUFFEN add.
// 2009/08/08 : ddr2cal add.
// 2009/08/12 : console add.
// 2009/08/18 : adin ch2 BUFFEN add.
//
//==========================================================================
#include <redboot.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/plf_mmap.h>
#include <cyg/hal/fsl_board.h>          // Platform specific hardware definitions
#include <cyg/io/mxc_mmc.h>

#ifdef MX51_ERDOS
#define DCIN_GOOD_VOLT   (8500)		// DCin Good Voltage level

/*
 * SD start parameter
 */
#define BOOTCONFIG_FILENAME  ("/boot/boot.conf")
#define KERNEL_FILENAME      ("/boot/zImage")
static char *sdLoad [] = {
 "load",	/* [0] */
 "-v",		/* [1] */
 "-r",		/* [2] */
 "-b",		/* [3] */
 "0x800000",	/* [4] */
 "-m",		/* [5] */
 "disk",	/* [6] */
 "" };		/* [7]:kernel file name */
static char *sdExec = "noinitrd root=/dev/%s rw rootfstype=ext2 rootdelay=3 console=tty1";

#define SPI_FLASH_INFO_BASE (0x090000)		/* info area base address in SPI-Flash */
#endif /* MX51_ERDOS */

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
#ifdef MX51_ERDOS
extern cyg_uint32 mmc_data_read (cyg_uint32 *,cyg_uint32 ,cyg_uint64);
#else
extern cyg_uint32 mmc_data_read (cyg_uint32 *,cyg_uint32 ,cyg_uint32);
#endif /* MX51_ERDOS */
extern int spi_nor_erase_block(void* block_addr, unsigned int block_size);
extern int spi_nor_program_buf(void *addr, void *data, int len);
extern void __attribute__((__noinline__)) launchRunImg(unsigned long addr);
extern unsigned int pmic_reg(unsigned int reg, unsigned int val, unsigned int write);

#ifdef CYGPKG_IO_FLASH
extern cyg_uint32 emmc_set_boot_partition (cyg_uint32 *src_ptr, cyg_uint32 length);
extern cyg_uint32 esd_set_boot_partition(cyg_uint32 *src_ptr, cyg_uint32 length);
#ifdef MX51_ERDOS
extern cyg_uint32 mmc_data_write (cyg_uint32 *,cyg_uint32 ,cyg_uint64);
#else
extern cyg_uint32 mmc_data_write (cyg_uint32 *,cyg_uint32 ,cyg_uint32);
#endif /* MX51_ERDOS */
void romupdate(int argc, char *argv[])
{
    void *err_addr, *base_addr;
    int stat;

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
#ifdef MX51_ERDOS
        mmc_data_read((cyg_uint32*)(SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000),
                      0x400, (cyg_uint64)base_addr);
        diag_printf("Programming Redboot to MMC/SD flash\n");
        mmc_data_write((cyg_uint32*)(SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000),
                       CYGBLD_REDBOOT_MIN_IMAGE_SIZE, (cyg_uint64)base_addr);
#else
        mmc_data_read((cyg_uint32*)(SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000),
                      0x400, (cyg_uint32)base_addr);
        diag_printf("Programming Redboot to MMC/SD flash\n");
        mmc_data_write((cyg_uint32*)(SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000),
                       CYGBLD_REDBOOT_MIN_IMAGE_SIZE, (cyg_uint32)base_addr);
#endif /* MX51_ERDOS */
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
                              (void *)((unsigned long)SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000),
                              CYGBLD_REDBOOT_MIN_IMAGE_SIZE,
                              (void **)&err_addr)) != 0) {
        diag_printf("Can't program region at %p: %s\n",
                    err_addr, flash_errmsg(stat));
    }
}

#ifdef MX51_ERDOS
extern int mmcflash_hwr_slot_init(int slot);
extern int spi_norForce_hwr_init(int silent);
extern int sd_fs_register (int slot);
extern int spi_nor_read(void *src, void *dest, int len);
extern int spi_nor_erase_64k(void* block_addr, unsigned int size);
extern mxc_mmc_check_sdhc_boot_slot *check_sdhc_slot;
/*
 * sdupdate : write redboot which downloaded to SD.
 *  sdupdate SD-write-image-address
 */
void sdupdate(int argc, char *argv[])
{
    unsigned int adrs;
    int          i, usage, rc, part, slot;

    adrs  = 0xffffffff;
    part  = 0;
    slot  = -1;
    usage = 0;
    for ( i = 1 ; i < argc ; i++ ) {
        if (argv [i][0] == '-') {
            if ((i + 1) < argc) {
                if (argv [i][1] == 'p') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&part,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid part parameter\n");
                        return;
                    }
                    i++;
                } else if (argv [i][1] == 's') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&slot,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid slot parameter\n");
                        return;
                    }
                    i++;
                } else {
                    usage++;
                }
            } else {
                usage++;
            }
        } else if (adrs == 0xffffffff) {
            if (!parse_num (*(&argv[i]), (unsigned long *)&adrs, &argv[i], ":")) {
                diag_printf ("Error: Invalid ram parameter\n");
                return;
            }
        } else {
            usage++;
        }
    }
    if (slot == -1) {
        /*
         * select boot slot
         */
        if ( check_sdhc_slot ) {
            unsigned int sdadr;
            if (check_sdhc_slot(READ_PORT_FROM_FUSE, &sdadr) != 0) {
                if (sdadr == MMC_SDHC1_BASE_ADDR) {
                    slot = 1;
                } else {
                    slot = 2;
                }
            } else {
                diag_printf("sdupdate: don't boot from SD-card. please specify -s 1/2\n");
            }
        }
    }
    if ((slot == 1) || (slot == 2)) {
        ;
    } else {
        usage++;
    }
    if (adrs == 0xffffffff) {
        usage++;
    }
    if (0 < usage) {
        diag_printf ("sdupdate <address> [-p <forcepart>] [-s <slot 1/2>]\n");
        return;
    }

    if (part == 0) {
        if (((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf) == 0x2) {
            part = 1;
        }
    }

    rc = mmcflash_hwr_slot_init(slot);
    if (rc != 0) {
        diag_printf ("sdupdate: MMCflash_hwr_init(%d) [%d] ERROR\n", slot, rc);
        return;
    }

    if ( part ) {
        rc = esd_set_boot_partition((cyg_uint32 *)adrs, CYGBLD_REDBOOT_MIN_IMAGE_SIZE);
        if (rc == 0) {
            /* eSD 2.1 */
            diag_printf("Card supports SD-2.1, programming for boot operation from 0x%x.\n", adrs);
            return;
        }
        diag_printf("Card not supports SD-2.1.\n");
    }

    diag_printf ("MAJOR:%x(%x) Programming 0x%x to MMC/SD-%d  ", ((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf), system_rev, adrs, slot-1);
    /*
     * First 0x400 byte read from SD (MBR)
     */
    mmc_data_read  ((cyg_uint32 *)adrs, 0x400, (cyg_uint64)0);
    mmc_data_write ((cyg_uint32 *)adrs, CYGBLD_REDBOOT_MIN_IMAGE_SIZE, (cyg_uint64)0);
    diag_printf (".. done\n");
}
RedBoot_cmd("sdupdate",
            "SD/MMC update Redboot by specify address",
            "<address> [-p <forcepart>] [-c <slot 1/2>]",
            sdupdate
           );

/*
 * sdwrite : write image which downloaded to SD.
 *  sdwrite SD-write-image-address
 */
void sdwrite(int argc, char *argv[])
{
    unsigned int adrs, sect, ram, len;
    int          i, usage, rc, slot;
    cyg_uint64   offset;

    adrs  = 0xffffffff;
    sect  = 0xffffffff;
    ram   = 0xffffffff;
    len   = 0xffffffff;
    slot  = -1;
    usage = 0;
    for ( i = 1 ; i < argc ; i++ ) {
        if (argv [i][0] == '-') {
            if ((i + 1) < argc) {
                if (argv [i][1] == 'f') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&adrs,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid adrs parameter\n");
                        return;
                    }
                    i++;
                } else if (argv [i][1] == 'o') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&sect,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid sect parameter\n");
                        return;
                    }
                    i++;
                } else if (argv [i][1] == 'b') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&ram,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid ram parameter\n");
                        return;
                    }
                    i++;
                } else if (argv [i][1] == 'l') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&len,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid len parameter\n");
                        return;
                    }
                    i++;
                } else if (argv [i][1] == 's') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&slot,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid slot parameter\n");
                        return;
                    }
                    i++;
                } else {
                    usage++;
                }
            } else {
                usage++;
            }
        } else {
            usage++;
        }
    }
    if (slot == -1) {
        /*
         * select boot slot
         */
        if ( check_sdhc_slot ) {
            unsigned int sdadr;
            if (check_sdhc_slot(READ_PORT_FROM_FUSE, &sdadr) != 0) {
                if (sdadr == MMC_SDHC1_BASE_ADDR) {
                    slot = 1;
                } else {
                    slot = 2;
                }
            } else {
                diag_printf("sdwrite: don't boot from SD-card. please specify -s 1/2\n");
            }
        }
    }
    if ((slot == 1) || (slot == 2)) {
        ;
    } else {
        usage++;
    }
    if ((adrs == 0xffffffff) && (sect == 0xffffffff)) {
        usage++;
    }
    if ((ram == 0xffffffff) || (len == 0xffffffff)) {
        usage++;
    }
    if (0 < usage) {
        diag_printf ("sdwrite -f <sd address>/-o <sd sector> -b <ram address> -l <length> [-s <slot 1/2>]\n");
        return;
    }

    rc = mmcflash_hwr_slot_init(slot);
    if (rc != 0) {
        diag_printf ("sdwrite: MMCflash_hwr_init(%d) [%d] ERROR\n", slot, rc);
        return;
    }

    if (sect != 0xffffffff) {
        offset = (cyg_uint64)sect * 512;
    } else {
        offset = (cyg_uint64)adrs;
    }
    diag_printf ("MAJOR:%x(%x) Programming 0x%x to MMC/SD-%d 0x%llx %dbytes ", ((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf), system_rev, ram, slot-1, offset, len);
    mmc_data_write ((cyg_uint32 *)ram, len, offset);
    diag_printf (".. done\n");
}
RedBoot_cmd("sdwrite",
            "SD/MMC write image by specify address",
            "-f <sd address>/-o <sd sector> -b <ram address> -l <length> [-s <slot 1/2>]",
            sdwrite
           );
/*
 * sdread : read image which downloaded to SD.
 *  sdread SD-read-image-address
 */
void sdread(int argc, char *argv[])
{
    EXTERN unsigned long entry_address, load_address, load_address_end;
    unsigned int adrs, sect, ram, len;
    int          i, usage, rc, slot;
    cyg_uint64   offset;

    adrs  = 0xffffffff;
    sect  = 0xffffffff;
    ram   = 0xffffffff;
    len   = 0xffffffff;
    slot  = -1;
    usage = 0;
    for ( i = 1 ; i < argc ; i++ ) {
        if (argv [i][0] == '-') {
            if ((i + 1) < argc) {
                if (argv [i][1] == 'f') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&adrs,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid adrs parameter\n");
                        return;
                    }
                    i++;
                } else if (argv [i][1] == 'o') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&sect,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid sect parameter\n");
                        return;
                    }
                    i++;
                } else if (argv [i][1] == 'b') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&ram,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid ram parameter\n");
                        return;
                    }
                    i++;
                } else if (argv [i][1] == 'l') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&len,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid len parameter\n");
                        return;
                    }
                    i++;
                } else if (argv [i][1] == 's') {
                    if (!parse_num(*(&argv[i+1]),(unsigned long *)&slot,&argv[i+1],":")) {
                        diag_printf ("Error: Invalid slot parameter\n");
                        return;
                    }
                    i++;
                } else {
                    usage++;
                }
            } else {
                usage++;
            }
        } else {
            usage++;
        }
    }
    if (slot == -1) {
        /*
         * select boot slot
         */
        if ( check_sdhc_slot ) {
            unsigned int sdadr;
            if (check_sdhc_slot(READ_PORT_FROM_FUSE, &sdadr) != 0) {
                if (sdadr == MMC_SDHC1_BASE_ADDR) {
                    slot = 1;
                } else {
                    slot = 2;
                }
            } else {
                diag_printf("sdread: don't boot from SD-card. please specify -s 1/2\n");
            }
        }
    }
    if ((slot == 1) || (slot == 2)) {
        ;
    } else {
        usage++;
    }
    if ((adrs == 0xffffffff) && (sect == 0xffffffff)) {
        usage++;
    }
    if ((ram == 0xffffffff) || (len == 0xffffffff)) {
        usage++;
    }
    if (0 < usage) {
        diag_printf ("sdread -f <sd address>/-o <sd sector> -b <ram address> -l <length> [-s <slot 1/2>]\n");
        return;
    }

    rc = mmcflash_hwr_slot_init(slot);
    if (rc != 0) {
        diag_printf ("sdread: MMCflash_hwr_init(%d) [%d] ERROR\n", slot, rc);
        return;
    }

    if (sect != 0xffffffff) {
        offset = (cyg_uint64)sect * 512;
    } else {
        offset = (cyg_uint64)adrs;
    }
    diag_printf ("MAJOR:%x(%x) Reading 0x%x from MMC/SD-%d 0x%llx %dbytes ", ((system_rev >> MAJOR_NUMBER_OFFSET) & 0xf), system_rev, ram, slot-1, offset, len);
    mmc_data_read ((cyg_uint32 *)ram, len, offset);
    entry_address    = ram;
    load_address     = ram;
    load_address_end = (unsigned long)ram + len;
    diag_printf (".. done\n");
}
RedBoot_cmd("sdread",
            "SD/MMC read image from specify address",
            "-f <sd address>/-o <sd sector> -b <ram address> -l <length> [-s <slot 1/2>]",
            sdread
           );

/*
 * sdregister : mount for SD.
 *  sdregister slot
 */
void sdregister(int argc, char *argv[])
{
    int rc, slot;

    if (argc != 2) {
        diag_printf ("Usage: %s <slot 1/2>\n", argv [0]);
        return;
    }
    if (!parse_num (*(&argv[1]), (unsigned long *)&slot, &argv[1],":")) {
        diag_printf ("Error: Invalid slot parameter\n");
        return;
    }
    rc = sd_fs_register (slot);
    if (rc != 0) {
        diag_printf ("sdregister: sd_fs_register (%d) failed  %d\n", slot, rc);
    }
    return;
}
RedBoot_cmd("sdregister",
            "SD/MMC register DISK",
            "<slot 1/2>",
            sdregister
           );

/*
 * parse /boot/boot.conf
 *  # comment
 *  filename  cmdline
 */
static char *getLine (char *str, char *work)
{
    int   len = 0;
    char *p   = str;

    for ( ; *p != '\0' ; p++ ) {
        if (*p == '\r') {
            continue;
        }
        if (*p == '\n') {
            if (len == 0) {
                /*
                 * newline only
                 */
                *work++ = *p;
                len++;
            }
            break;
        }
        *work++ = *p;
        len++;
    }
    *work++ = '\0';
    if (len == 0) {
        return 0;
    }
    if (*p != '\0') {
        p++;
    }
    return ( p );
}
static int getKernelCmd (char *conf, char *fname, char *cmd)
{
    char *p, buf [256];
    char *f, *c, *wk;
    int   err, i, flen, clen;

    err = -1;
    p   = conf;
    while ( 1 ) {
        p = getLine (p, buf);
        if (p == 0) {
            break;
        }
        if ((buf [0] == '#') || (buf [0] == '\n')) {
            continue;
        }
        /*
         * skip front space
         */
        for ( wk = buf ; *wk ; wk++ ) {
            if ((*wk == ' ') || (*wk == '\t')) {
                continue;
            }
            break;
        }
        if (*wk != '\0') {
            /*
             * pickup file
             */
            f    = wk;
            flen = 0;
            for ( ; *wk ; wk++ ) {
                if ((*wk == ' ') || (*wk == '\t')) {
                    break;
                }
                f [flen++] = *wk;
            }
            /*
             * separater
             */
            for ( ; *wk ; wk++ ) {
                if ((*wk == ' ') || (*wk == '\t')) {
                    continue;
                }
                break;
            }
            /*
             * pickup cmdline
             */
            c    = wk;
            clen = 0;
            for ( ; *wk ; wk++ ) {
                c [clen++] = *wk;
            }
            /*
             * set result(fname)
             */
            err = 0;
            for ( i = 0 ; i < flen ; i++ ) {
                fname [i] = f [i];
            }
            fname [i++] = '\0';
            /*
             * set result(cmdline)
             */
            for ( i = 0 ; i < clen ; i++ ) {
                cmd [i] = c [i];
            }
            cmd [i++] = '\0';
            break;
        }
    }
    return err;
}

/*
 * sdstart : startup from SD(Kernel, RFS)
 *  return
 *   normal execute don't return
 *    1 : argument error
 *    2 : not found partition
 *    3 : not found kernel 
 *    4 : kernel load error
 *    5 : exec error
 */
static char sdCmdLine [256];
static char sdBootConf [1024];
static char sdBootName [128];
int sdstart (int slot, char *conf, char *cmdline)
{
    extern void *disk_partition_name (void *cookie, char *name);
    extern int   disk_stream_open(connection_info_t *info, int *err);
    extern int   disk_stream_read(char *buf, int size, int *err);
    extern int   disk_stream_close(int *err);
    extern void  do_load(int argc, char *argv[]);
    extern void  do_exec(int argc, char *argv[]);

    connection_info_t info;
    void             *cookie;
    int               rc, find, err, findCmd;
    char              pname [16], fname [32], kpart [32];
    char             *cmd [3];

    if (slot == -1) {
        /*
         * select boot slot
         */
        if ( check_sdhc_slot ) {
            unsigned int sdadr;
            if (check_sdhc_slot(READ_PORT_FROM_FUSE, &sdadr) != 0) {
                if (sdadr == MMC_SDHC1_BASE_ADDR) {
                    slot = 1;
                } else {
                    slot = 2;
                }
            } else {
                diag_printf("sdstart: don't boot from SD-card. please specify -s 1/2\n");
                return 1;
            }
        }
    }

    /*
     * SD register(search partition)
     */
    rc = sd_fs_register (slot);
    if (rc != 0) {
        diag_printf ("sdstart: sd_fs_register (%d) failed  %d\n", slot, rc);
        return 2;
    }

    /*
     * load kernel
     */
    find    = 0;
    findCmd = 0;
    cookie  = disk_partition_name ((void *)0, pname);
    while ( cookie != 0 ) {
        /*
         * default kernel file
         */
        strcpy (sdBootName, KERNEL_FILENAME);

        /*
         * try /boot/boot.conf file (boot config information)
         */
        if (conf == 0) {
            diag_sprintf (fname, "%s:%s", pname, BOOTCONFIG_FILENAME);
        } else {
            diag_sprintf (fname, "%s:%s", pname, conf);
        }
        info.filename = fname;
        rc = disk_stream_open (&info, &err);
        if (rc == 0) {
            rc = disk_stream_read (sdBootConf, sizeof(sdBootConf), &err);
            if (0 < rc) {
                rc = getKernelCmd (sdBootConf, sdBootName, sdCmdLine);
                if (rc == 0) {
                    findCmd = 1;
                }
            }
            disk_stream_close (&err);
        }

        /*
         * try kernel file open
         */
        diag_sprintf (fname, "%s:%s", pname, sdBootName);
        info.filename = fname;
        rc = disk_stream_open (&info, &err);
        if (rc == 0) {
            disk_stream_close (&err);
            find = 1;
            break;
        }
        /*
         * next partition
         */
        cookie = disk_partition_name (cookie, pname);
    }
    if (find == 0) {
        diag_printf ("sdstart: not found %s in SD%d\n", sdBootName, slot);
        return 3;
    }

    /*
     * load kernel image
     */
    diag_printf ("sdstart: loading %s from SD%d\n", fname, slot);
    sdLoad [7] = fname;
    do_load (8, sdLoad);
    if (entry_address == (unsigned long)NO_MEMORY) {
        diag_printf ("sdstart: failed loading %s from SD\n", fname);
        return 4;
    }

    if (findCmd == 1) {
        cmd [2] = sdCmdLine;
    } else {
        /*
         * start kernel
         *  sda1 -> mmcblk0p1
         *  sda2 -> mmcblk0p2
         *  sdb1 -> mmcblk1p1
         *  sdb2 -> mmcblk1p2
         */
        if (cmdline == 0) {
            diag_sprintf (kpart, "mmcblk%dp%c", pname [2]-'a', pname [3]);
            diag_sprintf (sdCmdLine, sdExec, kpart);
            cmd [2] = sdCmdLine;
        } else {
            cmd [2] = cmdline;
        }
    }
    cmd [0] = "exec";
    cmd [1] = "-c";
    diag_printf ("sdstart: %s %s %s\n", cmd [0], cmd [1], cmd [2]);
    do_exec (3, cmd);
    /*
     * don't reach here
     */
    return 5;
}

/*
 * do_sdstart : startup from SD(Kernel, RFS)
 */
static void do_sdstart (int argc, char *argv[])
{
    int   i, slot, usage;
    char *cmdline = 0;
    char *conf = 0;

    usage = 0;
    slot  = -1;
    for ( i = 1 ; i < argc ; i++ ) {
        if (argv [i][0] == '-') {
            if (argv [i][1] == 'c') {
                if ((i + 1) < argc) {
                    cmdline = argv [i+1];
                } else {
                    usage++;
                }
                i++;
            } else if (argv [i][1] == 'f') {
                if ((i + 1) < argc) {
                    conf = argv [i+1];
                } else {
                    usage++;
                }
                i++;
            } else if (argv [i][1] == 's') {
                if ((i + 1) < argc) {
                    if (!parse_num (*(&argv[i+1]), (unsigned long *)&slot, &argv[i+1],":")) {
                        usage++;
                    }
                } else {
                    usage++;
                }
                i++;
            } else {
                usage++;
            }
        } else {
            usage++;
        }
    }
    if (0 < usage) {
        diag_printf ("%s <-c \"kernel command line\"> <-f conf> <-s slot1/2>\n", argv [0]);
        return;
    }
    sdstart (slot, conf, cmdline);
}
RedBoot_cmd("sdstart",
            "start from SD",
            "<-c \"kernel command line\"> <-f conf> <-s slot1/2>",
            do_sdstart
           );

/*
 * spi_rom_read - read from SPI nor
 *  adrs   : SPI Flash address
 *  buf    : read area
 *  length : read length
 */
static int spi_rom_read (void *adrs, void *buf, int length)
{
    int  rc, size;
    unsigned char tmp [4];

    rc = spi_norForce_hwr_init(1);
    if (rc != 0) {
        diag_printf ("spi_rom_read: spi_norForce_hwr_init() [%d]\n", rc);
        return -1;
    }

    /*
     * read from SPI
     *  least 4byte
     */
    if (length < 4) {
        rc = spi_nor_read ((void *)adrs, (void *)tmp, 4);
        memcpy (buf, tmp, length);
    } else {
        size = (length / 4) * 4;
        rc   = spi_nor_read ((void *)adrs, (void *)buf, size);
        if ((rc == 0) && (size < length)) {
            rc = spi_nor_read ((void *)((char *)adrs + size), tmp, 4);
            memcpy (&(((char *)buf)[size]), tmp, length - size);
        }
    }
    return rc;
}

static unsigned char srompage [0x10000];
static int isprint (unsigned char c)
{
    if (('0' <= c) && (c <= '9')) return 1;
    if (('a' <= c) && (c <= 'z')) return 1;
    if (('A' <= c) && (c <= 'Z')) return 1;
    if (('!' == c) || (c == '"')) return 1;
    if (('#' == c) || (c == '$')) return 1;
    if (('%' == c) || (c == '&')) return 1;
    if (('\'' == c) || (c == '(')) return 1;
    if ((')' == c) || (c == '=')) return 1;
    if (('-' == c) || (c == '^')) return 1;
    if (('~' == c) || (c == '|')) return 1;
    if (('\\' == c) || (c == '`')) return 1;
    if (('@' == c) || (c == '[')) return 1;
    if ((']' == c) || (c == '{')) return 1;
    if (('}' == c) || (c == ':')) return 1;
    if ((';' == c) || (c == '<')) return 1;
    if (('>' == c) || (c == '/')) return 1;
    if ((',' == c) || (c == '.')) return 1;
    if (('_' == c) || (c == ' ')) return 1;
    return 0;
}

/*
 * inforead : information read from Serial-Flash
 */
void inforead(int argc, char *argv[])
{
    int            adrs, offset, ram, length;
    unsigned char *p;
    int            usage, i, rc, size, lset, bset;

    usage  = 0;
    size   = 4;
    offset = -1;
    bset   = 0;
    lset   = 0;
    for ( i = 1 ; i < argc ; i++ ) {
        if (argv [i][0] == '-') {
            if (argv [i][1] == '1') {
                size = 1;
            } else if (argv [i][1] == '2') {
                size = 2;
            } else if (argv [i][1] == '3') {
                size = 3;
            } else if (argv [i][1] == '4') {
                size = 4;
            } else if (argv [i][1] == 'l') {
                if ((i + 1) < argc) {
                    if (!parse_num (*(&argv[i+1]), (unsigned long *)&length, &argv[i+1], ":")) {
                        usage++;
                    }
                } else {
                    usage++;
                }
                i++;
                lset = 1;
            } else if (argv [i][1] == 'b') {
                if ((i + 1) < argc) {
                    if (!parse_num (*(&argv[i+1]), (unsigned long *)&ram, &argv[i+1], ":")) {
                        usage++;
                    }
                } else {
                    usage++;
                }
                i++;
                bset = 1;
            } else if (argv [i][1] == 'o') {
                if ((i + 1) < argc) {
                    if (!parse_num (*(&argv[i+1]), (unsigned long *)&offset, &argv[i+1], ":")) {
                        usage++;
                    }
                } else {
                    usage++;
                }
                i++;
            } else {
                usage++;
            }
        } else {
            usage++;
        }
    }
    if ((0 < usage) || (offset == -1)) {
        diag_printf ("inforead -o <offset> {-l <length> [-1|-2|-3|-4]} {-b <ram-address>}\n");
        return;
    }
    if (lset == 1) {
        /*
         * specify read length
         */
        if (sizeof(srompage) < length) {
            diag_printf ("length over less %dbytes\n", sizeof(srompage));
            return;
        }
        size = length;
    }

    /*
     * check info size
     */
    if (0x10000 < (offset + size)) {
        diag_printf ("offset+length over less %dbytes\n", 0x10000);
        return;
    }

    /*
     * Flash address
     */
    adrs = SPI_FLASH_INFO_BASE + offset;

    if (bset == 1) {
        /*
         * read to RAM
         */
        rc = spi_rom_read ((void *)adrs, (void *)ram, size);
        if (rc != 0) {
            diag_printf ("read error %d\n", rc);
        }
        return;
    }

    rc = spi_rom_read ((void *)adrs, (void *)srompage, size);
    if (rc != 0) {
        diag_printf ("read error %d\n", rc);
        return;
    }

    p = srompage;
    if (size == 1) {
        diag_printf ("offset:%X value:%02X\n", offset, p[0]);
    } else if (size == 2) {
        diag_printf ("offset:%X value:%02X%02X\n", offset, p[1], p[0]);
    } else if (size == 3) {
        diag_printf ("offset:%X value:%02X%02X%02X\n", offset, p[2], p[1], p[0]);
    } else if (size == 4) {
        diag_printf("offset:%X value:%02X%02X%02X%02X\n", offset, p[3], p[2], p[1], p[0]);
    } else {
        if (size <= 32) {
            diag_printf ("offset:%X %dbytes  ", offset, size);
            for ( i = 0 ; i < size ; i++ ) {
                unsigned char c = *p++;
                if (isprint (c)) {
                    diag_printf ("%c", c);
                } else {
                    diag_printf (".");
                }
            }
            diag_printf ("\n");
        } else {
            diag_printf ("offset:%X %dbytes\n", offset, size);
            for ( i = 0 ; i < size ; i++ ) {
                unsigned char c = *p++;
                if (isprint (c)) {
                    diag_printf ("%c", c);
                } else {
                    diag_printf (".");
                }
                if ((i % 64) == 63) {
                    diag_printf ("\n");
                }
            }
            if ( i % 64 ) {
                diag_printf ("\n");
            }
        }
    }
}
RedBoot_cmd("inforead",
            "read info from Serial-Flash",
            "-o <offset> {-l <length> [-1|-2|-3|-4]} {-b <ram-address>}",
            inforead
           );

/*
 * spi_rom_write - write from SPI nor
 *  adrs   : SPI Flash address
 *  buf    : write area
 *  length : write length
 */
static int spi_rom_write (void *adrs, void *buf, int length)
{
    unsigned long base, last;
    int           rc;

    /*
     * check 64Kbyte bound
     */
    last = (unsigned long)adrs + length;
    base = (unsigned long)adrs & ~0xFFFF;
    if (base != (last & ~0xFFFF)) {
        diag_printf ("spi_rom_write: address:%x don't support over 64Kbyte\n", (unsigned int)adrs);
        return -1;
    }

    rc = spi_norForce_hwr_init(1);
    if (rc != 0) {
        diag_printf ("spi_rom_write: spi_norForce_hwr_init() [%d]\n", rc);
        return -1;
    }

    /*
     * read page
     */
    rc = spi_nor_read ((void *)base, (void *)srompage, sizeof(srompage));
    if (rc != 0) {
        diag_printf ("spi_rom_write: read error %d\n", rc);
        return -1;
    }

    /*
     * update data
     */
    memcpy (&(srompage [(unsigned long)adrs & 0xFFFF]), buf, length);

    /*
     * erase page
     */
    if ((rc = spi_nor_erase_64k((void *)base, 0x10000)) != 0) {
        diag_printf("spi_rom_write: Can't erase region at %p-%x %d\n",
                    (void *)base, 0x10000, rc);
        return -1;
    }

    /*
     * write page
     */
    if ((rc = spi_nor_program_buf((void *)base, (void *)srompage, 0x10000)) != 0) {
        diag_printf("spi_rom_write: Can't program region at %p-%x %d\n",
                    (void *)base, 0x10000, rc);
    }
    return rc;
}

/*
 * infowrite : information write to Serial-Flash
 */
void infowrite(int argc, char *argv[])
{
    unsigned int   adrs, length, ram, value;
    int            usage, i, rc, size, vset, lset, bset, offset;
    unsigned char  tmp [4];
    unsigned char *pvalue;
    unsigned char *org;

    usage  = 0;
    size   = 4;
    adrs   = 0;
    value  = 0;
    offset = -1;
    vset   = 0;
    lset   = 0;
    bset   = 0;
    for ( i = 1 ; i < argc ; i++ ) {
        if (argv [i][0] == '-') {
            if (argv [i][1] == '1') {
                size = 1;
            } else if (argv [i][1] == '2') {
                size = 2;
            } else if (argv [i][1] == '3') {
                size = 3;
            } else if (argv [i][1] == '4') {
                size = 4;
            } else if (argv [i][1] == 'l') {
                if ((i + 1) < argc) {
                    if (!parse_num (*(&argv[i+1]), (unsigned long *)&length, &argv[i+1], ":")) {
                        usage++;
                    }
                } else {
                    usage++;
                }
                i++;
                lset = 1;
            } else if (argv [i][1] == 'b') {
                if ((i + 1) < argc) {
                    if (!parse_num (*(&argv[i+1]), (unsigned long *)&ram, &argv[i+1], ":")) {
                        usage++;
                    }
                } else {
                    usage++;
                }
                i++;
                bset = 1;
            } else if (argv [i][1] == 'o') {
                if ((i + 1) < argc) {
                    if (!parse_num (*(&argv[i+1]), (unsigned long *)&offset, &argv[i+1], ":")) {
                        usage++;
                    }
                } else {
                    usage++;
                }
                i++;
            } else {
                usage++;
            }
        } else if (vset == 0) {
            vset   = 1;
            pvalue = (unsigned char *)(argv [i]);
        } else {
            usage++;
        }
    }
    if ((0 < usage) || (offset == -1) || ((vset == 0) && (bset == 0))) {
        diag_printf ("infowrite -o <offset> <value> {-l <length> [-1|-2|-3|-4]} -b <ram-address>\n");
        return;
    }
    if (lset == 1) {
        size = length;
    }

    /*
     * check info size
     */
    if (0x10000 < (offset + size)) {
        diag_printf ("offset+length over less %dbytes\n", 0x10000);
        return;
    }

    /*
     * Flash address
     */
    adrs = SPI_FLASH_INFO_BASE + offset;

    /*
     * update data
     */
    if (bset == 1) {
        /*
         * SDRAM -> Serial-Flash
         */
        rc  = spi_rom_write ((void *)adrs, (void *)ram, size);
        org = (unsigned char *)ram;
    } else {
        if ((1 <= size) && (size <= 4)) {
            /*
             * value
             */
            if (!parse_num ((char *)pvalue, (unsigned long *)&value, (char **)&pvalue, ":")) {
                diag_printf ("infowrite: can't convert value\n");
                rc = -1;
            } else {
                tmp [0] = value & 0xFF;
                tmp [1] = (value >> 8) & 0xFF;
                tmp [2] = (value >> 16) & 0xFF;
                tmp [3] = (value >> 24) & 0xFF;
                rc  = spi_rom_write ((void *)adrs, (void *)tmp, size);
                org = tmp;
            }
        } else {
            /*
             * strings
             */
            rc  = spi_rom_write ((void *)adrs, pvalue, size);
            org = pvalue;
        }
    }

    /*
     * read check
     */
    if (rc == 0) {
        rc = spi_rom_read ((void *)adrs, (void *)srompage, size);
        if (rc == 0) {
            unsigned char *p1 = org;
            unsigned char *p2 = srompage;
            for ( i = 0 ; i < size ; i++ , p1++ , p2++ ) {
                if (*p1 != *p2) {
                    diag_printf ("infowrite: Verify error offset:%x %02X - %02X\n", i, *p1, *p2);
                    break;
                }
            }
        }
    }
}
RedBoot_cmd("infowrite",
            "wtite info to Serial-Flash",
            "-o <offset> <value> {-l <length> [-1|-2|-3|-4]} {-b <ram-address>}",
            infowrite
           );

/*
 * sromupdate : write redboot which downloaded to Serial-Flash.
 *  sromupdate Flash-write-image-address
 */
void sromupdate(int argc, char *argv[])
{
    unsigned int adrs;
    void        *base_addr;
    int          rc, stat;

    if (argc != 2) {
        diag_printf ("sromupdate <ram-addr>\n");
        return;
    }

    if (!parse_num (*(&argv[1]), (unsigned long *)&adrs, &argv[1], ":")) {
        diag_printf ("Error: Invalid ram parameter\n");
        return;
    }

    rc = spi_norForce_hwr_init(0);
    if (rc != 0) {
        diag_printf ("sromupdate: spi_norForce_hwr_init() [%d]\n", rc);
        return;
    }

    // Erase area to be programmed
    base_addr = (void *)0;
    diag_printf("erase at %p %xbytes ...", base_addr, CYGBLD_REDBOOT_MIN_IMAGE_SIZE);
    if ((stat = spi_nor_erase_64k((void *)base_addr,
                            CYGBLD_REDBOOT_MIN_IMAGE_SIZE)) != 0) {
        diag_printf("Can't erase region at %p-%x %d\n",
                    base_addr, CYGBLD_REDBOOT_MIN_IMAGE_SIZE, stat);
        return;
    }
    diag_printf("done.\n");
    diag_printf("program %p -> %p %xbytes ",
                (void *)adrs, base_addr, CYGBLD_REDBOOT_MIN_IMAGE_SIZE);
    // Now program it
    if ((stat = spi_nor_program_buf((void *)base_addr, (void *)adrs,
                              CYGBLD_REDBOOT_MIN_IMAGE_SIZE)) != 0) {
        diag_printf("Can't program region at %p -> %p-%x %d\n", (void *)adrs,
                    base_addr, CYGBLD_REDBOOT_MIN_IMAGE_SIZE, stat);
    } else {
        diag_printf("done.\n");
    }
}
RedBoot_cmd("sromupdate",
            "Serial-Flash update Redboot by specify address",
            "<address>",
            sromupdate
           );

/*
 * DDR caliblation
 *  running on IRAM
 */
void ddr2char (char c)
{
    volatile unsigned long *uartd = (volatile unsigned long *)0x73fbc040;
    volatile unsigned long *uarts = (volatile unsigned long *)0x73fbc094;
    int i;

    if (c == '\n') {
        *uartd = '\r';
        for ( i = 0 ; i < 0x1000 ; i++ ) {
            if ((*uarts & 0x2000) == 0x2000) {
                break;
            }
        }
    }
    *uartd = c;
    for ( i = 0 ; i < 0x1000 ; i++ ) {
        if ((*uarts & 0x2000) == 0x2000) {
            break;
        }
    }
}
void ddr2msg (char *msg)
{
    while ( *msg ) {
        ddr2char ( *msg++ );
    }
}
void ddr2hex (int val)
{
    char msg [10];
    if (0xa <= ((val >> 28) & 0xF)) msg [0] = ((val >> 28) & 0xF) - 0xa + 'A';
    else                            msg [0] = ((val >> 28) & 0xF) + '0';
    if (0xa <= ((val >> 24) & 0xF)) msg [1] = ((val >> 24) & 0xF) - 0xa + 'A';
    else                            msg [1] = ((val >> 24) & 0xF) + '0';
    if (0xa <= ((val >> 20) & 0xF)) msg [2] = ((val >> 20) & 0xF) - 0xa + 'A';
    else                            msg [2] = ((val >> 20) & 0xF) + '0';
    if (0xa <= ((val >> 16) & 0xF)) msg [3] = ((val >> 16) & 0xF) - 0xa + 'A';
    else                            msg [3] = ((val >> 16) & 0xF) + '0';
    if (0xa <= ((val >> 12) & 0xF)) msg [4] = ((val >> 12) & 0xF) - 0xa + 'A';
    else                            msg [4] = ((val >> 12) & 0xF) + '0';
    if (0xa <= ((val >>  8) & 0xF)) msg [5] = ((val >>  8) & 0xF) - 0xa + 'A';
    else                            msg [5] = ((val >>  8) & 0xF) + '0';
    if (0xa <= ((val >>  4) & 0xF)) msg [6] = ((val >>  4) & 0xF) - 0xa + 'A';
    else                            msg [6] = ((val >>  4) & 0xF) + '0';
    if (0xa <= ((val >>  0) & 0xF)) msg [7] = ((val >>  0) & 0xF) - 0xa + 'A';
    else                            msg [7] = ((val >>  0) & 0xF) + '0';
    msg [8] = '\0';
    ddr2msg (msg);
}
int ddr2check (int mask, int rd)
{
    volatile int *ram;
    int           wval, rval, i, j, x, v [4];

    ram   = (volatile int *)0x100000;
    wval  = 0x5aa5f00f;
    v [0] = 0x00000000;
    v [1] = 0xffffffff;
    v [2] = 0x55555555;
    v [3] = 0xaaaaaaaa;
    for ( i = 0 ; i < 10000 ; i++ ) {
        for ( j = 0 ; j < 128 ; j++ ) {
            ram [0x1000 + j] = v [j & 3];
        }
        for ( j = 0 ; j < 32 ; j++ ) {
            x = (i + j + rd) & 0xFF;
            ram [j * 64] = wval + ((x << 24) | (x << 16) | (x << 8) | x);
        }
        for ( j = 0 ; j < 128 ; j++ ) {
            rval = ram [0x1000 + j];
            if ((rval & mask) != (v [j & 3] & mask)) {
                return 1;
            }
        }
        for ( j = 0 ; j < 32 ; j++ ) {
            rval = ram [j * 64];
            x    = (i + j + rd) & 0xFF;
            x    = wval + ((x << 24) | (x << 16) | (x << 8) | x);
            if ((rval & mask) != (x & mask)) {
                return 1;
            }
        }
        j = *(volatile int *)IRAM_BASE_ADDR;
    }
    return 0;
}
void ddr2calib (void)
{
    int          i, j, k, wdly, bdly;
    unsigned int b3Rslt [46][2];
    unsigned int b2Rslt [46][2];
    unsigned int b1Rslt [46][2];
    unsigned int b0Rslt [46][2];
    unsigned int val;

    ddr2char('D'); ddr2char('D'); ddr2char('R'); ddr2char(' ');
    ddr2char('C'); ddr2char('a'); ddr2char('l'); ddr2char('i'); ddr2char('b');
    ddr2char('\n');

    val = *(volatile int *)(0x83FD9010);
    *(volatile int *)(0x83FD9010) = val | 0x00000800; /* FRC_MSR */

    for ( i = 0 ; i < 46 ; i++ ) {
        b3Rslt [i][0] = 0;
        b3Rslt [i][1] = 0;
        b2Rslt [i][0] = 0;
        b2Rslt [i][1] = 0;
        b1Rslt [i][0] = 0;
        b1Rslt [i][1] = 0;
        b0Rslt [i][0] = 0;
        b0Rslt [i][1] = 0;
    }
    wdly = -35;
    k    = 1;
    for ( i = 0 ; i < 46 ; i++ , wdly++ ) {
        *(volatile int *)(0x83FD9000 + 0x30) = (wdly << 16) & 0x00FF0000;
        bdly = -35;
        for ( j = 0 ; j < 46 ; j++ , bdly++ ) {
            *(volatile int *)(0x83FD9000 + 0x20) = (bdly << 16) & 0x00FF0000;
            *(volatile int *)(0x83FD9000 + 0x24) = (bdly << 16) & 0x00FF0000;
            *(volatile int *)(0x83FD9000 + 0x28) = (bdly << 16) & 0x00FF0000;
            *(volatile int *)(0x83FD9000 + 0x2C) = (bdly << 16) & 0x00FF0000;
            if (ddr2check (0xFF000000, k++) == 0) b3Rslt[i][j/32] |= 0x01 << (j%32); // OK
            if (ddr2check (0x00FF0000, k++) == 0) b2Rslt[i][j/32] |= 0x01 << (j%32); // OK
            if (ddr2check (0x0000FF00, k++) == 0) b1Rslt[i][j/32] |= 0x01 << (j%32); // OK
            if (ddr2check (0x000000FF, k++) == 0) b0Rslt[i][j/32] |= 0x01 << (j%32); // OK

#if 0
            ddr2hex (i<<24|j<<16|(wdly&0xFF)); ddr2char (' '); ddr2char (':'); ddr2char (' '); ddr2char (' ');
            ddr2hex(b3Rslt[i][1]);ddr2char(' ');ddr2hex(b3Rslt[i][0]);ddr2char(' ');
            ddr2hex(b2Rslt[i][1]);ddr2char(' ');ddr2hex(b2Rslt[i][0]);ddr2char(' ');
            ddr2hex(b1Rslt[i][1]);ddr2char(' ');ddr2hex(b1Rslt[i][0]);ddr2char(' ');
            ddr2hex(b0Rslt[i][1]);ddr2char(' ');ddr2hex(b0Rslt[i][0]);ddr2char('\n');
#endif
        }
        ddr2hex (wdly); ddr2char (' '); ddr2char (':'); ddr2char (' '); ddr2char (' ');
         ddr2hex (b3Rslt[i][1]); ddr2char (' '); ddr2hex (b3Rslt[i][0]); ddr2char (' ');
         ddr2hex (b2Rslt[i][1]); ddr2char (' '); ddr2hex (b2Rslt[i][0]); ddr2char (' ');
         ddr2hex (b1Rslt[i][1]); ddr2char (' '); ddr2hex (b1Rslt[i][0]); ddr2char (' ');
         ddr2hex (b0Rslt[i][1]); ddr2char (' '); ddr2hex (b0Rslt[i][0]); ddr2char ('\n');
    }
    val = *(volatile int *)(0x83FD9010);
    *(volatile int *)(0x83FD9010) = val & ~0x00000800; /* FRC_MSR */
}
static void ddr2cal (int argc, char *argv[])
{
    unsigned int src  = (unsigned int)ddr2char;
    unsigned int lsrc = (unsigned int)ddr2cal;
    unsigned int exe  = (unsigned int)ddr2calib;
    unsigned int dst  = (unsigned int)IRAM_BASE_ADDR;
    unsigned int nsp  = (unsigned int)(IRAM_BASE_ADDR + 0x7F00);
    void       (*func)(void);

    /*
     * run program on IRAM
     */
    memcpy ((void *)dst, (void *)src, lsrc - src);
    func = (void *)(exe - src + IRAM_BASE_ADDR);
    asm ("mov sp, %0" : : "r" (nsp));
    (*func)(); 
}
RedBoot_cmd("ddr2calib",
            "SDRAM calib",
            " ",
            ddr2cal
           );

/*
 * sdramtest : SDRAM random test
 *  sdramtest start-address size
 */
static int stest (unsigned long addr, unsigned long size, int type, int verbose)
{
    volatile int *p, *px;
    unsigned long pa;
    int           i, j, k, v, x;
    int          *pp;
    int           p0 [4] = { 0x00000000, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA };
    int           p1 [4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
    int           p2 [4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
    int           p3 [4] = { 0x55555555, 0x55555555, 0x55555555, 0x55555555 };
    int           p4 [4] = { 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA };
    int           p5 [4] = { 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF };
    int           p6 [4] = { 0x55555555, 0xAAAAAAAA, 0x55555555, 0xAAAAAAAA };
    int           p7 [4] = { 0x00000000, 0x55555555, 0xFFFFFFFF, 0xAAAAAAAA };

    if (verbose) {
        diag_printf ("Test from 0x%08lx to 0x%08lx (%ldM) type:0x%x\n", addr, addr+size, size/0x100000, type);
    }

    /*
     * 0000/FFFF/AAAA/5555 test
     */
    switch ( type ) {
    case 2:
    case 3:   pp = p1; break;
    case 4:
    case 5:   pp = p2; break;
    case 6:
    case 7:   pp = p3; break;
    case 8:
    case 9:   pp = p4; break;
    case 10:
    case 11:  pp = p5; break;
    case 12:
    case 13:  pp = p6; break;
    case 14:
    case 15:  pp = p7; break;
    default : pp = p0; break;
    }

    diag_printf ("%08x/%08x/%08x/%08x test\n", pp[0], pp[1], pp[2], pp[3]);
    for ( j = 0 ; j < 4 ; j++ ) {
        p = (volatile int *)addr;
        for ( i = 0 ; i < (size/sizeof(int)) ; i++ ) {
            if (type & 1) {
                v = *p;
            }
            *p++ = pp [(i+j)%4];
        }
        p = (volatile int *)addr;
        for ( i = 0 ; i < (size/sizeof(int)) ; i++ ) {
            v = *p++;
            x = pp [(i+j)%4];
            if (v != x) {
                diag_printf ("%x %x - %x\n", (int)p-4, v, x);
            }
        }
        if (verbose) {
            diag_printf ("%d/%d %lx - %lx pass\n", j, 4, addr, addr+size);
        }
    }
    /*
     * address write
     */
    diag_printf ("address test\n");
    p = (volatile int *)addr;
    for ( i = 0 ; i < (size/sizeof(int)) ; i++ ) {
        pa = (unsigned long)p;
        if (i & 1) {
            *p++ = (pa & 0xFFFF0000) | ((~pa) & 0x0000FFFF);
        } else {
            *p++ = ((~pa) & 0xFFFF0000) | (pa & 0x0000FFFF);
        }
    }
    p = (volatile int *)addr;
    for ( i = 0 ; i < (size/sizeof(int)) ; i++ ) {
        pa = (unsigned long)p;
        if (i & 1) {
            x = (pa & 0xFFFF0000) | ((~pa) & 0x0000FFFF);
        } else {
            x = ((~pa) & 0xFFFF0000) | (pa & 0x0000FFFF);
        }
        v = *p++;
        if (v != x) {
            diag_printf ("%x : %x - %x\n", (int)p - 4, v, x);
        }
    }
    if (verbose) {
        diag_printf ("address pattern %lx - %lx pass\n", addr, addr+size);
    }
    /*
     * cache size
     */
    diag_printf ("cache size test\n");
    for ( k = 0 ; k < 4 ; k++ ) {
        for ( i = 0 ; i < (32/sizeof(int)) ; i++ ) {
            p  = (volatile int *)(addr + i * sizeof(int));
            px = (volatile int *)(addr + i * sizeof(int));
            while ( (unsigned int)p < (addr + size) ) {
                *p = (int)((unsigned long)p + k);
                p  = (int *)((unsigned int)p + 32);
                if (((addr + size)/2) <= (unsigned long)p) {
                    v = *px;
                    x = (unsigned long)px + k;
                    if (v != x) {
                        diag_printf ("%p : %x - %x\n", px, v, x);
                    }
                    px = (int *)((unsigned int)px + 32);
                }
            }
        }
        for ( i = 0 ; i < (32/sizeof(int)) ; i++ ) {
            p = (volatile int *)(addr + i * sizeof(int));
            while ( (unsigned int)p < (addr + size) ) {
                v = *p;
                x = (unsigned long)p + k;
                if (v != x) {
                    diag_printf ("%p : %x - %x\n", p, v, x);
                }
                p = (int *)((unsigned int)p + 32);
            }
        }
        if (verbose) {
            diag_printf ("%d/%d %lx - %lx pass\n", k, 4, addr, addr+size);
        }
    }
    return 0;
}
static void sdramtest(int argc, char *argv[])
{
    unsigned long addr = 0;
    unsigned long size = 0;
    int           verbose = 1;
    int           rc, type;

    if (argc != 3) {
        diag_printf ("sdramtest <start-addr> <length>\n");
        return;
    }
    if (!parse_num (*(&argv[1]), (unsigned long *)&addr, &argv[1], ":")) {
        diag_printf ("Error: Invalid start-addr parameter\n");
        return;
    }
    if (!parse_num (*(&argv[2]), (unsigned long *)&size, &argv[2], ":")) {
        diag_printf ("Error: Invalid size parameter\n");
        return;
    }

    if ((addr == 0) || (size == 0)) {
        diag_printf ("addr:%lx size:%lx not checked.\n", addr, size);
        return;
    }

    for ( type = 0 ; type < 16 ; type++ ) {
        rc = stest (addr, size, type, verbose);
        if (rc != 0) {
            break;
        }
    }
}
RedBoot_cmd("sdramtest",
            "SDRAM test",
            "<start-address> <length>",
            sdramtest
           );

/*
 * poweroff : POWER OFF
 */
static void do_poweroff (int argc, char *argv[])
{
    unsigned int reg;

    /*
     * SYS_ON_OFF_CTL(GPIO1_23) OFF
     * BAT_AC2(GPIO1_9) Low(DCin Power)
     */
    reg  = readl(GPIO1_BASE_ADDR + 0x0);
    reg &= ~((1 << 23) | (1 << 9));
    writel(reg, GPIO1_BASE_ADDR + 0x0);

    /*
     * SYS_ON_OFF_CTL(pmic) OFF
     *  MC13892 PWGT1SPIEN/PWGT2SPIEN
     */
    reg  = pmic_reg (34, 0, 0);
    reg |= ((1 << 16) | (1 << 15));
    pmic_reg (34, reg, 1);
}
RedBoot_cmd("poweroff",
            "Power OFF",
            "",
            do_poweroff
           );

extern int adin (int channel, int *result);
/*
 * DCinVolt : DC input voltage
 *  rc : 0(DCin OK) / 1(DCin Low) / -1(error)
 */
int DCinVolt (int *volt)
{
    int  rc, dcin;

    rc = adin (7, &dcin);
    if (rc == 0) {
        if (DCIN_GOOD_VOLT <= dcin) {
            rc = 0;
        } else {
            rc = 1;
        }
        if (volt != 0) {
            *volt = dcin;
        }
    }
    return rc;
}

/*
 * adin : AD input
 *  adin channel
 */
int adin (int channel, int *result)
{
    extern void select_batt_volt (int onoff);
    extern int  is_phy_found (void);
    int          i, vsel = 0, rc = -1;
    unsigned int value;

    /*
     * RESET A/D
     */
    pmic_reg (43, 0x000100, 1);
    hal_delay_us (5000);

    /*
     * DCIN/BATT Volt Select
     */
    if ((channel == 5) || (channel == 7)) {
        vsel = 1;
        select_batt_volt ( 1 );
    }

    /*
     * ADC0/1
     */
    if (channel == 8) {
        /*
         * Die temp
         */
        value = 0x7FF813 | (7 << 8) | (7 << 5);
        pmic_reg (43, 0x008010, 1);
    } else {
        value = 0x7FF813 | ((channel & 7) << 8) | ((channel & 7) << 5);
        if (((channel & 7) == 5) || ((channel & 7) == 7) || ((channel & 7) == 2)) {
            pmic_reg (43, 0x008208, 1);	/* BUFFEN */
        } else {
            pmic_reg (43, 0x008200, 1);
        }
    }
    pmic_reg (44, value, 1);

    /*
     * wait AD complete
     */
    for ( i = 0 ; i < 100 ; i++ ) {
        hal_delay_us (2000);
        value = pmic_reg (44, 0, 0);
        if ((value & 0x100000) == 0) {
            rc = 0;
            break;
        }
    }

    /*
     * read A/D
     * (avaerage 2-sample)
     */
    if (rc == 0) {
        value = pmic_reg (45, 0, 0);
        value = (((value >> 14) & 0x3FF) + ((value >> 2) & 0x3FF)) / 2; 
        switch ( channel ) {
        case 0:		/* Battery Voltage 0-4800mV */
            *result = value * 4800 / 0x3FF;
            break;
        case 1:		/* Battery Current -3000mA - 3000mA */
            if ( value & 0x200 ) {
                *result = (~(value & 0x1FF) & 0x1FF) * 59;
            } else {
                *result = (value & 0x1FF) * 59;
            }
            *result = *result / 10;
            break;
        case 2:		/* Voltage 0-4800mV */
            *result = value * 4800 / 0x3FF;
            break;
        case 3:		/* Charger Voltage 0-20000mV */
            if (0x354 <= value) {
                *result = 20000;
            } else {
                *result = value * 20000 / 0x354;
            }
            break;
        case 4:		/* Changer Current -3000mA - 3000mA */
            if ( value & 0x200 ) {
                *result = (~(value & 0x1FF) & 0x1FF) * 6;
            } else {
                *result = (value & 0x1FF) * 6;
            }
            break;
        case 5:		/* ADIN5 0-2400mV -> BATT */
            if (is_phy_found () == 0) {
                /*
                 * 8.3V - 991
                 */
                *result = (value * 8300 / 991);
            } else {
                /*
                 *  200Kohm+40Kohm
                 */
                *result = (value * 2400 * (200 + 40)) / 40 / 0x3FF;
            }
            break;
        case 6:		/* CoinCell Voltage 0-3600mV */
            *result = value * 3600 / 0x3FF;
            break;
        case 7:		/* ADIN7 0-DCIN(11000mV)(scale 1/2) -> DCIN */
            *result = (value * 20398 * 2) / 0x3FF;
            break;
        case 8:		/* Die Temp 25oC-680 0.4244oC/bit */
            *result = 25 + ((value - 680) * 4244) / 10000;
            break;
        default:
            rc = -1;
            break;
        }
    }

    /*
     * disable AD
     */
    pmic_reg (44, 0, 1);

    /*
     * DCIN/BATT Volt unSelect
     */
    if (vsel == 1) {
        select_batt_volt ( 0 );
    }

    return rc;
}

static void do_adin(int argc, char *argv[])
{
    int  usage, i, rc, channel, low, lset, up, uset, value;

    usage   = 0;
    channel = -1;
    low     = 0;
    lset    = 0;
    up      = 0x7FFFFFFF;
    uset    = 0;
    for ( i = 1 ; i < argc ; i++ ) {
        if (argv [i][0] == '-') {
            if (argv [i][1] == 'l') {
                if ((i + 1) < argc) {
                    if (!parse_num (*(&argv[i+1]), (unsigned long *)&low, &argv[i+1], ":")) {
                        usage++;
                    }
                } else {
                    usage++;
                }
                i++;
                lset = 1;
            } else if (argv [i][1] == 'u') {
                if ((i + 1) < argc) {
                    if (!parse_num (*(&argv[i+1]), (unsigned long *)&up, &argv[i+1], ":")) {
                        usage++;
                    }
                } else {
                    usage++;
                }
                i++;
                uset = 1;
            } else {
                usage++;
            }
        } else if (channel == -1) {
            if (!parse_num (*(&argv[i]), (unsigned long *)&channel, &argv[i], ":")) {
                usage++;
            }
        } else {
            usage++;
        }
    }
    if ((0 < usage) || (channel == -1)) {
        diag_printf ("adin <channel> {-l <low-limit> -u <up-limit>}\n");
        return;
    }

    rc = adin (channel, &value);
    if (rc == 0) {
        if (channel == 7) {
            /*
             * DCIN
             */
            diag_printf ("AD channel:DC_IN(%d) value:%dmV\n", channel, value);
        } else if (channel == 5) {
            /*
             * BATT Volt
             */
            diag_printf ("AD channel:BATT(%d) value:%dmV\n", channel, value);
        } else if (channel == 8) {
            /*
             * Die Temp
             */
            diag_printf ("AD channel:DieTemp(%d) value:%doC\n", channel, value);
        } else {
            diag_printf ("AD channel:%d value:%d\n", channel, value);
        }
    } else {
        diag_printf ("AD channel:%d failed\n", channel);
    }
    if (rc == 0) {
        /*
         * low limit check
         */
        if (lset == 1) {
            if (value < low) {
                int  hflag = 1;
                int  dcin  = -1;
                if (channel == 5) {
                    /*
                     * check DC input
                     */
                    rc = DCinVolt (&dcin);
                    if (rc != 1) {
                        diag_printf ("adin: under low-limit (%dmV / %dmV) on DCin(%dmV).\n", value, low, dcin);
                        hflag = 0;
                    }
                    /*
                     * BATT/DC both low voltage, this case trouble in A/D.
                     */
                    if ((value < 1000) && (dcin < 1000)) {
                        diag_printf ("adin: under low-limit (%dmV / %dmV) maybe trouble DCin(%dmV).\n", value, low, dcin);
                        hflag = 0;
                    }
                }
                if (hflag == 1) {
                    diag_printf ("adin: under low-limit (%dmV / %dmV) DCin(%dmV) halt.\n", value, low, dcin);
                    do_poweroff (0, (char **)0);
                }
            }
        }
        /*
         * upper limit check
         */
        if (uset == 1) {
            if (up < value) {
                diag_printf ("adin: over up-limit (%d / %d) halt.\n", value, up);
                do_poweroff (0, (char **)0);
            }
        }
    }
}

RedBoot_cmd("adin",
            "AD input",
            "<channel> {-l low-limit} {-u up-limit}",
            do_adin
           );

/*
 * console : console enable(ON)/disable(OFF)
 */
static void console (int argc, char *argv[])
{
    extern void cyg_hal_plf_serial_silent (int silent);
    int  i, usage, silent;

    usage  = 0;
    silent = -1;
    for ( i = 1 ; i < argc ; i++ ) {
        if ((strcmp (argv [i], "ON") == 0) || (strcmp (argv [i], "on") == 0)) {
            silent = 0;
        } else if ((strcmp (argv [i], "OFF") == 0) || (strcmp (argv [i], "off") == 0)) {
            silent = 1;
        } else {
            usage++;
        }
    }
    if ((0 < usage) || (silent == -1)) {
        diag_printf ("console <ON/OFF>\n");
        return;
    }
    cyg_hal_plf_serial_silent (silent);
}
RedBoot_cmd("console",
            "console ON/OFF",
            "<ON/OFF>",
            console
           );

/*
 * iomux : IOMUX show
 *  iomux offset length
 */
static void iomux(int argc, char *argv[])
{
    unsigned long offset = 0;
    unsigned long size = 1;
    unsigned int  reg;
    int           i;

    if ((argc == 2) || (argc == 3)) {
        ;
    } else {
        diag_printf ("iomux <offset> <length>\n");
        return;
    }
    if (!parse_num (*(&argv[1]), (unsigned long *)&offset, &argv[1], ":")) {
        diag_printf ("Error: Invalid offset parameter\n");
        return;
    }
    if (argc == 3) {
        if (!parse_num (*(&argv[2]), (unsigned long *)&size, &argv[2], ":")) {
            diag_printf ("Error: Invalid size parameter\n");
            return;
        }
    }

    for ( i = 0 ; i < size ; i++ ) {
        reg = readl((unsigned int)IOMUXC_BASE_ADDR + offset + (i * 4));
        if ((i % 4) == 0) {
            diag_printf ("%x: ", (unsigned int)((unsigned int)IOMUXC_BASE_ADDR + offset + (i * 4)));
        }
        diag_printf (" %08x", reg);
        if ((i % 4) == 3) {
            diag_printf ("\n");
        }
    }
    if (i % 4) {
        diag_printf ("\n");
    }
}
RedBoot_cmd("iomux",
            "IOMUX read",
            "<offset> [<length>]",
            iomux
           );

/*
 * iomuxw : IOMUX write
 *  iomuxw offset data
 */
static void iomuxw(int argc, char *argv[])
{
    unsigned long offset = 0;
    unsigned long data = 0;
    unsigned int  reg1, reg2;

    if (argc != 3) {
        diag_printf ("iomuxw <offset> <data>\n");
        return;
    }
    if (!parse_num (*(&argv[1]), (unsigned long *)&offset, &argv[1], ":")) {
        diag_printf ("Error: Invalid offset parameter\n");
        return;
    }
    if (!parse_num (*(&argv[2]), (unsigned long *)&data, &argv[2], ":")) {
        diag_printf ("Error: Invalid data parameter\n");
        return;
    }
    reg1 = readl(IOMUXC_BASE_ADDR + offset);
    writel(data, IOMUXC_BASE_ADDR + offset);
    reg2 = readl(IOMUXC_BASE_ADDR + offset);
    diag_printf ("%x: %08x -> %08x -> %08x\n",
                 (unsigned int)(IOMUXC_BASE_ADDR + offset), reg1,
                 (unsigned int)data, reg2);
}
RedBoot_cmd("iomuxw",
            "IOMUX write",
            "<offset> <data>",
            iomuxw
           );

/*
 * gpio : GPIO show
 *  gpio group
 */
static void gpio(int argc, char *argv[])
{
    unsigned long group = 0;
    unsigned long base;
    unsigned int  reg;
    char         *name;
    int           i;

    if (argc != 2) {
        diag_printf ("gpio <group>\n");
        return;
    }
    if (!parse_num (*(&argv[1]), (unsigned long *)&group, &argv[1], ":")) {
        diag_printf ("Error: Invalid group parameter\n");
        return;
    }

    if (group == 1) {
        base = GPIO1_BASE_ADDR;
        name = "GPIO1";
    } else if (group == 2) {
        base = GPIO2_BASE_ADDR;
        name = "GPIO2";
    } else if (group == 3) {
        base = GPIO3_BASE_ADDR;
        name = "GPIO3";
    } else if (group == 4) {
        base = GPIO4_BASE_ADDR;
        name = "GPIO4";
    } else {
        diag_printf ("Error: Invalid group must 1/2/3/4\n");
        return;
    }
    for ( i = 0 ; i < 8 ; i++ ) {
        reg = readl(base + (i * 4));
        if ((i % 4) == 0) {
            diag_printf ("%s %x: ", name, (unsigned int)(base + (i * 4)));
        }
        diag_printf (" %08x", reg);
        if ((i % 4) == 3) {
            diag_printf ("\n");
        }
    }
    if (i % 4) {
        diag_printf ("\n");
    }
}
RedBoot_cmd("gpio",
            "GPIO read",
            "<group>",
            gpio
           );

/*
 * gpiow : GPIO write
 *  gpiow group offset data
 */
static void gpiow(int argc, char *argv[])
{
    unsigned long group = 0;
    unsigned long offset = 0;
    unsigned long data = 0;
    unsigned long base;
    char         *name;
    unsigned int  reg1, reg2;

    if (argc != 4) {
        diag_printf ("gpiow <group> <offset> <data>\n");
        return;
    }
    if (!parse_num (*(&argv[1]), (unsigned long *)&group, &argv[1], ":")) {
        diag_printf ("Error: Invalid group parameter\n");
        return;
    }
    if (!parse_num (*(&argv[2]), (unsigned long *)&offset, &argv[2], ":")) {
        diag_printf ("Error: Invalid offset parameter\n");
        return;
    }
    if (!parse_num (*(&argv[3]), (unsigned long *)&data, &argv[3], ":")) {
        diag_printf ("Error: Invalid data parameter\n");
        return;
    }
    if (group == 1) {
        base = GPIO1_BASE_ADDR;
        name = "GPIO1";
    } else if (group == 2) {
        base = GPIO2_BASE_ADDR;
        name = "GPIO2";
    } else if (group == 3) {
        base = GPIO3_BASE_ADDR;
        name = "GPIO3";
    } else if (group == 4) {
        base = GPIO4_BASE_ADDR;
        name = "GPIO4";
    } else {
        diag_printf ("Error: Invalid group must 1/2/3/4\n");
        return;
    }
    reg1 = readl(base + offset);
    writel(data, base + offset);
    reg2 = readl(base + offset);
    diag_printf ("%x: %08x -> %08x -> %08x\n",
                 (unsigned int)(base + offset), reg1, (unsigned int)data, reg2);
}
RedBoot_cmd("gpiow",
            "GPIO write",
            "<group> <offset> <data>",
            gpiow
           );
#endif /* MX51_ERDOS */

RedBoot_cmd("factive",
            "Enable one flash media for Redboot",
            "[MMC|SPI]",
            factive
           );

typedef void reset_func_t(void);

extern reset_func_t reset_vector;

void factive(int argc, char *argv[])
{
    unsigned int *fis_addr = (unsigned int *)IRAM_BASE_ADDR;

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
    launchRunImg((unsigned long)reset_vector);
}
#endif //CYGPKG_IO_FLASH

#define POST_SDRAM_START_OFFSET         0x800000
#define POST_MMC_OFFSET                 0x100000
#define POST_SIZE                       0x100000
#define POST_MAGIC_MARKER               0x43


void imx_launch_post(void)
{
#ifdef MX51_ERDOS
    mmc_data_read((cyg_uint32 *)0x100000,     // ram location
                  0x40000,                    // length
                  (cyg_uint64)0x100000);      // from MMC/SD offset 0x100000
#else
    mmc_data_read((cyg_uint32 *)0x100000,     // ram location
                  0x40000,                    // length
                  0x100000);                  // from MMC/SD offset 0x100000
#endif /* MX51_ERDOS */
    spi_nor_erase_block((void *)0, 0x10000);
    spi_nor_erase_block((void *)0x10000, 0x10000);
    spi_nor_erase_block((void *)0x20000, 0x10000);
    spi_nor_erase_block((void *)0x30000, 0x10000);
    // save the redboot to SPI-NOR
    spi_nor_program_buf((void *)0, (void *)0x100000, 0x40000);

    diag_printf("Reading POST from MMC to SDRAM...\n");
#ifdef MX51_ERDOS
    mmc_data_read((cyg_uint32 *)(SDRAM_BASE_ADDR + POST_SDRAM_START_OFFSET),//ram location
                  0x200000,                                     // length
                  (cyg_uint64)0x200000);                        // from MMC offset
#else
    mmc_data_read((cyg_uint32 *)(SDRAM_BASE_ADDR + POST_SDRAM_START_OFFSET),//ram location
                  0x200000,                                     // length
                  0x200000);                                     // from MMC offset
#endif /* MX51_ERDOS */
    diag_printf("Launching POST\n");
    launchRunImg(SDRAM_BASE_ADDR + POST_SDRAM_START_OFFSET);
}
//RedBoot_init(imx_launch_post, RedBoot_INIT_BEFORE_NET);

#endif /* CYG_HAL_STARTUP_ROMRAM */
