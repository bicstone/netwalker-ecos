// ==========================================================================
//
//   card_mx32.c
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
// Author(s):    Ivan Xu <yu.xu@freescale.com>
// Contributors: Ivan Xu <yu.xu@freescale.com>
// Date:         2008-06-13 Initial version
// Purpose:
// Description:
//     Support SD/MMC cards based on SDHC controller.
//     only base functionality is implemented: Card init, read, write and erase.
//     Write protection are not supported so far.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/io/card_mx32.h>
#include <cyg/hal/hal_io.h>
#include <redboot.h>
#include <stdlib.h>

//#define diag_printf1    diag_printf
#define diag_printf1(fmt,args...)

volatile psdhc_t pSDHC;
card_mode_t Card_Mode;
cyg_uint32 HighCapacityCard = 0;
cyg_uint32 card_address;
card_type  Card_type;            /* Card Type*/
CARD_ID card_id;
CARD_SPECIFIC_DATA csd;          /* Global variable for Card Specific Data */
cyg_uint32 CCC = 0;              /* Card Command Class */

static struct csd_v1_0 g_csd_val;

static void configure_cmd (command_t *cmd,cyg_uint32 index, cyg_uint32 argument,
                                cyg_uint32 transfer,cyg_uint32 response_format, cyg_uint32 data_enable,
                                cyg_uint32 bus_width )
{
	/* Configure Command index */
	cmd->index = index;

	/* Configure Command argument */
	cmd->arg = argument;
	/* workaround for CMD0, send 80 clock cycles before CMD0 */
	if (cmd->index == 0)
	{
		cmd->data_control = (((transfer) << SDHC_CMD_WRITE_READ_SHIFT) |
			((response_format) << SDHC_CMD_FROMAT_OF_RESP_SHIFT) |
			((data_enable) << SDHC_CMD_DATA_ENABLE_SHIFT) |
			((bus_width) << SDHC_CMD_BUS_WIDTH_SHIFT)) |
			(0x1 << SDHC_CMD_INIT_SHIFT );
	} else {
		cmd->data_control = (((transfer) << SDHC_CMD_WRITE_READ_SHIFT) |
			((response_format) << SDHC_CMD_FROMAT_OF_RESP_SHIFT) |
			((data_enable) << SDHC_CMD_DATA_ENABLE_SHIFT) |
			((bus_width) << SDHC_CMD_BUS_WIDTH_SHIFT));
	}
}

static void stop_clk(void)
{
	/* Stop the clock */
//	pSDHC->sdhc_clk = SDHC_CLK_STOP;

	/* Wait till the clock has stopped */
//	while((pSDHC->sdhc_status & SDHC_STATUS_CARD_BUS_CLK_RUN_MSK));
	return;
}

static void start_clk(void)
{
	/* Start the clock */
	pSDHC->sdhc_clk = SDHC_CLK_START;

	/* Wait till the clock has started */
	while(!(pSDHC->sdhc_status & SDHC_STATUS_CARD_BUS_CLK_RUN_MSK));
	return;
}

static void configure_clk(frequency_mode_t mode)
{
	if(mode == iden_mode)
	{
		/* Below 400 kHz  */
		pSDHC->sdhc_clk_rate = 0x207;
	}
	else if(mode == trans_mode)
	{
		/* Below 20 MHz  */
		pSDHC->sdhc_clk_rate = 0x3;
	}

    diag_printf1("pSDHC->sdhc_clk_rate=0x%x\n", pSDHC->sdhc_clk_rate);
}

static void read_response(cyg_uint32 response_mode, response_t*response)
{
	cyg_uint32 resp1=0;
	cyg_uint32 resp2=0;
	cyg_uint32 resp3=0;
	cyg_uint32 count;

	if(response_mode != 0)
	{
		if((response_mode == RESPONSE_48_CRC) || (response_mode == RESPONSE_48_WITHOUT_CRC))
		{
			resp1 = readl(0x50004000 + 0x34) & 0xffff;
			resp2 = readl(0x50004000 + 0x34) & 0xffff;
			resp3 = readl(0x50004000 + 0x34) & 0xffff;

			response->rsp0 = (resp1 << 24) | (resp2 << 8) | (resp3 >> 8);
		}
		else if(response_mode == RESPONSE_136)
		{
			resp1 = pSDHC->sdhc_res_fifo & 0xffff;
			resp2 = pSDHC->sdhc_res_fifo & 0xffff;
			response->rsp3 = (resp1 << 16) | resp2;
			resp1 = pSDHC->sdhc_res_fifo & 0xffff;
			resp2 = pSDHC->sdhc_res_fifo & 0xffff;
			response->rsp2 = (resp1 << 16) | resp2;

			resp1 = pSDHC->sdhc_res_fifo & 0xffff;
			resp2 = pSDHC->sdhc_res_fifo & 0xffff;
			response->rsp1 = (resp1 << 16) | resp2;

			resp1 = pSDHC->sdhc_res_fifo & 0xffff;
			resp2= pSDHC->sdhc_res_fifo & 0xffff;
			response->rsp0 = (resp1 << 16) | resp2;
		}

		/* Stop the clock */
		stop_clk();
		/* Clear w1c bits from STATUS register */
		pSDHC->sdhc_status |= SDHC_STATUS_CLEAR;
	}

	//return status;
}

