// ==========================================================================
//
//   mxcmci_mmc.c
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
// 2009/07/13 : mmc_data_write/read() offset 64bit.
//
//==========================================================================

#include <cyg/io/mxcmci_host.h>
#include <cyg/io/mxcmci_core.h>
#include <cyg/io/mxcmci_mmc.h>
#include <cyg/io/mxc_mmc.h>

extern int HighCapacityCard;

static cyg_uint32 mmc_set_rca(void);
static cyg_uint32 mmc_set_bus_width_sector_sz(cyg_uint32 bus_width);
static cyg_uint32 mmc_set_high_speed_mode(void);

cyg_uint32 address_mode;    /* Global variable for addressing mode */

cyg_uint32 mmc_init(cyg_uint32 bus_width)
{
    cyg_uint32 status = FAIL;
    cyg_uint32 spec_version;
    /* Get CID number of MMC Card */
    if (!mxcmci_get_cid()) {
        /* Set RCA of the MMC Card */
        if (!mmc_set_rca()) {
            flash_dprintf(FLASH_DEBUG_MAX, "%s:  mmc_set_rca OK!",
                      __FUNCTION__);
            /* Get Spec version supported by the card */
            spec_version = mmc_get_spec_ver();
            //diag_printf("SPEC Version:  %d\n", spec_version);

            /*Enable operating frequency */
            host_cfg_clock(OPERATING_FREQ);

            /*Put MMC in Transfer State */
            if (!mxcmci_trans_prepare()) {
#if 0
                if (mmc_set_high_speed_mode()) {
                    return FAIL;
                }
#endif

                flash_dprintf(FLASH_DEBUG_MAX, "%s:  mxcmci_trans_prepare OK!",
                          __FUNCTION__);

                if (!mmc_set_bus_width_sector_sz(bus_width)) {
                    esdhc_base_pointer->protocol_control &= ~(0x3 << 1);
                    esdhc_base_pointer->protocol_control |= (bus_width >> 2) << 1;
                    status = SUCCESS;
                    diag_printf("Bus Width:    %d\n",
                            bus_width);
                }

            }
        }
    }

    return status;

}

#ifdef MX51_ERDOS
cyg_uint32 mmc_data_read(cyg_uint32 * dest_ptr, cyg_uint32 length,
             cyg_uint64 offset64)
#else
cyg_uint32 mmc_data_read(cyg_uint32 * dest_ptr, cyg_uint32 length,
             cyg_uint32 offset)
#endif /* MX51_ERDOS */
{
    command_t cmd;
    int len;
    cyg_uint32 read_block_status = 0;
    cyg_uint32 blk_len = BLK_LEN;
    unsigned int SectorNum = 0;
#ifdef MX51_ERDOS
    cyg_uint32 offset;
#endif /* MX51_ERDOS */

    /* Assing length of data to be read */
    SectorNum = length / blk_len;
    if ((length % blk_len) != 0)
        SectorNum++;
    /* hight capacity card uses sector mode */
#ifdef MX51_ERDOS
    if (HighCapacityCard) {
	offset = offset64 / 512;
    } else {
        offset = (cyg_uint32)offset64;
    }
#else
    if(HighCapacityCard)
	offset = offset/512;
#endif /* MX51_ERDOS */

    /* wait until in transfer mode */
    while (mxcmci_trans_status()) {
        hal_delay_us(5);
    }

      reread:
    /* Configure interface block and number of blocks */
    host_cfg_block(BLK_LEN, SectorNum);

    if (SectorNum == 1) {
        //diag_printf("Send CMD17...\n");
        /* Comfigure command CMD17 for single block read */
        mxcmci_cmd_config(&cmd, CMD17, offset, READ, RESPONSE_48,
                  DATA_PRESENT, ENABLE, ENABLE);

        if (host_send_cmd(&cmd) == FAIL) {
            diag_printf("%s: Can't send CMD17!\n", __FUNCTION__);
            esdhc_softreset(ESDHC_RESET_CMD_MSK |
                    ESDHC_RESET_DAT_MSK);
            read_block_status = FAIL;

        } else {
            //diag_printf("host_data_read! dest_ptr: 0%x \n", dest_ptr);
            /* Call interface Data read function */
            read_block_status = host_data_read(dest_ptr, BLK_LEN);

            if (read_block_status) {    /* fail */
                //diag_printf("%s: Failed, read_block_status =%d\n", __FUNCTION__, read_block_status);
                /* re-transfer if data transfer error occurs */
                goto reread;
            }
        }
    } else {        /* read multi-blocks */

        /* Comfigure command CMD18 for multiple block read */
        mxcmci_cmd_config(&cmd, CMD18, offset, READ, RESPONSE_48,
                  DATA_PRESENT, ENABLE, ENABLE);

        if (host_send_cmd(&cmd) == FAIL) {
            diag_printf("%s: Can't send CMD18!\n", __FUNCTION__);
            esdhc_softreset(ESDHC_RESET_CMD_MSK | ESDHC_RESET_DAT_MSK);
            read_block_status = FAIL;
        } else {
            /* Call interface Data read function */
            read_block_status =
                host_data_read(dest_ptr, BLK_LEN * SectorNum);

            /* Comfigure command CMD12 for multi-block read stop */
            mxcmci_cmd_config(&cmd, CMD12, 0, READ, RESPONSE_48,
                      DATA_PRESENT_NONE, ENABLE, ENABLE);

            if (host_send_cmd(&cmd) == FAIL) {
                diag_printf("%s: Can't send CMD12!\n",
                        __FUNCTION__);
                esdhc_softreset(ESDHC_RESET_CMD_MSK | ESDHC_RESET_DAT_MSK);
                //read_block_status = FAIL;
            }

            if (read_block_status) {    /* fail */
                //diag_printf("%s: Failed, read_block_status =%d\n", __FUNCTION__, read_block_status);
                /* re-transfer if data transfer error occurs */
                goto reread;
            }

        }

    }
    return read_block_status;
}

