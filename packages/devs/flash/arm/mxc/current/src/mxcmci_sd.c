// ==========================================================================
//
//   mxcmci_sd.c
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
// 2009/07/13 : mmc_data_write() offset 64bit.
//
//==========================================================================

#include <cyg/io/mxcmci_host.h>
#include <cyg/io/mxcmci_core.h>
#include <cyg/io/mxcmci_mmc.h>

static cyg_uint32 sd_get_rca(void);
static cyg_uint32 sd_get_bit_mode_support(void);
static cyg_uint32 sd_set_bus_width(cyg_uint32);
static cyg_uint32 sd_set_high_speed_mode(void);

#define SD_OCR_VALUE_HV_LC 0x00ff8000    /* nirp_oct07: <- 3.3v, LC */
#define SD_OCR_VALUE_HV_HC 0x40ff8000    /* nirp_oct07: <- 3.3v, HC */
/* nirp_oct07: LV_LC not needed - 1.8v is only supported under eSD which supports HC by default (SD>2.00) */
#define SD_OCR_VALUE_LV_HC 0x40000080    /* nirp_oct07: <- 1.8v, HC */

#define SD_OCR_HC_RES 0x40000000
#define SD_OCR_LC_RES 0x00000000

#define SD_IF_HV_COND_ARG 0x000001AA
#define SD_IF_LV_COND_ARG 0x000002AA

#define RCA_SHIFT 16
#define SD_R1_STATUS_APP_CMD_MSK 0x20
#define BIT_MODE_4_SUPPORT 5
#define SD_BUS_WIDTH_OFFSET 6
#define BIT_4_MODE 4
#define SD_STATUS_LEN 64

#define SD_BOOT_SWITCH_ARG 0x80FFFF2F
#define SD_PARTITION1 0x01000000

cyg_uint32 sd_init(cyg_uint32 bus_width)
{
    cyg_uint32 status = FAIL;
    cyg_uint32 bus_size = bus_width;

    /* Get CID number of SD Memory Card */
    if (!mxcmci_get_cid()) {
        diag_printf("%s:mxcmci_get_cid OK!\n", __FUNCTION__);
        /* Set RCA of the SD Card */
        if (!sd_get_rca()) {
            diag_printf("%s:sd_get_rca OK!\n", __FUNCTION__);
            /*Get CSD from Card */
            if (card_get_csd())
                return FAIL;

            /*Enable operating frequency */
            host_cfg_clock(OPERATING_FREQ);

            //diag_printf("Set SD Card in Transfer State.\n");

            /*Put SD Card in Transfer State */
            if (!mxcmci_trans_prepare()) {
#if 0
                if (sd_set_high_speed_mode()) {
                    return FAIL;
                }
#endif

                if (sdmmc_set_blklen(BLK_LEN))
                    return FAIL;

                /* SD can only support 1/4 bit bitwidth, 8 bit is not supported */
                if (EIGHT == bus_width) {
                    bus_width = FOUR;
                }
                if (!sd_set_bus_width(bus_width)) {
                    esdhc_base_pointer->protocol_control &=
                        ~(0x3 << 1);
                    esdhc_base_pointer->protocol_control |=
                        (bus_width / 4) << 1;
                    diag_printf("Bus Width:    %d\n",
                            bus_width);
                    status = SUCCESS;
                }
            }
        }

    } else {
        diag_printf("Get CID Failed.\n");

    }

    //diag_printf("%s:failed to Init SD card!\n", __FUNCTION__);
    return status;

}