static cyg_uint32 check_response(void)
{
	cyg_uint32 status = PASS;

	if((pSDHC->sdhc_status & SDHC_STATUS_END_CMD_RESP_MSK) &&
	   !(pSDHC->sdhc_status & SDHC_STATUS_TIME_OUT_RESP_MSK) &&
	   !(pSDHC->sdhc_status & SDHC_STATUS_RESP_CRC_ERR_MSK))
	{
		status = PASS;
	}
	else
	{
		status = FAIL;
		diag_printf("response status:  %x Fail!\n", pSDHC->sdhc_status);
	}
	return status;
}

static cyg_uint32 send_cmd_and_wait_resp(command_t *cmd)
{
	/* Clear Interrupt status Register  and enable int*/
	pSDHC->sdhc_status = 0xFFFFFFFF;
	pSDHC->sdhc_int_cntr = SDHC_INT;

	/* Write command index */
	pSDHC->sdhc_cmd = cmd->index;

	/* Write command arg */
	pSDHC->sdhc_arg = cmd->arg;

	/* Write command data control */
	pSDHC->sdhc_dat_cont = cmd->data_control;

	/* Start clock */
	start_clk();

	/* Wait for the response of command end */
	while(!(pSDHC->sdhc_status & SDHC_STATUS_END_CMD_RESP_MSK) );

	/* Mask all interrupts */
	pSDHC->sdhc_int_cntr = 0;

	/* Check if an error occured */
	return check_response();
}

static cyg_uint32 card_get_csd (void)
{
	command_t cmd;
	response_t response;
	cyg_uint32 status = FAIL;
	//cyg_uint32 card_address = (Card_rca << RCA_SHIFT);

	/* Configure CMD9 for MMC/SD card */
	/* 16bit card address is expected as Argument */
	configure_cmd(&cmd,CMD9,card_address,READ,RESPONSE_136, DISABLE, ONE);

	/* Send Command CMD9 to Extrace CSD register contents  */
	if(send_cmd_and_wait_resp(&cmd) != FAIL)
	{
		/* Read Command response */
		read_response (RESPONSE_136, &response);
		/* Assign Response to CSD Strcuture */
		csd.csd0 = response.rsp0;
		csd.csd1 = response.rsp1;
		csd.csd2 = response.rsp2;
		csd.csd3 = response.rsp3;
		diag_printf1("CSD:%x:%x:%x:%x\n", csd.csd0, csd.csd1, csd.csd2, csd.csd3);
		status = SUCCESS;
        // save csd
        memcpy(&g_csd_val, &csd, sizeof(struct csd_v1_0));
        diag_printf1("g_csd_val.c_size_mult=0x%x\n", g_csd_val.c_size_mult);
        diag_printf1("g_csd_val addr=%p\n", &g_csd_val);
	}

	return status;
}

static cyg_uint32 csd_get_value(CARD_SPECIFIC_DATA * pcsd, cyg_uint32 start_bit, cyg_uint32 end_bit)
{
	cyg_uint32 value;
    if (start_bit == 84) {
        value = g_csd_val.ccc;
    } else if (start_bit == 62) {
        value = (g_csd_val.c_size_up << 2) | g_csd_val.c_size_lo;
    } else if (start_bit == 47) {
        value = g_csd_val.c_size_mult;
    } else if (start_bit == 80) {
        value = g_csd_val.read_bl_len;
    } else if (start_bit == 48) {
        struct csd_v2_0 *ptr = (struct csd_v2_0 *) &g_csd_val;
        value = (ptr->c_size_up << 16) | ptr->c_size_lo;
    } else {
        diag_printf1("start_bit=%d is not supported\n", start_bit);
        while (1) {}
    }
    diag_printf1("start_bit=%d, end_bit=%d, value=0x%x\n", start_bit, end_bit, value);
    return value;
}

