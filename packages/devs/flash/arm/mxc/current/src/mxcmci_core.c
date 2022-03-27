// ==========================================================================
//
//   mxcmci_core.c
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
// modification information
// ------------------------
// 2009/07/13 : mmc_data_read() / mxcmci_data_read() offset 64bit.
//
//==========================================================================

#include <cyg/io/mxcmci_host.h>
#include <cyg/io/mxcmci_core.h>
#include <cyg/io/mxcmci_mmc.h>
#include <cyg/hal/hal_soc.h>
#include <cyg/io/mxc_mmc.h>

static cyg_uint32 csd_get_value(cyg_uint32 * pcsd, cyg_uint32 start_bit,
                cyg_uint32 end_bit);

#define MMCSD_INIT_DELAY 64

cyg_uint32 Card_rca = 0x1;    /* Relative Card Address */
card_ident Card_identification;    /* Card Identification Data */
card_type Card_type;        /* Card Type */
cyg_uint32 MMC_Spec_vers = 0x1;        /* Spec vers used for MMC */
card_specific_data csd;        /* Global variable for Card Specific Data */
cyg_uint32 Card_capacity_size = 0;    /*Card capacity size */
cyg_uint32 CCC = 0;        /* Card Command Class */
int Card_Mode = 2;
int HighCapacityCard = 0;

/*==========================================================================
                                    Global FUNCTIONS
==========================================================================*/

cyg_uint32 mxcmci_init(cyg_uint32 bus_width, cyg_uint32 base_address)
{
    cyg_uint32 init_status = FAIL;

    flash_dprintf(FLASH_DEBUG_MAX, "%s:try to init base address...\n",
              __FUNCTION__);
    /* initialize Interface Controller */
    host_init(base_address);
    flash_dprintf(FLASH_DEBUG_MAX, "%s:try to software reset...\n",
              __FUNCTION__);

    /* Software Reset to Interface Controller */
    host_reset(ESDHC_ONE_BIT_SUPPORT, ESDHC_LITTLE_ENDIAN_MODE);
    flash_dprintf(FLASH_DEBUG_MAX, "%s:try to set identification freq...\n",
              __FUNCTION__);

    /* Enable Identification Frequency */
    host_cfg_clock(IDENTIFICATION_FREQ);

    /* Add delay of 2 msec, let mmc/sd card to initialize */
    hal_delay_us(2 * 1000);

    flash_dprintf(FLASH_DEBUG_MAX, "%s:try to software resetto card...\n",
              __FUNCTION__);

    //diag_printf("SW Reset...\n");
    /* Issue Software Reset to card */
    if (mxcmci_software_reset())
        return FAIL;

    //diag_printf("Check Card...\n");

    /* Check if the card is SD Memory Card */
    if (!sd_voltage_validation()) {
        flash_dprintf(FLASH_DEBUG_MAX, "%s:try to verify SD card...\n",
                  __FUNCTION__);
        /* Call SD Initialization Function */
        init_status = sd_init(bus_width);
        Card_type =
            ((csd.csd3 & CSD_STRUCT_MSK) ? SD_CSD_2_0 : SD_CSD_1_0);
        Card_Mode = 1;
        /* Card Command Class */
        CCC = csd_get_value(&csd, 84, 95);
    } else {
        flash_dprintf(FLASH_DEBUG_MAX, "%s:try to verify MMC card...\n",
                  __FUNCTION__);
        /* Check if the card is MMC Memory Card */
        if (!mmc_voltage_validation()) {

            /* Call MMC Initialization Function */
            init_status = mmc_init(bus_width);
            Card_Mode = 0;
            Card_type = ((csd.csd3 & CSD_STRUCT_MSK) >> CSD_STRUCT_SHIFT) + SD_CSD_2_0;
            MMC_Spec_vers = (csd.csd3 & MMC_CSD_SPEC_VERS_MASK) >> MMC_CSD_SPEC_VERS_SHIFT;
            /* Card Command Class */
            CCC = csd_get_value(&csd, 84, 95);
        }
    }

    return init_status;
}