cyg_uint32 sd_voltage_validation(void)
{
    //wait max timeout (unit: ms)
    cyg_uint32 timeout = 15000;

    command_t cmd;
    command_response_t response;
    cyg_uint32 voltage_validation_command = 0;
    cyg_uint32 default_rca = 0;

    cyg_uint32 ocr_value = SD_OCR_VALUE_HV_LC;    /* nirp_oct07: <- split OCR to 3.3v and 1.8v cases */
    cyg_uint32 voltage_validation = FAIL;
    cyg_uint32 interface_value = 0;
    cyg_uint32 card_usable = SUCCESS;

    /* Configure Command CMD8 to check for High capacity support */
    /* try 3.3V first */
    mxcmci_cmd_config(&cmd, CMD8, SD_IF_HV_COND_ARG, READ, RESPONSE_48,
              DATA_PRESENT_NONE, ENABLE, ENABLE);

    /* Issue Command CMD8  to SD Memory card */
    if (host_send_cmd(&cmd) == SUCCESS) {    /* nirp_oct07: <- changed order of detection */
        diag_printf("%s:CMD8 OK!\n", __FUNCTION__);
        response.format = RESPONSE_48;
        host_read_response(&response);

        /* Obtain Interface value from the response buffer */
        interface_value = response.cmd_rsp0;

        /* Check if volatge lies in range or not */
        if ((interface_value & SD_IF_HV_COND_ARG) == SD_IF_HV_COND_ARG) {
            ocr_value = ((cyg_uint32) (SD_OCR_VALUE_HV_HC) & 0xFFFFFFFF);    /* nirp_oct07: <- split OCR to 3.3v and 1.8v cases */
        }



        /* start timer for a  delay of 1.5sec, for ACMD41 */
        hal_delay_us(1500);

        while ((voltage_validation_command < 200)
               && (voltage_validation != SUCCESS)
               && (card_usable == SUCCESS)) {
            /* Configure CMD55 for SD card */
            /* This command expects defualt RCA 0x0000 as argument. */
            mxcmci_cmd_config(&cmd, CMD55, default_rca, READ,
                      RESPONSE_48, DATA_PRESENT_NONE,
                      ENABLE, ENABLE);

            /* Issue CMD55 to SD Memory card */
            if (host_send_cmd(&cmd) == FAIL) {
                voltage_validation = FAIL;
                diag_printf("Send CMD55 Failed.\n");
                break;
            } else {
	    	 diag_printf("Send CMD55 OK.\n");

                /* Configure ACMD41 for SD card */
                /* This command expects operating voltage range as argument. */
                /* CODE REVIEW START: Need to check why BUSY was expected */
                /* INTERNAL CODE REVIEW: Accepted - to fix original code if needed */
                /* nirp: changed RESPONSE_48_CHECK_BUSY to RESPONSE_48 */
                /* nirp_oct03: why with busy again? ACMD41 doesn't hold busy line */
                mxcmci_cmd_config(&cmd, ACMD41, ocr_value, READ,
                          RESPONSE_48, DATA_PRESENT_NONE, DISABLE,
                          DISABLE);



                /* Issue ACMD41 to SD Memory card to determine OCR value */
                if (host_send_cmd(&cmd) == FAIL) {
                    voltage_validation = FAIL;
                    diag_printf("Send CMD41 Failed.\n");
                    break;
                } else {
		    diag_printf("Send CMD41 OK.\n");
                    /* Read Response from CMDRSP0 Register */
                    response.format = RESPONSE_48;
                    host_read_response(&response);

                    /* Obtain OCR Values from the response */
                    /* Obtain OCR value from the response buffer */
                    ocr_value = response.cmd_rsp0;

                    /* Check if card busy bit is cleared or not */
                    if (!(response.cmd_rsp0 & CARD_BUSY_BIT)) {
                        /* Iterate One more time */
                        voltage_validation_command++;
			diag_printf("Card busy bit still not cleared....\n");
                    } else {
		    	diag_printf("Card busy bit cleared !! \n");

                        /*CODE REVIEW START: Update code and check only bit 30, HC or LC card type. All voltage bits needs to be masked. */
                        /* INTERNAL CODE REVIEW: Accepted - need fix the code accordingly */
                        /* nirp: It may be better to check the actual power supply voltage - requiring the entire range (0xff8000) may fail the sequence even if the device can be supported */
                        /*CODE REVIEW END: */

                        if ((response.cmd_rsp0 & SD_OCR_HC_RES) == SD_OCR_HC_RES) {
                            address_mode = SECT_MODE;
                            voltage_validation = SUCCESS;
                        }
                        /* CODE REVIEW 3: (same as above) Check is logically correct, but seems redundent.
                           Anything that fails the HC check, is assumed Low Capacity */
                        /* nirp_oct03: this can be just an "else". the LC macro is 0 anyway,
                           and anything not HC is LC by default */
                        /* removed else if */
                        else {
                            address_mode = BYTE_MODE;
                            voltage_validation = SUCCESS;
                        }
                    }
                }
            }

            hal_delay_us(1000);
	    
    	    // Temporarily add the delay. 
    	    hal_delay_us(2 * 1000 * 10);
	    
        }

        if (voltage_validation == FAIL) {
            card_usable = FAIL;
        }

    } else {
        /*3.3v test failed, try to test 1.8v mode! */
        mxcmci_cmd_config(&cmd, CMD8, SD_IF_LV_COND_ARG, READ,
                  RESPONSE_48, DATA_PRESENT_NONE, ENABLE,
                  ENABLE);

        /* Issue Command CMD8  to SD Memory card */
        if (host_send_cmd(&cmd) == FAIL) {
            diag_printf("%s:CMD8 for 1.8v failed!\n", __FUNCTION__);
            /* nirp_oct07: CMD8 failed both in 3.3 and in 1.8v, try SD 1.x case - no CMD8, LC, 3.3v only */
            ocr_value = ((cyg_uint32) (SD_OCR_VALUE_HV_LC) & 0xFFFFFFFF);    /* nirp_oct07: <- changed order of detection */
        } else {
            diag_printf("%s:CMD8 for 1.8v OK!\n", __FUNCTION__);
            response.format = RESPONSE_48;
            host_read_response(&response);

            /* Obtain Interface value from the response buffer */
            interface_value = response.cmd_rsp0;

            /* Check if volatge lies in range or not */
            if ((interface_value & SD_IF_LV_COND_ARG) == SD_IF_LV_COND_ARG) {
                ocr_value = ((cyg_uint32) (SD_OCR_VALUE_LV_HC) & 0xFFFFFFFF);    /* nirp_oct07: <- split OCR to 3.3v and 1.8v cases */
            }
            /* nirp_oct07: otherwise, try with HV_LC settings (set at function start) */
        }

    }

    /* start timer for a  delay of 1.5sec, for ACMD41 */
    hal_delay_us(1500);

    /* nirp_oct03: MMCSD_READY_TIMEOUT too long.
       ACMD41 also takes longer than CMD1 (twice - ~200 clocks for CMD55+resp+CMD41+resp */
    /* In any case ,ACMD 41 will loop not more than 1.5 sec */
    while ((voltage_validation_command < 200)
           && (voltage_validation != SUCCESS) && (card_usable == SUCCESS)) {
        /* Configure CMD55 for SD card */
        /* This command expects defualt RCA 0x0000 as argument. */
        mxcmci_cmd_config(&cmd, CMD55, default_rca, READ, RESPONSE_48,
                  DATA_PRESENT_NONE, ENABLE, ENABLE);

        /* Issue CMD55 to SD Memory card */
        if (host_send_cmd(&cmd) == FAIL) {
            voltage_validation = FAIL;
            diag_printf("Send CMD55 Failed!\n");
            break;
        } else {
	    diag_printf("Send CMD55 OK!\n");
            /* Configure ACMD41 for SD card */
            /* This command expects operating voltage range as argument. */
            /* CODE REVIEW START: Need to check why BUSY was expected */
            /* INTERNAL CODE REVIEW: Accepted - to fix original code if needed */
            /* nirp: changed RESPONSE_48_CHECK_BUSY to RESPONSE_48 */
            /* nirp_oct03: why with busy again? ACMD41 doesn't hold busy line */
            mxcmci_cmd_config(&cmd, ACMD41, ocr_value, READ,
                      RESPONSE_48, DATA_PRESENT_NONE,
                      DISABLE, DISABLE);

            /* CODE REVIEW END:  */

            /* Issue ACMD41 to SD Memory card to determine OCR value */
            if (host_send_cmd(&cmd) == FAIL) {
                voltage_validation = FAIL;
                diag_printf("Send ACMD41 Failed!\n");
                break;
            } else {
	    	diag_printf("Send ACMD41 OK !\n");
                /* Read Response from CMDRSP0 Register */
                response.format = RESPONSE_48;
                host_read_response(&response);

                /* Obtain OCR Values from the response */
                /* Obtain OCR value from the response buffer
                 */
                ocr_value = response.cmd_rsp0;

                /* Check if card busy bit is cleared or not */
                if (!(response.cmd_rsp0 & CARD_BUSY_BIT)) {
                    /* Iterate One more time */
                    voltage_validation_command++;
		    
		    diag_printf("Card busy bit still not cleared....\n");
                } else {
		    diag_printf("Card busy bit cleared !! \n");
		  
                    /*CODE REVIEW START: Update code and check only bit 30, HC or LC card type. All voltage bits needs to be masked. */
                    /* INTERNAL CODE REVIEW: Accepted - need fix the code accordingly */
                    /* nirp: It may be better to check the actual power supply voltage - requiring the entire range (0xff8000) may fail the sequence even if the device can be supported */
                    /*CODE REVIEW END: */

                    if ((response.cmd_rsp0 & SD_OCR_HC_RES) == SD_OCR_HC_RES) {
                        address_mode = SECT_MODE;
                        voltage_validation = SUCCESS;
                    }
                    /* CODE REVIEW 3: (same as above) Check is logically correct, but seems redundent.
                       Anything that fails the HC check, is assumed Low Capacity */
                    /* nirp_oct03: this can be just an "else". the LC macro is 0 anyway,
                       and anything not HC is LC by default */
                    else {
                        address_mode = BYTE_MODE;
                        voltage_validation = SUCCESS;
                    }
                }
            }

        }

        hal_delay_us(1000);

    }

    return voltage_validation;
}

