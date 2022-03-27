//==========================================================================
//
//      spi_nor.c
//
//      SPI NOR flash support
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
// 2009/07/03 : unsupport_cmd_AAI support.
// 2009/08/02 : redboot_200929 base,  fixed flash_dev_query() -> spi_norflash_query().
// 2009/08/06 : address range check add.
// 2009/08/07 : spi_nor_read() diag_printf delete.
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include <redboot.h>
#include CYGHWR_MEMORY_LAYOUT_H
#include <cyg/hal/hal_io.h>
#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

#include <cyg/io/imx_spi.h>
#include <cyg/io/imx_spi_nor.h>

static unsigned char g_tx_buf[256];
static unsigned char g_rx_buf[256];
static int spi_nor_init_ok;

#define WRITE_ENABLE()          spi_nor_cmd_1byte(WREN)
#define WRITE_DISABLE()         spi_nor_cmd_1byte(WRDI)
#define ENABLE_WRITE_STATUS()   spi_nor_cmd_1byte(EWSR)

#ifdef MX51_ERDOS
static int queryNoVerbose;
static int hwr_initialized;
#endif /* MX51_ERDOS */

#ifndef MXCFLASH_SELECT_MULTI
void flash_query(void* data)
#else
void spi_norflash_query(void* data)
#endif
{
    unsigned char tmp[4];
    unsigned char *ptr = (unsigned char *)data;

    g_tx_buf[3] = JEDEC_ID;
    if (spi_nor_xfer(&imx_spi_nor, g_tx_buf, tmp, 4) != 0) {
        return;
    }
#ifdef MX51_ERDOS
    if (queryNoVerbose == 0) {
        diag_printf("JEDEC ID: 0x%02x:0x%02x:0x%02x\n", tmp[2], tmp[1], tmp[0]);
    }
#else
    diag_printf("JEDEC ID: 0x%02x:0x%02x:0x%02x\n", tmp[2], tmp[1], tmp[0]);
#endif /* MX51_ERDOS */
    ptr[0] = tmp[2];
    ptr[1] = tmp[1];
    ptr[2] = tmp[0];
}

#ifndef MXCFLASH_SELECT_MULTI
int flash_program_buf(void* addr, void* data, int len)
#else
int spi_norflash_program_buf(void* addr, void* data, int len)
#endif
{
    return spi_nor_program_buf(addr, data, len);
}

#ifndef MXCFLASH_SELECT_MULTI
int flash_erase_block(void* block, unsigned int size)
#else
int spi_norflash_erase_block(void* block, unsigned int size)
#endif
{
    return spi_nor_erase_block(block, size);
}

#ifndef MXCFLASH_SELECT_MULTI
bool flash_code_overlaps(void *start, void *end)
#else
bool spi_norflash_code_overlaps(void *start, void *end)
#endif
{
    extern unsigned char _stext[], _etext[];

    return ((((unsigned long)&_stext >= (unsigned long)start) &&
             ((unsigned long)&_stext < (unsigned long)end)) ||
            (((unsigned long)&_etext >= (unsigned long)start) &&
             ((unsigned long)&_etext < (unsigned long)end)));
}

#ifndef MXCFLASH_SELECT_MULTI
int flash_hwr_map_error(int e)
#else
int spi_norflash_hwr_map_error(int e)
#endif
{
    return e;
}

//----------------------------------------------------------------------------
// Now that device properties are defined, include magic for defining
// accessor type and constants.
#include <cyg/io/flash_dev.h>

// Information about supported devices
typedef struct flash_dev_info {
    cyg_uint8   device_id;
    cyg_uint8   device_id2;
    cyg_uint8   device_id3;
    cyg_uint8   device_id4;
    cyg_uint32  block_size;
    cyg_int32   block_count;
    cyg_uint32  device_size;
    cyg_uint32  fis_start_addr;
    cyg_uint8   vendor_info[96];
#ifdef MX51_ERDOS
    cyg_uint8   unsupport_cmd_AAI;	/* UNsupport Auto Address Increment */
#endif /* MX51_ERDOS */
} __attribute__((aligned(4),packed))flash_dev_info_t;