/*==========================================================================
FUNCTION: static cyg_uint32 card_get_csd (void)
DESCRIPTION:
   this function will read MMC/SD CSD register and store in the global Variable.

ARGUMENTS PASSED:
   None

RETURN VALUE:
   cyg_uint32

PRE-CONDITIONS:
   None

POST-CONDITIONS:
   None

Detailed Description:
  1.Send CMD9 to get CSD value of MMC/SD Card.
  2.Extract CSD value from CMDRSP0,CMDRSP1,CMDRSP2,CMDRSP3 registers.
==============================================================================*/
cyg_uint32 card_get_csd(void)
{

    command_t cmd;
    command_response_t response;
    cyg_uint32 status = FAIL;
    cyg_uint32 card_address = (Card_rca << RCA_SHIFT);

    /* Configure CMD9 for MMC/SD card */
    /* 16bit card address is expected as Argument */
    mxcmci_cmd_config(&cmd, CMD9, card_address, READ, RESPONSE_136,
              DATA_PRESENT_NONE, ENABLE, DISABLE);

    /* Issue Command CMD9 to Extrace CSD register contents     */

    if (host_send_cmd(&cmd) != FAIL) {
        /* Read Command response */
        response.format = RESPONSE_136;
        host_read_response(&response);

        /* Assign Response to CSD Strcuture */
        csd.csd0 = response.cmd_rsp0;
        csd.csd1 = response.cmd_rsp1;
        csd.csd2 = response.cmd_rsp2;
        csd.csd3 = response.cmd_rsp3;

        flash_dprintf(FLASH_DEBUG_MAX, "CSD:%x:%x:%x:%x\n", csd.csd0,
                  csd.csd1, csd.csd2, csd.csd3);
        status = SUCCESS;
    } else {
        diag_printf("Get CSD Failed.\n");
    }

    return status;

}

static cyg_uint32 csd_get_value(cyg_uint32 * pcsd, cyg_uint32 start_bit,
                cyg_uint32 end_bit)
{
    cyg_uint32 index = (start_bit / 32);
    cyg_uint32 end_index = (end_bit / 32);
    cyg_uint32 offset = (start_bit - 8) % 32;
    cyg_uint32 end_offset = (end_bit - 8) % 32;
    cyg_uint32 value;
    cyg_uint32 temp;
    //pcsd = &(csd.csd0);
    flash_dprintf(FLASH_DEBUG_MAX,
              "start_bit=%d, end_bit=%d, index=%d, end_index=%d, offset=%d\n",
              start_bit, end_bit, index, end_index, offset);

    if (index == end_index) {
        flash_dprintf(FLASH_DEBUG_MAX, "onl1y in index register\n");
        value =
            (*((cyg_uint32 *) ((cyg_uint32) pcsd + (index << 2)))) &
            ((1 << (end_offset + 1)) - (1 << offset));
        value = (value >> offset);
    } else {
        flash_dprintf(FLASH_DEBUG_MAX, "index and index+1 registers\n");
        value =
            *((cyg_uint32 *) ((cyg_uint32) pcsd +
                      (index << 2))) & (0xFFFFFFFF -
                            (1 << offset) + 1);
        value = (value >> offset);
        temp = (1 << (offset + end_bit - start_bit - 31)) - 1;
        temp =
            (*((cyg_uint32 *) ((cyg_uint32) pcsd + (index + 1) * 4)) &
             temp);
        value += temp << (32 - offset);
    }

    flash_dprintf(FLASH_DEBUG_MAX, "%s:value=%x (CSD:%x:%x:%x:%x)\n",
              __FUNCTION__, value, *pcsd, *(pcsd + 1), *(pcsd + 2),
              *(pcsd + 3));
    return value;

}