static cyg_uint32 sd_get_rca(void)
{
    command_t cmd;
    cyg_uint32 card_state = 0;
    cyg_uint32 rca_request = 0;
    command_response_t response;

    /* Configure CMD3 for MMC card */
    /* 32bit card address is expected as Argument */
    mxcmci_cmd_config(&cmd, CMD3, NO_ARG, READ, RESPONSE_48,
              DATA_PRESENT_NONE, ENABLE, ENABLE);

    /* Get relative address of the card     */

    if (host_send_cmd(&cmd) == FAIL) {
        rca_request = FAIL;
        diag_printf("Send CMD3 Failed.\n");
    } else {
        /* Read Command response */
        response.format = RESPONSE_48;
        host_read_response(&response);

        Card_rca = ((cyg_uint32) (response.cmd_rsp0 >> RCA_SHIFT));

        card_state = CURR_CARD_STATE(response.cmd_rsp0);

        if (card_state == IDENT) {
            rca_request = SUCCESS;
        } else {
            rca_request = FAIL;
            diag_printf("Get RCA Failed.\n");
        }
    }

    return rca_request;
}

static cyg_uint32 sd_get_bit_mode_support(void)
{
    command_t cmd;
    cyg_uint32 rd_data_buff[128];
    cyg_uint32 bit4_mode_support;
    command_response_t response;
    cyg_uint32 card_address = (Card_rca << RCA_SHIFT);

    /* Configure CMD55 for SD card */
    /* This command expects RCA as argument. */
    mxcmci_cmd_config(&cmd, CMD55, card_address, READ, RESPONSE_48,
              DATA_PRESENT_NONE, ENABLE, ENABLE);

    /* Issue CMD55 to SD Memory card */
    if (host_send_cmd(&cmd) == FAIL) {
        bit4_mode_support = 0;
    } else {
        /* Read Command response */
        response.format = RESPONSE_48;
        host_read_response(&response);

        /* Afetr giving ACMD Command, the R1 response should have
         * STATUS_APP_CMD set
         */
        if (response.cmd_rsp0 & SD_R1_STATUS_APP_CMD_MSK) {

            /* Configure ACMD51 for SD card */
            /* This command expects No argument. */

            mxcmci_cmd_config(&cmd, ACMD51, NO_ARG, READ,
                      RESPONSE_48, DATA_PRESENT, ENABLE,
                      ENABLE);

            /* Issue ACMD51 to SD Memory card */
            if (host_send_cmd(&cmd) == FAIL) {
                bit4_mode_support = 0;
            } else {
                /* Read Response from e-SDHC buffer */
                host_data_read(rd_data_buff, 512);

                /* Check for bus width supported */
                bit4_mode_support = (rd_data_buff[SD_BUS_WIDTH_OFFSET] & BIT_MODE_4_SUPPORT);

                if (bit4_mode_support) {
                    bit4_mode_support = BIT_4_MODE;
                }

            }
        }
    }

    return bit4_mode_support;
}