#ifdef MX51_ERDOS
cyg_uint32 mmc_data_write(cyg_uint32 * src_ptr, cyg_uint32 length,
              cyg_uint64 offset64)
#else
cyg_uint32 mmc_data_write(cyg_uint32 * src_ptr, cyg_uint32 length,
              cyg_uint32 offset)
#endif /* MX51_ERDOS */
{

    command_t cmd;
    cyg_int32 len;
    cyg_uint32 blk_len = BLK_LEN;
    cyg_uint32 write_block_status = SUCCESS;
    unsigned int SectorNum;
#ifdef MX51_ERDOS
    cyg_uint32 offset;
#endif /* MX51_ERDOS */

    //int counter;
    //diag_printf("%s: src: 0x%x, offset: 0x%x, length: 0x%x\n", __FUNCTION__, (unsigned int)src_ptr, offset, length);
    /* Write data size aligned with block size */
    SectorNum = length / blk_len;
    if ((length % blk_len) != 0)
        SectorNum++;

    /* hight capacity card uses sector mode */
#ifdef MX51_ERDOS
    if(HighCapacityCard) {
        offset = offset64 / 512;
    } else {
        offset = (cyg_uint32)offset64;
    }
#else
    if(HighCapacityCard)
        offset = offset/512;
#endif /* MX51_ERDOS */

    //need waiting until CARD out of Prg status, or will cause CMD25 timeout
    //hal_delay_us(100);

    //StartCounter();

    while (mxcmci_trans_status()) {
        hal_delay_us(2);
    }

    //counter = StopCounter();
    //diag_printf("counter: 0x%x\n",counter);

      rewrite:
    /* Configure interface block and number of blocks , SctorNum will decrease to zero after transfer */
    host_cfg_block(BLK_LEN, SectorNum);

    if (SectorNum == 1) {
        //diag_printf("Send CMD24...\n");
        /* Comfigure command CMD24 for single block write */
        mxcmci_cmd_config(&cmd, CMD24, offset, WRITE, RESPONSE_48,
                  DATA_PRESENT, ENABLE, ENABLE);

        if (host_send_cmd(&cmd) == FAIL) {
            diag_printf("%s: Failed in configuring CMD24\n",
                    __FUNCTION__);
            esdhc_softreset(ESDHC_RESET_CMD_MSK | ESDHC_RESET_DAT_MSK);
            write_block_status = FAIL;

            //hal_delay_us(1000);
            goto rewrite;

        } else {
            //diag_printf("Start host_data_write:\n");
            /* Call interface write read function */
            write_block_status = host_data_write(src_ptr, BLK_LEN);
            //diag_printf("0x%x\n", esdhc_base_pointer->present_state);

            if (write_block_status) {    /* fail */
                //diag_printf("transfer failed.(0x%x)\n", esdhc_base_pointer->block_attributes);
                while (mxcmci_trans_status()) ;
                //diag_printf("%s: Failed, write_block_status=%d\n", __FUNCTION__, write_block_status);
                /* re-transfer */
                goto rewrite;
            }

        }
    } else {        /* multi-block write */

        //diag_printf("Send CMD25...\n");
        /* Comfigure command CMD25 for single block write */
        mxcmci_cmd_config(&cmd, CMD25, offset, WRITE, RESPONSE_48,
                  DATA_PRESENT, ENABLE, ENABLE);

        if (host_send_cmd(&cmd) == FAIL) {
            //diag_printf("%s: Failed in configuring CMD25\n",
            //        __FUNCTION__);
            esdhc_softreset(ESDHC_RESET_CMD_MSK | ESDHC_RESET_DAT_MSK);
            write_block_status = FAIL;
            goto rewrite;
        } else {
            /* Call interface write read function */
            write_block_status =
                host_data_write(src_ptr, SectorNum * BLK_LEN);

            /* Comfigure command CMD12 for multi-block read stop */
            mxcmci_cmd_config(&cmd, CMD12, 0, READ, RESPONSE_48,
                      DATA_PRESENT_NONE, ENABLE, ENABLE);

            if (host_send_cmd(&cmd) == FAIL) {
                diag_printf("%s: Can't send CMD12!\n",
                        __FUNCTION__);
                esdhc_softreset(ESDHC_RESET_CMD_MSK | ESDHC_RESET_DAT_MSK);
                //write_block_status = FAIL;
            }

            if (write_block_status) {    /* fail */
                //diag_printf("%s: Failed, write_block_status=%d\n", __FUNCTION__, write_block_status);
                while (mxcmci_trans_status());
                /* re-transfer */
                goto rewrite;
            }
        }
    }

    return write_block_status;

}

