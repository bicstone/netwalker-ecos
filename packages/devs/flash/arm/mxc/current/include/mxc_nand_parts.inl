#ifndef CYGONCE_DEVS_FLASH_MXC_NAND_PARTS_INL
#define CYGONCE_DEVS_FLASH_MXC_NAND_PARTS_INL
//==========================================================================
//
//      mxc_nfc.h
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
// Date:         2006-01-23
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================

    {
        device_id  : 0x35ec, // Samsung K9F5608x0C (on EVB SDR memory card)
        device_id2 : 0xFFFF,
        device_id3 : 0xFFFF,
        device_id4 : 0xFFFF,
        col_cycle: 1,
        row_cycle: 2,
        page_size  : 512,
        spare_size : 16,
        pages_per_block : 32,
        block_size : 0x4000,
        block_count: 2048,
        device_size: 0x2000000,
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_SLC,
        options    : NAND_BBT_SCAN1STPAGE | NAND_BBT_SCAN2NDPAGE,
        fis_start_addr: 0x80000,       // first 0.5MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is fixed at 5th byte in the spare area. This value is not used
        bi_off     : 0,
        vendor_info: "Samsung K9F5608x0C 8-bit 512B page 32MB",
    },
    {
        device_id  : 0x36ec, // Samsung K9F1208R0B (on MXC91131 EVB mem1)
        device_id2 : 0xFFFF,
        device_id3 : 0xFFFF,
        device_id4 : 0xFFFF,
        col_cycle: 1,
        row_cycle: 3,
        page_size  : 512,
        spare_size : 16,
        pages_per_block : 32,
        block_size : 0x4000,
        block_count: 4096,
        device_size: 0x4000000,
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_SLC,
        options    : NAND_BBT_SCAN1STPAGE | NAND_BBT_SCAN2NDPAGE,
        fis_start_addr: 0x80000,       // first 0.5MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is fixed at 5th byte in the spare area. This value is not used
        bi_off     : 0,
        vendor_info: "Samsung K9F1208R0B 8-bit 512B page 64MB",
    },
    {
        device_id  : 0x76ec, // Samsung K9F1208x0B
        device_id2 : 0xFFFF,
        device_id3 : 0xFFFF,
        device_id4 : 0xFFFF,
        col_cycle: 1,
        row_cycle: 3,
        page_size  : 512,
        spare_size : 16,
        pages_per_block : 32,
        block_size : 0x4000,
        block_count: 4096,
        device_size: 0x4000000,
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_SLC,
        options    : NAND_BBT_SCAN1STPAGE | NAND_BBT_SCAN2NDPAGE,
        fis_start_addr: 0x80000,       // first 0.5MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is fixed at 5th byte in the spare area. This value is not used
        bi_off     : 0,
        vendor_info: "Samsung K9F1208x0B 8-bit 512B page 64MB",
    },
    {
        device_id  : 0x79ec, // Samsung K9K1G08x0B (MX31 ADS 512B page 8 bit)
        device_id2 : 0xFFFF,
        device_id3 : 0xFFFF,
        device_id4 : 0xFFFF,
        col_cycle: 1,
        row_cycle: 3,
        page_size  : 512,
        spare_size : 16,
        pages_per_block : 32,
        block_size : 0x4000,
        block_count: 4096 * 2,
        device_size: 0x4000000 * 2,
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_SLC,
        options    : NAND_BBT_SCAN1STPAGE | NAND_BBT_SCAN2NDPAGE,
        fis_start_addr: 0x80000,       // first 0.5MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is fixed at 5th byte in the spare area. This value is not used
        bi_off     : 0,
        vendor_info: "Samsung K9K1G08x0B 8-bit 512B page 128MB",
    },
    {
        device_id  : 0xf1ec, // Samsung K9F1G08U0A (MX31 ADS 2KB page 8 bit nand)
        device_id2 : 0xFFFF,
        device_id3 : 0xFFFF,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 2,
        page_size  : 512*4,
        spare_size : 16*4,
        pages_per_block : 64,
        block_size : 64*2*1024,
        block_count: 1024,
        device_size: 128*1024*1024, // 128MB device =0x08000000
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_SLC,
        options    : NAND_BBT_SCAN1STPAGE | NAND_BBT_SCAN2NDPAGE,
        fis_start_addr: 0x80000,       // first 0.5MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is at 4096th byte out of factory (0-indexed)
            // our NFC read out data like this:
            // | 528 | 528 | 528 | 528 | 528 | 528 | 528 | 528 |
            //    P1      P2       P3         P4       P5        P6        P7        P8
            // |0-527|528-1055/1056-1583/1584-2111/2112-2639/2640-3167/3168-3695/3696-4223 |
            // So the last subpage starts: 3696th byte. 4096th byte is at offset 400.
        bi_off     : 7 * 512 + 400,
        vendor_info: "Samsung K9F1G08U0A 8-bit 2K page 128MB",
    },
    {
        device_id  : 0xa1ec, // Samsung K9F1G08R0A (2KB page 8 bit nand)
        device_id2 : 0xFFFF,
        device_id3 : 0xFFFF,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 2,
        page_size  : 512*4,
        spare_size : 16*4,
        pages_per_block : 64,
        block_size : 64*2*1024,
        block_count: 1024,
        device_size: 0x08000000, // 128MB device =0x08000000
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_SLC,
        fis_start_addr: 0x80000,       // first 0.5MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is at 2048th byte out of factory (0-indexed)
            // our NFC read out data like this:
            // | 528 | 528 | 528 | 528 |
            //    P1      P2       P3         P4
            // 0-527|528-1055/1056-1583/1584-2111
            // So the last subpage starts: 1584th byte. 2048th byte is at offset 464.
        bi_off     : 3 * 512 + 464, // BUF3 offset + 464
        options    : NAND_BBT_SCAN1STPAGE | NAND_BBT_SCAN2NDPAGE,
        vendor_info: "Samsung K9F1G08R0A 8-bit 2K page 128MB",
    },
    {
        device_id  : 0xaaec, // Samsung K9F2G08R0A (2KB page 8 bit nand)
        device_id2 : 0xFFFF,
        device_id3 : 0xFFFF,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 3,
        page_size  : 512*4,
        spare_size : 16*4,
        pages_per_block : 64,
        block_size : 64*2*1024,
        block_count: 2048,
        device_size: 0x10000000, // 256MB device =0x10000000
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_SLC,
        fis_start_addr: 0x80000,       // first 0.5MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is at 2048th byte out of factory (0-indexed)
            // our NFC read out data like this:
            // | 528 | 528 | 528 | 528 |
            //    P1      P2       P3         P4
            // 0-527|528-1055/1056-1583/1584-2111
            // So the last subpage starts: 1584th byte. 2048th byte is at offset 464.
        bi_off     : 3 * 512 + 464, // BUF3 offset + 464
        options    : NAND_BBT_SCAN1STPAGE | NAND_BBT_SCAN2NDPAGE,
        vendor_info: "Samsung K9F2G08R0A 8-bit 2K page 256MB",
    },
    {
        device_id  : 0xd5ec, // Samsung K9LAG08U0M (2KB page 2G x 8 bit MLC nand)
        device_id2 : 0x2555, // interleaved NAND used on MX51 TO 1.0
        device_id3 : 0xFFFF,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 3,
        page_size  : 512*4,
        spare_size : 16*4,
        pages_per_block : 128,
        block_size : 128*2*1024,
        block_count: 8192,
        device_size: 0x80000000, // 2GB device =0x8000,0000
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_MLC,
        fis_start_addr: 0x80000,       // first 0.5MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is at 2048th byte out of factory (0-indexed)
            // our NFC read out data like this:
            // | 528 | 528 | 528 | 528 |
            //    P1      P2       P3         P4
            // 0-527|528-1055/1056-1583/1584-2111
            // So the last subpage starts: 1584th byte. 2048th byte is at offset 464.
        bi_off     : 3 * 512 + 464, // BUF3 offset + 464
        options    : NAND_BBT_SCANLSTPAGE,
        vendor_info: "Samsung K9LAG08U0M 8-bit 2K page 2GB MLC",
    },
    {
        device_id  : 0xd3ec, // Samsung K9G8G08U0M (2KB page 1G x 8 bit MLC nand)
        device_id2 : 0x2514, //  default for MX51
        device_id3 : 0xFFFF,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 3,
        page_size  : 512*4,
        spare_size : 16*4,
        pages_per_block : 128,
        block_size : 128*2*1024,
        block_count: 4096,
        device_size: 0x40000000,
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_MLC,
        fis_start_addr: 0x80000,       // first 0.5MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is at 2048th byte out of factory (0-indexed)
            // our NFC read out data like this:
            // | 528 | 528 | 528 | 528 |
            //    P1      P2       P3         P4
            // 0-527|528-1055/1056-1583/1584-2111
            // So the last subpage starts: 1584th byte. 2048th byte is at offset 464.
        bi_off     : 3 * 512 + 464, // BUF3 offset + 464
        options    : NAND_BBT_SCANLSTPAGE,
        vendor_info: "Samsung K9G8G08U0M 8-bit 2K page 1GB MLC",
    },