static cyg_uint32 mmc_init(void)
{
	cyg_uint32 status = FAIL;
	command_t cmd;
	response_t resp;

	cyg_uint32 card_status = 0;


	card_address = 0x1<<16;
	/* get cid of MMC */
	/* Configure CMD2 for card */
	configure_cmd(&cmd,CMD2,NO_ARG,READ,RESPONSE_136,DISABLE,ONE);

	/* Send CMD2 to card to determine CID contents */
	if(send_cmd_and_wait_resp(&cmd) == FAIL)
	{
		status = FAIL;
		return status;
	}
	else
	{
		 /* Read Command response  */
		 read_response(RESPONSE_136, &resp);
		 /* Assign CID values to mmc_cid structures */
		card_id.cid0 = resp.rsp0;
		card_id.cid1 = resp.rsp1;
		card_id.cid2 = resp.rsp2;
		card_id.cid3 = resp.rsp3;

		//status = PASS;
	}

	/* get rca of MMC */
	/* Configure CMD3 for MMC card */
	configure_cmd(&cmd,CMD3,card_address,READ,RESPONSE_48_CRC, DISABLE, ONE);

	/* Assigns relative address to the card */
	if(send_cmd_and_wait_resp(&cmd) == FAIL)
	{
		status = FAIL;
		return status;
	}
	else
	{
		 /* Read Command response */
		read_response(RESPONSE_48_CRC, &resp);
		card_status = resp.rsp0;
		card_status = (((cyg_uint32) (card_status & CARD_STATE)) >> CARD_STATE_SHIFT);
		if(card_status == IDENT)
		{
			status = PASS;
		}
		else
		{
			status = FAIL;
			return status;
		}
	}

	card_get_csd();

	configure_clk(trans_mode);

	/*Send MMC to Transfer State */
	 /* Configure CMD7 for MMC card */
	configure_cmd(&cmd,CMD7,card_address,READ,RESPONSE_48_CRC, DISABLE,ONE);

	if(send_cmd_and_wait_resp(&cmd) == FAIL)
	{
		status = FAIL;
		return status;
	}
	else
	{
		/* Configure CMD13 to read status of the card becuase CMD7 has R1b response */
		configure_cmd(&cmd,CMD13,card_address,READ,RESPONSE_48_CRC,DISABLE,ONE);
		if(send_cmd_and_wait_resp(&cmd) == FAIL)
		{
			status = FAIL;
			return status;
		}
		else
		{
			/* Read Command response */
			read_response (RESPONSE_48_CRC, &resp);
			card_status = resp.rsp0;
			card_status = (((cyg_uint32) (card_status & CARD_STATE)) >> CARD_STATE_SHIFT);

			if(card_status == TRAN)
			{
				status = PASS;
			}
			else
			{
				status= FAIL;
			}
		}
	}

	return status;
}

static cyg_uint32 check_sd(void)
{
	command_t cmd;
	//command_response_t response;
	cyg_uint32 count =0;
	cyg_uint32 default_rca = 0;
	cyg_uint32 ocr_value=0;
	cyg_uint32 status = FAIL;
	response_t resp;

	configure_cmd(&cmd,CMD8,0x1AA,READ,RESPONSE_48_CRC, DISABLE, ONE);
	send_cmd_and_wait_resp(&cmd);

	while((count < 3000) && (status != PASS))
	{
		/* Configure CMD55 for SD card */
		configure_cmd(&cmd,CMD55,default_rca,READ,RESPONSE_48_CRC, DISABLE, ONE);

		/* Send CMD55 to SD Memory card*/
		if(send_cmd_and_wait_resp(&cmd) == FAIL)
		{
			status = FAIL;
			//count++;
			diag_printf1("CMD55 FAIL!\n");
			break;
			//continue;
		}
		else
		{
			ocr_value = ((cyg_uint32)(OCR_VALUE) & 0xFFFFFFFF);
			/* Configure ACMD41 for SD card */
			configure_cmd(&cmd,ACMD41,ocr_value,READ,RESPONSE_48_WITHOUT_CRC,DISABLE, ONE);
			/* SEND ACMD41 to SD Memory card to determine OCR value */
			if(send_cmd_and_wait_resp(&cmd) == FAIL)
			{
				status = FAIL;
				diag_printf1("ACMD41 FAIL!\n");
				break;
			}
			else
			{
				/* Read Response from CMDRSP0 Register */
				read_response(RESPONSE_48_WITHOUT_CRC, &resp);
				ocr_value = resp.rsp0;
				 diag_printf1("SD: response ocr value: 0x%x\n", ocr_value);
				/* Check if volatge lies in range or not*/
				if((ocr_value & OCR_VALUE_MASK) == OCR_VALUE_MASK)
				{
					diag_printf1("response.cmd_rsp0: 0x%x\n", ocr_value);
					/* Check if card busy bit is cleared or not */
					if(ocr_value & CARD_BUSY)
					{
						status = PASS;
					}
					else
					{
						count++;
						diag_printf1("SD: Busy! \n");
					}
				}
				else
				{
					count++;
					diag_printf("SD: response ocr value: 0x%x  FAIL!\n", ocr_value);
				}
			}
		}
	}
	return status;
}


