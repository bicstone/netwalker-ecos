/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2004 eCosCentric Limited 
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jani
// Contributors:jskov, gthomas
// Date:        2001-07-12
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/


//===========================================================================
//                                INCLUDES
//===========================================================================
#include <pkgconf/hal.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types

#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_intr.h>           // HAL_ENABLE/MASK/UNMASK_INTERRUPTS
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED
#include <cyg/hal/hal_diag.h>

#include <cyg/hal/var_io.h>             // USART registers
#include <cyg/hal/lpc24xx_misc.h>       // peripheral identifiers


//===========================================================================
//                                DATA TYPES
//===========================================================================-
typedef struct 
{
    cyg_uint8* base;     
    cyg_int32  msec_timeout;     
    int        isr_vector;
    int        baud_rate;
    cyg_uint8  periph_id;
} channel_data_t;


//
// Diagnostic serial channel data
//
static channel_data_t lpc2xxx_ser_channels[2] = 
{
    { (cyg_uint8*)CYGARC_HAL_LPC24XX_REG_UART0_BASE, 
       1000, 
       CYGNUM_HAL_INTERRUPT_UART0, 
       CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD,
       CYNUM_HAL_LPC24XX_PCLK_UART0},
      
    { (cyg_uint8*)CYGARC_HAL_LPC24XX_REG_UART1_BASE, 
       1000, 
       CYGNUM_HAL_INTERRUPT_UART1, 
       CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD,
       CYNUM_HAL_LPC24XX_PCLK_UART1}
};


//===========================================================================
// Initialize diagnostic serial channel
//===========================================================================
static void cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8* base = chan->base;
    cyg_uint16 divider = CYG_HAL_ARM_LPC24XX_BAUD_GENERATOR(chan->periph_id, 
                                                            chan->baud_rate);
    // Set baudrate
    HAL_WRITE_UINT32(base + CYGARC_HAL_LPC24XX_REG_UxLCR, 
                     CYGARC_HAL_LPC24XX_REG_UxLCR_DLAB);
    HAL_WRITE_UINT32(base + CYGARC_HAL_LPC24XX_REG_UxDLM, divider >> 8);
    HAL_WRITE_UINT32(base + CYGARC_HAL_LPC24XX_REG_UxDLL, divider & 0xFF);

    // 8-1-no parity.
    HAL_WRITE_UINT32(base + CYGARC_HAL_LPC24XX_REG_UxLCR, 
                     CYGARC_HAL_LPC24XX_REG_UxLCR_WORD_LENGTH_8 |
                     CYGARC_HAL_LPC24XX_REG_UxLCR_STOP_1);

    // Reset and enable FIFO
    HAL_WRITE_UINT32(base + CYGARC_HAL_LPC24XX_REG_UxFCR, 
                     CYGARC_HAL_LPC24XX_REG_UxFCR_FIFO_ENA |
                     CYGARC_HAL_LPC24XX_REG_UxFCR_RX_FIFO_RESET | 
                     CYGARC_HAL_LPC24XX_REG_UxFCR_TX_FIFO_RESET);
}


//===========================================================================
// Write single character
//===========================================================================
void cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 stat;
    CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT32(base + CYGARC_HAL_LPC24XX_REG_UxLSR, stat);
    } while ((stat & CYGARC_HAL_LPC24XX_REG_UxLSR_THRE) == 0);

    HAL_WRITE_UINT32(base + CYGARC_HAL_LPC24XX_REG_UxTHR, c);

    CYGARC_HAL_RESTORE_GP();
}


//===========================================================================
// Read single character non blocking
//===========================================================================
static cyg_bool cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8* base = chan->base;
    cyg_uint8 stat;

    HAL_READ_UINT32(base + CYGARC_HAL_LPC24XX_REG_UxLSR, stat);
    if ((stat & CYGARC_HAL_LPC24XX_REG_UxLSR_RDR) == 0)
        return false;

    HAL_READ_UINT32(base + CYGARC_HAL_LPC24XX_REG_UxRBR, *ch);

    return true;
}


//===========================================================================
// Read single character blocking
//===========================================================================
cyg_uint8 cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}


//===========================================================================
// Write data buffer via serial line
//===========================================================================
static void cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}


//===========================================================================
// Read data buffer
//===========================================================================
static void cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, 
                                    cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}


//===========================================================================
// Read single character with timeout
//===========================================================================
cyg_bool cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}


//===========================================================================
// Control serial channel configuration
//===========================================================================
static int cyg_hal_plf_serial_control(void *__ch_data, 
                                      __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    int ret = 0;
    va_list ap;

    CYGARC_HAL_SAVE_GP();
    va_start(ap, __func);

    switch (__func) {
    case __COMMCTL_GETBAUD:
        ret = chan->baud_rate;
        break;
    case __COMMCTL_SETBAUD:
        chan->baud_rate = va_arg(ap, cyg_int32);
        // Should we verify this value here?
        cyg_hal_plf_serial_init_channel(chan);
        ret = 0;
        break;
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;
        HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        HAL_WRITE_UINT32(base+CYGARC_HAL_LPC24XX_REG_UxIER, 
                         CYGARC_HAL_LPC24XX_REG_UxIER_RXDATA_INT);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_INTERRUPT_MASK(chan->isr_vector);
        HAL_WRITE_UINT32(base+CYGARC_HAL_LPC24XX_REG_UxIER, 0);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);
    default:
        break;
    }

    va_end(ap);
    CYGARC_HAL_RESTORE_GP();
    return ret;
}


//===========================================================================
// Serial channel ISR
//===========================================================================
static int cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                                  CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8 c;
    cyg_uint8 iir;
    
    CYGARC_HAL_SAVE_GP();

    *__ctrlc = 0;

    HAL_READ_UINT32(chan->base + CYGARC_HAL_LPC24XX_REG_UxIIR, iir);
    
    if((iir & (CYGARC_HAL_LPC24XX_REG_UxIIR_IIR0 | 
               CYGARC_HAL_LPC24XX_REG_UxIIR_IIR1 | 
               CYGARC_HAL_LPC24XX_REG_UxIIR_IIR2)) 
       == CYGARC_HAL_LPC24XX_REG_UxIIR_IIR2)
    {
        // Rx data available or character timeout
        // Read data in order to clear interrupt
        HAL_READ_UINT32(chan->base + CYGARC_HAL_LPC24XX_REG_UxRBR, c);
        if( cyg_hal_is_break( &c , 1 ) ) *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);

    CYGARC_HAL_RESTORE_GP();
    return res;
}


//===========================================================================
// Initialize serial channel
//===========================================================================
void cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur;

    cur = 
      CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Init channels
    cyg_hal_plf_serial_init_channel(&lpc2xxx_ser_channels[0]);
#if CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS > 1
    cyg_hal_plf_serial_init_channel(&lpc2xxx_ser_channels[1]);
#endif

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &lpc2xxx_ser_channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

#if CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS > 1
    // Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &lpc2xxx_ser_channels[1]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
#endif

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}


//===========================================================================
// Set diagnostic led
//===========================================================================
void hal_diag_led(int mask)
{
    hal_lpc24xx_set_leds(mask);
}

//-----------------------------------------------------------------------------
// End of hal_diag.c
