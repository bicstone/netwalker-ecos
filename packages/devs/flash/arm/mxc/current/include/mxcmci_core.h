#ifndef _MXCMCI_CORE_H_
#define _MXCMCI_CORE_H_

/*=================================================================================

    Module Name:  mxcmci_core.h

    General Description: Limited Bootloader eSDHC Driver.

===================================================================================
                               Copyright: 2004,2005,2006,2007,2008 FREESCALE, INC.
                   All Rights Reserved. This file contains copyrighted material.
                   Use of this file is restricted by the provisions of a
                   Freescale Software License Agreement, which has either
                   accompanied the delivery of this software in shrink wrap
                   form or been expressly executed between the parties.


Revision History:
                            Modification     Tracking
Author (core ID)                Date          Number     Description of Changes
-------------------------   ------------    ----------   --------------------------
Lewis Liu                  18-June-2008


Portability: Portable to other compilers or platforms.

 modification information
 ------------------------
 2009/07/13 : mxcmci_data_read() offset 64bit.

====================================================================================================*/

#include "mxcmci_mmc.h"
#include "mxcmci_host.h"

#define SUCCESS 0
#define FAIL 1
#define NO_ARG 0
#define RCA_SHIFT 16
#define ONE 1
#define FOUR 4
#define EIGHT 8
#define TWO_K_SIZE 2048
#define MMCSD_READY_TIMEOUT    3000  /* ~(3s / (2 * 48 * 10us)) */
#define ESDHC_ACMD41_TIMEOUT 48000 /* 1.5 sec =1500 msec delay for ACMD41 cmd */
#define MMCSD_SUPPORT

#define CURR_CARD_STATE(r) ((cyg_uint32) ((r) & 0x1E00) >> 9)

/*Defines of CSD data*/
#define CSD_STRUCT_MSK                       0x00C00000
#define CSD_STRUCT_SHIFT                    22
#define MMC_CSD_SPEC_VERS_MASK      0x003C0000
#define MMC_CSD_SPEC_VERS_SHIFT     18

extern cyg_uint32 Card_rca;
extern cyg_uint32 address_mode;
extern cyg_uint32 MMC_Spec_vers;
extern card_specific_data csd;  /* Global variable for Card Specific Data */
extern cyg_uint32 Card_capacity_size; /* Capacity size (C_SIZE) for card*/
extern cyg_uint32 CCC; /* Card Command Class */


/* Defines the id for each command */
enum commands
{
	CMD0= 0,
	CMD1= 1,
	CMD2= 2,
	CMD3= 3,
	CMD5= 5,
	CMD6=6,
	ACMD6= 6,
	CMD7= 7,
	CMD8=8,
	CMD9=9,
	CMD12   = 12,
	CMD13   = 13,
	CMD16   = 16,
	CMD17   = 17,
	CMD18   = 18,
	CMD24   = 24,
	CMD25   = 25,
	CMD26   = 26,
	CMD32   = 32,
	CMD33   = 33,
	CMD35   = 35,
	CMD36   = 36,
	CMD37   = 37,
	CMD38   = 38,
	CMD39   = 39,
	ACMD41  = 41,
	CMD43   = 43,
	ACMD51  = 51,
	CMD55   = 55,
	CMD60   = 60,
	CMD61   = 61,
	CMD62   = 62,
};

/* Defines for the states of the card*/
enum states
{
	IDLE,
	READY,
	IDENT,
	STBY,
	TRAN,
	DATA,
	RCV,
	PRG,
	DIS
};

/* Defines for card types */
typedef enum
{
	TYPE_NONE,
	SD_CSD_1_0,
	SD_CSD_2_0,
	MMC_CSD_1_0,
	MMC_CSD_1_1,
	MMC_CSD_1_2,
	MMC_UNKNOWN
}card_type;

typedef struct
{
	cyg_uint32 cid0;
	cyg_uint32 cid1;
	cyg_uint32 cid2;
	cyg_uint32 cid3;
}card_ident;


/* CARD Flash Configuration Parameters Structure */
typedef struct {
    cyg_uint32  length;         /* Length of Card data to read */
} CARD_FLASH_CFG_PARMS_T;

/*==================================================================================================
                                             ENUMS
==================================================================================================*/

/*==================================================================================================
                                          Global Function
==================================================================================================*/
extern cyg_uint32 mxcmci_init (cyg_uint32 bus_width, cyg_uint32 base_address);
#ifdef MX51_ERDOS
extern cyg_uint32 mxcmci_data_read (cyg_uint32* dest_ptr,cyg_uint32 len,cyg_uint64 offset);
#else
extern cyg_uint32 mxcmci_data_read (cyg_uint32* dest_ptr,cyg_uint32 len,cyg_uint32 offset);
#endif/* MX51_ERDOS */
extern cyg_uint32 mxcmci_software_reset (void);
extern cyg_uint32 mxcmci_get_cid (void);
extern cyg_uint32 mxcmci_trans_prepare(void);
extern void   mxcmci_cmd_config (command_t *cmd_config,cyg_uint32 index,cyg_uint32 argument,xfer_type_t transfer,response_format_t format,
                                  data_present_select data,crc_check_enable crc,cmdindex_check_enable cmdindex);


#endif //_MXCMCI_CORE_H_
