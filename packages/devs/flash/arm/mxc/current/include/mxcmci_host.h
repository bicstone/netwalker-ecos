#ifndef _MXCMCI_HOST_H_
#define _MXCMCI_HOST_H_

// ==========================================================================
//
//    Module Name:  mxcmci_host.h
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
//
//####DESCRIPTIONEND####
//====================================================================================================

#include <cyg/infra/cyg_type.h>


#define ESDHC_SOFTWARE_RESET 	0x01000000  /* RSTA bit of ESDHC system control register*/
#define ESDHC_CMD_INHIBIT       0x00000003  /* Command inhibit bits*/
#define ESDHC_SOFTWARE_INIT     0x08000000  /* INITA bit of ESDHC system control register */
#define ESDHC_LITTLE_ENDIAN_MODE 0x00000020  /* Little Endian mode */
#define ESDHC_HW_BIG_ENDIAN_MODE 0x00000010  /* Half Word Big Endian mode */
#define ESDHC_BIG_ENDIAN_MODE    0x00000000  /* Big Endian mode */
#define ESDHC_ONE_BIT_SUPPORT    0x00000000  /* 1 Bit Mode support */
#define ESDHC_FOUR_BIT_SUPPORT   0x00000002  /* 4 Bit Mode support */
#define ESDHC_EIGHT_BIT_SUPPORT  0x00000004  /* 8 Bit Mode support */
#define ESDHC_CLOCK_ENABLE 		0x00000007    /* Clock Enable */
#define ESDHC_ENABLE                        0x00000008    /* Enable SD */

#define ESDHC_FREQ_MASK 0xffff0007
#define ESDHC_IDENT_FREQ 0x0000800e  /* SDCLKFS 0x08 ; DVS 0xe */
#define ESDHC_OPERT_FREQ 0x00000200  /* SDCLKFS 0x02 ; DVS 0x0 */
#define ESDHC_INTERRUPT_ENABLE 	0x007f0123	  /* Enable Interrupts */
#define ESDHC_CONFIG_BLOCK 0x00010200             /* 512 byte block size*/
#define ESDHC_CLEAR_INTERRUPT 0xffffffff

#define ESDHC_CONFIG_BLOCK_512 0x00000200             /* 512 byte block size*/
#define ESDHC_CONFIG_BLOCK_64  0x00000040             /* 64 byte block size*/
#define ESDHC_BLOCK_SHIFT		16

#define ESDHC_CLEAR_INTERRUPT 0xffffffff
#define ESDHC_OPER_TIMEOUT 96 /* 3 msec time out */
#define ESDHC_READ_TIMEOUT 3264 /* 102 msec read time out */
#define ESDHC_ACMD41_TIMEOUT 48000 /* 1.5 sec =1500 msec delay for ACMD41 cmd */

#define  ESDHCI_SPACE_AVAILABLE	0x00000400
#define  ESDHCI_DATA_AVAILABLE	0x00000800

/*==================================================================================================
                                            DEFINES
==================================================================================================*/
#define DATA_TRANSFER_SHIFT 4
#define RESPONSE_FORMAT_SHIFT 16
#define DATA_PRESENT_SHIFT 21
#define CRC_CHECK_SHIFT 19
#define CMD_INDEX_CHECK_SHIFT 20
#define CMD_INDEX_SHIFT 24
#define BLOCK_COUNT_ENABLE_SHIFT 1
#define MULTI_SINGLE_BLOCK_SELECT_SHIFT 5
#define BLK_LEN 512
#define SWITCH_BLK_LEN 64
#define FIFO_SIZE 128
#define WRITE_READ_WATER_MARK_LEVEL 0x00200020
#define ESDHC3 2
#define ESDHC2 1
#define ONE 1
#define ESDHC1 0
/*==================================================================================================
                                             ENUS
==================================================================================================*/
#define ESDHC_STATUS_END_CMD_RESP_MSK 0x1
#define ESDHC_STATUS_END_CMD_RESP_TIME_MSK 0x00010001
#define ESDHC_STATUS_TIME_OUT_RESP_MSK  0x10000
#define ESDHC_STATUS_RESP_CRC_ERR_MSK 0x20000
#define ESDHC_STATUS_RESP_INDEX_ERR_MSK 0x80000
#define ESDHC_STATUS_BUF_READ_RDY_MSK 0x20
#define ESDHC_STATUS_BUF_WRITE_RDY_MSK 0x10
#define ESDHC_STATUS_TRANSFER_COMPLETE_MSK 0x2
#define ESDHC_STATUS_TIME_OUT_READ 0x100000
#define ESDHC_STATUS_READ_CRC_ERR_MSK 0x200000