#ifdef MX51_ERDOS
    {
        device_id  : 0xd5ec,  // Samsung K9GAG08U0M 8-bit 4K page 2GB MLC.
        device_id2 : 0xb614,
        device_id3 : 0xec74,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 3,
        page_size  : 512*8,	// 4096bytes
        spare_size : 16*8,	//  128bytes
        pages_per_block : 128,
        block_size : 128*4*1024,
        block_count: 4096,
        device_size: 0x80000000,
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_MLC,
        options    : NAND_BBT_SCANLSTPAGE,
        fis_start_addr: 0x000000,
        bbt_blk_max_nr: 4,	// (reserve 4 blocks for the bad block tables)
#ifdef NFC_V3_0
        bi_off     : 8 * 512,
#else
            // | 528 | 528 | 528 | 528 | 528 | 528 | 528 | 528 |
            //    P1      P2       P3         P4       P5        P6        P7        P8
            // |0-527|528-1055/1056-1583/1584-2111/2112-2639/2640-3167/3168-3695/3696-4223 |
            // So the last subpage starts: 3696th byte. 4096th byte is at offset 400.
        bi_off     : 7 * 512 + 400,
#endif /* NFC_V3_0 */
        vendor_info: "Samsung K9GAG08U0M 8-bit 4K page 2GB MLC.",
    },
    {
        device_id  : 0xd5ec,  // Samsung K9GAG08U0D 8-bit 4K page 2GB MLC.
        device_id2 : 0x2994,
        device_id3 : 0x4134,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 3,
        page_size  : 512*8,	// 4096bytes
        spare_size : 26*8+10,	//  218bytes
        pages_per_block : 128,
        block_size : 128*4*1024,
        block_count: 4096,
        device_size: 0x80000000,
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_MLC,
        options    : NAND_BBT_SCANLSTPAGE,
        fis_start_addr: 0x000000,
        bbt_blk_max_nr: 4,	// (reserve 4 blocks for the bad block tables)
#ifdef NFC_V3_0
        bi_off     : 8 * 512,
#else
            // BI is at 4096th byte out of factory (0-indexed)
            // our NFC read out data like this:
            // | 538 | 538 | 538 | 538 | 538 | 538 | 538 | 538 |  (538=512+26)
            //    P1      P2       P3         P4       P5        P6        P7        P8
            // |0-537|538-1075/1076-1613/1614-2151/2152-2689/2690-3227/3228-3765/3766-4313 |
            // So the last subpage starts: 3766th byte. 4096th byte is at offset 330.
        bi_off     : 7 * 512 + 330,
#endif /* NFC_V3_0 */
        vendor_info: "Samsung K9GAG08U0U 8-bit 4K page 2GB MLC.",
    },
