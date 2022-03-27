// ==========================================================================
//
//   mxcmci_host.c
//   (c) 2008, Freescale
//
//   MMC card driver for MXC platform
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
//==========================================================================

#include <cyg/io/mxcmci_host.h>
#include <cyg/io/mxcmci_core.h>
#include <cyg/io/mxcmci_mmc.h>
#include <cyg/hal/hal_soc.h>
#include <cyg/io/mxc_mmc.h>

host_register_ptr esdhc_base_pointer;
extern void mxc_mmc_init(unsigned int module_base);

static void esdhc_cmd_config(command_t *);
static int esdhc_wait_end_cmd_resp_intr(void);
static cyg_uint32 esdhc_check_response(void);
static void esdhc_wait_buf_rdy_intr(cyg_uint32, multi_single_block_select);
static void esdhc_wait_op_done_intr(cyg_uint32);
static cyg_uint32 esdhc_check_data(cyg_uint32, cyg_uint32, cyg_uint32);
static void esdhc_set_data_transfer_width(cyg_uint32 data_transfer_width);
static void esdhc_set_endianness(cyg_uint32 endian_mode);
static int esdhc_check_for_send_cmd(int data_present);

void host_reset(cyg_uint32 data_transfer_width, cyg_uint32 endian_mode)
{
    int counter = 0;

    /* Reset the entire host controller by writing 1 to RSTA bit of SYSCTRL Register */
    esdhc_base_pointer->system_control |= ESDHC_SOFTWARE_RESET;

    //use WDOG timer: 3 ms delay
    hal_delay_us(3 * 1000);

    /* Wait for clearance of CIHB and CDIHB Bits */
    while (esdhc_base_pointer->present_state & ESDHC_CMD_INHIBIT) {
        if (counter++ > 200) {
            diag_printf
                ("%s: something goes wrong with the DSDHC and int is not received!\n",
                 __FUNCTION__);
            counter = 0;
            break;
        }
    }

    /* send 80 clock ticks for card to power up */
    esdhc_base_pointer->system_control |= ESDHC_SOFTWARE_INIT;

    /* Set data bus width of ESDCH */
    esdhc_set_data_transfer_width(data_transfer_width);

    /* Set Endianness of ESDHC */
    esdhc_set_endianness(endian_mode);

}

void esdhc_softreset(cyg_uint32 mask)
{
    //wait max timeout 100ms
    cyg_uint32 timeout = 100;

    esdhc_base_pointer->system_control |= mask;

    /* hw clears the bit when it's done */
    while (esdhc_base_pointer->system_control & mask) {
        if (timeout == 0) {
            flash_dprintf(FLASH_DEBUG_MAX,
                      "%s:Reset 0x%X never complete!\n");
            return;
        }
        timeout--;
        hal_delay_us(100);
    }
}

void host_init(cyg_uint32 base_address)
{
    esdhc_base_pointer = (host_register_ptr) base_address;

    flash_dprintf(FLASH_DEBUG_MAX, "%s: interface_esdc=%d\n", __FUNCTION__,
              base_address);

    mxc_mmc_init(base_address);
}

void host_cfg_clock(sdhc_freq_t frequency)
{
    unsigned int timeout = 9000;
    /* Enable ipg_perclk, HCLK enable, IPG Clock enable.  */
    esdhc_base_pointer->system_control |= ESDHC_CLOCK_ENABLE;

    esdhc_base_pointer->system_control |= 0xe0000;    //set timeout counter

    /* Clear DTOCV SDCLKFS bits, clear SD clk enable bit to change frequency */
    esdhc_base_pointer->system_control &= ESDHC_FREQ_MASK;

    /* Disable SD clock */
    esdhc_base_pointer->system_control &= ~ESDHC_ENABLE;

    if (frequency == IDENTIFICATION_FREQ) {
        /* Input frequecy to eSDHC is 36 MHZ */
        /* PLL3 is the source of input frequency */
        /*Set DTOCV and SDCLKFS bit to get SD_CLK of frequency below 400 KHZ (70.31 KHZ) */
        esdhc_base_pointer->system_control |= ESDHC_IDENT_FREQ;
    } else if (frequency == OPERATING_FREQ) {
        /*Set DTOCV and SDCLKFS bit to get SD_CLK of frequency around 25 MHz.(18 MHz) */
        esdhc_base_pointer->system_control |= ESDHC_OPERT_FREQ;
    }

    /* Wait for clock to be steady */
    while (((esdhc_base_pointer->present_state & 0x8) == 0) && (timeout != 0)) {
        timeout--;
        hal_delay_us(10);
    }

    /* Enable SD clock */
    esdhc_base_pointer->system_control |= ESDHC_ENABLE;
}

