/*=============================================================================
//
//      board_diag.c
//
//      HAL diagnostic output code
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
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_if.h>             // Calling-if API
#include <cyg/hal/drv_api.h>            // driver API
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/hal_soc.h>            // Hardware definitions
#include <cyg/hal/fsl_board.h>          // Platform specifics

extern void cyg_hal_plf_serial_init(void);

void cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    /* Setup GPIO and enable transceiver for UARTs */
    cyg_hal_plf_serial_init();
}

//=============================================================================
// Compatibility with older stubs
//=============================================================================

//=============================================================================
// Compatibility with older stubs
//=============================================================================

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

#include <cyg/hal/hal_stub.h>           // cyg_hal_gdb_interrupt

#if (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL == 0)
#define __BASE   CMA101_DUARTA
#define _INT     CYGNUM_HAL_INTERRUPT_SERIAL_A
#elif (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL == 1)
#define __BASE   CMA101_DUARTB
#define _INT     CYGNUM_HAL_INTERRUPT_SERIAL_B
#endif

#ifdef __BASE

#ifdef CYGSEM_HAL_ROM_MONITOR
#define CYG_HAL_STARTUP_ROM
#define CYG_HAL_STARTUP_ROMRAM
#undef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#endif

#if (defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)) && !defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#define HAL_DIAG_USES_HARDWARE
#elif !defined(CYGDBG_HAL_DIAG_TO_DEBUG_CHAN)
#define HAL_DIAG_USES_HARDWARE
#elif CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL != CYGNUM_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL
#define HAL_DIAG_USES_HARDWARE
#endif

static channel_data_t channel = {
    (uart_width*) _BASE, 0, 0
};

#ifdef HAL_DIAG_USES_HARDWARE

void hal_diag_init(void)
{
    static int init = 0;
    char *msg = "\n\rARM eCos\n\r";
    uart_width lcr;

    if (init++) return;

    init_duart_channel(&channel);

    while (*msg) hal_diag_write_char(*msg++);
}

#ifdef DEBUG_DIAG
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#define DIAG_BUFSIZE 32
#else
#define DIAG_BUFSIZE 2048
#endif
static char diag_buffer[DIAG_BUFSIZE];
static int diag_bp = 0;
#endif

void hal_diag_write_char(char c)
{
    uart_width lsr;

    hal_diag_init();

    cyg_hal_plf_duart_putc(&channel, c)

#ifdef DEBUG_DIAG
    diag_buffer[diag_bp++] = c;
    if (diag_bp == DIAG_BUFSIZE) {
        while (1) ;
        diag_bp = 0;
    }
#endif
}

void hal_diag_read_char(char *c)
{
    *c = cyg_hal_plf_duart_getc(&channel);
}

#else // HAL_DIAG relies on GDB

// Initialize diag port - assume GDB channel is already set up
void hal_diag_init(void)
{
    if (0) init_duart_channel(&channel); // avoid warning
}

// Actually send character down the wire
static void hal_diag_write_char_serial(char c)
{
    cyg_hal_plf_duart_putc(&channel, c);
}

static bool hal_diag_read_serial(char *c)
{
    long timeout = 1000000000;  // A long time...

    while (!cyg_hal_plf_duart_getc_nonblock(&channel, c))
        if (0 == --timeout) return false;

    return true;
}

void hal_diag_read_char(char *c)
{
    while (!hal_diag_read_serial(c)) ;
}

void hal_diag_write_char(char c)
{
    static char line[100];
    static int pos = 0;

    // No need to send CRs
    if (c == '\r') return;

    line[pos++] = c;

    if (c == '\n' || pos == sizeof(line)) {
        CYG_INTERRUPT_STATE old;

        // Disable interrupts. This prevents GDB trying to interrupt us
        // while we are in the middle of sending a packet. The serial
        // receive interrupt will be seen when we re-enable interrupts
        // later.

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION(old);
#else
        HAL_DISABLE_INTERRUPTS(old);
#endif

        while (1) {
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
            char c1;

            hal_diag_write_char_serial('$');
            hal_diag_write_char_serial('O');
            csum += 'O';
            for (i = 0; i < pos; i++) {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                hal_diag_write_char_serial(h);
                hal_diag_write_char_serial(l);
                csum += h;
                csum += l;
            }
            hal_diag_write_char_serial('#');
            hal_diag_write_char_serial(hex[(csum>>4)&0xF]);
            hal_diag_write_char_serial(hex[csum&0xF]);

            // Wait for the ACK character '+' from GDB here and handle
            // receiving a ^C instead.  This is the reason for this clause
            // being a loop.
            if (!hal_diag_read_serial(&c1))
                continue;   // No response - try sending packet again

            if ( c1 == '+' )
                break;          // a good acknowledge

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
            cyg_drv_interrupt_acknowledge(CYG_DEV_SERIAL_INT);
            if ( c1 == 3 ) {
                // Ctrl-C: breakpoint.
                cyg_hal_gdb_interrupt (__builtin_return_address(0));
                break;
            }
#endif
            // otherwise, loop round again
        }

        pos = 0;

        // And re-enable interrupts
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION(old);
#else
        HAL_RESTORE_INTERRUPTS(old);
#endif

    }
}
#endif

#endif // __BASE

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

/*---------------------------------------------------------------------------*/