static cyg_uint32 sd_init(cyg_uint32 bus_width)
{
	cyg_uint32 status = FAIL;
	command_t cmd;
	response_t resp;
	cyg_uint32 card_status = 0;
	cyg_uint32 read_resp = 0;

	card_address = 0;

	/* get cid of MMC */
	/* Configure CMD2 for card */
	configure_cmd(&cmd,CMD2,NO_ARG,READ,RESPONSE_136,DISABLE,ONE);

	/* Send CMD2 to card to determine CID contents */
	if(send_cmd_and_wait_resp(&cmd) == FAIL)
	{
		status = FAIL;
		return status;
	}
	else
	{
		 /* Read Command response  */
		 read_response(RESPONSE_136, &resp);
		 /* Assign CID values to mmc_cid structures */
		card_id.cid0 = resp.rsp0;
		card_id.cid1 = resp.rsp1;
		card_id.cid2 = resp.rsp2;
		card_id.cid3 = resp.rsp3;

		//status = PASS;
	}

	/* get rca of card */
	/* Configure CMD3 for card */
	configure_cmd(&cmd,CMD3,NO_ARG,READ,RESPONSE_48_CRC, DISABLE, ONE);

	/* Assigns relative address to the card */
	if(send_cmd_and_wait_resp(&cmd) == FAIL)
	{
		status = FAIL;
		return status;
	}
	else
	{
		 /* Read Command response */
		read_response(RESPONSE_48_CRC, &resp);
		 card_status = resp.rsp0;
		 card_address = ((cyg_uint32) (card_status & (0xffffff00)));
		card_status = (((cyg_uint32) (card_status & CARD_STATE)) >> CARD_STATE_SHIFT);
		if(card_status == IDENT)
		{
			status = PASS;
		}
		else
		{
			status = FAIL;
			return status;
		}
	}

	card_get_csd();
	configure_clk(trans_mode);

	/*Send card to Transfer State */
	 /* Configure CMD7 for card */
	configure_cmd(&cmd,CMD7,card_address,READ,RESPONSE_48_CRC, DISABLE,ONE);
	if(send_cmd_and_wait_resp(&cmd) == FAIL)
	{
		status = FAIL;
		return status;
	}
	else
	{
		/* Configure CMD13 to read status of the card becuase CMD7 has R1b response */
		configure_cmd(&cmd,CMD13,card_address,READ,RESPONSE_48_CRC,
			            DISABLE,ONE);

		if(send_cmd_and_wait_resp(&cmd) == FAIL)
		{
			status = FAIL;
			return status;
		}
		else
		{
			/* Read Command response */
			read_response (RESPONSE_48_CRC, &resp);
			card_status = resp.rsp0;
			card_status = (((cyg_uint32) (card_status & CARD_STATE)) >> CARD_STATE_SHIFT);
			if(card_status == TRAN)
			{
				status = PASS;
			}
			else
			{
				status = FAIL;
			}
		}
	}


	/* set bus width */
	if ((bus_width == FOUR ) || (bus_width == ONE))
	{
		/* Configure CMD55 for SD card */
		configure_cmd(&cmd,CMD55,card_address,READ,RESPONSE_48_CRC, DISABLE, ONE);

		/* Issue CMD55 to SD Memory card*/
		if(send_cmd_and_wait_resp(&cmd) == FAIL)
		{
			status = FAIL;
			return status;
		}
		else
		{
			/* Read Command response */
			read_response(RESPONSE_48_CRC, &resp);
			read_resp = resp.rsp0;
			if(read_resp & SD_R1_APP_CMD_MSK)
			{
				bus_width = (bus_width>>ONE);

				/* Configure ACMD6 for SD card */
				configure_cmd(&cmd,ACMD6,bus_width,READ,RESPONSE_48_CRC, DISABLE, ONE);
				/* Send ACMD6 to SD Memory card*/
				if(send_cmd_and_wait_resp(&cmd) == FAIL)
				{
					status = FAIL;
					return status;
				}
				else
				{
					status = PASS;
				}
			}
		}
	}

	return status;
}