cyg_uint32 mmc_data_erase(cyg_uint32 offset, cyg_uint32 size)
{
    command_t cmd;
    extern int Card_Mode;
    cyg_uint8 startEraseBlockCmd = CMD35;
    cyg_uint8 endEraseBlockCmd = CMD36;

    cyg_uint32 startBlock = offset / BLK_LEN;
    cyg_uint32 endBlock = (offset + size - 1) / BLK_LEN;
    cyg_uint32 ret;
//    diag_printf("card_data_erase\n");
    if (Card_Mode == 0) {
        startBlock *= BLK_LEN;
        endBlock *= BLK_LEN;
        startEraseBlockCmd = CMD35;
        endEraseBlockCmd = CMD36;
    }

    else if (Card_Mode == 1) {
        startBlock *= BLK_LEN;
        endBlock *= BLK_LEN;
        startEraseBlockCmd = CMD32;
        endEraseBlockCmd = CMD33;
    }
#if 1
    /* hight capacity card uses sector mode */
    if(HighCapacityCard)
        startBlock /= BLK_LEN;
        endBlock /= BLK_LEN;
#endif
//	diag_printf("0x%x - 0x%x, size: 0x%x\n", startBlock, endBlock, size);
    /* Configure start erase command to set first block */
    mxcmci_cmd_config(&cmd, startEraseBlockCmd, startBlock, READ,
              RESPONSE_48, DATA_PRESENT_NONE, ENABLE, ENABLE);
    /* wait response */
    if ((ret = host_send_cmd(&cmd)) == SUCCESS) {
        flash_dprintf(FLASH_DEBUG_MAX,
                  "%s: successful for host_send_cmd\n",
                  __FUNCTION__);
        /* Configure end erase command to set end block */
        mxcmci_cmd_config(&cmd, endEraseBlockCmd, endBlock, READ,
                  RESPONSE_48, DATA_PRESENT_NONE, ENABLE, ENABLE);
        if ((ret = host_send_cmd(&cmd)) == SUCCESS) {
            flash_dprintf(FLASH_DEBUG_MAX,
                      "%s: successful for host_send_cmd:2\n",
                      __FUNCTION__);
            /* Comfigure command to start erase */
            mxcmci_cmd_config(&cmd, CMD38, 0, READ, RESPONSE_48,
                      DATA_PRESENT_NONE, ENABLE, ENABLE);
            if ((ret = host_send_cmd(&cmd)) == SUCCESS) {
                flash_dprintf(FLASH_DEBUG_MAX,
                          "%s: successful for host_send_cmd:3\n",
                          __FUNCTION__);
                //wait for completion
                return ret;
            }
        }
    }

    flash_dprintf(FLASH_DEBUG_MAX, "%s: Error return (%d)\n", __FUNCTION__,
              ret);
    return ret;
}

