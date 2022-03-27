//==========================================================================
//
//      mxc_ata.c
//
//      Flash programming to support ATA flash on Freescale MXC platforms
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Mahesh Mahadevan <mahesh.mahadevan@freescale.com>
// Contributors: Mahesh Mahadevan <mahesh.mahadevan@freescale.com>
// Date:         2008-11-18 Initial version
//
//==========================================================================
//

#include <cyg/hal/hal_cache.h>
#include <stdlib.h>
#include <cyg/io/mxc_ata.h>
#include <redboot.h>

static struct fsl_ata_time_regs {
    unsigned char time_off, time_on, time_1, time_2w;
    unsigned char time_2r, time_ax, time_pio_rdx, time_4;
    unsigned char time_9, time_m, time_jn, time_d;
    unsigned char time_k, time_ack, time_env, time_rpx;
    unsigned char time_zah, time_mlix, time_dvh, time_dzfs;
    unsigned char time_dvs, time_cvh, time_ss, time_cyc;
};
extern void mxc_ata_iomux_setup(void);

/*
 * This structure contains the timing parameters for
 * ATA bus timing in the 5 PIO modes.  The timings
 * are in nanoseconds, and are converted to clock
 * cycles before being stored in the ATA controller
 * timing registers.
 */
static struct {
    short t0, t1, t2_8, t2_16, t2i, t4, t9, tA;
} pio_specs[] = {
    [0] = {
    .t0 = 600, .t1 = 70, .t2_8 = 290, .t2_16 = 165, .t2i = 40, .t4 =
        30, .t9 = 20, .tA = 50,},
    [1] = {
    .t0 = 383, .t1 = 50, .t2_8 = 290, .t2_16 = 125, .t2i = 0, .t4 =
        20, .t9 = 15, .tA = 50,},
    [2] = {
    .t0 = 240, .t1 = 30, .t2_8 = 290, .t2_16 = 100, .t2i = 0, .t4 =
        15, .t9 = 10, .tA = 50,},
    [3] = {
    .t0 = 180, .t1 = 30, .t2_8 = 80, .t2_16 = 80, .t2i = 0, .t4 =
        10, .t9 = 10, .tA = 50,},
    [4] = {
    .t0 = 120, .t1 = 25, .t2_8 = 70, .t2_16 = 70, .t2i = 0, .t4 =
        10, .t9 = 10, .tA = 50,},
    };

#define NR_PIO_SPECS (sizeof pio_specs / sizeof pio_specs[0])

static void update_timing_config(struct fsl_ata_time_regs *tp)
{
    unsigned int  *lp = (unsigned int  *) tp;
    unsigned int  *ctlp = (unsigned int  *) ATA_BASE_ADDR;
    int i;

    for (i = 0; i < 5; i++) {
        writel(*lp, ctlp);
        lp++;
        ctlp++;
    }
}

static void set_ata_bus_timing(unsigned char xfer_mode)
{
    int speed = xfer_mode;
    struct fsl_ata_time_regs tr = { 0 };
    int T = 1 * 1000 * 1000 * 1000 / get_main_clock(IPG_CLK);

    if (speed >= NR_PIO_SPECS)
        return;
    tr.time_off = 3;
    tr.time_on = 3;

    tr.time_1 = (pio_specs[speed].t1 + T) / T;
    tr.time_2w = (pio_specs[speed].t2_8 + T) / T;

    tr.time_2r = (pio_specs[speed].t2_8 + T) / T;
    tr.time_ax = (pio_specs[speed].tA + T) / T + 2;
    tr.time_pio_rdx = 1;
    tr.time_4 = (pio_specs[speed].t4 + T) / T;

    tr.time_9 = (pio_specs[speed].t9 + T) / T;

    update_timing_config(&tr);
}

static unsigned char ata_sff_busy_wait(unsigned int bits, unsigned int max, unsigned int delay)
{
    unsigned char status;
    unsigned int iterations = 1;

    if (max != 0)
        iterations = max;

    do {
        hal_delay_us(delay);
        status = readb(ATA_BASE_ADDR + FSL_ATA_DCDR);
        if (max != 0)
            iterations--;
    } while (status != 0xff && (status & bits) && (iterations > 0));

    if (iterations == 0) {
        diag_printf("ata_sff_busy_wait timeout status = %x\n", status);
        return 0xff;
    }

    return status;
}

