#ifndef _MXC_NFC_V3_H_
#define _MXC_NFC_V3_H_
//==========================================================================
//
//      mxc_nfc_v3.h
//
//      Flash programming to support NAND flash on Freescale MXC platforms
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
// Author(s):    Kevin Zhang <k.zhang@freescale.com>
// Contributors: Kevin Zhang <k.zhang@freescale.com>
// Date:         2008-06-02
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/devs_flash_onmxc.h>
#include "mxc_nand_specifics.h"

#define PG_2K_DATA_OP_MULTI_CYCLES()        false
#define ADDR_INPUT_SIZE                 8

#define NAND_MAIN_BUF0                  (NFC_BASE + 0x000)
#define NAND_MAIN_BUF1                  (NFC_BASE + 0x200)
#define NAND_MAIN_BUF2                  (NFC_BASE + 0x400)
#define NAND_MAIN_BUF3                  (NFC_BASE + 0x600)
#define NAND_MAIN_BUF4                  (NFC_BASE + 0x800)
#define NAND_MAIN_BUF5                  (NFC_BASE + 0xA00)
#define NAND_MAIN_BUF6                  (NFC_BASE + 0xC00)
#define NAND_MAIN_BUF7                  (NFC_BASE + 0xE00)
#define NAND_SPAR_BUF0                  (NFC_BASE + 0x1000)
#define NAND_SPAR_BUF1                  (NFC_BASE + 0x1040)
#define NAND_SPAR_BUF2                  (NFC_BASE + 0x1080)
#define NAND_SPAR_BUF3                  (NFC_BASE + 0x10C0)
#define NAND_SPAR_BUF4                  (NFC_BASE + 0x1100)
#define NAND_SPAR_BUF5                  (NFC_BASE + 0x1140)
#define NAND_SPAR_BUF6                  (NFC_BASE + 0x1180)
#define NAND_SPAR_BUF7                  (NFC_BASE + 0x11C0)

// The following defines are not used. Just for compilation purpose
#define ECC_STATUS_RESULT_REG     0xDEADFFFF
#define NFC_DATA_INPUT(buf_no, earea, en)
#define NFC_DATA_INPUT_2k(buf_no)
// dummy function as it is not needed for automatic operations
#define NFC_ADDR_INPUT(addr)
#define NFC_ARCH_INIT()
#define NUM_OF_CS_LINES                 8
#define NFC_BUFSIZE                          4096

enum nfc_internal_buf {
    RAM_BUF_0 = 0x0 << 4,
    RAM_BUF_1 = 0x1 << 4,
    RAM_BUF_2 = 0x2 << 4,
    RAM_BUF_3 = 0x3 << 4,
    RAM_BUF_4 = 0x4 << 4,
    RAM_BUF_5 = 0x5 << 4,
    RAM_BUF_6 = 0x6 << 4,
    RAM_BUF_7 = 0x7 << 4,
};

enum nfc_output_mode {
    FDO_PAGE_SPARE      = 0x0008,
    FDO_SPARE_ONLY      = 0x1008,  // LSB has to be 0x08
    FDO_FLASH_ID        = 0x0010,
    FDO_FLASH_STATUS    = 0x0020,
};

#define wait_for_auto_prog_done()                 \
    do {                                                                            \
        while ((readl(NFC_IPC_REG) & NFC_IPC_AUTO_DONE) == 0)  \
            {} \
        write_nfc_ip_reg((readl(NFC_IPC_REG) & ~NFC_IPC_AUTO_DONE), NFC_IPC_REG); \
    } while (0)

// Polls the NANDFC to wait for an operation to complete
#define wait_op_done()                                                              \
    do {                                                                            \
        while ((readl(NFC_IPC_REG) & NFC_IPC_INT) == 0)  \
            {} \
        write_nfc_ip_reg(0, NFC_IPC_REG); \
    } while (0)

static void write_nfc_ip_reg(u32 val, u32 reg)
{
    writel(NFC_IPC_CREQ, NFC_IPC_REG);
    while((readl(NFC_IPC_REG) & NFC_IPC_CACK) == 0);

    writel(val, reg);
    writel((readl(NFC_IPC_REG) & ~NFC_IPC_CREQ), NFC_IPC_REG);
}