static void esdhc_set_data_transfer_width(cyg_uint32 data_transfer_width)
{

    /* Set DWT bit of protocol control register according to bus_width */
    esdhc_base_pointer->protocol_control &= ~0x6;
    esdhc_base_pointer->protocol_control |= data_transfer_width;

}

static void esdhc_set_endianness(cyg_uint32 endian_mode)
{

    /* Set DWT bit of protocol control register according to bus_width */
    esdhc_base_pointer->protocol_control |= endian_mode;

}

cyg_uint32 host_send_cmd(command_t * cmd)
{

    /* Clear Interrupt status register */
    esdhc_base_pointer->interrupt_status = ESDHC_CLEAR_INTERRUPT;
    //esdhc_base_pointer->interrupt_status = 0x117f01ff;

    /* Enable Interrupt */
    esdhc_base_pointer->interrupt_status_enable |= ESDHC_INTERRUPT_ENABLE;
    //esdhc_base_pointer->interrupt_status_enable |= 0x007f0123;

#if 0
    if (esdhc_check_for_send_cmd(cmd->data_present)) {
        diag_printf("Data/Cmd Line Busy.\n");
        return FAIL;
    }
#endif

    /* Configure Command    */
    esdhc_cmd_config(cmd);

    /* Wait interrupt (END COMMAND RESPONSE)  */
    //diag_printf("Wait for CMD Response.\n");
    if (esdhc_wait_end_cmd_resp_intr()) {
        diag_printf("Wait CMD (%d) RESPONSE TIMEOUT.\n", cmd->command);
        return FAIL;
    }
    //Just test for Erase functionality:Lewis-20080505:
    if (cmd->command == CMD38) {
        flash_dprintf(FLASH_DEBUG_MAX, "%s:Check DAT0 status:\n",
                  __FUNCTION__);
        //while(((esdhc_base_pointer->present_state) & 0x01000004)){
        //   flash_dprintf(FLASH_DEBUG_MAX,".");
        //   hal_delay_us(1000);
        //}
        /* I'm not sure the minimum value of delay */
        hal_delay_us(100000);
        hal_delay_us(100000);
        hal_delay_us(100000);
        flash_dprintf(FLASH_DEBUG_MAX,
                  "\nCheck DAT0 status DONE: present_state=%x\n",
                  (cyg_uint32) (esdhc_base_pointer->present_state));
    }

    /* Mask all interrupts     */
    //esdhc_base_pointer->interrupt_signal_enable =0;

    /* Check if an error occured    */
    return esdhc_check_response();
}

static void esdhc_cmd_config(command_t * cmd)
{
    unsigned int transfer_type;

    /* Write Command Argument in Command Argument Register */
    esdhc_base_pointer->command_argument = cmd->arg;

    /*    *Configure e-SDHC Register value according to Command    */
    transfer_type = (((cmd->data_transfer) << DATA_TRANSFER_SHIFT) |
             ((cmd->response_format) << RESPONSE_FORMAT_SHIFT) |
             ((cmd->data_present) << DATA_PRESENT_SHIFT) |
             ((cmd->crc_check) << CRC_CHECK_SHIFT) |
             ((cmd->cmdindex_check) << CMD_INDEX_CHECK_SHIFT) |
             ((cmd->command) << CMD_INDEX_SHIFT) |
             ((cmd->
               block_count_enable_check) <<
              BLOCK_COUNT_ENABLE_SHIFT) | ((cmd->
                            multi_single_block) <<
                               MULTI_SINGLE_BLOCK_SELECT_SHIFT));

    esdhc_base_pointer->command_transfer_type = transfer_type;

    //diag_printf("arg: 0x%x | tp: 0x%x\n", esdhc_base_pointer->command_argument, esdhc_base_pointer->command_transfer_type);

}

