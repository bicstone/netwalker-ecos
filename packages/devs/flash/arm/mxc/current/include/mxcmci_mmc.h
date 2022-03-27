#ifndef _MXCMCI_MMC_H_
#define _MXCMCI_MMC_H_

// ========================================================================== 
//                                                                           
//    Module Name:  mxcmci_mmc.h
//
//    General Description: Limited Bootloader eSDHC Driver.
//                                 
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
//     
//####DESCRIPTIONEND####
//
// modification information
// ------------------------
// 2009/07/13 : mmc_data_write/read() offset 64bit.
//
//====================================================================================================


#define MMC_OCR_VALUE 0x40FF8000
#define MMC_OCR_VALUE_BAK 0x80FFC000
#define MMC_OCR_HC_RES 0xC0FF8000
#define MMC_OCR_LC_RES 0x80FF8000
#define MMC_OCR_VALUE_MASK 0x00FF8000
#define BYTE_MODE 0
#define SECT_MODE 1
#define CARD_BUSY_BIT 0x80000000
#define CURR_STATE_SHIFT 9
#define MMC_SPEC_VER 0x003C0000
#define MMC_SPEC_VER_SHIFT 18  
#define MMC_R1_SWITCH_ERROR_MASK 0x80
#define SWITCH_ERROR_SHIFT 7
#define BUS_SIZE_SHIFT 2
#define BUS_WIDTH 0x3b700000


extern cyg_uint32 mmc_init(cyg_uint32);
#ifdef MX51_ERDOS
extern cyg_uint32 mmc_data_read (cyg_uint32 *,cyg_uint32 ,cyg_uint64);
extern cyg_uint32 mmc_data_write (cyg_uint32 *src_ptr,cyg_uint32 length,cyg_uint64 offset);
#else
extern cyg_uint32 mmc_data_read (cyg_uint32 *,cyg_uint32 ,cyg_uint32);
extern cyg_uint32 mmc_data_write (cyg_uint32 *src_ptr,cyg_uint32 length,cyg_uint32 offset);
#endif /* MX51_ERDOS */
extern cyg_uint32 mmc_data_erase (cyg_uint32 offset, cyg_uint32 size);
extern cyg_uint32 mmc_voltage_validation (void);
extern cyg_uint32 mmc_get_spec_ver (void);
extern cyg_uint32 sd_voltage_validation (void);
extern cyg_uint32 sd_init(cyg_uint32);
extern cyg_uint32 card_flash_query(void* data);

typedef struct 
{
	cyg_uint32 csd0;
	cyg_uint32 csd1;
	cyg_uint32 csd2;
	cyg_uint32 csd3;
}card_specific_data;

#endif  /* _MXCMCI_MMC_H_ */