cyg_uint32 mmc_voltage_validation(void)
{
    command_t cmd;
    command_response_t response;
    cyg_uint32 voltage_validation_command = 0;
    cyg_uint32 ocr_val = 0;
    cyg_uint32 voltage_validation = FAIL;

    ocr_val = (cyg_uint32) ((MMC_OCR_VALUE) & 0xFFFFFFFF);

    while ((voltage_validation_command < MMCSD_READY_TIMEOUT)
           && (voltage_validation != SUCCESS)) {
        /* Configure CMD1 for MMC card */
        mxcmci_cmd_config(&cmd, CMD1, ocr_val, READ, RESPONSE_48,
                  DATA_PRESENT_NONE, DISABLE, DISABLE);

        /* Issue CMD1 to MMC card to determine OCR value */
        if (host_send_cmd(&cmd) == FAIL) {
            voltage_validation = FAIL;
            break;
        } else {
            /* Read Response from CMDRSP0 Register */
            response.format = RESPONSE_48;
            host_read_response(&response);

            /* Check if card busy bit is cleared or not */
            if (!(response.cmd_rsp0 & CARD_BUSY_BIT)) {
                /* Iterate One more time */
                voltage_validation_command++;
            } else {
                if ((response.cmd_rsp0 & MMC_OCR_HC_RES) ==
                    MMC_OCR_HC_RES) {
                    address_mode = SECT_MODE;
                    voltage_validation = SUCCESS;
                } else if ((response.cmd_rsp0 & MMC_OCR_LC_RES)
                       == MMC_OCR_LC_RES) {
                    address_mode = BYTE_MODE;
                    voltage_validation = SUCCESS;
                }
            }

        }
    }

    return voltage_validation;
}

static cyg_uint32 mmc_set_rca(void)
{
    command_t cmd;
    cyg_uint32 card_state = 0;
    cyg_uint32 rca_request = 0;
    command_response_t response;
    cyg_uint32 card_address = (Card_rca << RCA_SHIFT);

    /* Configure CMD3 for MMC card */
    /* 32bit card address is expected as Argument */
    mxcmci_cmd_config(&cmd, CMD3, card_address, READ, RESPONSE_48,
              DATA_PRESENT_NONE, ENABLE, ENABLE);

    /* Assigns relative address to the card
     */

    if (host_send_cmd(&cmd) == FAIL) {
        rca_request = FAIL;
    }

    else {
        /* Read Command response */
        response.format = RESPONSE_48;
        host_read_response(&response);
        card_state = CURR_CARD_STATE(response.cmd_rsp0);
        if (card_state == IDENT) {
            rca_request = SUCCESS;

        } else {
            rca_request = FAIL;
        }
    }

    return rca_request;
}

cyg_uint32 mmc_get_spec_ver(void)
{

    cyg_uint32 mmc_spec_version;

    if (card_get_csd() == FAIL) {
        mmc_spec_version = 0;
    } else {
        mmc_spec_version = ((csd.csd3 && MMC_SPEC_VER) >> MMC_SPEC_VER_SHIFT);
    }

    return mmc_spec_version;

}