static int esdhc_wait_end_cmd_resp_intr(void)
{
    /* Wait interrupt (END COMMAND RESPONSE)  */
    cyg_uint32 i = 50000;
    while (!
           ((esdhc_base_pointer->
         interrupt_status) & ESDHC_STATUS_END_CMD_RESP_TIME_MSK) && i) {
        i--;
        hal_delay_us(10);
        //diag_printf("0x%x\n", esdhc_base_pointer->interrupt_status);
    }

    if (!
        ((esdhc_base_pointer->
          interrupt_status) & ESDHC_STATUS_END_CMD_RESP_TIME_MSK)) {
        //diag_printf("%s: can't get END COMMAND RESPONSE! Tried %d times\n", __FUNCTION__, (5000000-i));
        return FAIL;
    }

    return SUCCESS;
}

static cyg_uint32 esdhc_check_response(void)
{
    cyg_uint32 status = FAIL;

    /* Check whether the interrupt is an END_CMD_RESP
     * or a response time out or a CRC error
     */
    if ((esdhc_base_pointer->
         interrupt_status & ESDHC_STATUS_END_CMD_RESP_MSK)
        && !(esdhc_base_pointer->
         interrupt_status & ESDHC_STATUS_TIME_OUT_RESP_MSK)
        && !(esdhc_base_pointer->
         interrupt_status & ESDHC_STATUS_RESP_CRC_ERR_MSK)
        && !(esdhc_base_pointer->
         interrupt_status & ESDHC_STATUS_RESP_INDEX_ERR_MSK)) {

        status = SUCCESS;
    } else {
        //diag_printf("Warning: Check CMD response, Intr Status: 0x%x\n", esdhc_base_pointer->interrupt_status);
        status = FAIL;
    }

    return status;

}

void host_read_response(command_response_t * cmd_resp)
{
    /* get response values from e-SDHC CMDRSP registers. */
    cmd_resp->cmd_rsp0 = (cyg_uint32) esdhc_base_pointer->command_response0;
    cmd_resp->cmd_rsp1 = (cyg_uint32) esdhc_base_pointer->command_response1;
    cmd_resp->cmd_rsp2 = (cyg_uint32) esdhc_base_pointer->command_response2;
    cmd_resp->cmd_rsp3 = (cyg_uint32) esdhc_base_pointer->command_response3;
}

static void esdhc_wait_buf_rdy_intr(cyg_uint32 mask,
                    multi_single_block_select
                    multi_single_block)
{

    /* Wait interrupt (BUF_READ_RDY)    */

    cyg_uint32 i;
    for (i = 3000; i > 0; i--) {
        if (esdhc_base_pointer->interrupt_status & mask) {
            break;
        }
        hal_delay_us(100);
    }

    if (multi_single_block == MULTIPLE
        && esdhc_base_pointer->interrupt_status & mask)
        esdhc_base_pointer->interrupt_status |= mask;
    if (i == 0)
        flash_dprintf(FLASH_DEBUG_MAX, "%s:Debug: tried %d times\n",
                  __FUNCTION__, (3000 - i));

}

static void esdhc_wait_op_done_intr(cyg_uint32 transfer_mask)
{
    /* Wait interrupt (Transfer Complete)    */

    cyg_uint32 i;
    while (!(esdhc_base_pointer->interrupt_status & transfer_mask)) ;

    //diag_printf("Wait OP Done Failed.\n");
    //flash_dprintf(FLASH_DEBUG_MAX,"%s:Debug: tried %d times\n", __FUNCTION__, (3001-i));

}

static cyg_uint32 esdhc_check_data(cyg_uint32 op_done_mask,
                   cyg_uint32 read_time_out_mask,
                   cyg_uint32 read_crc_err_mask)
{

    cyg_uint32 status = FAIL;

    /* Check whether the interrupt is an OP_DONE
     * or a data time out or a CRC error     */
    if ((esdhc_base_pointer->interrupt_status & op_done_mask) &&
        !(esdhc_base_pointer->interrupt_status & read_time_out_mask) &&
        !(esdhc_base_pointer->interrupt_status & read_crc_err_mask)) {
        status = SUCCESS;
    } else {
        status = FAIL;
        //diag_printf("Warning: Check data, interrupt_status=%X\n", (esdhc_base_pointer->interrupt_status));
    }

    return status;
}