cyg_uint32 card_get_capacity_size(void)
{
    cyg_uint32 capacity = 0;
    cyg_uint32 c_size, c_size_mult, blk_len;

    if (!csd.csd0 && !csd.csd1 && !csd.csd2 && !csd.csd3)
        flash_dprintf(FLASH_DEBUG_MAX,
                  "WARNINGS:mxcmci_init should be done first!\n");

    switch (Card_type) {
    case SD_CSD_1_0:
    case MMC_CSD_1_0:
    case MMC_CSD_1_1:
    case MMC_CSD_1_2:
        c_size = csd_get_value(&csd, 62, 73);
        c_size_mult = csd_get_value(&csd, 47, 49);
        blk_len = csd_get_value(&csd, 80, 83);
        capacity = (c_size + 1) << (c_size_mult + 2 + blk_len - 10);
        break;
    case SD_CSD_2_0:
        //blk_len = csd_get_value(&csd, 80, 83);
        c_size = csd_get_value(&csd, 48, 69);
        capacity = (c_size + 1) * 512;    /* block length is fixed to 512B */
        break;
    default:
        capacity = 1;
        break;
    }

    /* check whether the card is high capacity card */
    if(capacity>2*1024*1024)
	HighCapacityCard = 1;
    else
	HighCapacityCard = 0;

    return capacity;

}

#ifdef MX51_ERDOS
cyg_uint32 mxcmci_data_read(cyg_uint32 * dest_ptr, cyg_uint32 len,
                cyg_uint64 offset)
#else
cyg_uint32 mxcmci_data_read(cyg_uint32 * dest_ptr, cyg_uint32 len,
                cyg_uint32 offset)
#endif /* MX51_ERDOS */
{
    cyg_uint32 read_status = FAIL;

    read_status = mmc_data_read(dest_ptr, len, offset);

    if (read_status) {
        len = 0;
    }
    return len;

}

cyg_uint32 mxcmci_software_reset(void)
{
    command_t cmd;
    cyg_uint32 response = FAIL;

    /*Configure CMD0 for MMC/SD card */
    /*CMD0 doesnt expect any response */
    mxcmci_cmd_config(&cmd, CMD0, NO_ARG, READ, RESPONSE_NONE,
              DATA_PRESENT_NONE, DISABLE, DISABLE);

    /*Issue CMD0 to MMC/SD card to put in active state */
    if (host_send_cmd(&cmd) != FAIL) {
        response = SUCCESS;
    } else {
        diag_printf("Card SW Reset Failed.\n");
    }

    return response;
}

cyg_uint32 mxcmci_get_cid(void)
{

    command_t cmd;
    cyg_uint32 cid_request = FAIL;
    command_response_t response;

    /* Configure CMD2 for card */
    /* No Argument is expected for CMD2 */
    mxcmci_cmd_config(&cmd, CMD2, NO_ARG, READ, RESPONSE_136,
              DATA_PRESENT_NONE, ENABLE, DISABLE);

    /* Issue CMD2 to card to determine CID contents */
    if (host_send_cmd(&cmd) == FAIL) {
        cid_request = FAIL;
        diag_printf("Send CMD2 Failed.\n");
    } else {
        /* Read Command response  */
        response.format = RESPONSE_136;
        host_read_response(&response);

        /* Assign CID values to mmc_cid structures */
        Card_identification.cid0 = response.cmd_rsp0;
        Card_identification.cid1 = response.cmd_rsp1;
        Card_identification.cid2 = response.cmd_rsp2;
        Card_identification.cid3 = response.cmd_rsp3;

        /* Assign cid_request as SUCCESS */
        cid_request = SUCCESS;
    }

    flash_dprintf(FLASH_DEBUG_MAX, "%s:CID=%X:%X:%X:%X\n", __FUNCTION__,
              Card_identification.cid0, Card_identification.cid1,
              Card_identification.cid2, Card_identification.cid3);
    return cid_request;
}