static cyg_uint32 sd_set_bus_width(cyg_uint32 bus_width)
{
    command_t cmd;
    cyg_uint32 set_bus_width_status = 0;
    command_response_t response;
    cyg_uint32 card_address = (Card_rca << RCA_SHIFT);

    if ((bus_width == FOUR) || (bus_width == ONE)) {
        /* Configure CMD55 for SD card */
        /* This command expects RCA as argument. */

        mxcmci_cmd_config(&cmd, CMD55, card_address, READ, RESPONSE_48,
                  DATA_PRESENT_NONE, ENABLE, ENABLE);

        /* Issue CMD55 to SD Memory card */
        if (host_send_cmd(&cmd) == FAIL) {
            set_bus_width_status = FAIL;
        } else {
            /* Read Command response */
            response.format = RESPONSE_48;
            host_read_response(&response);

            /* Afetr giving ACMD Command, the R1 response should have
             * STATUS_APP_CMD set
             */
            if (response.cmd_rsp0 & SD_R1_STATUS_APP_CMD_MSK) {
                bus_width = (bus_width >> ONE);

                /* Configure ACMD6 for SD card */
                mxcmci_cmd_config(&cmd, ACMD6, bus_width, READ,
                          RESPONSE_48,
                          DATA_PRESENT_NONE, ENABLE,
                          ENABLE);

                /* Issue ACMD6 to SD Memory card */
                if (host_send_cmd(&cmd) == FAIL) {
                    set_bus_width_status = FAIL;
                } else {
                    set_bus_width_status = SUCCESS;
                }
            }
        }
    }

    return set_bus_width_status;
}