static const flash_dev_info_t* flash_dev_info;
static const flash_dev_info_t supported_devices[] = {
#include <cyg/io/spi_nor_parts.inl>
};

#define NUM_DEVICES (sizeof(supported_devices)/sizeof(flash_dev_info_t))

#define ASSERT_SPI_NOR_INIT()       \
    do {                                                                    \
        if (spi_nor_init(&imx_spi_nor) != 0) {                              \
            diag_printf("Error: failed to initialize SPI NOR\n");           \
            return -1;                                                      \
        }                                                                   \
    } while (0);                                                            \

int
#ifndef MXCFLASH_SELECT_MULTI
flash_hwr_init(void)
#else
spi_norflash_hwr_init(void)
#endif
{
    cyg_uint8 id[4];
    int i;

    if (!spi_nor_init_ok) {
        diag_printf("Initializing SPI-NOR flash...\n");
        if (spi_nor_init(&imx_spi_nor) != 0) {
            diag_printf("Error: failed to initialize SPI NOR\n");
            return -1;
        }
        spi_nor_init_ok = 1;
    }
    // Look through table for device data
#if 1
    spi_norflash_query(id);
#else
    flash_dev_query(id);
#endif
    flash_dev_info = supported_devices;
    for (i = 0; i < NUM_DEVICES; i++) {
        if ((flash_dev_info->device_id == id[0]) &&
            (flash_dev_info->device_id2 == id[1]) &&
            (flash_dev_info->device_id3 == id[2]))
            break;
        flash_dev_info++;
    }

    // Do we find the device? If not, return error.
    if (NUM_DEVICES == i) {
        diag_printf("Unrecognized SPI NOR part: 0x%02x, 0x%02x, 0x%02x\n",
                    id[0], id[1], id[2]);
        return FLASH_ERR_DRV_WRONG_PART;
    }

    // Hard wired for now
    flash_info.block_size = flash_dev_info->block_size;
    flash_info.blocks = flash_dev_info->block_count;
    flash_info.start = (void *)0;
    flash_info.end = (void *)flash_dev_info->device_size;

    diag_printf("SPI NOR: block_size=0x%x, blocks=0x%x, start=%p, end=%p\n",
               flash_info.block_size, flash_info.blocks,
               flash_info.start, flash_info.end);

#ifdef MX51_ERDOS
    hwr_initialized = 1;
#endif /* MX51_ERDOS */
    return FLASH_ERR_OK;
}

#ifdef MX51_ERDOS
/*
 * spi_norForce_hwr_init : force re-initialize Serial-Flash
 *  used by hal/arm/mx51/erdos/current/src/redboot_cmds.c
 */
int spi_norForce_hwr_init(int silent)
{
    cyg_uint8 id[4];
    int i;
    int oquery = queryNoVerbose;

    if (hwr_initialized == 1) {
        return 0;	/* already initailized */
    }
    if (silent == 0) {
        diag_printf("Initializing(Force) SPI-NOR flash...\n");
        queryNoVerbose = 0;
    } else {
        queryNoVerbose = 1;
    }
    if (spi_nor_init(&imx_spi_nor) != 0) {
        diag_printf("Error: failed to initialize SPI NOR\n");
        return -1;
    }
    spi_nor_init_ok = 1;

    // Look through table for device data
    spi_norflash_query(id);
    flash_dev_info = supported_devices;
    for (i = 0; i < NUM_DEVICES; i++) {
        if ((flash_dev_info->device_id == id[0]) &&
            (flash_dev_info->device_id2 == id[1]) &&
            (flash_dev_info->device_id3 == id[2]))
            break;
        flash_dev_info++;
    }

    // Do we find the device? If not, return error.
    if (NUM_DEVICES == i) {
        diag_printf("Unrecognized SPI NOR part: 0x%02x, 0x%02x, 0x%02x\n",
                    id[0], id[1], id[2]);
        return FLASH_ERR_DRV_WRONG_PART;
    }

    // Hard wired for now
    flash_info.block_size = flash_dev_info->block_size;
    flash_info.blocks = flash_dev_info->block_count;
    flash_info.start = (void *)0;
    flash_info.end = (void *)flash_dev_info->device_size;

    if (silent == 0) {
        diag_printf("SPI NOR: [%s] block_size=0x%x, blocks=0x%x, start=%p, end=%p\n",
                   flash_dev_info->vendor_info, flash_info.block_size, flash_info.blocks,
                   flash_info.start, flash_info.end);
    }
    queryNoVerbose = oquery;
    hwr_initialized = 1;

    return FLASH_ERR_OK;
}
#endif /* MX51_ERDOS */