cyg_uint32 mxcmci_trans_prepare(void)
{
    command_t cmd;
    cyg_uint32 card_state = 0;
    cyg_uint32 transfer_status = 0;
    command_response_t response;
    cyg_uint32 card_address = (Card_rca << RCA_SHIFT);

    /* Configure CMD7 for MMC card */
    /* 16bit card address is expected as Argument */
    mxcmci_cmd_config(&cmd, CMD7, card_address, READ, RESPONSE_48,
              DATA_PRESENT_NONE, ENABLE, ENABLE);

    /* Sending the card from stand-by to transfer state    */
    if (host_send_cmd(&cmd) == FAIL) {
        transfer_status = FAIL;
        diag_printf("Send CMD7 Failed.\n");
    } else {

        /* Configure CMD13 to read status of the card becuase CMD7 has R1b response */
        mxcmci_cmd_config(&cmd, CMD13, card_address, READ, RESPONSE_48,
                  DATA_PRESENT_NONE, ENABLE, ENABLE);

        if (host_send_cmd(&cmd) == FAIL) {
            transfer_status = FAIL;
            diag_printf("Send CMD13 Failed.\n");
        } else {
            /* Read Command response */
            response.format = RESPONSE_48;
            host_read_response(&response);

            card_state = CURR_CARD_STATE(response.cmd_rsp0);

            if (card_state == TRAN) {
                transfer_status = SUCCESS;

            } else {
                diag_printf("card_state: 0x%x\n", card_state);
                transfer_status = FAIL;
            }
        }

    }

    return transfer_status;

}

cyg_uint32 mxcmci_trans_status(void)
{
    command_t cmd;
    cyg_uint32 card_state = 0;
    cyg_uint32 transfer_status = 0;
    command_response_t response;
    cyg_uint32 card_address = (Card_rca << RCA_SHIFT);

    /* Configure CMD13 to read status of the card becuase CMD7 has R1b response */
    mxcmci_cmd_config(&cmd, CMD13, card_address, READ, RESPONSE_48,
              DATA_PRESENT_NONE, ENABLE, ENABLE);

    if (host_send_cmd(&cmd) == FAIL) {
        diag_printf("Fail, CMD13\n");
        transfer_status = FAIL;
    }

    else {
        /* Read Command response */
        response.format = RESPONSE_48;
        host_read_response(&response);

        card_state = CURR_CARD_STATE(response.cmd_rsp0);

        if (card_state == TRAN) {
            transfer_status = SUCCESS;
            //diag_printf("card_state: 0x%x\n", card_state);
        }

        else {
            //diag_printf("card_state: 0x%x\n", card_state);
            transfer_status = FAIL;
        }
    }
    return transfer_status;

}

void mxcmci_cmd_config(command_t * cmd_config, cyg_uint32 index,
               cyg_uint32 argument, xfer_type_t transfer,
               response_format_t format, data_present_select data,
               crc_check_enable crc, cmdindex_check_enable cmdindex)
{

    command_t *cmd;

    /* Assign cmd to cmd_config */
    cmd = cmd_config;

    /* Configure Command index */
    cmd->command = index;

    /* Configure Command Argument */
    cmd->arg = argument;

    /* Configure Data transfer type */
    cmd->data_transfer = transfer;

    /* Configure Response Format */
    cmd->response_format = format;

    /* Configure Data Present Select */
    cmd->data_present = data;

    /* Configiure CRC check Enable */
    cmd->crc_check = crc;

    /*Configure Command index check enable */
    cmd->cmdindex_check = cmdindex;

    /* if multi-block is used */
    if (CMD18 == index || CMD25 == index) {
        /*Configure Block count enable */
        cmd->block_count_enable_check = ENABLE;
        /*Configure Multi single block select */
        cmd->multi_single_block = MULTIPLE;
    } else {
        /*Configure Block count enable */
        cmd->block_count_enable_check = DISABLE;

        /*Configure Multi single block select */
        cmd->multi_single_block = SINGLE;
    }
}