/*==========================================================================
FUNCTION: cyg_uint32 sd_set_boot_partition (void)
DESCRIPTION:
  sd_set_boot_partition() will set set boot partition for Partition1

RETURN VALUE:
   SUCCESS / FAILURE

PRE-CONDITIONS:
   None

POST-CONDITIONS:
   None

Detailed Description:

==============================================================================*/

cyg_uint32 esd_set_boot_partition(cyg_uint32 *src_ptr, cyg_uint32 length)
{
    command_t cmd;
    cyg_uint32 set_partition_status = FAIL;
    command_response_t response;
    cyg_uint8 response_data[512];
    cyg_uint32 *response_pointer = (cyg_uint32 *) response_data;
    cyg_uint32 card_address = (Card_rca << RCA_SHIFT);
    cyg_uint32 card_state;

   /* Send CMD43 to select partition PARTITION1 active */
    mxcmci_cmd_config(&cmd, CMD43,
                              0x1<<24,
                              READ,
                              RESPONSE_48,
                              DATA_PRESENT_NONE,
                              ENABLE,
                              ENABLE);

    if(host_send_cmd(&cmd) == FAIL) {
        //diag_printf("%s: Send CMD43 Failed.\n", __FUNCTION__);
        return 1;
    }

#ifdef MX51_ERDOS
    set_partition_status = mmc_data_write (src_ptr, length, (cyg_uint64)0);
#else
    set_partition_status = mmc_data_write (src_ptr, length, 0);
#endif /* MX51_ERDOS */
    if(set_partition_status) {
        return 1; /* failed */
    }

    return 0;
}

static cyg_uint32 sd_set_high_speed_mode(void)
{
    command_t cmd;
    cyg_uint32 status = FAIL;
    command_response_t response;

    /* Configure CMD6 for SD card */
    mxcmci_cmd_config(&cmd, CMD6, 0xfffff1, READ, RESPONSE_48,
              DATA_PRESENT_NONE, ENABLE, ENABLE);

    /* Issue CMD6 to SD Memory card */
    if (host_send_cmd(&cmd) == FAIL) {
        status = FAIL;
        diag_printf("Send CMD6 Failed.\n");
        return FAIL;
    } else {
        hal_delay_us(1000);
        status = SUCCESS;

    }

    mxcmci_cmd_config(&cmd, CMD6, 0x80fffff1, READ, RESPONSE_48,
              DATA_PRESENT_NONE, ENABLE, ENABLE);

    /* Issue CMD6 to SD Memory card */
    if (host_send_cmd(&cmd) == FAIL) {
        status = FAIL;
        diag_printf("Send CMD6 Failed.\n");
    } else {
        /* wait until in transfer mode */
        while (mxcmci_trans_status()) {
            hal_delay_us(5);
        }

        status = SUCCESS;
    }

    return status;
}

/* end of mxcmic_sd.c */
