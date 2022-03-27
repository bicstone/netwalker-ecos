#ifndef _IMX_SPI_NOR_H_
#define _IMX_SPI_NOR_H_
//==========================================================================
//
//      imx_nfc.h
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
// Date:         2008-11-14
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================

// dummy defines - not used
#define CYGNUM_FLASH_INTERLEAVE         1
#define CYGNUM_FLASH_SERIES             1
#define CYGNUM_FLASH_WIDTH              8
#define CYGNUM_FLASH_BASE               0
#define CYGNUM_FLASH_BLANK              1

#define READ        0x03    // tx: 1 byte cmd + 3 byte addr; rx: variable bytes
#define READ_HS     0x0B    // tx: 1 byte cmd + 3 byte addr + 1 byte dummy; rx: variable bytes
#define RDSR        0x05    // read status register 1 byte tx cmd + 1 byte rx status
    #define RDSR_BUSY       (1 << 0)    // 1=write-in-progress (default 0)
    #define RDSR_WEL        (1 << 1)    // 1=write enable (default 0)
    #define RDSR_BP0        (1 << 2)    // block write prot level (default 1)
    #define RDSR_BP1        (1 << 3)    // block write prot level (default 1)
    #define RDSR_BP2        (1 << 4)    // block write prot level (default 1)
    #define RDSR_BP3        (1 << 5)    // block write prot level (default 1)
    #define RDSR_AAI        (1 << 6)    // 1=AAI prog mode; 0=byte prog (default 0)
    #define RDSR_BPL        (1 << 7)    // 1=BP3,BP2,BP1,BP0 RO; 0=R/W (default 0)    
#define WREN        0x06    // write enable. 1 byte tx cmd
#define WRDI        0x04    // write disable. 1 byte tx cmd
#define EWSR        0x50    // Enable write status. 1 byte tx cmd
#define WRSR        0x01    // Write status register. 1 byte tx cmd + 1 byte tx value
#define ERASE_4K    0x20    // sector erase. 1 byte cmd + 3 byte addr
#define ERASE_32K   0x52    // 32K block erase. 1 byte cmd + 3 byte addr
#define ERASE_64K   0xD8    // 64K block erase. 1 byte cmd + 3 byte addr
#define ERASE_CHIP  0x60    // whole chip erase
#define BYTE_PROG   0x02    // all tx: 1 cmd + 3 addr + 1 data
#define AAI_PROG    0xAD    // all tx: [1 cmd + 3 addr + 2 data] + RDSR
                            //   + [1cmd + 2 data] + .. + [WRDI] + [RDSR]
#define JEDEC_ID    0x9F    // read JEDEC ID. tx: 1 byte cmd; rx: 3 byte ID


/* Atmel SPI-NOR commands */
#define WR_2_MEM_DIR  0x82
#define BUF1_WR             0x84
#define BUF2_WR             0x87
#define BUF1_TO_MEM     0x83
#define BUF2_TO_MEM     0x86
#define STAT_READ          0xD7
    #define STAT_PG_SZ    (1 << 0)  // 1=Page size is 512, 0=Page size is 528 (default 0)
    #define STAT_PROT      (1 << 1)  // 1=sector protection enabled (default 0)
    #define STAT_COMP      (1 << 6)
    #define STAT_BUSY      (1 << 7) // 1=Device not busy
#define CONFIG_REG1      0x3D
#define CONFIG_REG2      0x2A
#define CONFIG_REG3      0x80
#define CONFIG_REG4      0xA6

#define SZ_64K      0x10000
#define SZ_32K      0x8000
#define SZ_4K       0x1000
#define TRANS_FAIL  -1

extern imx_spi_init_func_t *spi_nor_init;
extern imx_spi_xfer_func_t *spi_nor_xfer;
extern struct imx_spi_dev imx_spi_nor;
static int spi_nor_status(void);
static int spi_nor_cmd_1byte(unsigned char cmd);
int spi_nor_erase_block(void* block_addr, unsigned int block_size);
static int spi_nor_write_status(unsigned char val);
int spi_nor_program_buf(void *addr, void *data, int len);

int spi_nor_program_buf_sst(void *addr, void *data, int len, unsigned int block_size);
int spi_nor_erase_block_sst(void* block_addr, unsigned int block_size);

int spi_nor_program_buf_atm(void *addr, void *data, int len, unsigned int block_size);
int spi_nor_erase_block_atm(void* block_addr, unsigned int block_size);

typedef int imx_spi_write_func_t(void *addr, void *data, int len, unsigned int block_size);
typedef int imx_spi_erase_func_t(void* block_addr, unsigned int block_size);

#endif // _IMX_SPI_NOR_H_