/*!
 * NAND flash data output operation (reading data from NAND flash)
 * @param buf_no    internal ram buffer number that will contain data
 *                  to be outputted from the NAND flash after operation done
 * @param mode      one of the mode defined in enum nfc_output_mode
 * @param ecc_en    1 - ecc enabled; 0 - ecc disabled
 */
static void NFC_DATA_OUTPUT(enum nfc_internal_buf buf_no, enum nfc_output_mode mode,
                            int ecc_en)
{
    u32 v = readl(NFC_FLASH_CONFIG2_REG);

    if ((v & NFC_FLASH_CONFIG2_ECC_EN) != 0 && ecc_en == 0) {
        write_nfc_ip_reg(v & ~NFC_FLASH_CONFIG2_ECC_EN, NFC_FLASH_CONFIG2_REG);
    }
    if ((v & NFC_FLASH_CONFIG2_ECC_EN) == 0 && ecc_en != 0) {
        write_nfc_ip_reg(v | NFC_FLASH_CONFIG2_ECC_EN, NFC_FLASH_CONFIG2_REG);
    }

    v = readl(NAND_CONFIGURATION1_REG);

    if (mode == FDO_SPARE_ONLY) {
        v = (v & ~0x71) | buf_no | NAND_CONFIGURATION1_SP_EN;
    } else {
        v = (v & ~0x71) | buf_no;
    }

    writel(v, NAND_CONFIGURATION1_REG);

    writel(mode & 0xFF, NAND_LAUNCH_REG);
    wait_op_done();
}

static void NFC_CMD_INPUT(u32 cmd)
{
    writel(cmd & 0xFFFF, NAND_CMD_REG);
    writel(NAND_LAUNCH_FCMD, NAND_LAUNCH_REG);
    wait_op_done();
}

static void NFC_SET_NFC_ACTIVE_CS(u32 cs_line)
{
    u32 v;

    v = readl(NAND_CONFIGURATION1_REG) & (~0x7071);
    v |= (cs_line << 12);
    writel(v, NAND_CONFIGURATION1_REG);
}

static u16 NFC_STATUS_READ(void)
{
    u32 status;
    u16 status_sum = 0;
    int i;

#ifdef IMX51_TO_2
    write_nfc_ip_reg((readl(NFC_IPC_REG) & ~NFC_IPC_INT), NFC_IPC_REG);
    /* Wait till NAND is not busy */
    do {
        writel(NAND_LAUNCH_AUTO_STAT, NAND_LAUNCH_REG);
        wait_op_done();
        status = (readl(NAND_CONFIGURATION1_REG) & 0x00FF0000) >> 16;
    } while ((status & 0x40) == 0); // make sure I/O 6 == 1
    return readl(NAND_STATUS_SUM_REG);
#else
    /* Cannot rely on STATUS_SUM register due to errata */
    for (i = 0; i < num_of_nand_chips; i++) {
        NFC_SET_NFC_ACTIVE_CS(i);
        do {
            writel(NAND_LAUNCH_AUTO_STAT, NAND_LAUNCH_REG);
            status = (readl(NAND_CONFIGURATION1_REG) & 0x00FF0000) >> 16;
        } while ((status & 0x40) == 0); // make sure I/O 6 == 1
        /* Get Pass/Fail status */
        status = (readl(NAND_CONFIGURATION1_REG) >> 16) & 0x1;
        status_sum |= (status << i);
    }
    return status_sum;
#endif
}

/* This function uses a global variable for the page size. It shouldn't be a big
 * problem since we don't expect mixed page size nand flash parts on the same IC.
 * Note for address 0, it will always be correct regardless the page size. So for
 * ID read, it doesn't need to have the correct page size global variable first.
 */