static void ata_sff_exec_command(unsigned short cmd)
{
    writeb(cmd, ATA_BASE_ADDR + FSL_ATA_DCDR);
    readb(ATA_BASE_ADDR + FSL_ATA_DRIVE_CONTROL);
    hal_delay_us(4);
}

static int ata_dev_set_feature(unsigned int feature)
{
    unsigned char status;

    writeb(feature, ATA_BASE_ADDR + FSL_ATA_DFTR);
    //Issue Set feature command
    ata_sff_exec_command(ATA_CMD_SET_FEATURES);
    status = ata_sff_busy_wait(ATA_BUSY, 5000, 500);

    if (status == 0xff)
        return 1;
    if (status & ATA_ERR) {
        return 1;
    }
    return 0;
}

void ata_id_string(int *id, unsigned char *s,
                                 unsigned int ofs, unsigned int len)
{
    unsigned int c;

    while (len > 0) {
        c = id[ofs] >> 8;
        *s = c;
        s++;

        c = id[ofs] & 0xff;
        *s = c;
        s++;

        ofs++;
        len -= 2;
    }
}

/**
 *  ata_id_c_string - Convert IDENTIFY DEVICE page into C string
 *  @id: IDENTIFY DEVICE results we will examine
 *  @s: string into which data is output
 *  @ofs: offset into identify device page
 *  @len: length of string to return. must be an odd number.
 *
 *  This function is identical to ata_id_string except that it
 *  trims trailing spaces and terminates the resulting string with
 *  null.  @len must be actual maximum length (even number) + 1.
 *
 *  LOCKING:
 *  caller.
 */
void ata_id_c_string(int *id)
{
    unsigned char model_num[ATA_ID_PROD_LEN + 1];

    ata_id_string(id, model_num, ATA_ID_PROD, ATA_ID_PROD_LEN);

    model_num[ATA_ID_PROD_LEN] = '\0';

    diag_printf("ATA Model number = %s\n", model_num);
}

static int read_dev_id(void)
{
    int i, tried_spinup = 0;
    int CIS[256], err_mask = 0;

retry:

    //identify device command
    ata_sff_exec_command(ATA_CMD_ID_ATA);
    if (ata_sff_busy_wait(ATA_BUSY, 5000, 500) == 0xff)
        return 1;
    memset((void *)CIS, 0, sizeof(int) * 256);

    for (i=0 ; i < 256; i++ ) {
        CIS[i] = readw(ATA_BASE_ADDR + FSL_ATA_DRIVE_DATA);
    }

    if ((CIS[0] & (1 << 15)) == 0) {
        if (!tried_spinup && (CIS[2] == 0x37c8 || CIS[2] == 0x738c)) {
            tried_spinup = 1;
            err_mask = ata_dev_set_feature(0x7);
            if (err_mask && CIS[2] != 0x738c) {
                diag_printf("ATA SPINUP Failed \n");
                goto err_out;
            }
            if (CIS[2] == 0x37c8)
                goto retry;
        }
        ata_id_c_string(CIS);
        return 0;
    } else {
        diag_printf("ATA IDENTIFY DEVICE command Failed \n");
    }
err_out:
    return 1;
}

static void write_sector_pio(unsigned int *addr, int num_of_sectors)
{
    int i, j;

    for (i = 0; i < num_of_sectors; i++) {
        for (j= 0; j < ATA_SECTOR_SIZE; j = j + 4) {
            /* Write 4 bytes in each iteration */
            writew((*addr & 0xFFFF), ATA_BASE_ADDR + FSL_ATA_DRIVE_DATA) ;
            writew(((*addr >> 16 ) & 0xFFFF), ATA_BASE_ADDR + FSL_ATA_DRIVE_DATA) ;
            addr++;
        }
        ata_sff_busy_wait(ATA_BUSY, 5000, 50);
    }
    readb(ATA_BASE_ADDR + FSL_ATA_DRIVE_CONTROL);
}

static void read_sector_pio(unsigned int *addr, int num_of_sectors)
{
    int i, j;
    unsigned int data[2];

    for (i = 0; i < num_of_sectors; i++) {
        for (j = 0; j < ATA_SECTOR_SIZE; j = j + 4) {
            /* Read 4 bytes in each iteration */
            data[0] = readw(ATA_BASE_ADDR + FSL_ATA_DRIVE_DATA);
            data[1] = readw(ATA_BASE_ADDR + FSL_ATA_DRIVE_DATA);
            *addr = ((data[1] << 16) & 0xFFFF0000) | (data[0] & 0xFFFF);
            addr++;
        }
        ata_sff_busy_wait(ATA_BUSY, 5000, 10);
    }
    readb(ATA_BASE_ADDR + FSL_ATA_DRIVE_CONTROL);
}