#endif /* MX51_ERDOS */
    {
        device_id  : 0xd5ec,
        device_id2 : 0xb614,
        device_id3 : 0xec74,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 3,
        page_size  : 512*4,
        spare_size : 16*4,
        pages_per_block : 128,
        block_size : 128*2*1024,
        block_count: 8192,
        device_size: 0x80000000, // 2GB device =0x8000,0000
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_MLC,
        vendor_info: "Samsung K9HCG08U5M 8-bit 2K page 8GB Quad MLC",
    },
    {
        device_id  : 0xd7ec,  // Samsung K9LBG08U0M 8-bit 4K page 4GB MLC. - used on MX37
        device_id2 : 0xb655,
        device_id3 : 0xec78,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 3,
        page_size  : 512*8,
        spare_size : 16*8,
        pages_per_block : 128,
        block_size : 128*4*1024,
        block_count: 8192 / 2,  // for now
        device_size: 0x80000000, // only 2GB supported
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_MLC,
        options    : NAND_BBT_SCANLSTPAGE,
        fis_start_addr: 0x100000,       // first 1MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is at 4096th byte out of factory (0-indexed)
            // our NFC read out data like this:
            // | 528 | 528 | 528 | 528 | 528 | 528 | 528 | 528 |
            //    P1      P2       P3         P4       P5        P6        P7        P8
            // |0-527|528-1055/1056-1583/1584-2111/2112-2639/2640-3167/3168-3695/3696-4223 |
            // So the last subpage starts: 3696th byte. 4096th byte is at offset 400.
        bi_off     : 7 * 512 + 400,
        vendor_info: "Samsung K9LBG08U0M 8-bit 4K page 4GB MLC. Only 2GB supported.",
    },
    {
        device_id  : 0xD5AD, // Hynix HY27UV08BG5M 8-bit 2K page ?? GB MLC nand
        device_id2 : 0xA555,
        device_id3 : 0xAD68,
        col_cycle: 2,
        row_cycle: 3,
        page_size  : 512*4,
        spare_size : 16*4,
        pages_per_block : 128,
        block_size : 128*2*1024,
        block_count: 2*2* 2048,
        device_size: 0x80000000, // 2GB device
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_MLC,
        vendor_info: "Hynix HY27UV08BG5M 8-bit 2K page ?? GB MLC nand",
    },
    {
        device_id  : 0xAD, // Hynix HYD0SQH0MF3(P) 16-bit 2K page 128MB (1Gb) MLC nand
        device_id2 : 0xB1,
        device_id3 : 0x80,
        device_id4 : 0x55,
        col_cycle: 2,
        row_cycle: 2,
        page_size  : 512*4,
        spare_size : 16*4,
        pages_per_block : 64,
        block_size : 64*2*1024,
        block_count: 1024,
        device_size: 0x08000000, // 128MB device =0x0800,0000
        port_size  : MXC_NAND_16_BIT,
        type       : NAND_MLC,
        vendor_info: "Hynix HYD0SQH0MF3(P) 16-bit 2K page 128MB MLC nand",
    },
    {
        // Micron 29F32G08TAA 8-bit 2K page 4GB (32Gb) nand
        // Even though it is 4GB device, so far we only use 2GB. Will work on it more
        // once we have the schematic for this MX32 3DS board with Wolfson
        // Note: this device doesn't work for NAND boot since it requires a
        //       "reset" command issued to the NAND flash which is missing
        //       from our NFC controller on i.MX31/32 and earlier.
        device_id  : 0xD52C,
        device_id2 : 0xA5D5,
        device_id3 : 0xFFFF,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 3,
        page_size  : 512*4,
        spare_size : 16*4,
        pages_per_block : 128,
        block_size : 128*2*1024,
        block_count: 2 * 2 * 2048,
        device_size: 0x80000000, // 2GB device
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_MLC,
        vendor_info: "Micron 29F32G08TAA 16-bit 2K page 4GB (32Gb) nand",
    },
    {
        // Micron MT29F8G08AAA 8-bit 4K page 1GB (8Gb) nand, 218B spare
        device_id  : 0xD32C,
        device_id2 : 0x2E90,
        device_id3 : 0xFFFF,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 3,
        page_size  : 512*8,
        spare_size : 218,
        pages_per_block : 64,
        block_size : 128*2*1024,
        block_count: 2 * 2048,
        device_size: 0x40000000, // 2GB device
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_SLC,
        options    : NAND_BBT_SCANLSTPAGE,
        fis_start_addr: 0x100000,       // first 1MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is at 4096th byte out of factory (0-indexed)
            // our NFC read out data like this:
            // | 538 |   538  |   538   |    538  |   538   |  538    |   538   |  538     |
            //    P1      P2       P3         P4       P5       P6         P7       P8
            // |0-537|538-1075|1076-1613|1614-2151|2152-2689|2690-3227|3228-3765|3766-4303 |
            // So the last subpage starts: 3696th byte. 4096th byte is at offset 330.
        bi_off     : 7 * 512 + 330,
        vendor_info: "Micron MT29F8G08AAA 8-bit 4K page 1GB (8Gb) nand, 218B spare",
    },
    {
        // Micron MT29F32G08QAA 8-bit 4K page 4GB (32Gb) nand, 218B spare
        device_id  : 0xD52C,
        device_id2 : 0x3E94,
        device_id3 : 0xFF74,
        device_id4 : 0xFFFF,
        col_cycle: 2,
        row_cycle: 3,
        page_size  : 512*8,
        spare_size : 218,
        pages_per_block : 128,
        block_size : 128*8*512,
        block_count: 4096,
        device_size: 0x80000000, // 2GB device
        port_size  : MXC_NAND_8_BIT,
        type       : NAND_MLC,
        options    : NAND_BBT_SCANLSTPAGE,
        fis_start_addr: 0x100000,       // first 1MB reserved for Redboot
        bbt_blk_max_nr: 4,      // reserve 4 blocks for the bad block tables
            // BI is at 4096th byte out of factory (0-indexed)
            // our NFC read out data like this:
            // | 538 |   538  |   538   |    538  |   538   |  538    |   538   |  538     |
            //    P1      P2       P3         P4       P5       P6         P7       P8
            // |0-537|538-1075|1076-1613|1614-2151|2152-2689|2690-3227|3228-3765|3766-4303 |
            // So the last subpage starts: 3696th byte. 4096th byte is at offset 330.
        bi_off     : 7 * 512 + 330,
        vendor_info: "Micron MT29F32G08QAA 8-bit 4K page 4GB (32Gb) nand, 218B spare",
    },
#endif // CYGONCE_DEVS_FLASH_MXC_NAND_PARTS_INL