// used by redboot/current/src/flash.c
int mxc_spi_nor_fis_start(void)
{
    return (flash_dev_info->fis_start_addr);
}

static int spi_nor_cmd_1byte(unsigned char cmd)
{
    g_tx_buf[0] = cmd;
    if (spi_nor_xfer(&imx_spi_nor, g_tx_buf, g_rx_buf, 1) != 0) {
        diag_printf("Error: %s(): %d\n", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

/*!
 * Read from SPI NOR at src address to RAM at dest with len bytes
 * @param   src     source address in the flash
 * @param   dest    destination address in the RAM
 * @param   len     # of bytes to copy
 */
int spi_nor_read(void *src, void *dest, int len)
{
    unsigned int *cmd = (unsigned int *)g_tx_buf;
    unsigned int max_rx_sz = imx_spi_nor.fifo_sz - 4; // max rx bytes per burst
    unsigned char *d_buf = (unsigned char *) dest;
    unsigned char *s_buf;
    int i;

    imx_spi_nor.us_delay = 100;
    diag_printf1("%s(from flash=%p to ram=%p len=0x%x)\n", __FUNCTION__,
                src, dest, len);

    if (len == 0)
        return 0;


    if (((unsigned int)src + len) > flash_dev_info->device_size) {
	diag_printf("spi_nor_read: Invalid flash address or size 0x%x\n", flash_dev_info->device_size);
	return -1;
    }

    *cmd = (READ << 24) | ((unsigned int)src & 0x00FFFFFF);

    while (1) {
        if (len == 0) {
            imx_spi_nor.us_delay = 0;
            return 0;
        }
        if (len < max_rx_sz) {
            diag_printf1("last read len=0x%x\n", len);
            // deal with the last read
            if (spi_nor_xfer(&imx_spi_nor, g_tx_buf, g_rx_buf, len + 4) != 0) {
                diag_printf("Error: %s(%d): failed\n", __FILE__, __LINE__);
                return -1;
            }
            s_buf = g_rx_buf + 4;   // throw away 4 bytes (5th received bytes is real)
            // now adjust the endianness
            for (i = len; i >= 0; i -= 4, s_buf += 4) {
                if (i < 4) {
                    if (i == 1) {
                        *d_buf = s_buf[0];
                    } else if (i == 2) {
                        *d_buf++ = s_buf[1];
                        *d_buf++ = s_buf[0];
                    } else if (i == 3) {
                        *d_buf++ = s_buf[2];
                        *d_buf++ = s_buf[1];
                        *d_buf++ = s_buf[0];
                    }
                    imx_spi_nor.us_delay = 0;
                    return 0;
                }
                // copy 4 bytes
                *d_buf++ = s_buf[3];
                *d_buf++ = s_buf[2];
                *d_buf++ = s_buf[1];
                *d_buf++ = s_buf[0];
            }
        }
        // now grab max_rx_sz data (+4 is needed due to 4-throw away bytes
        if (spi_nor_xfer(&imx_spi_nor, g_tx_buf, g_rx_buf, max_rx_sz + 4) != 0) {
            diag_printf("Error: %s(%d): failed\n", __FILE__, __LINE__);
            return -1;
        }
        s_buf = g_rx_buf + 4;   // throw away 4 bytes (5th received bytes is real)
        // now adjust the endianness
        for (i = 0; i < max_rx_sz; i += 4, s_buf += 4) {
            *d_buf++ = s_buf[3];
            *d_buf++ = s_buf[2];
            *d_buf++ = s_buf[1];
            *d_buf++ = s_buf[0];
        }
        *cmd += max_rx_sz;  // increase # of bytes in NOR address as cmd == g_tx_buf
        len -= max_rx_sz;   // # of bytes left

        diag_printf1("d_buf=%p, g_rx_buf=%p, len=0x%x\n", d_buf, g_rx_buf, len);
    }

    imx_spi_nor.us_delay = 0;
}

static int spi_nor_program_1byte(unsigned char data, void *addr)
{
    unsigned int addr_val = (unsigned int) addr;

    // need to do write-enable command
    if (WRITE_ENABLE() != 0) {
        diag_printf("Error : %d\n", __LINE__);
        return -1;
    }
    g_tx_buf[0] = BYTE_PROG;    // need to skip bytes 1, 2, 3
    g_tx_buf[4] = data;
    g_tx_buf[5] = addr_val & 0xFF;
    g_tx_buf[6] = (addr_val >> 8) & 0xFF;
    g_tx_buf[7] = (addr_val >> 16) & 0xFF;

    diag_printf("0x%x: 0x%x\n", *(unsigned int*)g_tx_buf, *(unsigned int*)(g_tx_buf + 4));
    diag_printf("addr=0x%x\n", addr_val);

    if (spi_nor_xfer(&imx_spi_nor, g_tx_buf, g_rx_buf, 5) != 0) {
        diag_printf("Error: %s(%d): failed\n", __FILE__, __LINE__);
        return -1;
    }

    while (spi_nor_status() & RDSR_BUSY) {
    }
    return 0;
}

/*!
 * program data from RAM to flash
 * @param addr          destination address in flash
 * @param data          source address in RAM
 * @param len           # of bytes to program
 * Note: - when starting AAI programming, 
 *       1) the starting addr has to be 16-bit aligned
 *       2) the prog len has to be even number of bytes
 */
int spi_nor_program_buf(void *addr, void *data, int len)
{
    unsigned int d_addr = (unsigned int) addr;
    unsigned char *s_buf = (unsigned char *) data;

    if (len == 0)
        return 0;

    diag_printf1("%s(flash addr=%p, ram=%p, len=0x%x)\n", __FUNCTION__, addr, data, len);

    if (d_addr + len > flash_dev_info->device_size) {
	diag_printf("spi_nor_program_buf: Invalid flash address or size 0x%x\n", flash_dev_info->device_size);
	return -1;
    }

    imx_spi_nor.us_delay = 0;

    if (ENABLE_WRITE_STATUS() != 0 || spi_nor_write_status(0) != 0) {
        diag_printf("Error: %s: %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if ((d_addr & 1) != 0) {
        // program 1st byte
        if (spi_nor_program_1byte(s_buf[0], (void *)d_addr) != 0) {
            diag_printf("Error: %s(%d)\n", __FUNCTION__, __LINE__);
            return -1;
        }
        if (--len == 0)
            return 0;
        d_addr++;
        s_buf++;
    }

    // need to do write-enable command
    if (WRITE_ENABLE() != 0) {
        diag_printf("Error : %d\n", __LINE__);
        return -1;
    }

#ifdef MX51_ERDOS
    /*
     * check UNsupport command AAI
     */
    if (flash_dev_info->unsupport_cmd_AAI == 1) {
        int s_ofs = 0;
        /*
         *  SPI write data ex)10bytes (data 6bytes)
         *       +--------+--------+--------+--------+
         *    +0 |   -    |    -   |   cmd  | Adr(H) |
         *       +--------+--------+--------+--------+
         *    +4 | Adr(M) | Adr(L) | Data[0]| Data[1]|
         *       +--------+--------+--------+--------+
         *    +8 | Data[2]| Data[3]| Data[4]| Data[5]| *must fill 4bytes
         *       +--------+--------+--------+--------+
         */
        while ( 0 < len ) {
            int          slen, sz;
            unsigned int val;
            if (16 <= len) {
                sz = 16;
            } else {
                sz = len;
            }
            switch ( sz ) {
            case 16:
            case 12:
            case 8:
            case 4:
                /*
                 * cmd, address
                 */
                val = (BYTE_PROG << 24) | (d_addr & 0x00FFFFFF);
                memcpy (&(g_tx_buf [0]), &val, 4);
                /*
                 * data
                 */
                val = (s_buf [s_ofs + 0] << 24) | (s_buf [s_ofs + 1] << 16) |
                      (s_buf [s_ofs + 2] <<  8) | (s_buf [s_ofs + 3]);
                memcpy (&(g_tx_buf [4]), &val, 4);
                slen = 4;
                break;
            case 15:
            case 11:
            case 7:
            case 3:
                /*
                 * cmd, address
                 */
                val = (BYTE_PROG << 16) | ((d_addr & 0x00FFFF00) >> 8);
                memcpy (&(g_tx_buf [0]), &val, 4);
                /*
                 * address, data
                 */
                val = ((d_addr & 0x000000FF) << 24) | (s_buf [s_ofs + 0] << 16) |
                      (s_buf [s_ofs + 1] <<  8)     | (s_buf [s_ofs + 2]);
                memcpy (&(g_tx_buf [4]), &val, 4);
                slen = 3;
                break;
            case 14:
            case 10:
            case 6:
            case 2:
                /*
                 * cmd, address
                 */
                val = (BYTE_PROG << 8) | ((d_addr & 0x00FF0000) >> 16);
                memcpy (&(g_tx_buf [0]), &val, 4);
                /*
                 * address, data
                 */
                val = ((d_addr & 0x0000FFFF) << 16) |
                      (s_buf [s_ofs + 0] <<  8)     | (s_buf [s_ofs + 1]);
                memcpy (&(g_tx_buf [4]), &val, 4);
                slen = 2;
                break;
            default:
                /*
                 * cmd
                 */
                val = BYTE_PROG;
                memcpy (&(g_tx_buf [0]), &val, 4);
                /*
                 * address, data
                 */
                val = ((d_addr & 0x00FFFFFF) << 8) | (s_buf [s_ofs + 0]);
                memcpy (&(g_tx_buf [4]), &val, 4);
                slen = 1;
                break;
            }
            while ( slen < sz ) {
                val = (s_buf [s_ofs + slen + 0] << 24) |
                      (s_buf [s_ofs + slen + 1] << 16) |
                      (s_buf [s_ofs + slen + 2] <<  8) |
                      (s_buf [s_ofs + slen + 3]);
                slen += 4;
                memcpy (&(g_tx_buf [((slen + 3) / 4) * 4]), &val, 4);
            }

            /*
             * execute write
             */
            if (spi_nor_xfer(&imx_spi_nor, g_tx_buf, g_rx_buf, (4 + slen)) != 0) {
                diag_printf("Error: %s(%d): failed\n", __FILE__, __LINE__);
                return -1;
            }

            /*
             * wait ready
             */
            while (spi_nor_status() & RDSR_BUSY) {
            }

            /*
             * next data offset
             */
            len    -= slen;
            s_ofs  += slen;
            d_addr += slen;

            if ((s_ofs % flash_dev_info->block_size) == 0) {
                diag_printf(".");
            }

            /*
             * write execute
             */
            if (spi_nor_write_status(0) != 0) {
                diag_printf("Error: %s: %d\n", __FUNCTION__, __LINE__);
                return -1;
            }

            /*
             * write enable for next write
             */
            if (0 < len) {
                if (WRITE_ENABLE() != 0) {
                    diag_printf("Error: %s: %d\n", __FUNCTION__, __LINE__);
                    return -1;
                }
            }
        }

        WRITE_DISABLE();
        while (spi_nor_status() & RDSR_BUSY) {
        }

        return 0;
    }
#endif /* MX51_ERDOS */

    // These two bytes write will be copied to txfifo first with
    // g_tx_buf[1] being shifted out and followed by g_tx_buf[0].
    // The reason for this is we will specify burst len=6. So SPI will
    // do this kind of data movement.
    g_tx_buf[0] = d_addr >> 16;
    g_tx_buf[1] = AAI_PROG;    // need to skip bytes 1, 2
    // byte shifted order is: 7, 6, 5, 4
    g_tx_buf[4] = s_buf[1];
    g_tx_buf[5] = s_buf[0];
    g_tx_buf[6] = d_addr;
    g_tx_buf[7] = d_addr >> 8;    
    if (spi_nor_xfer(&imx_spi_nor, g_tx_buf, g_rx_buf, 6) != 0) {
        diag_printf("Error: %s(%d): failed\n", __FILE__, __LINE__);
        return -1;
    }

    while (spi_nor_status() & RDSR_BUSY) {
    }

    for (d_addr += 2, s_buf += 2, len -= 2 ;
         len > 1;
         d_addr += 2, s_buf += 2, len -= 2) {
        // byte shifted order is: 2,1,0
        g_tx_buf[2] = AAI_PROG;
        g_tx_buf[1] = s_buf[0];
        g_tx_buf[0] = s_buf[1];

        if (spi_nor_xfer(&imx_spi_nor, g_tx_buf, g_rx_buf, 3) != 0) {
            diag_printf("Error: %s(%d): failed\n", __FILE__, __LINE__);
            return -1;
        }

        while (spi_nor_status() & RDSR_BUSY) {
        }
	if ((len % flash_dev_info->device_size) == 0) {
            diag_printf(".");
        }
    }
    WRITE_DISABLE();
    while (spi_nor_status() & RDSR_BUSY) {
    }

    if (WRITE_ENABLE() != 0) {
        diag_printf("Error : %d\n", __LINE__);
        return -1;
    }
    if (len == 1) {
        // need to do write-enable command
        // only 1 byte left
        if (spi_nor_program_1byte(s_buf[0], (void *)d_addr) != 0) {
            diag_printf("Error: %s(%d)\n", __FUNCTION__, __LINE__);
            return -1;
        }
    }
    return 0;
}

static int spi_nor_status(void)
{
    g_tx_buf[1] = RDSR;
    if (spi_nor_xfer(&imx_spi_nor, g_tx_buf, g_rx_buf, 2) != 0) {
        diag_printf("Error: %s(): %d\n", __FUNCTION__, __LINE__);
        return 0;
    }
    return g_rx_buf[0];
}

/*!
 * Write 'val' to flash WRSR (write status register)
 */
static int spi_nor_write_status(unsigned char val)
{
    g_tx_buf[0] = val;
    g_tx_buf[1] = WRSR;
    if (spi_nor_xfer(&imx_spi_nor, g_tx_buf, g_rx_buf, 2) != 0) {
        diag_printf("Error: %s(): %d\n", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

/*!
 * Erase a block_size data from block_addr offset in the flash
 */
int spi_nor_erase_block(void* block_addr, unsigned int block_size)
{
    unsigned int *cmd = (unsigned int *)g_tx_buf;
    unsigned int addr = (unsigned int) block_addr;

    imx_spi_nor.us_delay = 0;
    
    if (block_size != SZ_64K && block_size != SZ_32K && block_size != SZ_4K) {
        diag_printf("Error - block_size is not 64kB: 0x%x\n", block_size);
        return -1;
    }

    if ((addr & (block_size -1)) != 0) {
        diag_printf("Error - block_addr is not 64kB aligned: %p\n", block_addr);
        return -1;
    }
    if (ENABLE_WRITE_STATUS() != 0 || spi_nor_write_status(0) != 0) {
        diag_printf("Error: %s: %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    // need to do write-enable command
    if (WRITE_ENABLE() != 0) {
        diag_printf("Error : %d\n", __LINE__);
        return -1;
    }

    if (block_size == SZ_64K) {
        *cmd = (ERASE_64K << 24) | (addr & 0x00FFFFFF);
    } else if (block_size == SZ_32K) {
        *cmd = (ERASE_32K << 24) | (addr & 0x00FFFFFF);
    } else if (block_size == SZ_4K) {
        *cmd = (ERASE_4K << 24) | (addr & 0x00FFFFFF);
    }

    // now do the block erase
    if (spi_nor_xfer(&imx_spi_nor, g_tx_buf, g_rx_buf, 4) != 0) {
        return -1;
    }

    while (spi_nor_status() & RDSR_BUSY) {
    }
    return 0;
}

/*!
 * Erase a variable bytes data from SPI NOR flash for 64K blocks
 * @param block_addr        starting addresss in the SPI NOR flash
 * @param size              # of bytes to erase
 */
int spi_nor_erase_64k(void* block_addr, unsigned int size)
{
    unsigned int addr = (unsigned int) block_addr;

    if ((size % SZ_64K) != 0 || size == 0) {
        diag_printf("Error: size (0x%x) is not integer multiples of 64kB(0x10000)\n", size);
        return -1;
    }
    if ((addr & (SZ_64K -1)) != 0) {
        diag_printf("Error - addr is not 64kB(0x10000) aligned: %p\n", block_addr);
        return -1;
    }
    if (addr + size > flash_dev_info->device_size) {
	diag_printf("spi_nor_erase_64k: Invalid flash address or size 0x%x\n", flash_dev_info->device_size);
	return -1;
    }

    for (; size > 0; size -= SZ_64K, addr += SZ_64K) {
        if (spi_nor_erase_block((void *)addr, SZ_64K) != 0) {
            diag_printf("Error: spi_nor_erase_64k(): %d\n", __LINE__);
            return -1;
        }
    }
    return 0;
}

void spi_nor_setup(void)
{
    if (!spi_nor_init_ok) {
        diag_printf("Initializing SPI-NOR flash...\n");
        if (spi_nor_init(&imx_spi_nor) != 0) {
            diag_printf("Error: failed to initialize SPI NOR\n");
        }
        spi_nor_init_ok = 1;
    }
}

RedBoot_init(spi_nor_setup, RedBoot_INIT_PRIO(6800));

////////////////////////////// commands ///////////////////
static void do_spi_nor_op(int argc, char *argv[]);
RedBoot_cmd("spiflash",
            "Read/Write/Erase SPI NOR flash",
            "<ram-addr> <flash-addr> <len-bytes> <r/w/e>",
            do_spi_nor_op
           );

static void do_spi_nor_op(int argc,char *argv[])
{
    unsigned int ram, flash, len;
    unsigned char op;
    int stat = -1;

    if (argc == 1 || argc != 5) {
        diag_printf("\tRead:  spiflash <ram-addr> <flash-addr> <len-bytes> <r>\n");
        diag_printf("\tWrite: spiflash <ram-addr> <flash-addr> <len-bytes> <w>\n");
        diag_printf("\tErase: spiflash <ram-addr> <flash-addr> <len-bytes> <e>\n");
        diag_printf("    NOTE: For erase, the ram-addr is ignored\n");
        return;
    }

    if (!parse_num(*(&argv[1]), (unsigned long *)&ram, &argv[1], ":")) {
        diag_printf("Error: Invalid ram parameter\n");
        return;
    }

    if (!parse_num(*(&argv[2]), (unsigned long *)&flash, &argv[2], ":")) {
        diag_printf("Error: Invalid flash parameter\n");
        return;
    }

    if (!parse_num(*(&argv[3]), (unsigned long *)&len, &argv[3], ":")) {
        diag_printf("Error: Invalid length parameter\n");
        return;
    }

#ifdef MX51_ERDOS
    spi_norForce_hwr_init (1);
#endif /* MX51_ERDOS */

    op = argv[4][0];
    switch (op) {
    case 'r':
    case 'R':
        diag_printf("Reading SPI NOR flash 0x%x [0x%x bytes] -> ram 0x%x\n", flash, len, ram);
        stat = spi_nor_read((void *)flash, (void *)ram, len);
        break;
    case 'w':
    case 'W':
        diag_printf("Writing SPI NOR flash 0x%x [0x%x bytes] <- ram 0x%x\n", flash, len, ram);
        stat = spi_nor_program_buf((void *)flash, (void *)ram, len);
        break;
    case 'e':
    case 'E':
        diag_printf("Erasing SPI NOR flash 0x%x [0x%x bytes]\n", flash, len);
        stat = spi_nor_erase_64k((void *)flash, len);
        break;
    default:
        diag_printf("Error: unknown operation: 0x%02x\n", op);
    }
    diag_printf("%s\n\n", (stat == 0)? "SUCCESS": "FAILED");
    return;
}
