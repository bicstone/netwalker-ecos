#ifndef CYGONCE_FSL_BOARD_H
#define CYGONCE_FSL_BOARD_H

//=============================================================================
//
//      Platform specific support (register layout, etc)
//
//=============================================================================
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
//===========================================================================

#include <cyg/hal/hal_soc.h>        // Hardware definitions

/* CPLD offsets */
#define PBC_LED_CTRL		(0x20000)
#define PBC_SB_STAT		(0x20008)
#define PBC_ID_AAAA		(0x20040)
#define PBC_ID_5555		(0x20048)
#define PBC_VERSION		(0x20050)
#define PBC_ID_CAFE		(0x20058)
#define PBC_INT_STAT		(0x20010)
#define PBC_INT_MASK		(0x20038)
#define PBC_INT_REST		(0x20020)
#define PBC_SW_RESET		(0x20060)
#define BOARD_CS_UART_BASE	(0x8000)

#define FEC_PHY_ADDR		0x06
#define REDBOOT_IMAGE_SIZE	0x40000

#define EXT_UART_x16
/* MX51 3-Stack SDRAM is from 0x40000000, 128M */
#define SDRAM_BASE_ADDR	CSD0_BASE_ADDR
#define SDRAM_SIZE		0x08000000
#define RAM_BANK0_BASE		SDRAM_BASE_ADDR

#define LED_MAX_NUM	8

#endif /* CYGONCE_FSL_BOARD_H */