static cyg_uint32 check_mmc(void)
{
	 command_t cmd;
	 response_t resp;
	//cyg_uint32 response;
	cyg_uint32 count=0;
	cyg_uint32 ocr_value=0;
	cyg_uint32 status = FAIL;


	while((count < 10) && (status != PASS))
	{
	/* Configure CMD1 for MMC card */
		configure_cmd(&cmd, CMD1, OCR_VALUE, READ, RESPONSE_48_WITHOUT_CRC,DISABLE, ONE);

		/* Issue CMD1 to MMC card to determine OCR value */
		if(send_cmd_and_wait_resp(&cmd) == FAIL)
		{
			status = FAIL;
			count++;
			diag_printf1("CMD1 FAIL!\n");
			break;
			//continue;
		}
		else
		{
			read_response(RESPONSE_48_WITHOUT_CRC, &resp);
			ocr_value = resp.rsp0;

			/* Mask OCR value against 0x00FF8000 and compare with response*/
			if ((ocr_value & OCR_VALUE_MASK) == OCR_VALUE_MASK)
			{
				/* Check if card busy bit is cleared or not */
				if(ocr_value & CARD_BUSY)
				{
					status = PASS;
				}
				else
				{
					count++;
				}
			}
			else
			{
				count++;
			}
		}
	}

	return status;
}

static cyg_uint32 check_card(cyg_uint32 bus_width)
{

	cyg_uint32 status = FAIL;
	Card_Mode = NONE;

	//wait
    hal_delay_us(2000);
	diag_printf1("check SD\n");
	if(check_sd() == PASS){
		Card_Mode = SD;
		diag_printf1("SD init\n");
		status = sd_init(bus_width);
		Card_type  = ((csd.csd3 & CSD_STRUCT_MSK)? SD_CSD_2_0: SD_CSD_1_0);

		/* Card Command Class */
		CCC  = csd_get_value(&csd, 84, 95);
	}
	else{
		//wait
        hal_delay_us(2000);
		diag_printf1("check MMC\n");
		if(check_mmc() == PASS){
			Card_Mode = MMC;

			status = mmc_init();
			Card_type = ((csd.csd3 & CSD_STRUCT_MSK) >> CSD_STRUCT_SHIFT) + SD_CSD_2_0;
		/* Card Command Class */
			CCC  = csd_get_value(&csd, 84, 95);
		}
	}
	return status;
}

static void sdhc_init(cyg_uint32 base_address)
{
	cyg_uint32 iomux_base  = 0x43FAC000;
	cyg_uint32 gpio_base  = 0x53FA4000;
	cyg_uint32 iomux_sw_mux_ctl1 = readl(iomux_base + 0x18);
	cyg_uint32 iomux_sw_mux_ctl2 = readl(iomux_base + 0x1C);
	unsigned long reg;

	iomux_sw_mux_ctl1 &= 0x000000FF;
	iomux_sw_mux_ctl1 |=  0x12121200;
	writel(iomux_sw_mux_ctl1, iomux_base + 0x18);

	iomux_sw_mux_ctl2 &= 0xFF000000;
	iomux_sw_mux_ctl2 |=  0x00121012;
	writel(iomux_sw_mux_ctl2, iomux_base + 0x1C);

	writel(0x0A529485, iomux_base + 0x168);
	writel(0x0A5294A5, iomux_base + 0x16c);

	/* Initialize base address */
	pSDHC = (psdhc_t)base_address;
}

static void sdhc_reset(void)
{
	pSDHC->sdhc_clk = SDHC_CLK_RESET;
	pSDHC->sdhc_clk = SDHC_CLK_RESET | SDHC_CLK_STOP;
	pSDHC->sdhc_clk = SDHC_CLK_STOP;
	pSDHC->sdhc_clk = SDHC_CLK_STOP;
	pSDHC->sdhc_clk = SDHC_CLK_STOP;
	pSDHC->sdhc_clk = SDHC_CLK_STOP;
	pSDHC->sdhc_clk = SDHC_CLK_STOP;
	pSDHC->sdhc_clk = SDHC_CLK_STOP;
	pSDHC->sdhc_clk = SDHC_CLK_STOP;
	pSDHC->sdhc_clk = SDHC_CLK_STOP;
}