#define ESDHC_RESET_CMD_MSK 0x02000000
#define ESDHC_RESET_DAT_MSK 0x04000000
#define ESDHC_RESET_ALL_MSK 0x01000000

typedef enum
{
    WRITE = 0,
    READ = 1,
}xfer_type_t;

typedef enum
{
    RESPONSE_NONE,
    RESPONSE_136,
    RESPONSE_48,
    RESPONSE_48_CHECK_BUSY
}response_format_t;


typedef enum
{
    	DATA_PRESENT_NONE = 0,
    	DATA_PRESENT = 1
}data_present_select;

typedef enum
{
    	DISABLE = 0,
    	ENABLE = 1
}crc_check_enable,cmdindex_check_enable,block_count_enable;

typedef enum
{
	SINGLE = 0,
    	MULTIPLE = 1
}multi_single_block_select;

typedef struct
{
    cyg_uint32 command;
    cyg_uint32 arg;
    xfer_type_t data_transfer;
    response_format_t response_format;
    data_present_select data_present;
    crc_check_enable crc_check;
    cmdindex_check_enable cmdindex_check;
	block_count_enable block_count_enable_check;
	multi_single_block_select multi_single_block;
}command_t;

typedef struct
{
    response_format_t format;
    cyg_uint32 cmd_rsp0;
    cyg_uint32 cmd_rsp1;
    cyg_uint32 cmd_rsp2;
    cyg_uint32 cmd_rsp3;
}command_response_t;

typedef enum
{
	BIG_ENDIAN,
	HALF_WORD_BIG_ENDIAN,
	LITTLE_ENDIAN
}endian_mode_t;

typedef enum
{
    OPERATING_FREQ = 20000,   /* in kHz */
    IDENTIFICATION_FREQ = 400   /* in kHz */
}sdhc_freq_t;


enum esdhc_data_status
{
	ESDHC_DATA_ERR = 3,
	ESDHC_DATA_OK = 4
};

enum esdhc_int_cntr_val
{
	ESDHC_INT_CNTR_END_CD_RESP = 0x4,
	ESDHC_INT_CNTR_BUF_WR_RDY = 0x8
};

enum esdhc_reset_status
{
	ESDHC_WRONG_RESET = 0,
	ESDHC_CORRECT_RESET = 1
};

typedef struct
{
    volatile cyg_uint32 dma_system_address;
    volatile cyg_uint32 block_attributes;
    volatile cyg_uint32 command_argument;
    volatile cyg_uint32 command_transfer_type;
    volatile cyg_uint32 command_response0;
    volatile cyg_uint32 command_response1;
    volatile cyg_uint32 command_response2;
    volatile cyg_uint32 command_response3;
    volatile cyg_uint32 data_buffer_access;
    volatile cyg_uint32 present_state;
    volatile cyg_uint32 protocol_control;
    volatile cyg_uint32 system_control;
    volatile cyg_uint32 interrupt_status;
    volatile cyg_uint32 interrupt_status_enable;
    volatile cyg_uint32 interrupt_signal_enable;
    volatile cyg_uint32 autocmd12_status;
    volatile cyg_uint32 host_controller_capabilities;
    volatile cyg_uint32 watermark_level;
    cyg_uint32 reserved1[2];
    volatile cyg_uint32 force_event;
    volatile cyg_uint32 adma_error_status_register;
    volatile cyg_uint32 adma_system_address;
    cyg_uint32 reserved[40];
    volatile cyg_uint32 host_controller_version;
}host_register, *host_register_ptr;


extern host_register_ptr esdhc_base_pointer;
//extern cyg_uint32 available_mask;

extern void host_reset(cyg_uint32 data_transfer_width, cyg_uint32 endian_mode);
extern void host_cfg_clock(sdhc_freq_t);
extern void host_read_response(command_response_t *);
extern cyg_uint32 host_send_cmd(command_t *cmd);
extern cyg_uint32 host_data_read(cyg_uint32 *,cyg_uint32);
extern cyg_uint32 host_data_write(cyg_uint32 *,cyg_uint32);
extern void host_cfg_block(cyg_uint32 blk_len, cyg_uint32 nob);
extern void host_init(cyg_uint32 base_address);
extern void esdhc_softreset(cyg_uint32 mask);
/*================================================================================================*/
#endif  /* _MXCMCI_HOST_H_ */