cyg_uint32 card_flash_query(void *data)
{
    command_t cmd;
    cyg_uint32 cid_request = FAIL;
    command_response_t response;

    /* Configure CMD2 for card */
    mxcmci_cmd_config(&cmd, CMD2, NO_ARG, READ, RESPONSE_136,
              DATA_PRESENT_NONE, ENABLE, DISABLE);
    /* Issue CMD2 to card to determine CID contents */
    if (host_send_cmd(&cmd) == FAIL) {
        cid_request = FAIL;
        flash_dprintf(FLASH_DEBUG_MAX, "%s: can't send query command\n",
                  __FUNCTION__);
    } else {
        cyg_uint32 *d = (cyg_uint32 *) data;
        /* Read Command response  */
        response.format = RESPONSE_136;
        host_read_response(&response);
        /* Assign CID values to mmc_cid structures */
        *d++ = response.cmd_rsp0;
        *d++ = response.cmd_rsp1;
        *d++ = response.cmd_rsp2;
        *d = response.cmd_rsp3;

        /* Assign cid_request as SUCCESS */
        cid_request = SUCCESS;
    }
    flash_dprintf(FLASH_DEBUG_MAX,
              "%s(Success?=%d):(ID=0x%x: 0x%x, 0x%x, 0x%x)\n",
              __FUNCTION__, cid_request, *(cyg_uint32 *) (data),
              *(cyg_uint32 *) ((cyg_uint32) data + 4),
              *(cyg_uint8 *) ((cyg_uint32) data + 8),
              *(cyg_uint8 *) ((cyg_uint32) data + 12));
    return;
}

static cyg_uint32 mmc_set_bus_width_sector_sz(cyg_uint32 bus_width)
{
    command_t cmd;
    cyg_uint32 set_bus_width_status = FAIL;
    command_response_t response;
    cyg_uint32 card_address = (Card_rca << RCA_SHIFT);

    if ((bus_width == FOUR) || (bus_width == EIGHT) || (bus_width == ONE)) {

        /* Configure CMD6 to write to EXT_CSD register for BUS_WIDTH */
        mxcmci_cmd_config(&cmd, CMD6, 0x03b70001 | ((bus_width >> 2) << 8), READ,
                  RESPONSE_48, DATA_PRESENT_NONE, ENABLE, ENABLE);

        if (host_send_cmd(&cmd) == SUCCESS) {
            /* wait until in transfer mode */
            while (mxcmci_trans_status()) {
                hal_delay_us(5);
            }
            mxcmci_cmd_config(&cmd, CMD16, 512, READ,
                      RESPONSE_48, DATA_PRESENT_NONE, ENABLE, ENABLE);
            if (host_send_cmd(&cmd) == SUCCESS) {
                set_bus_width_status = SUCCESS;
            } else
                diag_printf("Setting MMC sector size failed.\n");
        } else {
            diag_printf("Setting MMC bus width failed.\n");
        }
    }

    return set_bus_width_status;
}

static cyg_uint32 mmc_set_high_speed_mode(void)
{
    command_t cmd;
    command_response_t response;
    cyg_uint32 status = FAIL;

    //diag_printf("Send CMD6 to Set High Speed Mode.\n");
    /* Configure CMD6 to write to EXT_CSD register for BUS_WIDTH */
    mxcmci_cmd_config(&cmd, CMD6, 0x03b90100, READ, RESPONSE_48,
              DATA_PRESENT_NONE, ENABLE, ENABLE);

    if (host_send_cmd(&cmd) == SUCCESS) {
        /* wait until in transfer mode */
        while (mxcmci_trans_status()) {
            hal_delay_us(5);
        }

        status = SUCCESS;
    } else {
        diag_printf("Setting MMC High Speed Mode FAILED.\n");
    }

    return status;
}

int sdmmc_set_blklen(int len)
{
    int status = FAIL;
    command_t cmd;
    command_response_t response;

    /* Configure CMD16 to set block length as 512 bytes. */
    mxcmci_cmd_config(&cmd, CMD16, len, READ, RESPONSE_48,
              DATA_PRESENT_NONE, ENABLE, ENABLE);

    /* Issue command CMD16 to set block length as 512 bytes */
    if (host_send_cmd(&cmd) == FAIL) {
        diag_printf("%s: Can't set block length!(CMD16)\n",
                __FUNCTION__);
        esdhc_softreset(ESDHC_RESET_CMD_MSK);
        status = FAIL;
    } else {
        status = SUCCESS;
    }

    return status;
}

int sdmmc_stop_transmission(void)
{
    int status = FAIL;
    command_t cmd;
    command_response_t response;

    /* Comfigure command CMD12 for read stop */
    mxcmci_cmd_config(&cmd, CMD12, 0, READ, RESPONSE_48,
              DATA_PRESENT_NONE, ENABLE, ENABLE);

    if (host_send_cmd(&cmd) == FAIL) {
        //diag_printf("%s: Can't send CMD12!\n", __FUNCTION__);
        //esdhc_softreset(ESDHC_RESET_CMD_MSK | ESDHC_RESET_DAT_MSK);
        //read_block_status = FAIL;
    }

    return 0;
}