static cyg_uint32 card_reset(void)
{
	command_t  cmd;

	configure_clk(iden_mode);

	/*set size of read and response fifo */
	//pSDHC->sdhc_read_to = 0xffff;
	pSDHC->sdhc_read_to = 0x2DB4;
	pSDHC->sdhc_response_to = 0xff;
	hal_delay_us(20000);

	/* CMD0 to reset SD/MMC cards */
	configure_cmd(&cmd,CMD0,NO_ARG,READ, RESPONSE_NO, DISABLE, ONE);

	return send_cmd_and_wait_resp(&cmd);
}

static void wait_transfer_done(cyg_uint32 mask)
{
	/* Wait interrupt (WRITE_OP_DONE/READ_OP_DONE) */
	while(!(pSDHC->sdhc_status & mask));
}

static cyg_uint32 check_data(cyg_uint32 done_mask, cyg_uint32 crc_err_code_mask, cyg_uint32 crc_err_mask)
{
	cyg_uint32 status = FAIL;
	/* Check whether the interrupt is an OP_DONE or a data time out or a CRC error  */
	if((pSDHC->sdhc_status & done_mask) &&
	    !(pSDHC->sdhc_status & crc_err_code_mask) &&
	    !(pSDHC->sdhc_status & crc_err_mask))
	{
		status = PASS;
	}
	else
	{
		status = FAIL;
	}
	return status;
}

static cyg_uint32 check_card_status(void)
{
	command_t cmd;
	cyg_uint32 status = PASS;
	cyg_uint32 card_state;
	cyg_uint32 read_resp;
	response_t resp;
	//cyg_uint32 card_address = (Card_rca << RCA_SHIFT);

	configure_cmd(&cmd,CMD13,card_address,READ,RESPONSE_48_CRC, DISABLE, ONE);

	if(send_cmd_and_wait_resp(&cmd) == FAIL)
	{
		status = FAIL;
	}
	else
	{
		/* Read Command response */
		read_response (RESPONSE_48_CRC, &resp);
		read_resp = resp.rsp0;
		card_state = ((cyg_uint32) (read_resp & CARD_STATE) >> CARD_STATE_SHIFT);

		if(card_state == TRAN)
		{
			status = PASS;
		}
		else
		{
			status = FAIL;
		}
	}
	return status;
}



/*==========================================================================
FUNCTION: static cyg_uint32 card_get_capacity_size(void)
DESCRIPTION:
   this function will analize MMC/SD CSD register and return the capacity size (in unit of KB)

ARGUMENTS PASSED:
   None

RETURN VALUE:
   cyg_uint32

PRE-CONDITIONS:
   None

POST-CONDITIONS:
   None

Detailed Description:
==============================================================================*/
cyg_uint32 card_get_capacity_size (void)
{
	cyg_uint32 capacity = 0;
	cyg_uint32 c_size, c_size_mult, blk_len;

	if(!csd.csd0  && !csd.csd1  && !csd.csd2 && !csd.csd3)
		diag_printf("WARNINGS:card_init should be done first!\n");

	switch(Card_type)
	{
		case SD_CSD_1_0:
		case MMC_CSD_1_0:
		case MMC_CSD_1_1:
		case MMC_CSD_1_2:
		case SD_CSD_2_0:
			c_size = csd_get_value(&csd, 62, 73);
			c_size_mult = csd_get_value(&csd, 47, 49);
			blk_len = csd_get_value(&csd, 80, 83);
			capacity = (((c_size+1) << (c_size_mult +2)) << blk_len) / 1024;
			diag_printf1("c_size=0x%x, c_size_mult=0x%x, blk_len=0x%x, capacity(KB)=0x%x\n",
						  c_size, c_size_mult, blk_len, capacity);
			break;
#if 0 // todo
		case SD_CSD_2_0:
			//blk_len = csd_get_value(&csd, 80, 83);
			c_size = csd_get_value(&csd, 48, 69);
			capacity = (c_size + 1) * 512; // block length is fixed to 512B
            diag_printf1("card capacity2=0x%x\n", capacity);
			break;
#endif
		default:
			break;
	}
	if (capacity > (0x80000000 / 1024))
		HighCapacityCard = 1;
	else
		HighCapacityCard = 0;

	return capacity;
}

