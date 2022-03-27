#ifndef CYGONCE_DEVS_FLASH_SPI_NOR_PARTS_INL
#define CYGONCE_DEVS_FLASH_SPI_NOR_PARTS_INL
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
// modification information
// ------------------------
// 2009/07/01 : W25Q16BV support.
// 2009/07/03 : unsupport_cmd_AAI add.
//
//####DESCRIPTIONEND####
//
//==========================================================================

    {
        device_id  : 0xbf, // Samsung K9F5608x0C (on EVB SDR memory card)
        device_id2 : 0x25,
        device_id3 : 0x41,
        device_id4 : 0xFF,
        block_size : SZ_64K,
        block_count: 32,
        device_size: SZ_64K * 32,
        fis_start_addr: 0x80000,       // first 0.5MB reserved for Redboot
        vendor_info: "SST25VF016B - 2MB ",
#ifdef MX51_ERDOS
	unsupport_cmd_AAI: 0,	// Support AAI
#endif /* MX51_ERDOS */
    },
    {
        device_id  : 0xef, // Winbond W25Q16BV
        device_id2 : 0x40,
        device_id3 : 0x15,
        device_id4 : 0xFF,
        block_size : SZ_64K,
        block_count: 32,
        device_size: SZ_64K * 32,
        fis_start_addr: 0x80000,       // first 0.5MB reserved for Redboot
        vendor_info: "W25Q16BV - 2MB ",
#ifdef MX51_ERDOS
	unsupport_cmd_AAI: 1,	// UNsupport AAI
#endif /* MX51_ERDOS */
    },
#endif // CYGONCE_DEVS_FLASH_SPI_NOR_PARTS_INL