static unsigned int mmc_set_extendCSD(unsigned int ECSD_index, unsigned int value, unsigned int access_mode)
{
    unsigned int argument = 0;
    command_t cmd;

    /* access mode: 0b01 set bits/ 0b10 clear bits/ 0b11 write bytes */
    argument = (access_mode << 24) | (ECSD_index << 16) | (value << 8);
    //argument = 0x1b30000;

    mxcmci_cmd_config(&cmd, CMD6, argument, READ, RESPONSE_48,
                      DATA_PRESENT_NONE, ENABLE, ENABLE);

    if(host_send_cmd(&cmd) == SUCCESS) {
        return 0;
    } else {
        //diag_printf("%s: Setting MMC boot Failed.\n", __FUNCTION__);
        return 1;
    }
}

static void mmc_set_boot_partition_size(unsigned int value)
{
    command_t cmd;
    command_response_t response;
    cyg_uint32 card_state = 0;
    cyg_uint32 card_address = (Card_rca << RCA_SHIFT);

    mxcmci_cmd_config(&cmd, CMD62, 0XEFAC62EC, READ, RESPONSE_48,
                      DATA_PRESENT_NONE, ENABLE, ENABLE);
    host_send_cmd(&cmd);

    mxcmci_cmd_config(&cmd, CMD62, 0X00CBAEA7, READ, RESPONSE_48,
                      DATA_PRESENT_NONE, ENABLE, ENABLE);
    host_send_cmd(&cmd);

    mxcmci_cmd_config(&cmd, CMD62, value, READ, RESPONSE_48,
                      DATA_PRESENT, ENABLE, ENABLE);
    host_send_cmd(&cmd);
}

cyg_uint32 emmc_set_boot_partition (cyg_uint32 *src_ptr, cyg_uint32 length)
{
    cyg_uint32 status=FAIL;
    unsigned int value;
    unsigned int eMMCBootDataSize = (length / (128 * 1024)) + 1;

    if (MMC_Spec_vers < 4)
        return 1;

    /* read back 1KB data as we are programming to user are and want to aviod erasing MBR
      * will be removed once we program Redboot to boot partition of the card
      */
#ifdef MX51_ERDOS
    mmc_data_read(src_ptr, 0x400, (cyg_uint64)0);
#else
    mmc_data_read(src_ptr, 0x400, 0);
#endif /* MX51_ERDOS */

    /* Set boot partition */
    /* 1. Configure CMD6 to write to EXT_CSD register for eMMC boot partition, Byte 179*/
    /* boot partition: user area enable and r/w enable */
    value = (0x7 << 3) | (0x7);
    //value = (0x1 << 3) | (0x1);
    status = mmc_set_extendCSD(179, value, 0x3);
    if(status) {
        return 1; /* failed */
    }

    /* 2. Set boot partition size: n*128KB */
    value = eMMCBootDataSize;
    //status = mmc_set_extendCSD(226, value, 0x3);
    //if(status) {
    //    return 1; /* failed */
    //}
    //mmc_set_boot_partition_size(value);

    //diag_printf("Boot partition size: 0x%xKB\n", eMMCBootDataSize * 128);

    /* 3. Program to boot partition, default address is alway 0x0  */
#ifdef MX51_ERDOS
    status = mmc_data_write (src_ptr, eMMCBootDataSize*128*1024, (cyg_uint64)0);
#else
    status = mmc_data_write (src_ptr, eMMCBootDataSize*128*1024, 0);
#endif /* MX51_ERDOS */
    if(status) {
        return 1; /* failed */
    }

    while (mxcmci_trans_status());

    /* 4. Clear boot partition access bits, to protect w/r of boot partition */
    /* bit 6: send boot ack signal, boot partition: user area enable and r/w access disable */
    //value = (0x1 << 6) | (0x1 << 3) | (0x0);
    value = (0x1 << 6) | (0x7 << 3) | (0x0);
    status = mmc_set_extendCSD(179, value, 0x3);
    if(status) {
        return 1; /* failed */
    }

    return 0;
}

/* end of mxcmci_mmc.c */