cyg_uint32 mxcmci_init (cyg_uint32 bus_width, cyg_uint32 base_address)
{
	sdhc_init(base_address);

	/* Software Reset to SDHC */
	sdhc_reset();

	/* Software Reset to card */
	card_reset();

	return check_card(bus_width);
}

cyg_uint32 mmc_data_read (cyg_uint32 *ram_ptr, cyg_uint32 length, cyg_uint32 offset)
{
	command_t cmd;
	cyg_uint32 len, retry = 15;
	cyg_uint32 status = PASS;
	cyg_uint32 i, j, k = 0;

	diag_printf1("\ncard_data_read !-- offset: %x, length: %x \n", offset, length);

	len = (length + BLOCK_LEN - 1) & (~(BLOCK_LEN - 1));

	if (HighCapacityCard)
		offset = offset / 512;

	/* Configure SDHC block and number of blocks */
	pSDHC->sdhc_blk_len = BLOCK_LEN;
	pSDHC->sdhc_nob = 0x1;

	/* Configure CMD16 to set block length as 512 bytes.*/
	configure_cmd(&cmd,CMD16,BLOCK_LEN,READ,RESPONSE_48_CRC, DISABLE, ONE);
	if(send_cmd_and_wait_resp(&cmd) == FAIL)
	{
		status = FAIL;
		diag_printf1("CMD16 Fail!\n");
	}
	else
	{
		while(len != 0 && !status)
		{
			//check card status whether it is in transfer mode, so as to start next transfer
			while((status = check_card_status())!=PASS);

			diag_printf1("length left: %x \n", len);

			/* Send CMD17 for single block read */
			configure_cmd(&cmd,CMD17,offset,READ,RESPONSE_48_CRC, ENABLE, ONE);
			if(send_cmd_and_wait_resp(&cmd) == FAIL)
			{
				status= FAIL;
				diag_printf1("CMD17 Fail!\n");
			}
			else
			{
				 /* Enable int */
				pSDHC->sdhc_int_cntr = SDHC_INT;
				for(i = 0; i < BLOCK_LEN/16; i++)
				{
					/* Wait for BRR bit to be set */
					while(!(pSDHC->sdhc_status & SDHC_STATUS_BUF_READ_RDY_MSK)) {
						hal_delay_us(10);
					}
					for(j=0;j<4;j++)
					{
						/* Read 32 bit data from buffer access fifo */
						*ram_ptr = pSDHC->sdhc_buffer_access;
						ram_ptr++;
					}
				}
				/* Wait for transfer complete  */
				wait_transfer_done(SDHC_STATUS_READ_OP_DONE_MSK);

				/* Check for status errors (crc or timeout)*/
				status = check_data(SDHC_STATUS_READ_OP_DONE_MSK, SDHC_STATUS_TIME_OUT_READ, SDHC_STATUS_READ_CRC_ERR_MSK);

				offset = offset + BLOCK_LEN;
				len = len - BLOCK_LEN;
				//ram_ptr= ram_ptr + (BLOCK_LEN/4);
				diag_printf1("length left3: %x \n", len);
			}
		}
	}
	diag_printf1("End of card data read!\n");
	return status;
}

cyg_uint32 mmc_data_write (cyg_uint32 *ram_ptr, cyg_uint32 length, cyg_uint32 offset)
{
	command_t cmd;
	cyg_uint32 len;
	cyg_uint32 status = PASS;
	cyg_uint32 i, j = 0;

	len = (length + BLOCK_LEN - 1) & (~(BLOCK_LEN - 1));

	/* Configure SDHC block and number of blocks */
	pSDHC->sdhc_blk_len = BLOCK_LEN;
	pSDHC->sdhc_nob = 0x1;

    /* high capacity card uses sector mode */
	if (HighCapacityCard)
		offset = offset / 512;

	/* Send CMD16 to set block length as 512 bytes.*/
	configure_cmd(&cmd,CMD16,BLOCK_LEN,READ,RESPONSE_48_CRC, DISABLE, ONE);
	if(send_cmd_and_wait_resp(&cmd) == FAIL)
	{
		status = FAIL;
	}
	else
	{
		while(len != 0 && !status)
		{
			//check card status whether it is in transfer mode, so as to start next transfer
			while((status = check_card_status())!=PASS);
			/* Comfigure command CMD24 for block write--write address */
			configure_cmd(&cmd,CMD24,offset,WRITE,RESPONSE_48_CRC, ENABLE, ONE);
			if(send_cmd_and_wait_resp(&cmd) == FAIL)
			{
				status = FAIL;
			}
			else
			{
				 /* Enable int */
				pSDHC->sdhc_int_cntr = SDHC_INT;

				for(i = 0; i < (BLOCK_LEN)/4; i++)
				{
					/* Wait for BWR bit to be set */
					while(!(pSDHC->sdhc_status & SDHC_STATUS_BUF_WRITE_RDY_MSK));
					//copy data from ram to sdhc buffer access fifo
					pSDHC->sdhc_buffer_access = *ram_ptr;
					ram_ptr++;
				}

				/* Wait for transfer done */
				wait_transfer_done(SDHC_STATUS_WRITE_OP_DONE_MSK);

				/* Check for status errors (crc or timeout)*/
				status = check_data(SDHC_STATUS_WRITE_OP_DONE_MSK, 0, SDHC_STATUS_WRITE_CRC_ERR_MSK);

				len = len - BLOCK_LEN;
				offset +=  BLOCK_LEN;
				//ram_ptr = ram_ptr + (BLOCK_LEN/4);
			}
		}
	}
	return status;
}