void ata_hwr_init(void)
{
    mxc_ata_iomux_setup();

    /* Deassert the reset bit to enable the interface */
    writel(FSL_ATA_CTRL_ATA_RST_B, ATA_BASE_ADDR + FSL_ATA_CONTROL);
    writel(FSL_ATA_CTRL_ATA_RST_B | FSL_ATA_CTRL_FIFO_RST_B, ATA_BASE_ADDR + FSL_ATA_CONTROL);
    /* Set initial timing and mode */
    set_ata_bus_timing(PIO_XFER_MODE_4);
    writeb(20, ATA_BASE_ADDR+ FSL_ATA_FIFO_ALARM) ; /* set fifo alarm to 20 halfwords, midway */

    /* software reset */
    writeb(ATA_IEN, ATA_BASE_ADDR + FSL_ATA_DRIVE_CONTROL);
    hal_delay_us(20);
    writeb(ATA_IEN | ATA_SRST, ATA_BASE_ADDR + FSL_ATA_DRIVE_CONTROL);
    hal_delay_us(20);
    writeb(ATA_IEN, ATA_BASE_ADDR + FSL_ATA_DRIVE_CONTROL);

    writeb(0, ATA_BASE_ADDR + FSL_ATA_DDHR);
    if (ata_sff_busy_wait(ATA_BUSY | ATA_DRQ, 6000, 1000) == 0xff) {
        diag_printf("Failed to initialize the ATA drive\n");
        return;
    }

    /* Read the device ID */
    if (read_dev_id())
        diag_printf("Failed to initialize the ATA drive\n");
}

static void ata_read_buf(int argc, char *argv[]);
RedBoot_cmd("ata_read",
            "Read Ata",
            "-f <flash_addr> -b <mem_base> -l <image_length>",
            ata_read_buf
           );

static void ata_program_buf(int argc, char *argv[]);
RedBoot_cmd("ata_write",
            "Write Ata",
            "-f <flash_addr> -b <mem_base> -l <image_length>",
            ata_program_buf
           );

static void ata_read_buf(int argc, char *argv[])
{
    unsigned int total_sectors, num_of_sectors;
    unsigned char lba_addr[4];
    CYG_ADDRESS addr, data;
    unsigned long sect_addr;
    unsigned long len;
    unsigned char status;
    bool mem_addr_set = false;
    bool flash_addr_set = false;
    bool length_set = false;
    struct option_info opts[3];

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM,
              (void *)&data, (bool *)&mem_addr_set, "memory base address");
    init_opts(&opts[1], 'f', true, OPTION_ARG_TYPE_NUM,
              (void *)&addr, (bool *)&flash_addr_set, "FLASH memory base address");
    init_opts(&opts[2], 'l', true, OPTION_ARG_TYPE_NUM,
              (void *)&len, (bool *)&length_set, "image length [in FLASH]");

    if (!scan_opts(argc, argv, 1, opts, 3, 0, 0, 0)) {
        diag_printf("invalid arguments");
        return;
    }

    if (!mem_addr_set || !flash_addr_set || !length_set) {
        diag_printf("required parameter missing\n");
        return;
    }

    if ((addr % ATA_SECTOR_SIZE) != 0) {
        diag_printf("Need a sector-aligned (512 byte) address in ATA\n\n");
        return;
    }

    total_sectors = (len / ATA_SECTOR_SIZE);
    sect_addr = addr / ATA_SECTOR_SIZE;

    do {
        lba_addr[0] = sect_addr & 0xFF;
        lba_addr[1] = (sect_addr >> 8) & 0xFF;
        lba_addr[2] = (sect_addr >> 16) & 0xFF;
        /* Enable the LBA bit */
        lba_addr[3] = (1 << 6) | ((sect_addr >> 24) & 0xF);

        if (total_sectors >= MAX_NUMBER_OF_SECTORS)
            num_of_sectors = 0;
        else
            num_of_sectors = total_sectors;

        ata_sff_busy_wait(ATA_BUSY | ATA_DRQ, 5000, 50);
        writeb(num_of_sectors, ATA_BASE_ADDR + FSL_ATA_DSCR);
        writeb(lba_addr[0], ATA_BASE_ADDR + FSL_ATA_DSNR);
        writeb(lba_addr[1], ATA_BASE_ADDR + FSL_ATA_DCLR);
        writeb(lba_addr[2], ATA_BASE_ADDR + FSL_ATA_DCHR);
        writeb(lba_addr[3], ATA_BASE_ADDR + FSL_ATA_DDHR);

        //Issue Read command
        ata_sff_exec_command(ATA_CMD_READ);
        status = ata_sff_busy_wait(ATA_BUSY, 5000, 50);
        if (status & ATA_ERR) {
            diag_printf("Error while issuing ATA Read command\n");
            return;
        }
        if (num_of_sectors == 0) {
            read_sector_pio((unsigned int *)data, MAX_NUMBER_OF_SECTORS);
            total_sectors -= MAX_NUMBER_OF_SECTORS;
            sect_addr += MAX_NUMBER_OF_SECTORS;
            data += (MAX_NUMBER_OF_SECTORS * ATA_SECTOR_SIZE);
        } else {
            read_sector_pio((unsigned int *)data, num_of_sectors);
            total_sectors -= num_of_sectors;
            sect_addr += num_of_sectors;
            data += (num_of_sectors * ATA_SECTOR_SIZE);
        }
    } while (total_sectors > 0);
}