static void start_nfc_addr_ops(u32 ops, u32 pg_no, u16 pg_off, u32 is_erase, u32 cs_line, u32 num_of_chips)
{
    u32 add0, add8, page_number;
    int num_of_bits[] = {0, 0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4};

    if (ops == FLASH_Read_ID) {
        // issue addr cycle
        writel(0x0, NAND_ADD0_REG + (4 * cs_line));
        writel(NAND_LAUNCH_FADD, NAND_LAUNCH_REG);
        wait_op_done();
        return;
    }

    if (num_of_chips >  1) {
        page_number = (pg_no << num_of_bits[num_of_chips]) | (cs_line & (num_of_chips - 1));
    } else {
        page_number = pg_no;
    }
    if (is_erase) {
        add0 = page_number;
        add8 = 0;
    } else {
        // for both read and write
        if (g_is_2k_page || g_is_4k_page) {
            // the first two addr cycles are for column addr. Page number starts
            // from the 3rd addr cycle.
            add0 = pg_off | (page_number << 16);
            add8 = page_number >> 16;
        } else {
            diag_printf("too bad, die\n");
            asm("1: b 1b");
            // For 512B page, the first addr cycle is for column addr. Page number
            // starts from the 2nd addr cycle.
            add0 = (pg_off & 0xFF) | (page_number << 8);
            add8 = page_number >> 24;
        }
    }
    writel(add0, NAND_ADD0_REG);
    writel(add8, NAND_ADD8_REG);
}

/*
 * Do a page read at random address
 *
 * @param pg_no             page number offset from 0
 * @param pg_off             byte offset within the page
 * @param ecc_force        can force ecc to be off. Otherwise, by default it is on
 *                                    unless the page offset is non-zero
 * @param cs_line            indicates which NAND of interleaved NAND devices is used
 *
 * @return  0 if successful; non-zero otherwise
 */
static int nfc_read_pg_random(u32 pg_no, u32 pg_off, u32 ecc_force, u32 cs_line, u32 num_of_chips)
{
    u32 ecc = NFC_FLASH_CONFIG2_ECC_EN;
    u32 v, res = 0;

    // clear the NAND_STATUS_SUM_REG register
    writel(0, NAND_STATUS_SUM_REG);

    // the 2nd condition is to test for unaligned page address -- ecc has to be off.
    if (ecc_force == ECC_FORCE_OFF || pg_off != 0 ) {
        ecc = 0;
    }

    // Take care of config1 for RBA and SP_EN
    v = readl(NAND_CONFIGURATION1_REG) & (~0x71);
    writel(v, NAND_CONFIGURATION1_REG);

    // For ECC
    v = readl(NFC_FLASH_CONFIG2_REG) & (~NFC_FLASH_CONFIG2_ECC_EN);
    // setup config2 register for ECC enable or not
    write_nfc_ip_reg(v | ecc, NFC_FLASH_CONFIG2_REG);

    start_nfc_addr_ops(FLASH_Read_Mode1, pg_no, pg_off, 0, cs_line, num_of_chips);

    if (g_is_2k_page || g_is_4k_page) {
        // combine the two commands for 2k/4k page read
        writel((FLASH_Read_Mode1_LG << 8) | FLASH_Read_Mode1, NAND_CMD_REG);
    } else {
        // just one command is enough for 512 page
        writel(FLASH_Read_Mode1, NAND_CMD_REG);
    }

    // start auto-read
    writel(NAND_LAUNCH_AUTO_READ, NAND_LAUNCH_REG);
    wait_op_done();

    v = readl(NAND_STATUS_SUM_REG);
    // test for CS0 ECC error from the STATUS_SUM register
    if ((v & (0x0100 << cs_line)) != 0) {
        // clear the status
        writel((0x0100 << cs_line), NAND_STATUS_SUM_REG);
        diag_printf("ECC error from NAND_STATUS_SUM_REG(0x%x) = 0x%x\n",
                    NAND_STATUS_SUM_REG, v);
        diag_printf("NAND_ECC_STATUS_RESULT_REG(0x%x) = 0x%x\n", NAND_ECC_STATUS_RESULT_REG,
                    readl(NAND_ECC_STATUS_RESULT_REG));
        res = -1;
#ifdef MX51_ERDOS
    } else {
        /*
         * correct symbol
         */
        u32 eccs = readl(NAND_ECC_STATUS_RESULT_REG);
        ecc_correct += ((eccs >> (4 * cs_line)) & 0x0F);
#endif /* MX51_ERDOS */
    }
    return res;
}

/*!
 * The NFC has to be preset before performing any operation
 */
static void NFC_PRESET(u32 max_block_count)
{
    // not needed. It is done in plf_hardware_init()
}

#endif // _MXC_NFC_V3_H_