cyg_uint32 mmc_data_erase (cyg_uint32 offset, cyg_uint32 size)
{
	command_t cmd;
	cyg_uint32 startEraseBlockCmd;
	cyg_uint32 endEraseBlockCmd;
	cyg_uint32 startBlock = offset/BLOCK_LEN;
	cyg_uint32 endBlock = (offset+size)/BLOCK_LEN;
	cyg_uint32 status = FAIL;

	/* Fix erase operation on MX31/32 */
	return 0;
	if(Card_Mode == MMC) {
		startBlock *=BLOCK_LEN;
		endBlock *= BLOCK_LEN;
		startEraseBlockCmd = CMD35;
		endEraseBlockCmd   = CMD36;
	}
	else if(Card_Mode == SD) {
		startBlock *=BLOCK_LEN;
		endBlock *= BLOCK_LEN;
		startEraseBlockCmd = CMD32;
		endEraseBlockCmd   = CMD33;
	}
	if (HighCapacityCard) {
		startBlock /= BLOCK_LEN;
		endBlock /= BLOCK_LEN;
	}

	/* Configure start erase command to set first block*/
	configure_cmd(&cmd,startEraseBlockCmd,startBlock,READ,RESPONSE_48_CRC, DISABLE, ONE);
	if((status = send_cmd_and_wait_resp(&cmd)) == PASS){

		/* Configure end erase command to set end block*/
		configure_cmd(&cmd,endEraseBlockCmd,endBlock,READ,RESPONSE_48_CRC, DISABLE, ONE);
		if((status = send_cmd_and_wait_resp(&cmd)) == PASS){
			/* Comfigure command to start erase*/
			configure_cmd(&cmd,CMD38,0,READ,RESPONSE_48_CRC, DISABLE, ONE);
			if((status = send_cmd_and_wait_resp(&cmd)) == PASS){
				//wait for completion
				return status;
			}
		}
	}

	return status;
}

cyg_uint32 card_flash_query(void* data)
{
	command_t cmd;
	cyg_uint32 status = PASS;
	response_t response;

	 // Configure CMD2 for card  No Argument is expected for CMD2
	configure_cmd(&cmd,CMD2,NO_ARG,READ,RESPONSE_136, DISABLE, ONE);

	// Send CMD2 to card to determine CID contents
	if(send_cmd_and_wait_resp(&cmd) == FAIL)
	{
	status = FAIL;
		diag_printf("%s: can't send query command\n", __FUNCTION__);
	}
	else
	{
		cyg_uint32* d = (cyg_uint32*)data;
		// Read Command response
		read_response (RESPONSE_136, &response);

		// Assign CID values to mmc_cid structures
		*d++ = response.rsp0;
		*d++ = response.rsp1;
		*d++= response.rsp2;
		*d= response.rsp3;

		// Assign cid_request as SUCCESS
		status = PASS;
	}
	diag_printf( "%s(PASS?=%d):(ID=0x%x: 0x%x, 0x%x, 0x%x)\n",
	__FUNCTION__, status,*(cyg_uint32*)(data), *(cyg_uint32*)((cyg_uint32)data+4),
	       *(cyg_uint8*)((cyg_uint32)data+8), *(cyg_uint8*)((cyg_uint32)data+12));
	return;
}


