#ifndef _IMX_NFC_H_
#define _IMX_NFC_H_
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
// Date:         2008-06-02
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================

#define NFC_DEBUG_MIN   1
#define NFC_DEBUG_MED   2
#define NFC_DEBUG_MAX   3
#define NFC_DEBUG_DEF   NFC_DEBUG_MAX

extern int _mxc_boot;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned char u8;

//----------------------------------------------------------------------------
// Common device details.
#define FLASH_Read_ID                   (0x90)
#ifdef CYGHWR_DEVS_FLASH_MXC_NAND_RESET_WORKAROUND
#define FLASH_Reset                     0xFFFF
#else
#define FLASH_Reset                     (0xFF)
#endif
#define FLASH_Read_Mode1                (0x00)
#define FLASH_Read_Mode1_LG             (0x30)
#define FLASH_Read_Mode2                (0x01)
#define FLASH_Read_Mode3                (0x50)
#define FLASH_Program                   (0x10)
#define FLASH_Send_Data                 (0x80)
#define FLASH_Status                    (0x70)
#define FLASH_Block_Erase               (0x60)
#define FLASH_Start_Erase               (0xD0)

enum nfc_page_area {
    NFC_SPARE_ONLY,
    NFC_MAIN_ONLY,
};

enum {
    MXC_NAND_8_BIT = 8,
    MXC_NAND_16_BIT =  16,
};

enum {
    NAND_SLC = 0,
    NAND_MLC = 1,
};

// read column 464-465 byte but only 464 for bad block marker
#define BAD_BLK_MARKER_464          (NAND_MAIN_BUF3 + 464)
// read column 4-5 byte, but only 5 is used for swapped main area data
#define BAD_BLK_MARKER_SP_5         (NAND_SPAR_BUF3 + 4)

typedef void nfc_iomuxsetup_func_t(void);

#endif // _IMX_NFC_H_