static void ata_program_buf(int argc, char *argv[])
{
    int total_sectors, num_of_sectors, lba_addr[4];
    CYG_ADDRESS addr, data;
    unsigned long len;
    unsigned long sect_addr;
    unsigned char status;
    bool mem_addr_set = false;
    bool flash_addr_set = false;
    bool length_set = false;
    struct option_info opts[3];

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM,
              (void *)&data, (bool *)&mem_addr_set, "memory base address");
    init_opts(&opts[1], 'f', true, OPTION_ARG_TYPE_NUM,
              (void *)&addr, (bool *)&flash_addr_set, "FLASH memory base address");
    init_opts(&opts[2], 'l', true, OPTION_ARG_TYPE_NUM,
              (void *)&len, (bool *)&length_set, "image length [in FLASH]");

    if (!scan_opts(argc, argv, 1, opts, 3, 0, 0, 0)) {
        diag_printf("invalid arguments");
        return;
    }

    if (!mem_addr_set || !flash_addr_set || !length_set) {
        diag_printf("required parameter missing\n");
        return;
    }

    if ((addr % ATA_SECTOR_SIZE) != 0) {
        diag_printf("Need a sector-aligned (512 byte) address in ATA\n\n");
        return;
    }

    total_sectors = (len / ATA_SECTOR_SIZE);
    sect_addr = addr / ATA_SECTOR_SIZE;

    do {
        lba_addr[0] = sect_addr & 0xFF;
        lba_addr[1] = (sect_addr >> 8) & 0xFF;
        lba_addr[2] = (sect_addr >> 16) & 0xFF;
        /* Enable the LBA bit */
        lba_addr[3] = (1 << 6) | ((sect_addr >> 24) & 0xF);

        if (total_sectors >= MAX_NUMBER_OF_SECTORS)
            num_of_sectors = 0;
        else
            num_of_sectors = total_sectors;

        ata_sff_busy_wait(ATA_BUSY | ATA_DRQ, 5000, 50);
        writeb(num_of_sectors, ATA_BASE_ADDR + FSL_ATA_DSCR);
        writeb(lba_addr[0], ATA_BASE_ADDR + FSL_ATA_DSNR);
        writeb(lba_addr[1], ATA_BASE_ADDR + FSL_ATA_DCLR);
        writeb(lba_addr[2], ATA_BASE_ADDR + FSL_ATA_DCHR);
        writeb(lba_addr[3], ATA_BASE_ADDR + FSL_ATA_DDHR);

        //Issue Write command
        ata_sff_exec_command(ATA_CMD_WRITE);
        ata_sff_busy_wait(ATA_BUSY, 5000, 50);
        if (status & ATA_ERR) {
            diag_printf("Error while issuing ATA Write command\n");
            return;
        }
        if (num_of_sectors == 0) {
            write_sector_pio((unsigned int *)data, MAX_NUMBER_OF_SECTORS);
            total_sectors -= MAX_NUMBER_OF_SECTORS;
            sect_addr += MAX_NUMBER_OF_SECTORS;
            data += (MAX_NUMBER_OF_SECTORS * ATA_SECTOR_SIZE);
        } else {
            write_sector_pio((unsigned int *)data, num_of_sectors);
            total_sectors -= num_of_sectors;
            sect_addr += num_of_sectors;
            data += (num_of_sectors * ATA_SECTOR_SIZE);
        }
    } while (total_sectors > 0);
}