void host_cfg_block(cyg_uint32 blk_len, cyg_uint32 nob)
{
    /* Configre block Attributes register */
    esdhc_base_pointer->block_attributes =
        ((nob << 16) | (blk_len & 0xffff));

    //diag_printf("nob: 0x%x, block_attributes: 0x%x\n", nob, esdhc_base_pointer->block_attributes);

    /* Set Read Water Mark Level register */
    esdhc_base_pointer->watermark_level = WRITE_READ_WATER_MARK_LEVEL;
}

cyg_uint32 host_data_read(cyg_uint32 * dest_ptr, cyg_uint32 read_len)
{
    cyg_uint32 j, k;
    cyg_uint32 status = FAIL;
    unsigned int len = WRITE_READ_WATER_MARK_LEVEL & 0xff;
    //int counter = 0;

    /* Enable Interrupt */
    esdhc_base_pointer->interrupt_status_enable |= ESDHC_INTERRUPT_ENABLE;

    for (j = 0; j < read_len / (len * 4); j++) {
        //StartCounter();
        /* wait for read fifo full (equal or beyond the watermark) */
        while (!(esdhc_base_pointer->present_state & (1 << 11))) ;

        //counter = StopCounter();
        //diag_printf("counter: 0x%x\n", counter);

        for (k = 0; k < len; k++) {
            *dest_ptr++ = esdhc_base_pointer->data_buffer_access;
        }
    }

    /* Wait for transfer complete operation interrupt */
    esdhc_wait_op_done_intr(ESDHC_STATUS_TRANSFER_COMPLETE_MSK);

    /* Check for status errors */
    status =
        esdhc_check_data(ESDHC_STATUS_TRANSFER_COMPLETE_MSK,
                 ESDHC_STATUS_TIME_OUT_READ, ESDHC_STATUS_READ_CRC_ERR_MSK);

    return status;

}

cyg_uint32 host_data_write(cyg_uint32 * src_ptr, cyg_uint32 write_len)
{
    cyg_uint32 i = 0, k;
    cyg_uint32 status = FAIL;
    unsigned int len = (WRITE_READ_WATER_MARK_LEVEL >> 16) & 0xff;
    //cyg_uint32 counter = 0;

    /* Enable Interrupt */
    esdhc_base_pointer->interrupt_status_enable |= ESDHC_INTERRUPT_ENABLE;

    //StartCounter();
    for (i = 0; i < (write_len) / (len * 4); i++) {
        /* wait for write fifo empty (equal or less than the watermark), BWEN */
        while (!(esdhc_base_pointer->present_state & (1 << 10))) ;

        for (k = 0; k < len; k++) {
            esdhc_base_pointer->data_buffer_access = *src_ptr++;
        }

    }

    /* Wait for transfer complete operation interrupt */
    esdhc_wait_op_done_intr(ESDHC_STATUS_TRANSFER_COMPLETE_MSK);

    //counter = StopCounter();
    //diag_printf("0x%x\n", counter);

    /* Check for status errors */
    status =
        esdhc_check_data(ESDHC_STATUS_TRANSFER_COMPLETE_MSK,
                 ESDHC_STATUS_TIME_OUT_READ, ESDHC_STATUS_READ_CRC_ERR_MSK);

    return status;

}

static int esdhc_check_for_send_cmd(int data_present)
{

    int status = SUCCESS;
    int counter;

    /* Wait for the command line to be free (poll the CIHB bit of
     * the present state register.
     */
    counter = 1000;
    while (((esdhc_base_pointer->present_state & 0x1) == 0x1) && counter--) {
        hal_delay_us(10);
    }

    if (!counter)
        return FAIL;

    /* Wait for the data line to be free (poll the CDIHB bit of
     * the present state register.
     */
    counter = 1000;
    if (data_present == DATA_PRESENT) {
        while (((esdhc_base_pointer->present_state & 0x2) == 0x2) && counter--) {
            hal_delay_us(10);
        }

    }

    if (!counter)
        return FAIL;

    return status;
}
