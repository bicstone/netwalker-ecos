//==========================================================================
//
//      usbs_imx.c
//
//      Device driver for the i.MX51 or i.MX37 USB OTG port.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is a part of Diagnosis Package based on eCos for Freescale i.MX 
// Family microprocessor.
// Copyright (C) 2008 Freescale Semiconductor, Inc.
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
// Author(s):    fisherz
// Contributors: fisherz
// Date:         2008-10-16
//
// This code implements support for the on-chip USB OTG port on the Freescale i.MX
// family of processors. The code has been developed on the i.MX and
// may or may not work on other members of the i.MX family. There
// have problems with the USB support on certain revisions of the silicon,
// so the errata sheet appropriate to the specific processor being used
// should be consulted. There also appear to be problems which do not
// appear on any errata, which this code attempts to work around.
//
// [Note] DMA is not enabled for USB transfer
//####DESCRIPTIONEND####
//
//####REVISION HISTORY####
//	Date			Author				Comments
//  22Jul08			Fisher ZHU(b18985)	Created for i.MX37 eCos USB device driver
//  16Oct08			Fisher ZHU(b18985)  Ported to i.MX51 USB OTG core
//==========================================================================
#include <string.h>	//use memset() of C run-time library
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/diag.h>
#include <pkgconf/hal_arm.h>
#include <pkgconf/devs_usb_imx_otg.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_cache.h>
#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
#include <cyg/error/codes.h>
#endif
#include <cyg/io/usb/usb.h>
#include <cyg/io/usb/usbs.h>
#include <cyg/io/usb/usbs_imx.h>

#pragma O0	//this pragma is useful when Realview tool chain is used
#define VOLATILE volatile

#if defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
//this error code is defined in error/include/codes.h
//but when the usb driver is used in redboot, the codes.h won't
//be included, so that this definition will solve the problem
#define EPIPE 304	

/* Constants */
#define SDP_CMD_MAX_LEN	0x10  /* 16 bytes */
#define SDP_CMD_ACK_LEN	0x4   /*  4 bytes */
/* Command Packet Format: Header(2)+Address(4)+Format(1)+ByteCount(4)+Data(4) */
#define	READ_HEADER		0x0101	//Read the flag in an assigned address on the target board
#define	WRITE_HEADER	0x0202
#define	WRITE_FILE		0x0404	//Write a file in host PC to target board
#define	READ_FILE		0x0A0A	//Read a block of RAM to host PC and save in a file
#define	ERROR_STATUS_HEADER     0x0505

/* SDP Responses */
#define WRITE_COMPLETE	0x128A8A12

/* SDP States */
#define CONTINUE        0
#define DONE            1
#define COMPLETE        0x88

#define USB_DOWNLOAD_TIMEOUT_LIMIT	0x1D000000
cyg_uint32 usb_download_address;
cyg_uint32 usb_download_length;
static cyg_uint8 sdp_payload_data[SDP_CMD_MAX_LEN]; /* Used to send or receive Command/ACK */
static cyg_uint8 sdp_command[SDP_CMD_MAX_LEN];      /* Used to store Command */
static cyg_uint8 g_error_status;
static cyg_uint8 g_usb_download_state = CONTINUE;
static cyg_uint32 g_timeout_value = 0;
static cyg_uint32 g_load_cycle;
static cyg_bool pl_get_command(void);
static cyg_uint8 pl_command_start(void);
static cyg_uint8 pl_handle_command(cyg_uint8 g_error_status);
static void pl_command_ack(cyg_uint32 ack);
static void pl_handle_write_file(cyg_uint32 address, cyg_uint32 total_bytes);
static cyg_uint32 usb_rx_processing(cyg_uint8* read_ptr, usb_status_t* status, cyg_uint32 data_length);
static usb_status_t usb_tx_processing(cyg_uint8* write_ptr, cyg_uint32 data_len);
#endif

/*	Bit3 - Mass Storage Information
	Bit2 - Enumeration Information
	Bit1 - Transaction Information
	Bit0 - Basic Information	
*/
//#define DEBUG_TRANS	0x8	//also defined in usbs_msc.c
#define DEBUG_ENUM	0x4
#define DEBUG_TRANS	0x2
#define DEBUG_BASIC	0x1

//#define USBDBGMSG(str) if(g_debug_switch&0x1) diag_printf(str)
#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
extern cyg_uint32 g_debug_switch; //the lowest 4-bit is used for USB debug
#if 1
#define USBDBGMSG(opt,fmt,args...) if(g_debug_switch&opt) diag_printf(fmt, ## args)
#else
#define USBDBGMSG(opt,fmt,args...)
#endif

#else
#define USBDBGMSG(opt,fmt,args...) //diag_printf(fmt, ## args)
#define D(fmt,args...) diag_printf(fmt, ## args)
#endif

// ----------------------------------------------------------------------------
//volatile cyg_uint8 g_bulkbuffer[BULK_TD_BUFFER_TOTAL_SIZE*NUM_OF_BULK_BUFFER] __attribute__((aligned(0x1000)));
bulk_buffer_t g_bulkbuffer_a;
bulk_buffer_t g_bulkbuffer_b;

//This variable is used to workaround the 31-byte packet issue in i.MX37
//It is initialized as "0x1",
//When data read/write, it must initialize as '0x0'
cyg_uint32 g_td_buffer_offset = 0;	

//The below two flags is used to distinguish the received data is data or command
cyg_uint32 g_received_data_type;

/* This is used to pause the EP2 In wait for complete, just a workaround for this issue
   It is not sure to be a bug of IC or software, need to check later.
*/
//cyg_uint8 g_tx_done=1;	//to keep EP1 issue sempahore to scsi after the previous CBW processed
cyg_uint8 g_ep2_complete_bit_set = 0;

#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
extern cyg_sem_t usbs_msc_sem;				//semaphore to schedule mass storage command handling thread
#endif

// ----------------------------------------------------------------------------
// Static pointers for USB buffer layout
/*=============================================================================
                                      STATIC VARIABLES
//============================================================================*/

// Allocate 2k-byte buffer as USB QueueHeaderList region.
// Don't use #pragma arm section in GCC
// !!!!USB buffer should not be cached and buffered.
//#pragma arm section rwdata="usb_buffer_no_init", zidata="usb_buffer_no_init"
//2k aligned, this buffer must be uncacheable and unbufferable.

#if defined(CYGHWR_IMX_USB_BUFFER_USE_IRAM)
static volatile cyg_uint8 usb_buffer[BUFFER_SIZE] __attribute__((aligned(0x800)));	
static volatile cyg_uint8 bulk_buffer[BULK_TD_BUFFER_TOTAL_SIZE*NUM_OF_BULK_BUFFER] __attribute__((aligned(0x1000)));
//#pragma arm section
#else
/* iRAM is configured as uncacheable and unbufferable in MMU initialization
    Reserve 0x800 bytes as USB buffer
    Don't use 0x10001000~0x10001800 for other program. */
#if defined(CYGHWR_USB_DEVS_MX37_OTG)
static volatile cyg_uint8 * usb_buffer=(cyg_uint8 *)(0x10001000); 
static volatile cyg_uint8 * bulk_buffer = (cyg_uint8 *)(0x10002000);
#endif

#if defined(CYGHWR_USB_DEVS_MX51_OTG)
static volatile cyg_uint8 * usb_buffer=(cyg_uint8 *)(0x1FFE9000); 
static volatile cyg_uint8 * bulk_buffer = (cyg_uint8 *)(0x1FFEA000);
#endif
// 
#endif	//defined(CYGHWR_IMX_USB_BUFFER_USE_IRAM)

VOLATILE usbs_imx_otg_hardware* usbs_imx_otg_base = (VOLATILE usbs_imx_otg_hardware* const) USB_BASE_ADDRESS;

static void usbs_imx_otg_config_utmi_clock(void);


/* Base address of the buffer allocated to IP Layer */
static VOLATILE cyg_uint32		g_bulkbuffer_address_base;
/* length of the buffer */
static VOLATILE cyg_uint32		g_bulkbuffer_length;	
/* Buffer information used for data transfer */
static VOLATILE buffer_map_t		g_bulkbuffer_map; 		
/* Number of Endpoints configured in system */
static VOLATILE cyg_uint8		g_max_ep_supported;   
/* State os USB Device */
static VOLATILE usb_state_t		g_usb_dev_state = USB_DEV_DUMMY_STATE; 
/* Length of setup data received */
static VOLATILE cyg_uint8 *		g_usb_setup_data;
/* Array to keep information about the endpoints used */
static VOLATILE usb_end_pt_info_t	g_end_pt_info[USB_DEV_INF_DESC_NUM_OF_EP]; 
/* Number of endpoints */
static VOLATILE cyg_uint8		g_number_of_endpoints; 
/* USB Descriptors */
static VOLATILE usb_descriptor	g_usb_desc;		
/* Number of Endpoint configured as IN */
static VOLATILE cyg_uint8		g_in_endpoint;	
/* Number of Endpoint configured as OUT*/
static VOLATILE cyg_uint8		g_out_endpoint;		

/* Support for the interrupt handling code.*/
static cyg_interrupt 	g_usbs_dev_intr_data;
static cyg_handle_t		g_usbs_dev_intr_handle;
static volatile int		g_isr_status_bits = 0;

// ----------------------------------------------------------------------------
// get the base address of queue header for an endpointer
#define USBS_EP_GET_dQH(endptno,dir) (g_bulkbuffer_map.ep_dqh_base_addrs + (SIZE_OF_QHD * (endptno * 2 + dir)))
#define USBS_EP_GET_dTD(endptno,dir)	(g_bulkbuffer_map.ep_dtd_base_addrs + (SIZE_OF_DTD0 + SIZE_OF_DTD1) * ( endptno * 2 + dir))
// ----------------------------------------------------------------------------
// USB interrupt enable/disable macros
#define USBS_IMX_OTG_INTR_MASK() 		(usbs_imx_otg_base->usbintrclr = 0xFFFFFFFF)//0|IMX_USB_INTR_DEV_RESET|IMX_USB_INTR_DEV_USBINT)
#define USBS_IMX_OTG_INTR_UNMASK(intr)	(usbs_imx_otg_base->usbintr = 0|(intr))

// ----------------------------------------------------------------------------
// Check if the IOS bit of QueueHeader or the IOC bit of Transfer Descriptor are set
#define USBS_dQH_IOS_CHECK(ep_num,dir) (((*(cyg_uint32*)USBS_EP_GET_dQH(ep_num,dir))&0x8000)?1:0)
#define USBS_dTD_IOC_CHECK(ep_num,dir) (((*(cyg_uint32*)USBS_EP_GET_dTD(ep_num,dir))&0x8000)?1:0)

// ----------------------------------------------------------------------------
// Set USB device address
#define USBS_DEVICE_SET_ADDRESS(addr) (usbs_imx_otg_base->devaddr = ((cyg_uint32)addr & 0x7F) << 25)
/*  
#*************  
#   OTG  
#*************  
*/
#define  USB_OTG_ID					(&(usbs_imx_otg_base->id))					/*   Identification Register					*/
#define  USB_OTG_HWGENERAL			(&(usbs_imx_otg_base->hwgeneral))	/*   General Hardware Parameters			*/
#define  USB_OTG_HWHOST				(&(usbs_imx_otg_base->hwhost))			/*   Host Hardware Parameters 				*/
#define  USB_OTG_HWDEVICE			(&(usbs_imx_otg_base->hwdevice))		/*   Device Hardware Parameters 			*/
#define  USB_OTG_HWTXBUF			(&(usbs_imx_otg_base->hwtxbuf))		/*   TX Buffer Hardware Parameters		*/
#define  USB_OTG_HWRXBUF			(&(usbs_imx_otg_base->hwrxbuf))		/*   RX Buffer Hardware Parameters		*/

#define  USB_OTG_CAPLENGTH			(&(usbs_imx_otg_base->caplength))	/*   Capability Register Length 			*/
#define  USB_OTG_HCIVERSION			(&(usbs_imx_otg_base->hciversion)) /*   Host Interface Version Number		*/
#define  USB_OTG_HCSPARAMS			(&(usbs_imx_otg_base->hcsparams))	/*   Host Ctrl. Structural Parameters */
#define  USB_OTG_HCCPARAMS			(&(usbs_imx_otg_base->hccparams))  /*   Host Ctrl. Capability Parameters */
#define  USB_OTG_DCIVERSION			(&(usbs_imx_otg_base->dciversion)) /*   Dev. Interface Version Number 		*/
#define  USB_OTG_DCCPARAMS			(&(usbs_imx_otg_base->dccparams))	/*   Dev. Ctrl. Capability Parameters */

#define  USB_OTG_USBCMD				(&(usbs_imx_otg_base->usbcmd))			/*   USB Command 											*/
#define  USB_OTG_USBSTS				(&(usbs_imx_otg_base->usbsts))			/*   USB Status 											*/
#define  USB_OTG_USBINTR			(&(usbs_imx_otg_base->usbintr))		/*   USB Interrupt Enable 						*/
#define  USB_OTG_FRINDEX			(&(usbs_imx_otg_base->frindex))		/*   USB Frame Index 									*/

#define  USB_OTG_DEVICEADDR			(&(usbs_imx_otg_base->devaddr))		/*   USB Device Address 							*/
#define  USB_OTG_PERIODICLISTBASE	USB_OTG_DEVICEADDR 						/*   Frame List Base Address 					*/
#define  USB_OTG_ENDPOINTLISTADDR	(&(usbs_imx_otg_base->endptlistaddr)) /*Address of Endpt list in memory*/
#define  USB_OTG_ASYNCLISTADDR		USB_OTG_ENDPOINTLISTADDR				/*   Next Asynchronous List Address 	*/

#define  USB_OTG_BURSTSIZE			(&(usbs_imx_otg_base->burstsize))  /*   Programmable Burst Size 					*/
#define  USB_OTG_TXFILLTUNING		(&(usbs_imx_otg_base->txfilltuning)) /* Host TX Pre-Buffer Packet Tuning */
#define  USB_OTG_VIEWPORT			(&(usbs_imx_otg_base->ulpiviewport)) /* ULPI Register 										*/
#define  USB_OTG_ENDPTNAK			(&(usbs_imx_otg_base->endptnak)) 	/*Endpoint NAK 												*/
#define  USB_OTG_ENDPTNAKEN			(&(usbs_imx_otg_base->endptnaken)) /*Endpoint NAK Enable								*/
#define  USB_OTG_CONFIGFLAG			(&(usbs_imx_otg_base->configflg))	/*   Configured Flag Register 				*/
#define  USB_OTG_PORTSC1			(&(usbs_imx_otg_base->portsc1))  /*   Port 0 Status/Control   					*/
#define  USB_OTG_OTGSC				(&(usbs_imx_otg_base->otgsc))  		/*   OTG Status and Control						*/  
#define  USB_OTG_USBMODE 			(&(usbs_imx_otg_base->usbmode))  	/*   USB Device Mode 									*/
#define  USB_OTG_ENDPTSETUPSTAT		(&(usbs_imx_otg_base->endptsetupstat)) /*   Endpoint Setup Status 				*/
#define  USB_OTG_ENDPTPRIME			(&(usbs_imx_otg_base->endptprime)) /*   Endpoint Initialization 					*/
#define  USB_OTG_ENDPTFLUSH			(&(usbs_imx_otg_base->endptflush)) /*   Endpoint De-Initialize 					*/
#define  USB_OTG_ENDPTSTATUS		(&(usbs_imx_otg_base->endptstatus))/*   Endpoint Status 									*/
#define  USB_OTG_ENDPTCOMPLETE		(&(usbs_imx_otg_base->endptcomplete))	/*   Endpoint Complete 						*/
#define  USB_OTG_ENDPTCTRL0 		(&(usbs_imx_otg_base->endptctrl[0]))		/*   Endpoint Control 0 					*/
#define  USB_OTG_ENDPTCTRL1			(&(usbs_imx_otg_base->endptctrl[1]))		/*   Endpoint Control 1 					*/
#define  USB_OTG_ENDPTCTRL2			(&(usbs_imx_otg_base->endptctrl[2]))		/*   Endpoint Control 2 					*/
#define  USB_OTG_ENDPTCTRL3			(&(usbs_imx_otg_base->endptctrl[3]))		/*   Endpoint Control 3 					*/
#define  USB_OTG_ENDPTCTRL4			(&(usbs_imx_otg_base->endptctrl[4]))		/*   Endpoint Control 4 					*/
#define  USB_OTG_ENDPTCTRL5			(&(usbs_imx_otg_base->endptctrl[5]))		/*   Endpoint Control 5 					*/
#define  USB_OTG_ENDPTCTRL6			(&(usbs_imx_otg_base->endptctrl[6]))		/*   Endpoint Control 6 					*/
#define  USB_OTG_ENDPTCTRL7			(&(usbs_imx_otg_base->endptctrl[7]))		/*   Endpoint Control 7 					*/
// ****************************************************************************
// -----------------------USB Device Descriptors-------------------------------
// ****************************************************************************
/* USB Device Descriptor according to USB2.0 Specification */
static VOLATILE usb_device_descriptor g_usb_device_desc ={
	USB_DEV_DESC_LEN,
    USB_DEV_DESC_TYPE,             
    USB_DEV_DESC_SPEC_LB,              
    USB_DEV_DESC_SPEC_HB,        
    USB_DEV_DESC_DEV_CLASS,          
    USB_DEV_DESC_DEV_SUBCLASS,        
    USB_DEV_DESC_DEV_PROTOCOL,         
    USB_DEV_DESC_EP0_MAXPACKETSIZE,     
    USB_DEV_DESC_VENDORID_LB,               
    USB_DEV_DESC_VENDORID_HB,               
    USB_DEV_DESC_PRODUCTID_LB,               
    USB_DEV_DESC_PRODUCTID_HB,               
    USB_DEV_DESC_DEV_RELEASE_NUM_LB,          
    USB_DEV_DESC_DEV_RELEASE_NUM_HB,         
    USB_DEV_DESC_DEV_STRING_IND_MANUFACTURE,
    USB_DEV_DESC_DEV_STRING_IND_PRODUCT,  
    USB_DEV_DESC_DEV_STRING_IND_SERIAL_NUM,
    USB_DEV_DESC_DEV_NUM_CONFIGURATIONS
};


/* USB Config Descriptor according to USB2.0 Specification */
static VOLATILE usb_conf_desc g_usb_config_desc = {
	{ 
	USB_DEV_CONFIG_DESC_LEN,   
    USB_DEV_CONFIG_DESC_TYPE,   
    USB_DEV_CONFIG_DESC_TTL_LEN_LB ,   
    USB_DEV_CONFIG_DESC_TTL_LEN_HB ,   
    USB_DEV_CONFIG_DESC_NUM_0F_INF,  
    USB_DEV_CONFIG_DESC_CONFIG_VALUE ,  
    USB_DEV_CONFIG_DESC_STRING_INDEX, 
    USB_DEV_CONFIG_DESC_ATTRIBUTES,    
  	USB_DEV_CONFIG_DESC_MAX_POWER
	},
		/* USB Interface Descriptor according to USB2.0 Specification */
	{//09
  	USB_DEV_INF_DESC_LEN,  
    USB_DEV_INF_DESC_TYPE,  
    USB_DEV_INF_DESC_INF_INDEX, 
    USB_DEV_INF_DESC_ALT_SETTING,
    USB_DEV_INF_DESC_NUM_OF_EP,  /* NOTE : This should not be more than 2 */
    #if defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
	USB_DEV_INF_DESC_INF_CLASS_VENDOR,
	USB_DEV_INF_DESC_INF_SUBCLASS_NS_BLANK,
	USB_DEV_INF_DESC_INF_PROTOCOL,
	#else
	USB_DEV_INF_DESC_INF_CLASS_MSC, 
    USB_DEV_INF_DESC_INF_SUBCLASS_MSC_SCSI,   
    USB_DEV_INF_DESC_INF_PROTOCOL_MSC_BOT, 
    #endif
  	USB_DEV_INF_DESC_STRING_INDEX
	},
		/* USB Endpoint 1 Descriptors according to USB2.0 Specification, OUT */
 	{
	{//18
	USB_EP1_DESC_SIZE,
    USB_EP1_DESC_TYPE,
    USB_EP1_DESC_EP_ADDR,
    USB_EP1_DESC_ATTRIBUTES, 
    USB_EP1_DESC_MAX_PACKET_SIZE_HS_LB, 
    USB_EP1_DESC_MAX_PACKET_SIZE_HS_HB, 
  	USB_EP1_DESC_INTERVAL
  	},
		/* USB Endpoint 2 Descriptors according to USB2.0 Specification, IN */
	{//25
	USB_EP2_DESC_SIZE, 
    USB_EP2_DESC_TYPE,
    USB_EP2_DESC_EP_ADDR,    
    USB_EP2_DESC_ATTRIBUTES,  
    USB_EP2_DESC_MAX_PACKET_SIZE_HS_LB,  
    USB_EP2_DESC_MAX_PACKET_SIZE_HS_HB,  
  	USB_EP2_DESC_INTERVAL
  	}
  	}
};

/* USB String Descriptors 0, according to USB2.0 Specification */
static VOLATILE usb_str0_desc g_usb_otg_str0_desc ={
    USB_STR0_DESC_SIZE,
    USB_STR0_DESC_TYPE,
    USB_LANGUAGE_ID_LB,
    USB_LANGUAGE_ID_HB
};

/* 
 STRING DESCRIPTOR
 See table 9-15 in USB2.0 spec (www.usb.org)
 iManufacturer
*/
static VOLATILE usb_str1_desc g_usb_otg_string_desc1 ={
	USB_STR1_DESC_SIZE, 		/* bLength */
	USB_STR1_DESC_TYPE,			/* bDescriptorType */
	{
	'F', 0x00,							/* bString */
	'r', 0x00,
	'e', 0x00,
	'e', 0x00,
	's', 0x00,
	'c', 0x00,
	'a', 0x00,
	'l', 0x00,
	'e', 0x00,
	' ', 0x00,
	'S', 0x00,
	'e', 0x00,
	'm', 0x00,
	'i', 0x00,
	'C', 0x00,
	'o', 0x00,
	'n', 0x00,
	'd', 0x00,
	'u', 0x00,
	'c', 0x00,
	't', 0x00,
	'o', 0x00,
	'r', 0x00,
	' ', 0x00,
	'I', 0x00,
	'n', 0x00,
	'c', 0x00,
	'.', 0x00
	}
};
#if defined(CYGHWR_USB_DEVS_MX37_OTG)
/*iProduct*/	 
static VOLATILE usb_str2_desc g_usb_otg_string_desc2 = {
	USB_STR2_DESC_SIZE_NS,      	/* bLength */
	USB_STR2_DESC_TYPE,						/* bDescriptorType */
	{
	'M', 0x00,	                	/* bString */
	'A', 0x00,		
	'R', 0x00,			
	'L', 0x00,
	'E', 0x00,	
	'Y', 0x00,		
	' ', 0x00,	
	'U', 0x00,		
	'S', 0x00,			
	'B', 0x00,
	' ', 0x00,
	'O', 0x00,
	'T', 0x00,
	'G', 0x00,
	' ', 0x00
	}
};
//#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
/* USB Serial Number Descriptor which is mandatory to Mass Storage Device */
static VOLATILE usb_str4_desc g_usb_serialnumber_desc = {
		USB_SN_DESC_LEN,
		USB_SN_DESC_TYPE,
		{
		'2',0x00,
		'0',0x00,
		'0',0x00,
		'8',0x00,
		'1',0x00,
		'8',0x00,
		'9',0x00,
		'8',0x00,
		'5',0x00,
		'0',0x00,
		'3',0x00,
		'7',0x00
	  }
};
//#endif
#endif

#if defined(CYGHWR_USB_DEVS_MX51_OTG)
static VOLATILE usb_str2_desc g_usb_otg_string_desc2 = {
	USB_STR2_DESC_SIZE_NS,      	/* bLength */
	USB_STR2_DESC_TYPE,						/* bDescriptorType */
	{
	'E', 0x00,	                	/* bString */
	'l', 0x00,		
	'v', 0x00,			
	'i', 0x00,
	's', 0x00,	
	' ', 0x00,		
	'U', 0x00,	
	'S', 0x00,		
	'B', 0x00,			
	' ', 0x00,
	'O', 0x00,
	'T', 0x00,
	'T', 0x00,
	' ', 0x00,
	' ', 0x00
	}
};
//#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
/* USB Serial Number Descriptor which is mandatory to Mass Storage Device */
static VOLATILE usb_str4_desc g_usb_serialnumber_desc = {
		USB_SN_DESC_LEN,
		USB_SN_DESC_TYPE,
		{
		'2',0x00,
		'0',0x00,
		'0',0x00,
		'8',0x00,
		'1',0x00,
		'8',0x00,
		'9',0x00,
		'8',0x00,
		'5',0x00,
		'0',0x00,
		'5',0x00,
		'1',0x00
	  }
};
//#endif
#endif

/* STRING DESCRIPTOR
   See table 9-15 in USB2.0 spec (www.usb.org)
   iSerialNumber */
static VOLATILE usb_str3_desc g_usb_otg_string_desc3 = {
	USB_STR3_DESC_SIZE,					/* bLength */
	USB_STR3_DESC_TYPE,					/* bDescriptorType */
	{
	'F', 0x00,									/* bString */
	'r', 0x00,
	'e', 0x00,
	'e', 0x00,
	's', 0x00,
	'c', 0x00,
	'a', 0x00,
	'l', 0x00,
	'e', 0x00,
	' ', 0x00,
	'F', 0x00,
	'l', 0x00,
	'a', 0x00,
	's', 0x00,
	'h', 0x00
	}
};	 


// ****************************************************************************
// ----------------------------------------------------------------------------
// ****************************************************************************
// ----------------------------------------------------------------------------
// Static data. There is a data structure for each endpoint. The
// implementation is essentially a private class that inherits from
// common classes for control and data endpoints, but device drivers
// are supposed to be written in C so some ugliness is required.

// ----------------------------------------------------------------------------
// Endpoint 0 is always present, this module would not get compiled
// otherwise.
static void usbs_imx_otg_dev_ep0_start(usbs_control_endpoint*);
static void usbs_imx_otg_dev_poll(usbs_control_endpoint*);

typedef enum ep0_state {
    EP0_STATE_IDLE      = 0,
    EP0_STATE_IN        = 1,
    EP0_STATE_OUT       = 2
} ep0_state;

typedef struct ep0_impl {
    usbs_control_endpoint   common;			//struct usbs_control_endpoint defined in usbs.h
    ep0_state               ep_state;
    int                     length;
    int                     transmitted;
} ep0_impl;

static ep0_impl ep0 = {
    common:
    {
        state:                  USBS_STATE_POWERED, // The hardware does not distinguish  between detached, attached and powered.
        enumeration_data:       (usbs_enumeration_data*) 0,
        start_fn:               &usbs_imx_otg_dev_ep0_start,
        poll_fn:                &usbs_imx_otg_dev_poll,
        interrupt_vector:       IMX_IRQ_USB_DEV_SERVICE_REQUEST,
        control_buffer:         { 0, 0, 0, 0, 0, 0, 0, 0 },
        state_change_fn:        (void (*)(usbs_control_endpoint*, void*, usbs_state_change, int)) 0,
        state_change_data:      (void*) 0,
        standard_control_fn:    (usbs_control_return (*)(usbs_control_endpoint*, void*)) 0,
        standard_control_data:  (void*) 0,
        class_control_fn:       (usbs_control_return (*)(usbs_control_endpoint*, void*)) 0,
        class_control_data:     (void*) 0,
        vendor_control_fn:      (usbs_control_return (*)(usbs_control_endpoint*, void*)) 0,
        vendor_control_data:    (void*) 0,
        reserved_control_fn:    (usbs_control_return (*)(usbs_control_endpoint*, void*)) 0,
        reserved_control_data:  (void*) 0,
        buffer:                 (unsigned char*) 0,
        buffer_size:            0,
        fill_buffer_fn:         (void (*)(usbs_control_endpoint*)) 0,
        fill_data:              (void*) 0,
        fill_index:             0,
        complete_fn:            (usbs_control_return (*)(usbs_control_endpoint*, int)) 0
    },
    ep_state:           EP0_STATE_IDLE,
    length:             0,
    transmitted:        0
};

extern usbs_control_endpoint usbs_imx_otg_ep0 __attribute__((alias ("ep0")));

// Endpoint 1 is optional. If the application only involves control
// messages or only slave->host transfers then the endpoint 1
// support can be disabled.
//#ifdef CYGPKG_DEVS_USB_MX37_EP1

typedef struct ep1_impl {
    usbs_rx_endpoint    common;			//struct usbs_rx_endpoint defined in usbs.h
    int                 fetched;
    cyg_bool            using_buf_a;
} ep1_impl;

static void ep1_start_rx(usbs_rx_endpoint*);
static void ep1_set_halted(usbs_rx_endpoint*, cyg_bool);

static ep1_impl ep1 = {
    common: {
        start_rx_fn:        &ep1_start_rx,
        set_halted_fn:      &ep1_set_halted,
        complete_fn:        (void (*)(void*, int)) 0,
        complete_data:      (void*) 0,
        buffer:             (unsigned char*) 0,
        buffer_size:        0,
        halted:             0,
    },
    fetched:            0,
    using_buf_a:        0
};

extern usbs_rx_endpoint usbs_imx_otg_ep1 __attribute__((alias ("ep1")));
//#endif

// Endpoint 2 is optional. If the application only involves control
// messages or only host->slave transfers then the endpoint 2 support
// can be disabled.
//#ifdef CYGPKG_DEVS_USB_MX37_EP2

typedef struct ep2_impl {
    usbs_tx_endpoint        common;			//struct usbs_tx_endpoint defined in usbs.h
    int                     transmitted;
    int                     pkt_size;
} ep2_impl;

static void ep2_start_tx(usbs_tx_endpoint*);
static void ep2_set_halted(usbs_tx_endpoint*, cyg_bool);

static ep2_impl ep2 = {
    common: {
        start_tx_fn:        &ep2_start_tx,
        set_halted_fn:      &ep2_set_halted,
        complete_fn:        (void (*)(void*, int)) 0,
        complete_data:      (void*) 0,
        buffer:             (const unsigned char*) 0,
        buffer_size:        0,
        halted:             0,
    }, 
    transmitted:        0,
    pkt_size:           0
};

extern usbs_tx_endpoint usbs_imx_otg_ep2 __attribute__ ((alias ("ep2")));
//#endif

// ****************************************************************************
// -----------------------Static Functions Initialization----------------------
// ****************************************************************************
static void usbs_handle_get_descriptor(void);
static void usbs_handle_set_configuration(void);
static void usbs_handle_get_device_desc(void);
static void usbs_handle_get_config_desc(void);
static void usbs_handle_get_string_desc(void);
static void usbs_handle_get_configuration(void);
static void usbs_handle_set_address(void);

static void usbs_ep0in_fill_buffer(cyg_uint8 type, cyg_uint32 buffer_addrs);
static usb_status_t usbs_ep0_send_data(usb_buffer_descriptor_t* bd,cyg_uint8 zlt);
static usb_status_t usbs_ep0_receive_data(usb_buffer_descriptor_t* bd);

static void usbs_setup_queuehead(struct dqh_t* qhead);
static void usbs_setup_transdesc(struct dtd_t* td);
static void usbs_endpoint_stall(cyg_uint8 endpoint , cyg_uint8 direction);
static void usbs_status_phase(cyg_uint8 trans_type, cyg_uint8 direction);

static void usbs_imx_otg_dev_set_configuration(usb_end_pt_info_t* config_data);
static void usbs_imx_otg_dev_handle_bus_reset(void);
static void usbs_imx_otg_dev_handle_port_change(void);
static void usbs_imx_otg_hardware_init(void);

cyg_uint32 util_alloc_buffer(void);
void util_free_buffer(cyg_uint32 address);
void util_set_status_bulk_buffer(cyg_uint32 buffer_addr,int buffer_status);
// ****************************************************************************
// -----------------------Static Functions ------------------------------------
// ****************************************************************************
/*=============================================================================
FUNCTION:		usbs_setup_queuehead
DESCRIPTION:	This function is used to setup the dQH
	 ------------------------
	|	EP0 IN	(64 bytes)	 |
	| 				 |
	 ------------------------	dQH1
	|	EP0 OUT	(64 bytes)	 |
	| 				 |
	 ------------------------	dQH0
ARGUMENTS PASSED:
	cyg_uint32 dqh_base - Base Address of the dQH
	cyg_uint8 	zlt - zero lengh packet termination (enable - ZLT_ENABLE; disable - ZLT_DISABLE)
	cyg_uint16 mps - Max packet length
	cyg_uint8 	ios - interrupt on Setup
	cyg_uint32 next_link_ptr - Next Link Pointer, 
	cyg_uint8 	terminate - terminate - TERMINATE; not terminate - NOT_TERMINATE
	cyg_uint16 total_bytes - Total Bytes to be transfered in this dQH
	cyg_uint8 	ioc - interrupt on complete, set - IOC_SET, not set - IOC_NOTSET
	cyg_uint8 	status - status 
	cyg_uint32 buffer_ptr0 - Buffer Pointer page 0
	cyg_uint16 current_offset - current offset
	cyg_uint32 buffer_ptr1 - Buffer Pointer page 1
	cyg_uint32 buffer_ptr2 - Buffer Pointer page 1
	cyg_uint32 buffer_ptr3 - Buffer Pointer page 1
	cyg_uint32 buffer_ptr4 - Buffer Pointer page 1
	  
RETURN VALUE:	None		
IMPORTANT NOTES:None		
=============================================================================*/
static void 
usbs_setup_queuehead(struct dqh_t* qhead)
{
    volatile struct dqh_setup_t* dqh_word = (volatile struct dqh_setup_t*) qhead->dqh_base;

    /*Bit31:30 Mult; Bit29 zlt; Bit26:16 mps; Bit15 ios */
    dqh_word->dqh_word0 = (((cyg_uint32)(qhead->zlt) << 29)|((cyg_uint32)(qhead->mps) <<16) | ((cyg_uint32)(qhead->ios) <<15));

    /*Current dTD Pointer => for hw use, not modified by DCD software */
    dqh_word->dqh_word1 = 0x0;
	
    /*Next dTD Pointer */
    dqh_word->dqh_word2 = (((qhead->next_link_ptr) & 0xFFFFFFE0) | qhead->terminate);
	
    /*Bit30:16 total_bytes; Bit15 ioc; Bit11:10 MultO; Bit7:0 status */	
    dqh_word->dqh_word3 = ((((cyg_uint32)(qhead->total_bytes) & 0x7FFF)  << 16) | ((cyg_uint32)(qhead->ioc) <<15) | (qhead->status));

    /*Bit31:12 Buffer Pointer (Page 0) */
    dqh_word->dqh_word4 = ((qhead->buffer_ptr0 & 0xFFFFF000) | (qhead->current_offset & 0xFFF));

    /*Bit31:12 Buffer Pointer (Page 1) */
    dqh_word->dqh_word5 = (qhead->buffer_ptr1 & 0xFFFFF000);
	
    /*Bit31:12 Buffer Pointer (Page 2) */
    dqh_word->dqh_word6 = (qhead->buffer_ptr2 & 0xFFFFF000);

    /*Bit31:12 Buffer Pointer (Page 3) */
    dqh_word->dqh_word7 = (qhead->buffer_ptr3 & 0xFFFFF000);
	
    /*Bit31:12 Buffer Pointer (Page 4) */
    dqh_word->dqh_word8 = (qhead->buffer_ptr4 & 0xFFFFF000);

    /*Reserved */
    dqh_word->dqh_word9 = 0;

    /*Setup Buffer 0 */
    dqh_word->dqh_word10 = 0;

    /*Setup Buffer 1 */
    dqh_word->dqh_word11 = 0;
}
/*=============================================================================
FUNCTION: usbs_setup_transdesc
DESCRIPTION: This function is used to setup the dTD
ARGUMENTS PASSED:
	cyg_uint32 dtd_base - Base Address of the dTD
	cyg_uint32 next_link_ptr - Next Link Pointer, 
	cyg_uint8 	terminate - terminate - TERMINATE; not terminate - NOT_TERMINATE
	cyg_uint16 total_bytes - Total Bytes to be transfered in this dQH
	cyg_uint8 	ioc - interrupt on complete, set - IOC_SET, not set - IOC_NOTSET
	cyg_uint8 	Status - Status 
	cyg_uint32 buffer_ptr0 - Buffer Pointer page 0
	cyg_uint16 current_offset - current offset
	cyg_uint32 buffer_ptr1 - Buffer Pointer page 1
	cyg_uint32 buffer_ptr2 - Buffer Pointer page 1
	cyg_uint32 buffer_ptr3 - Buffer Pointer page 1
	cyg_uint32 buffer_ptr4 - Buffer Pointer page 1	  
RETURN VALUE:	None		
IMPORTANT NOTES:None		
==============================================================================*/
static void 
usbs_setup_transdesc(struct dtd_t* td)
{
    volatile struct dtd_setup_t* dtd_word = (volatile struct dtd_setup_t *) td->dtd_base;

    /* Bit31:5 Next Link Pointer ; Bit0 terminate */
    dtd_word->dtd_word0 = ((td->next_link_ptr & 0xFFFFFFE0) | td->terminate);

    /* Bit30:16 total_bytes, Bit15 ioc, Bit7:0 status */
    dtd_word->dtd_word1 = ((((cyg_uint32)td->total_bytes & 0x7FFF) << 16)| ((cyg_uint32)td->ioc <<15) | (td->status));
	
    /* Bit31:12 Buffer Pointer Page 0 ; Bit11:0 Current Offset */
    dtd_word->dtd_word2 = ((td->buffer_ptr0 & 0xFFFFF000) | (td->current_offset & 0xFFF));
	
    /* Bit31:12 Buffer Pointer Page 1 ; Bit10:0 Frame Number */
    dtd_word->dtd_word3 = (td->buffer_ptr1 & 0xFFFFF000);

    /* Bit31:12 Buffer Pointer Page 2 ; */
    dtd_word->dtd_word4 = (td->buffer_ptr2 & 0xFFFFF000);

    /* Bit31:12 Buffer Pointer Page 3 ; */
    dtd_word->dtd_word5 = (td->buffer_ptr3 & 0xFFFFF000);

    /* Bit31:12 Buffer Pointer Page 4 ; */
    dtd_word->dtd_word6 = (td->buffer_ptr4 & 0xFFFFF000);

}

/*==================================================================================================
FUNCTION: util_alloc_buffer
DESCRIPTION:    	This utility function allocate the free buffer available
ARGUMENTS PASSED: 	None
RETURN VALUE:		cyg_uint32 address : address of the allocated buffer	
IMPORTANT NOTES:	If Buffer1 is FREE then return Buffer1 and mark this as Busy else check for buffer2 . If 
	none of the buffer is free then return NULL.
==================================================================================================*/
cyg_uint32 util_alloc_buffer(void)
{
    cyg_uint32 buffer_addr = (cyg_uint32)NULL; //force type conversion for multiple NULL definitions
    
    /* Check if buffer1 is free then mark it busy and return address */ 
    if (g_bulkbuffer_map.buffer1_status == BUFFER_FREE )
    {
        buffer_addr = g_bulkbuffer_map.buffer1_address;
		g_bulkbuffer_map.buffer1_status = BUFFER_IN_USE;
    }
    /* Check if buffer2 is free then mark it busy and return address */ 
    else if(g_bulkbuffer_map.buffer2_status == BUFFER_FREE)
    {
        buffer_addr = g_bulkbuffer_map.buffer2_address;
		g_bulkbuffer_map.buffer2_status = BUFFER_IN_USE;
    }
    
    return buffer_addr ;
}
/*==================================================================================================
FUNCTION: 			util_free_buffer
DESCRIPTION: 		This function put the buffer in free state.
ARGUMENTS PASSED:	cyg_uint32 address : address of the buffer . 	
RETURN VALUE: 		None
IMPORTANT NOTES:	None
		
==================================================================================================*/
void util_free_buffer(cyg_uint32 address)
{
	if( address == g_bulkbuffer_map.buffer1_address )
    {
        g_bulkbuffer_map.buffer1_status = BUFFER_FREE;
    }
    else if ( address == g_bulkbuffer_map.buffer2_address )
    {
        g_bulkbuffer_map.buffer2_status = BUFFER_FREE;
    }
}
/*==================================================================================================
FUNCTION: 			util_set_bulk_buffer_stat
DESCRIPTION: 		This function change the bulk buffer status
ARGUMENTS PASSED:	cyg_uint32 buffer_addr: buffer base address
					int buffer_status: new buffer_status
enum {
	BUFFER_FREED,
	BUFFER_RELEASED,
	BUFFER_ALLOCATED
};
RETURN VALUE: 		None
IMPORTANT NOTES:	None
		
==================================================================================================*/
void util_set_status_bulk_buffer(cyg_uint32 buffer_addr,int buffer_status)
{
	if( buffer_addr == (cyg_uint32)g_bulkbuffer_a.buffer)
    {
        g_bulkbuffer_a.stat = buffer_status;
    }
    else if ( buffer_addr == (cyg_uint32)g_bulkbuffer_b.buffer )
    {
        g_bulkbuffer_b.stat = buffer_status;
    }
	else
		return;
}
/*=============================================================================
FUNCTION:	usbs_endpoint_stall
DESCRIPTION: This function Send/Receive the STALL HANDSHAKE to  USB Host
ARGUMENTS PASSED:
	cyg_uint8 endpoint  -	Endpoint Number .	
	cyg_uint8 direction -	IN/OUT :  direction of EndPoint.
RETURN VALUE:	None
IMPORTANT NOTES:None		
==============================================================================*/
static void
usbs_endpoint_stall(cyg_uint8 endpoint , cyg_uint8 direction)
{
    if( direction == OUT )
    { 
    	usbs_imx_otg_base->endptctrl[endpoint]|= STALL_RX;
    }		    
    else 
    {
    	usbs_imx_otg_base->endptctrl[endpoint] |= STALL_TX;
    }

	USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - EP%d - %d stalled\n",endpoint,direction);
}

static void
usbs_endpoint_unstall(cyg_uint8 endpoint , cyg_uint8 direction)
{
	if( direction == OUT )
    { 
    	usbs_imx_otg_base->endptctrl[endpoint]&= ~STALL_RX;
    }		    
    else 
    {
    	usbs_imx_otg_base->endptctrl[endpoint]&= ~STALL_TX;
    }
}

/*=============================================================================
FUNCTION:			usbs_status_phase
DESCRIPTION:		This function Send/Receive the Status to/from Host.
ARGUMENTS PASSED:   cyg_uint8    direction		OUT 	Receive Status Command From Host
			 								IN 	Send Status Command to Host
RETURN VALUE:		None
IMPORTANT NOTES:	
===============================================================================*/
static void 
usbs_status_phase(cyg_uint8 trans_type, cyg_uint8 direction)
{
	usb_buffer_descriptor_t bd ;
	
    /* Buffer pointer is not used for EP0 */
    bd.buffer = (cyg_uint32 *) 0xFFFFFFFF;
    bd.size = 0x0;
	
    if(trans_type==CONTROL)
    {
    	switch ( direction )
    	{
			case OUT :
	    		/*  Receive ZERO length Length Data */
	    		usbs_ep0_receive_data(&bd);
    	    	break;
			case IN :
	    		/* Send ZERO length Length Data */
	    		usbs_ep0_send_data(&bd,0);
	    		break;
    	}
    }
    else if(trans_type==BULK)/*TODO*/
    {
    	switch ( direction )
    	{
			case OUT :	    
	    		/* Send ZERO length Length Data */
	    		//usbs_ep2_send_data(EP2,&bd,FALSE);
    	    	break;
			case IN :
	    		/*  Receive ZERO length Length Data */
	    		//usbs_ep1_receive_data(EP1,&bd);
	    		break;
    	}
    } 
    
}
// ---------------------------------------------------------------------------
// The following static functions are for USB device enumeration processing
/*============================================================================
FUNCTION: 				usbs_handle_get_descriptor
DESCRIPTION:			This function Handle the GET DESCRIPTOR request
ARGUMENTS PASSED:	None  
RETURN VALUE:			None
IMPORTANT NOTES:	None		
============================================================================*/
static void
usbs_handle_get_descriptor()
{
	USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - get descriptor handler\n");
	switch (g_usb_setup_data[WVALUE_HIGHBYTE])
	{
		case DEVICE_DESC:  	/* device descriptor*/
			usbs_handle_get_device_desc();	//Device will send the MPS to host
			break;
		case CONF_DESC:  	/* configuration descriptor*/
			usbs_handle_get_config_desc();	//Device will send the whole device descriptor to host
	    	break;
		case STRING_DESC:	/* string descriptor*/
			usbs_handle_get_string_desc();
	    	break;
		case INTERFACE_DESC:
	 	case ENDPOINT_DESC:
  		case DEVICE_QUALIFIER:
   		case OTHER_SPEED_CONF_DESC:
		default:	/* Send STALL Handshake  */
	    	usbs_endpoint_stall(EP0,IN);
	    	break;
	}

	
}
/*=============================================================================
FUNCTION: 			usbs_handle_get_device_desc
DESCRIPTION: 		This function Handle the GET DEVICE DESCRIPTOR request
ARGUMENTS PASSED:	None
RETURN VALUE:		None
IMPORTANT NOTES:	None		
==============================================================================*/
static void 
usbs_handle_get_device_desc(void)
{
    usb_buffer_descriptor_t bd ;
    cyg_uint32	buffer_addrs;
    cyg_uint16	desc_length = 0x0;
    cyg_uint8	zlt = 0;//0 means false

	USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - get device descriptor\n");
    
    /* get the buffer address for data transfer over EP0 */
    buffer_addrs = g_bulkbuffer_map.ep0_buffer_addrs;	//256bytes before the two Bulk buffers
	
    /* Fill the buffer with the descriptor data */
    usbs_ep0in_fill_buffer(FILL_DEVICE_DESC,buffer_addrs);

    /* Get the length of descriptor requested */
    desc_length = g_usb_setup_data[WLENGTH_LOWBYTE];
    desc_length |= ( g_usb_setup_data[WLENGTH_HIGHBYTE] <<0x8);

    /* If requested length of descriptor is lesser than actual length of descriptor then send 
     * requested length of descroptor only else send the actual length of descriptor*/
    if( g_usb_dev_state == USB_DEV_DEFAULT_STATE )
    {
        bd.size = MPS_8;
    }
    else 
    {
        bd.size = USB_DEV_DESC_LEN;
    }

    /* Send descriptor - Data Phase*/
    usbs_ep0_send_data(&bd,zlt);	//zlt is false=>not zero length packet
    								//send dev descriptor to host.
	
    /* Status Phase -- OUT */
    usbs_status_phase(CONTROL,OUT); //Get Zero-length data packet from Host, Device sends status: ACK(success), NAK(busy), or STALL(failed)

		
}
/*=============================================================================
FUNCTION:		usbs_handle_get_config_desc
DESCRIPTION:	This function Handle the GET CONFIGURATION DESCRIPTOR request
ARGUMENTS PASSED:
RETURN VALUE:	None
IMPORTANT NOTES:None		
=============================================================================*/
static void 
usbs_handle_get_config_desc(void)
{
	usb_buffer_descriptor_t bd;
    cyg_uint32 buffer_addrs;
    cyg_uint16 desc_length_req = 0x0;
    cyg_uint16 desc_length = 0x0;
    int zlt = 0;

	USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - get config descriptor\n");
    /* get the buffer address for data transfer over EP0 */
    buffer_addrs = g_bulkbuffer_map.ep0_buffer_addrs;

	/* Fill the buffer with the descriptor data */
	usbs_ep0in_fill_buffer(FILL_CONF_DESC, buffer_addrs);
    
	/* total length of descriptor */
	desc_length = ((g_usb_desc.config_desc->usb_config_desc.total_length_lo) \
				| ( g_usb_desc.config_desc->usb_config_desc.total_length_hi << 0x8 ));
    /* Get the length of descriptor requested */
    desc_length_req = g_usb_setup_data[WLENGTH_LOWBYTE];
    desc_length_req |= ( g_usb_setup_data[WLENGTH_HIGHBYTE] <<0x8);


    /* If requested length of descriptor is lesser than actual length of descriotor then send 
     * requested length of descroptor only else send the actual length of descriptor*/
    if(desc_length_req <= desc_length)
    {
        bd.size = desc_length_req;
    }
    else
    {
        bd.size = desc_length;

		if ( bd.size > MPS_64)
		{
	    	zlt = 1;
		}
    }
    usbs_ep0_send_data(&bd,zlt);	
	
    /* Status Phase -- OUT */
    usbs_status_phase(CONTROL,OUT); 
		
}
/*=============================================================================
FUNCTION: 			usbs_handle_get_string_desc
DESCRIPTION: 		This function Handle the GET STRING DESCRIPTOR request
ARGUMENTS PASSED:	None
RETURN VALUE:		None	
IMPORTANT NOTES:	None		
==============================================================================*/
static void 
usbs_handle_get_string_desc(void)
{
	usb_buffer_descriptor_t bd;
    cyg_uint32 buffer_addrs;
    cyg_uint16 desc_length_req = 0x0;
    cyg_uint16 length_of_desc = 0x0;
    int zlt = 0;

	
    /* Get Buufer to fill the data to be received/transmitted.    */ 
    buffer_addrs = g_bulkbuffer_map.ep0_buffer_addrs;
	   
    /* Get the length of descriptor requested */
    desc_length_req = g_usb_setup_data[WLENGTH_LOWBYTE];
    desc_length_req |= ( g_usb_setup_data[WLENGTH_HIGHBYTE] <<0x8);

    switch (g_usb_setup_data[WVALUE_LOWBYTE])
    {
        case STR_DES0:
            usbs_ep0in_fill_buffer(FILL_STR_DES0,buffer_addrs);
    	    /* If requested length of descriptor is lesser than actual length of descriotor then send 
		     * requested length of descroptor only else send the actual length of descriptor*/
	    	if(desc_length_req <= g_usb_desc.str_desc0->length )
	    	{
	        	bd.size = desc_length_req;
	    	}
	    	else
	    	{
				bd.size = g_usb_desc.str_desc0->length;
                if (  bd.size > MPS_64)
	        	{
	            	zlt = 1;
	        	}
	    	}
	    	/* Data Phase -- IN */
	    	usbs_ep0_send_data(&bd,zlt);		
	    	/* Status Phase -- OUT */
	    	usbs_status_phase(CONTROL,OUT);    
			USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - get string descriptor 0\n");
	    	break;
        
        case STR_DES1:		/*iManufacturer */
            usbs_ep0in_fill_buffer(FILL_STR_DES1,buffer_addrs);
	        /* If requested length of descriptor is lesser than actual length of descriotor then send 
	    	 * requested length of descroptor only else send the actual length of descriptor*/
		    if(desc_length_req <= g_usb_desc.str_desc1->length )
		    {
	    	    bd.size = desc_length_req;
	    	}
	    	else
	    	{
				bd.size = g_usb_desc.str_desc1->length;
                if (  bd.size > MPS_64) 
				{
	            	zlt = 1;
	        	}
	    	}
	    	/* Data Phase -- IN */
	    	usbs_ep0_send_data(&bd,zlt);	
		    /* Status Phase -- OUT */
		    usbs_status_phase(CONTROL,OUT); 
			USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - get string descriptor 1\n");
    	    break;
        
        case STR_DES2:		/*iProduct */
            usbs_ep0in_fill_buffer(FILL_STR_DES2,buffer_addrs );
			length_of_desc = g_usb_desc.str_desc2->length; 
		    /* If requested length of descriptor is lesser than actual length of descriotor then send 
	     	* requested length of descroptor only else send the actual length of descriptor*/
		    if(desc_length_req <= length_of_desc )
		    {
	    	    bd.size = desc_length_req;
		    }
		    else
	    	{
				bd.size = length_of_desc;
                if (  bd.size > MPS_64)
	    	    {
	        	    zlt = 1;
		        }
		    }
		    /* Data Phase -- IN */
	    	usbs_ep0_send_data(&bd,zlt);	
		    /* Status Phase -- OUT */
		    usbs_status_phase(CONTROL,OUT);     
			USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - get string descriptor 2\n");
		    break;
        
        case STR_DES3:	
			/* send zero length data */
			usbs_status_phase(CONTROL,IN);
			/* Status Phase -- OUT */
            usbs_status_phase(CONTROL,OUT);
			break;
        
        case STR_DES5:		/*iSerialNumber */
			#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
            usbs_ep0in_fill_buffer(FILL_SN_DESC,buffer_addrs );
		    /* If requested length of descriptor is lesser than actual length of descriotor then send 
			* requested length of descroptor only else send the actual length of descriptor*/
		    if(desc_length_req <= g_usb_desc.sn_desc->length )
		    {
	    	    bd.size = desc_length_req;
		    }
		    else
		    {
				bd.size = g_usb_desc.sn_desc->length;
                if (  bd.size > MPS_64)
		        {
		            zlt = 1;
		        }
		    }
	    	/* Data Phase -- IN */
		    usbs_ep0_send_data(&bd,zlt);	
		    /* Status Phase -- OUT */
		    usbs_status_phase(CONTROL,OUT); 
			USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - get string descriptor - SN\n");
		    break;
			#endif
		case STR_DES4:		/*iSerialNumber */
			usbs_ep0in_fill_buffer(FILL_STR_DES3,buffer_addrs );
		    /* If requested length of descriptor is lesser than actual length of descriotor then send 
			* requested length of descroptor only else send the actual length of descriptor*/
		    if(desc_length_req <= g_usb_desc.str_desc3->length )
		    {
	    	    bd.size = desc_length_req;
		    }
		    else
		    {
				bd.size = g_usb_desc.str_desc3->length;
                if (  bd.size > MPS_64)
		        {
		            zlt = 1;
		        }
		    }
	    	/* Data Phase -- IN */
		    usbs_ep0_send_data(&bd,zlt);	
		    /* Status Phase -- OUT */
		    usbs_status_phase(CONTROL,OUT); 
			USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - get string descriptor 3\n");
		    break;
		default:
	    	/* Send STALL Handshake  */
		    usbs_endpoint_stall(EP0,IN);
			//USBDBGMSG("+USBDBGMSG:EP0 IN stalled at get string desc\n");
		    break;
    }

		
}

/*=============================================================================
FUNCTION:		usbs_handle_set_address
DESCRIPTION:   	This function Handle the SET ADDRESS Request from USB Host
ARGUMENTS PASSED:	None
RETURN VALUE:		None
IMPORTANT NOTES:	
==============================================================================*/
static void 
usbs_handle_set_address(void)
{
		cyg_uint16 device_addrs;

		USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - set address handler\n");   
    /* Get the Device Address to be SET from the Setup Data  */
    device_addrs = g_usb_setup_data[WVALUE_LOWBYTE] + (g_usb_setup_data[WVALUE_HIGHBYTE]<<8);
       
    if	((g_usb_setup_data[WINDEX_LOWBYTE] == 0) &&
		(g_usb_setup_data[WINDEX_HIGHBYTE] == 0) &&
		(g_usb_setup_data[WLENGTH_LOWBYTE] == 0) &&
		(g_usb_setup_data[WLENGTH_HIGHBYTE] == 0) &&
		(device_addrs <= USB_MAX_DEVICE_ADDR))
    { 
        switch(g_usb_dev_state)
		{
	    	case USB_DEV_DEFAULT_STATE :
				/* Send Ack to Host */
				usbs_status_phase(CONTROL,IN);
				if (device_addrs != USB_DEFAULT_ADDR)
				{
					/* Set the Device Address */
	            	USBS_DEVICE_SET_ADDRESS(device_addrs);
					/* Change state to ADDRESSED STATE  */
					g_usb_dev_state	 = USB_DEV_ADDRESSED_STATE;
				}
	      		break;
		
			case USB_DEV_ADDRESSED_STATE :
				/* Send Ack to Host */
				usbs_status_phase(CONTROL,IN);
				if ( device_addrs == USB_DEFAULT_ADDR )
				{
		    		/* Set the Device Address */
		    		USBS_DEVICE_SET_ADDRESS(USB_DEFAULT_ADDR);
					/* Change state to ADDRESSED STATE  */
					g_usb_dev_state = USB_DEV_DEFAULT_STATE;    
    			}
				else
				{
					/* Set the Device Address */
					USBS_DEVICE_SET_ADDRESS(device_addrs);
				}
				break;
	
    	    case USB_DEV_CONFIGURED_STATE :
		    	if ( device_addrs == USB_DEFAULT_ADDR)
				{
					/* Send Ack to Host */
					usbs_status_phase(CONTROL,IN);
					/* Set the Device Address */
					USBS_DEVICE_SET_ADDRESS(device_addrs);
					/* Change state to ADDRESSED STATE  */
					g_usb_dev_state = USB_DEV_DEFAULT_STATE;    
				}
				else
				{
					/* Send STALL Handshake  */ 
					usbs_endpoint_stall(EP0,IN);
				}
			default :
				break;
		}
    }
    else
    {
        /* Send STALL Handshake */
        usbs_endpoint_stall(EP0,IN);
    }

		
}
/*=============================================================================
FUNCTION:			usbs_handle_get_configuration
DESCRIPTION:    	This function Handle the GET CONFIGURATION request
ARGUMENTS PASSED:	None  
RETURN VALUE:		None	
IMPORTANT NOTES:	None		
=============================================================================*/
static void 
usbs_handle_get_configuration(void)
{
	usb_buffer_descriptor_t bd;
	cyg_uint32 buffer_addrs;
    cyg_uint32* buffer_ptr;
 
    if((g_usb_setup_data[WINDEX_LOWBYTE] == 0) &&
        (g_usb_setup_data[WINDEX_HIGHBYTE] == 0) &&
       	(g_usb_setup_data[WVALUE_LOWBYTE] == 0) &&
       	(g_usb_setup_data[WVALUE_HIGHBYTE] == 0) &&
		(g_usb_setup_data[WLENGTH_LOWBYTE] == LEN_OF_CONFIG_VALUE) &&
       	(g_usb_setup_data[WLENGTH_HIGHBYTE] == 0)) 
    {
        switch(g_usb_dev_state)
		{
	    	case USB_DEV_DEFAULT_STATE :
	        	/* Send STALL Handshake */
	        	usbs_endpoint_stall(EP0,IN);
				break;
	    	case USB_DEV_ADDRESSED_STATE:
				/* If the Device is in Address state then return 0x0 : See USB2.0 Spec */
				buffer_addrs = g_bulkbuffer_map.ep0_buffer_addrs;
				buffer_ptr = (cyg_uint32 *)buffer_addrs;
				*buffer_ptr = 0x0;

				bd.buffer = buffer_ptr;
				bd.size=LEN_OF_CONFIG_VALUE;

				usbs_ep0_send_data(&bd,0);

				/* Receive Ack from Host*/
				usbs_status_phase(CONTROL,OUT);
				break;

			case USB_DEV_CONFIGURED_STATE:
				buffer_addrs = g_bulkbuffer_map.ep0_buffer_addrs;
                buffer_ptr = (cyg_uint32 *)buffer_addrs;

				*buffer_ptr = (cyg_uint32 )g_usb_desc.config_desc->usb_config_desc.configuration_id;

				bd.buffer = buffer_ptr;
				bd.size=LEN_OF_CONFIG_VALUE;

				usbs_ep0_send_data(&bd,0);
		
				/* Receive Ack from Host*/
				usbs_status_phase(CONTROL,OUT);
				break;

			default:
	        	/* Send STALL Handshake */
	        	usbs_endpoint_stall(EP0,IN);
		}

    } 

		USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - get config handler\n");
}
/*=============================================================================
FUNCTION:			usbs_handle_set_configuration
DESCRIPTION:    	This function Handle the SET CONFIGURATION request
ARGUMENTS PASSED:	None  
RETURN VALUE:		None	
IMPORTANT NOTES:	None		
=============================================================================*/
static void 
usbs_handle_set_configuration(void)
{
	usb_end_pt_info_t config_data ; 
    cyg_uint8 i;

    switch (g_usb_dev_state)
    {
		case USB_DEV_ADDRESSED_STATE :
			if (g_usb_setup_data[WVALUE_LOWBYTE] == USB_DEV_VALUE_OF_UNCONFIG)
			{	
				/* Send Ack to Host*/
				usbs_status_phase(CONTROL,IN);
		    }
            /* Check if the configuration value received request is same as in Config descriptor */
		    else if(g_usb_setup_data[WVALUE_LOWBYTE] == g_usb_desc.config_desc->usb_config_desc.configuration_id)
		    {
				/* Configure endpoints */
				for ( i = 0 ; i< g_number_of_endpoints ; i++)
				{
		    		config_data.end_pt_no		= g_end_pt_info[i].end_pt_no; 
		    		config_data.direction  		= g_end_pt_info[i].direction;
		    		config_data.transfer_type	= g_end_pt_info[i].transfer_type;
		    		config_data.max_pkt_size	= g_end_pt_info[i].max_pkt_size;
		
					usbs_imx_otg_dev_set_configuration(&config_data);
				}

				/* Send Ack to Host*/
				usbs_status_phase(CONTROL,IN);

				g_usb_dev_state = USB_DEV_CONFIGURED_STATE ;
	    	}
	    	else
	    	{
				/* Invalid configuration value.  Send STALL Handshake */
	        	usbs_endpoint_stall(EP0,IN);
				//USBDBGMSG("+USBDBGMSG:EP0 IN stalled at set conf in addr state\n");
	    	}
			USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - set conf@ADDRESSED_STATE\n");
			break;	
		case USB_DEV_CONFIGURED_STATE :
	    	if(g_usb_setup_data[WVALUE_LOWBYTE] == USB_DEV_CONFIG_DESC_CONFIG_VALUE)
	    	{
				/* Send Ack to Host*/
				usbs_status_phase(CONTROL,IN);
	    	}
	    	else if (g_usb_setup_data[WVALUE_LOWBYTE] == USB_DEV_VALUE_OF_UNCONFIG)
	    	{	
				/* Send Ack to Host*/
				usbs_status_phase(CONTROL,IN);

				/* Change USB State to Addressed State	*/
				g_usb_dev_state = USB_DEV_ADDRESSED_STATE;
	    	}
			USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - set conf@CONFIGURED_STATE\n");
			break;
		default :
	        /* Send STALL Handshake */
	        usbs_endpoint_stall(EP0,IN);
			USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - set conf@incorrect state\n");
		break;	
    }

	USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: enum - set config handler\n");
}
/*=============================================================================
FUNCTION:			usbs_handle_msc_get_maxlun
DESCRIPTION:    	This function Handle the GET MAX LUN Mass Storage class
				specific request
ARGUMENTS PASSED:	None  
RETURN VALUE:		None	
IMPORTANT NOTES:	None		
=============================================================================*/
static void 
usbs_handle_msc_get_maxlun(void)
{
	usb_buffer_descriptor_t bd ;
    cyg_uint32	buffer_addrs;
    cyg_uint16	desc_length = 0x0;
    cyg_uint8	zlt = 0;//0 means false
	cyg_uint8 Max_Lun=0;
	USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: MASS - Get MAX LUN\n");
    
    /* get the buffer address for data transfer over EP0 */
    buffer_addrs = g_bulkbuffer_map.ep0_buffer_addrs;	//256bytes before the two Bulk buffers

    /* Get the length of descriptor requested */
    desc_length = g_usb_setup_data[WLENGTH_LOWBYTE];
    desc_length |= ( g_usb_setup_data[WLENGTH_HIGHBYTE] <<0x8);

    /* If requested length of descriptor is zero*/
	if(desc_length==0)
	{
		/* Fill the buffer with the descriptor data */
   		*(cyg_uint8 *)buffer_addrs = 0;//Max_Lun;
   		bd.size = 0; 
	}
	else
	{
		/* Fill the buffer with the descriptor data */
   		*(cyg_uint8 *)buffer_addrs = Max_Lun;
		bd.size = desc_length;
	}
	
    /* Send descriptor - Data Phase*/
    usbs_ep0_send_data(&bd,zlt);	//zlt is false=>not zero length packet
    								//send dev descriptor to host.
	
    /* Status Phase -- OUT */
    usbs_status_phase(CONTROL,OUT); //Get Zero-length data packet from Host, Device sends status: ACK(success), NAK(busy), or STALL(failed)

		
}
/*=============================================================================
FUNCTION: 		usbs_ep0in_fill_buffer
DESCRIPTION: 	This function is used to fill the corresponding 
				response for the data phase of SETUP Transfer
ARGUMENTS PASSED:
				cyg_uint8 type: type of descriptor
				cyg_uint32 buffer_addrs - buffer pointer to be filled	 
RETURN VALUE:	None
IMPORTANT NOTES:None
=============================================================================*/
static void 
usbs_ep0in_fill_buffer(cyg_uint8 type, cyg_uint32 buffer_addrs)
{
    const cyg_uint8 *data=0;
    cyg_uint32 *buffer_page = (cyg_uint32*)buffer_addrs;    
    int k = 0;
	//USBDBGMSG("+USBDBGMSG: enum - copy descriptor to buffer\n");
    switch (type)
    {
        case FILL_DEVICE_DESC:	/*5*32 bit */
	    	data = (cyg_uint8 *)g_usb_desc.device_desc;
    	    break;
		case FILL_CONF_DESC:		/*8*32 bit */
		    data = (cyg_uint8 *)g_usb_desc.config_desc;
	    	break;	
		case FILL_STR_DES0:		/*1*32 bit */
	    	data = (cyg_uint8 *)g_usb_desc.str_desc0;
	    	break;	
		case FILL_STR_DES1:		/*7*32 bit */
		    data =(cyg_uint8 *)g_usb_desc.str_desc1; 
	    	break;	
		case FILL_STR_DES2:		/*7*32 bit */
			data = (cyg_uint8 *)g_usb_desc.str_desc2;
		    break;	
		case FILL_STR_DES3:		/*6*32 bit */
		    data = (cyg_uint8 *)g_usb_desc.str_desc3;
	    	break;
		#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
		case FILL_SN_DESC:
			data = (cyg_uint8 *)g_usb_desc.sn_desc;
			break;
		#endif
    }
    
    for (k=0; k<(MPS_64/sizeof(cyg_uint32)); k++)
    {
        *buffer_page = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24); 
				 //USBDBGMSG("+USBDBGMSG: desc[k] = 0x%x\n",(*buffer_page));
         buffer_page++;
				 data += 4;
				
    }

		
}

/*=============================================================================
FUNCTION: 			usbs_ep0_init_dqh
DESCRIPTION: 		This function is used to initialize the queue header of EP0
ARGUMENTS PASSED:	NONE				 
RETURN VALUE:		NONE
IMPORTANT NOTES:	called by usbs_imx_otg_dev_ep0_init(),usbs_imx_otg_dev_handle_bus_reset()
=============================================================================*/
static void
usbs_ep0_init_dqh(void)
{
	struct dqh_t qhead;
	cyg_uint32 total_bytes;
	volatile cyg_uint32 * ep_q_hdr_base;
	cyg_int8 i;
	
	//clear queue header
	ep_q_hdr_base = ((volatile cyg_uint32 *)g_bulkbuffer_map.ep_dqh_base_addrs);
	/* Clear the dQH Memory */
    for ( i = 0; i < (SIZE_OF_QHD*g_max_ep_supported*2)/sizeof(cyg_uint32) ; i++)
    {
        *ep_q_hdr_base++ = 0;
    }
    
     /******************************************************************************
    / =================
    / dQH0 for EP0OUT
    / =================
    / Initialize device queue heads in system memory
    / 8 bytes for the 1st setup packet */
       
    total_bytes					= 0x8;
    qhead.dqh_base 				= USBS_EP_GET_dQH(EP0,OUT);
    qhead.zlt 					= ZLT_DISABLE;
    qhead.mps 					= MPS_64;
    qhead.ios 					= IOS_SET;
    qhead.next_link_ptr = USBS_EP_GET_dTD(EP0,OUT);
    qhead.terminate 		= NOT_TERMINATE;
    qhead.total_bytes  	= total_bytes;
    qhead.ioc 					= IOC_SET;
    qhead.status 				= NO_STATUS;
    qhead.buffer_ptr0  	= 0;
    qhead.current_offset= 0;
    qhead.buffer_ptr1  	= 0;
    qhead.buffer_ptr2 	= 0;
    qhead.buffer_ptr3  	= 0;
    qhead.buffer_ptr4 	= 0;
    /* Set Device Queue Head */
    usbs_setup_queuehead(&qhead);

    /* ==================
    END of dQH0 setup
    ====================*/
     /*=================
    dQH1 for EP0IN
    ================= */

    total_bytes 		= 0x8;
    qhead.dqh_base 		= USBS_EP_GET_dQH(EP0,IN);
    qhead.zlt 			= ZLT_DISABLE;
    qhead.mps 			= MPS_64;
    qhead.ios 			= IOS_SET;
    qhead.next_link_ptr = USBS_EP_GET_dTD(EP0,IN);
    qhead.terminate 	= TERMINATE;
    qhead.total_bytes  	= total_bytes;
    qhead.ioc 			= IOC_SET;
    qhead.status 		= NO_STATUS;
    qhead.buffer_ptr0  	= g_bulkbuffer_map.ep0_buffer_addrs;
    qhead.current_offset= (g_bulkbuffer_map.ep0_buffer_addrs & 0xFFF);
    qhead.buffer_ptr1  	= 0;
    qhead.buffer_ptr2 	= 0;
    qhead.buffer_ptr3  	= 0;
    qhead.buffer_ptr4 	= 0;

    /* Set Device Queue Head */
    usbs_setup_queuehead(&qhead);

    /* ==================
    /  END of dQH1 setup
    /  ================*/
}
/*=============================================================================
FUNCTION: 			usbs_ep0_send_data 
DESCRIPTION:    	This function Send Data to host through EP0-IN Pipe.
ARGUMENTS PASSED:
	usb_buffer_descriptor_t *bd : This is the pointer to the buffer descriptor. 
	cyg_uint8 zlt				: Flag to decide if Zero Length Packet to be sent
RETURN VALUE:
	USB_SUCCESS - The buffer was successfully processed by the USB device and 
					data sent to the Host.
	USB_FAILURE - Some failure occurred in transmitting the data.
IMPORTANT NOTES:	None		
=============================================================================*/
static usb_status_t 
usbs_ep0_send_data(usb_buffer_descriptor_t* bd,cyg_uint8 zlt)
{
	struct dtd_t td;
    cyg_uint32 total_bytes ;
    cyg_uint32 dtd_address,dqh_address;
    
    usb_status_t status = USB_FAILURE;

	/* Get Device Transfer Descriptor of the requested endpoint */
	dtd_address = USBS_EP_GET_dTD(EP0,IN);
    
	/* Get Device Queue head of the requested endpoint */
	dqh_address = USBS_EP_GET_dQH(EP0,IN);

	/* Get Total Bytes to Be recieved */
	total_bytes = bd->size;

	/* Setup Transfer Descriptor for EP0 IN*/
	td.dtd_base 		= dtd_address; 
	td.next_link_ptr  	= 0;
	td.terminate 		= TERMINATE;
	td.total_bytes  	= total_bytes;
	td.ioc				= IOC_SET;
	td.status			= ACTIVE;
	td.buffer_ptr0  	= g_bulkbuffer_map.ep0_buffer_addrs;
	td.current_offset 	= (g_bulkbuffer_map.ep0_buffer_addrs & 0xFFF);
	td.buffer_ptr1  	= 0;
	td.buffer_ptr2 		= 0;
	td.buffer_ptr3  	= 0;
	td.buffer_ptr4  	= 0;
	       
	/* Set the transfer descriptor */	
	usbs_setup_transdesc(&td);
	   
	/* Enable ZLT when data size is in multiple of Maximum Packet Size  */
	if(zlt)
	{
		/* set ZLT enable */
		(*(volatile cyg_uint32*)(dqh_address)) &= ~0x20000000;
	}
	    
	/* 1. write dQH next ptr and dQH terminate bit to 0  */
	*(volatile cyg_uint32*)(dqh_address+0x8)= (dtd_address); 
    	    
	/* 2. clear active & halt bit in dQH */
	*(volatile cyg_uint32*)(dqh_address+0xC) &= ~0xFF;
    	   
	/* 3. prime endpoint by writing '1' in ENDPTPRIME */
	usbs_imx_otg_base->endptprime |= BIT16;
    
	/* wait for complete set and clear */
	while (!(usbs_imx_otg_base->endptcomplete & EPIN_COMPLETE));
	
	usbs_imx_otg_base->endptcomplete = EPIN_COMPLETE;
	
	status = USB_SUCCESS;

	return status;
}
/*=============================================================================
FUNCTION: usbs_ep0_recevie_data
DESCRIPTION: 	This function Handle the Status Token (IN/OUT) from USB Host
ARGUMENTS PASSED:
	usb_buffer_descriptor_t *bd : This is the pointer to the buffer descriptor. 
RETURN VALUE:
	USB_SUCCESS - 	: The buffer was successfully processed by the USB device and 
			  				data is received from the host.
	USB_FAILURE - 	: Some failure occurred in receiving the data.
	USB_INVALID -   : If the endpoint is invalid.
IMPORTANT NOTES:None		
=============================================================================*/
static usb_status_t usbs_ep0_receive_data(usb_buffer_descriptor_t* bd)
{
		struct dtd_t td;
    usb_status_t status = USB_FAILURE;
    cyg_uint32 total_bytes;
    cyg_uint32 dtd_address;
    cyg_uint32 dqh_address;

		/* Get Device Device Queue Head of the requested endpoint */
    dqh_address = USBS_EP_GET_dQH(EP0, OUT);
        
		/* Get Device Transfer Descriptor of the requested endpoint */
    dtd_address = USBS_EP_GET_dTD(EP0, OUT);

		/* Get the total bytes to be received	*/
		total_bytes 		= bd->size; 
		
		td.dtd_base			= dtd_address;
		td.next_link_ptr	= dtd_address + 0x20;
		td.terminate 		= TERMINATE;
		td.total_bytes		= total_bytes;
		td.ioc				= IOC_SET;
		td.status			= ACTIVE;
		td.buffer_ptr0		= g_bulkbuffer_map.ep0_buffer_addrs;
		td.current_offset	= (g_bulkbuffer_map.ep0_buffer_addrs & 0xFFF);
		td.buffer_ptr1		= 0;
		td.buffer_ptr2		= 0;
		td.buffer_ptr3		= 0;
		td.buffer_ptr4		= 0;
		
	/* Set the Transfer Descriptor	*/
	usbs_setup_transdesc(&td);

	/* 1. write dQH next ptr and dQH terminate bit to 0 */
	*(volatile cyg_uint32*)(dqh_address+0x8)= dtd_address;
	    
	/* 2. clear active & halt bit in dQH */
	*(volatile cyg_uint32*)(dqh_address+0xC) &= ~0xFF;
	    
	/* 3. prime endpoint by writing '1' in ENDPTPRIME */
	usbs_imx_otg_base->endptprime |= (  EPOUT_PRIME << EP0 );

	/* 4. Wait for the Complete Status */
	while (!((usbs_imx_otg_base->endptprime) & ( EPOUT_COMPLETE << EP0)));
	    
	/*clear the complete status */
	usbs_imx_otg_base->endptprime = (EPOUT_COMPLETE << EP0);
   
	status = USB_SUCCESS;

	return status;
}
// ****************************************************************************
// -----------------------Endpoint 0 Functions---------------------------------
// ****************************************************************************
/*=============================================================================
// This is where all the hard work happens. It is a very large routine
// for a DSR, but in practice nearly all of it is nested if's and very
// little code actually gets executed. Note that there may be
// invocations of callback functions and the driver has no control
// over how much time those will take, but those callbacks should be
// simple.
// so far, ep0 DSR works only during enumeration here.
=============================================================================*/
static void
usbs_imx_otg_dev_ep0_dsr(void)
{
	usb_buffer_descriptor_t bd ;
	usb_status_t status = USB_FAILURE;
	volatile struct dqh_setup_t * dqh_word ;
	cyg_uint32 dqh_address;
	cyg_uint32 temp;
	
	//USBDBGMSG("+USBDBGMSG: enter ep0 dsr.\n");
	/* 1. Receive Setup Data*/
	bd.buffer = (cyg_uint32 *)g_usb_setup_data;
	bd.size   = 0;
	
	/* Get the Device Queue Head Address for EP0 OUT   */ 
	dqh_address = USBS_EP_GET_dQH(EP0,OUT);
	dqh_word = (volatile struct dqh_setup_t*)dqh_address;
  
	/* write '1' to clear corresponding bit in ENDPTSETUPSTAT */
	temp = usbs_imx_otg_base->endptsetupstat;
	usbs_imx_otg_base->endptsetupstat = temp;	    

//	if(usbs_imx_otg_base->endptsetupstat & BIT0)
//		usbs_imx_otg_base->endptsetupstat = BIT0;	    
	
	do{
	    /* write '1' to Setup Tripwire (SUTW) in USBCMD register */
	    usbs_imx_otg_base->usbcmd |= BIT13;
		
	    /* Copy the SetupBuffer into local software byte array */
	    temp  = (dqh_word->dqh_word10);

	  *((cyg_uint8 *)(bd.buffer)) = (cyg_uint8 )(temp & 0x000000FF);
		(bd.buffer) =(cyg_uint8 *)(bd.buffer) + 1;
		*((cyg_uint8 *)(bd.buffer)) = (cyg_uint8 )((temp & 0x0000FF00)>>8);
		(bd.buffer) =(cyg_uint8 *)(bd.buffer) + 1;
		*((cyg_uint8 *)(bd.buffer)) = (cyg_uint8 )((temp & 0x00FF0000)>>16);
		(bd.buffer) =(cyg_uint8 *)(bd.buffer) + 1;
		*((cyg_uint8 *)(bd.buffer)) = (cyg_uint8 )((temp & 0xFF000000)>>24);
		(bd.buffer) =(cyg_uint8 *)(bd.buffer) + 1;
	  
		temp  = (dqh_word->dqh_word11);
		*((cyg_uint8 *)(bd.buffer)) = (cyg_uint8 )(temp & 0x000000FF);
		(bd.buffer) =(cyg_uint8 *)(bd.buffer) + 1;
		*((cyg_uint8 *)(bd.buffer)) = (cyg_uint8 )((temp & 0x0000FF00)>>8);
		(bd.buffer) =(cyg_uint8 *)(bd.buffer) + 1;
		*((cyg_uint8 *)(bd.buffer)) = (cyg_uint8 )((temp & 0x00FF0000)>>16);
		(bd.buffer) =(cyg_uint8 *)(bd.buffer) + 1;
		*((cyg_uint8 *)(bd.buffer)) = (cyg_uint8 )((temp & 0xFF000000)>>24);
		(bd.buffer) =(cyg_uint8 *)(bd.buffer) + 1;
	}while (!(usbs_imx_otg_base->usbcmd & BIT13));	
	
	/* Write '0' to clear SUTW in USBCMD register */
	usbs_imx_otg_base->usbcmd &= ~BIT13;
	status = USB_SUCCESS;

	#if 0
	USBDBGMSG("+USBDBGMSG: setup packet:(LSB)");
	for(temp=0;temp<8;temp++)
	{
		USBDBGMSG("%02X",g_usb_setup_data[temp]);
	}
	USBDBGMSG("(MSB)\n");
	#endif
	
	/* 2. Process Setup Data*/
	/* switch construct to handle different request*/
	/* Parser the Setup Request Type */
	switch (g_usb_setup_data[BREQUEST])             
	{ 
		case USB_GET_DESCRIPTOR:
			/* Handle the GET DESCRIPTOR Request */
			usbs_handle_get_descriptor();
	    break;
	
		case USB_SET_ADDRESS:
			/* Handle the SET ADDRESS Request */
			usbs_handle_set_address();
			break;
	    
		case USB_SET_CONFIGURATION:
			/* Handle the SET CONFIGURATION Request */
			if ((g_usb_setup_data[WINDEX_LOWBYTE] == 0)	&&
				(g_usb_setup_data[WINDEX_HIGHBYTE] == 0)&&
				(g_usb_setup_data[WLENGTH_LOWBYTE] == 0)&&
				(g_usb_setup_data[WLENGTH_HIGHBYTE] == 0)&&
				(g_usb_setup_data[WVALUE_HIGHBYTE] == 0)) 
			{
				usbs_handle_set_configuration();
			}
			else
			{
				/* Send STALL Handshake   */
				usbs_endpoint_stall(EP0,IN);
				//USBDBGMSG("+USBDBGMSG:EP0 IN stalled at set conf in ep0 dsr\n");
	    	}
	    	break;
	    
		case USB_GET_CONFIGURATION:
			/* GET CONFIGURATION request handler */
			usbs_handle_get_configuration();
			break;
		case USB_MSC_GET_MAX_LUN:
			usbs_handle_msc_get_maxlun();
			break;
		case USB_MSC_BOT_RESET:
			
	  	default:
			/* Send STALL Handshake   */
	    	usbs_endpoint_stall(EP0,IN);			
			USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG:EP0 IN stalled in ep0 dsr\n");
			USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG:Setup Request Type 0x%02x,0x%02X\n",g_usb_setup_data[BMREQUESTTYPE],g_usb_setup_data[BREQUEST]);
	    	break;
	}
	
	USBDBGMSG(DEBUG_ENUM,"+USBDBGMSG: ep0 dsr\n");
}
/*=============================================================================
// Endpoint 0 initialization.
// Control Endpoint, bi-direction
// This may get called during system start-up or following a reset
// from the host.
=============================================================================*/
static void
usbs_imx_otg_dev_ep0_init(void)
{
	/*initialize Endpoint 0 Queue Header*/
	usbs_ep0_init_dqh();

	{
		/*fill the structure for ep0*/
		if ((EP0_STATE_IDLE != ep0.ep_state) &&
       ((usbs_control_return (*)(usbs_control_endpoint*, int)) 0 != ep0.common.complete_fn)) 
    {
		#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
	   (*ep0.common.complete_fn)(&ep0.common, -EPIPE);
		#endif
    }
    ep0.common.state            = USBS_STATE_POWERED;
    memset(ep0.common.control_buffer, 0, 8);
    ep0.common.buffer           = (unsigned char*) 0;
    ep0.common.buffer_size      = 0;
    ep0.common.fill_buffer_fn   = (void (*)(usbs_control_endpoint*)) 0;
    ep0.common.fill_data        = (void*) 0;
    ep0.common.fill_index       = 0;
    ep0.common.complete_fn      = (usbs_control_return (*)(usbs_control_endpoint*, int)) 0;
    ep0.ep_state                = EP0_STATE_IDLE;
    ep0.length                  = 0;
    ep0.transmitted             = 0;
	}
}
// ----------------------------------------------------------------------------
/*=============================================================================
// The start function is called by higher-level code when things have
// been set up, i.e. the enumeration data is available, appropriate
// handlers have been installed for the different types of control
// messages, and communication with the host is allowed to start. The
// next event that should happen is a reset operation from the host,
// so all other interrupts should be blocked. However it is likely
// that the hardware will detect a suspend state before the reset
// arrives, and hence the reset will act as a resume as well as a
// reset.
=============================================================================*/
static void
usbs_imx_otg_dev_ep0_start(usbs_control_endpoint* endpoint)
{
	cyg_uint32 temp;
	
	CYG_ASSERT( endpoint == &ep0.common, "USB startup involves the wrong endpoint");
	
	/*clear all interrupt status bits*/
	temp = usbs_imx_otg_base->usbsts;
	usbs_imx_otg_base->usbsts = temp;		//clear all the previous interrupts
	
	/*enable all the sub-interrupt sources for USB device*/
	USBS_IMX_OTG_INTR_UNMASK(IMX_USB_INTR_DEV_PCE|IMX_USB_INTR_DEV_RESET|IMX_USB_INTR_DEV_USBINT);
	
	/*set Run/Stop bit to Run Mode*/
	usbs_imx_otg_base->usbcmd |= BIT0;	

	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: mx37 ep0 start.\n");
}
// ****************************************************************************
// -----------------------Endpoint 1 Functions---------------------------------
// ****************************************************************************
/*=============================================================================
// Complete a transfer. This takes care of invoking the completion
// callback and resetting the buffer.
=============================================================================*/
static void
ep1_rx_complete(int result)
{
	//cyg_uint32 total_bytes;
	cyg_uint32 dtd_address;
	cyg_uint32 dqh_address;
	cyg_uint32 received_buffer_addrs = 0x0;
	cyg_uint32 received_data_length = 0x0;
	cyg_uint32* temp = 0x0;    

	int i;
	
	if(g_usb_dev_state != USB_DEV_CONFIGURED_STATE) 
		return; //EP1 only receives data when the USB device has been configured
	
	if(ep1.common.buffer == NULL)
	{
		USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep1_rx_complete: NULL buffer \n");
		return;	//there is not a buffer used to store the data from host
	}	
	
	/* Get Device Device Queue Head of the out endpoint */
	dqh_address = USBS_EP_GET_dQH(EP1,OUT);
        
	/* Get Device Transfer Descriptor of the out endpoint */
	dtd_address = USBS_EP_GET_dTD(EP1,OUT);
	
	/*clear the complete status */
	usbs_imx_otg_base->endptcomplete |= (EPOUT_COMPLETE << EP1);

	//received_buffer_addrs = (*((unsigned int *)dtd_address + 2)) & 0xFFFFF000;
	received_buffer_addrs = (cyg_uint32)ep1.common.buffer;
	USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep1_rx_complete: received_buffer_addrs 0x%08X \n",received_buffer_addrs);
	if( received_buffer_addrs == 0)
	{
		USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep1_rx_complete: NULL rx buffer \n");
		return;
	}
		
	/* calculate the received data length using number of bytes left in TD */
	temp =  (cyg_uint32 *)dtd_address;
	temp++; 		//pointer to total bytes in dtd, second work in dTD
	received_data_length = (ep1.common.buffer_size - (((*temp) >> 16 )&0x7FFF));	//recevied data length <= BULK_TD_BUFFER_TOTAL_SIZE
	USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep1_rx_complete: received length %d \n",received_data_length);
	#if 0
	/* Check if the received packet is SCSI WRITE, if yes, assign the TD buffer offset
	   is zero, otherwise, one. This is a bug in MX37 USB OTG */
	if((received_data_length==31)&&(*(destination_ptr+0xF)==0x2A))//WRITE10 received
	{
		g_bulk_out_transdesc_buffer_offset = 0;
		USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep1_start_rx - set offset to zero \n");
	}

	else if((g_bulk_out_sector_number_is_one == 1)&&(received_data_length!=31)) //last bulk out data sector
		g_bulk_out_transdesc_buffer_offset = 1;
	#endif
	
	/* tell ep1 how many bytes data is received*/
	ep1.fetched	= received_data_length;	
	
	if(ep1.fetched) 
	{
		if(ep1.fetched == 31)
			g_received_data_type = MASS_STORAGE_CBW_TYPE;
		else 
			g_received_data_type = MASS_STORAGE_DATA_TYPE;
		ep1.common.complete_data    = (void*)(ep1.common.buffer);

		#if 0
		USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep1_rx_complete: \n");
		USBDBGMSG(DEBUG_TRANS,"----Dump Bulk-Out Recevied Data----\n");
		for(i=0;i<32;i++)
		{
			USBDBGMSG(DEBUG_TRANS,"%02X ", *((cyg_uint8 *)(ep1.common.complete_data)+i));
		}
		USBDBGMSG(DEBUG_TRANS,"\n");
		#endif
		//USB_IMX_SET_TD_OFFSET(g_td_buffer_offset, 0);
	    ep1.common.buffer      = (unsigned char*) 0;
    	ep1.common.buffer_size = 0;
		ep1_start_rx((usbs_rx_endpoint *)(&(ep1.common))); //prevent to receive more CBW before processing done
	}
	
    #if 0
	if(ep1.fetched == 31)
	{
		USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep1_rx_complete - recevied data: \n");
		for(i=0;i<32;i++)
		{
			USBDBGMSG(DEBUG_TRANS,"%02X ", *((cyg_uint8 *)(ep1.common.complete_data)+i));
		}
		USBDBGMSG(DEBUG_TRANS,"\n");

		for(i=0;i<32;i++)
		{
			USBDBGMSG(DEBUG_TRANS,"%02X ", *((cyg_uint8 *)received_buffer_addrs+i));
		}
		USBDBGMSG(DEBUG_TRANS,"\n");
	}
	#endif
	
	
}
/*=============================================================================
// Start to receive data from host. This functionality is overloaded to cope with
// waiting for stalls to complete.
// The transfer descriptor is prepared 
=============================================================================*/
static void
ep1_start_rx(usbs_rx_endpoint* endpoint)
{
	struct dtd_t td;
	cyg_uint32 total_bytes;
	cyg_uint32 dtd_address;
	cyg_uint32 dqh_address;
    cyg_uint32 buffer_addrs_page0;

	if(g_usb_dev_state != USB_DEV_CONFIGURED_STATE) 
		return; //EP1 only receives data when the USB device has been configured
	#if 0	//don't check to prevent EP1 from receiving data before processing the previous.
	if(endpoint->buffer == NULL)
	{
		USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep1_start_rx: NULL buffer \n");
		return;	//there is not a buffer used to store the data from host
	}
	#endif
	/* Get Device Device Queue Head of the out endpoint */
	dqh_address = USBS_EP_GET_dQH(EP1,OUT);
        
	/* Get Device Transfer Descriptor of the out endpoint */
	dtd_address = USBS_EP_GET_dTD(EP1,OUT);

	/* ==Prepare TD for next bulk out transfer== */
	/* get the dTD buffer pointer */
	buffer_addrs_page0 = (cyg_uint32)(endpoint->buffer);
	
	/* Get the total bytes to be received	*/
	total_bytes = endpoint->buffer_size;
	
	/* OUT setup dTD */
	td.dtd_base 		= dtd_address;  
	td.next_link_ptr  	= dtd_address + 0x20;
	td.terminate 		= TERMINATE;
	td.total_bytes  	= total_bytes;
	td.ioc				= IOC_SET;
	td.status			= ACTIVE;
	td.buffer_ptr0		= buffer_addrs_page0 ; 
	td.current_offset	= ( buffer_addrs_page0 & 0xFFF ) + g_td_buffer_offset; 
	td.buffer_ptr1  	= 0;
	td.buffer_ptr2  	= 0;
	td.buffer_ptr3  	= 0;
	td.buffer_ptr4  	= 0;

	/* re-define the buffer page pointers based on the total_bytes*/
	if(total_bytes > BULK_TD_BUFFER_PAGE_SIZE)
		td.buffer_ptr1  	= (td.buffer_ptr0 + BULK_TD_BUFFER_PAGE_SIZE);
	if(total_bytes > BULK_TD_BUFFER_PAGE_SIZE*2)
		td.buffer_ptr2 		= (td.buffer_ptr1 + BULK_TD_BUFFER_PAGE_SIZE);
	if(total_bytes > BULK_TD_BUFFER_PAGE_SIZE*3)
		td.buffer_ptr3  	= (td.buffer_ptr2 + BULK_TD_BUFFER_PAGE_SIZE);
            
	/* Set the Transfer Descriptor	*/
	usbs_setup_transdesc(&td);

	/* 1. write dQH next ptr and dQH terminate bit to 0 */
	*(volatile cyg_uint32 *)(dqh_address+0x8)= dtd_address;
	    
	/* 2. clear active & halt bit in dQH */
	*(volatile cyg_uint32 *)(dqh_address+0xC) &= ~0xFF;

	/* 3. prime endpoint by writing '1' in ENDPTPRIME 
		prime bulk out endpoint after sending the CSW of last command
	*/
	//usbs_imx_otg_base->endptprime |= ( EPOUT_PRIME << EP1 );

}
/*=============================================================================
// The exported interface to halt the EP1
=============================================================================*/
static void
ep1_set_halted(usbs_rx_endpoint* endpoint, cyg_bool new_value)
{
	if (ep1.common.halted == new_value) {
        return;
    }
    if (new_value) {
        // The endpoint should be stalled. There is a potential race
        // condition here with the current transfer and DSR invocation.
        // Updating the stalled flag means that the DSR will do nothing.
		usbs_endpoint_stall(EP1,OUT);
		ep1.common.halted = 1;
    } 
	else {
        // Take care of the hardware so that a new transfer is allowed. 
        usbs_endpoint_unstall(EP1,OUT);
        ep1.common.halted = 0;
    }

}
/*=============================================================================
// The DSR is invoked following an interrupt. According to the docs an
// endpoint 1 interrupt can only happen if the receive-packet-complete
// bit is set.
// [Note] EP1 DSR is only used to receive the command block wrapper from host
// to USB mass storage device
=============================================================================*/

static void
usbs_imx_otg_dev_ep1_dsr(void)
{
	int result = 0;	//contains the actual recevied data length from bulk-out endpoint
	g_received_data_type = 0;//MASS_STORAGE_CBW_TYPE
	
	if(ep1.common.buffer)//buffer of TD is not null, then receive
	{
		ep1_rx_complete(result);
	}
	//USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep1 dsr - result = %d\n",result);
	//recevie mass storage device CBW
	
	if((ep1.fetched == 31)&&(g_received_data_type == MASS_STORAGE_CBW_TYPE))
	{
		USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep1 dsr - CBW received\n");
		//post the semaphore of MSC command handler thread
		#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
		cyg_semaphore_post(&usbs_msc_sem);
		#endif
		ep1.fetched = 0;
	}
	else
		USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep1 dsr - received %d byte\n",ep1.fetched);
	
}

/*=============================================================================
// Endpoint 1 initialization.
// Bulk-OUT Endpoint
// This may get called during system start-up or following a reset
// from the host.
=============================================================================*/
static void
usbs_imx_otg_dev_ep1_init(void)
{
	//at present, ep1.common.buffer is NULL. The buffer should be initialized 
	//by upper layer caller.
	/*buffer is assigned in MSC initialization*/
	
	// Endpoints should never be halted during a start-up.
    ep1.common.halted      = 0;	//false =0, true =1
	ep1.common.complete_fn = ep1_rx_complete;
    // If there has been a reset and there was a receive in progress,
    // abort it. This also takes care of sorting out the endpoint
    // fields ready for the next rx.
    ep1_rx_complete(-EPIPE);
}

// ****************************************************************************
// -----------------------Endpoint 2 Functions---------------------------------
// ****************************************************************************
/*=============================================================================
// A utility routine for completing a transfer. This takes care of the
// callback as well as resetting the buffer.
=============================================================================*/
static void
ep2_tx_complete(int result)
{
    void (*complete_fn)(void*, int)  = ep2.common.complete_fn;
    void* complete_data = ep2.common.complete_data;
    
    ep2.common.buffer           = (unsigned char*) 0;
    ep2.common.buffer_size      = 0;
    ep2.common.complete_fn      = (void (*)(void*, int)) 0;
    ep2.common.complete_data    = (void*) 0;

    if ((void (*)(void*, int))0 != complete_fn) {
        (*complete_fn)(complete_data, result);
    }
}

/*=============================================================================
// The exported interface to start to transmit data to Host
=============================================================================*/
static void
ep2_start_tx(usbs_tx_endpoint* endpoint)
{
	int timeout = 400;
	struct dtd_t td;
    cyg_uint32 total_bytes ;
    cyg_uint32 dtd_address,dqh_address;
    cyg_uint32 buffer_addrs_page0;
    cyg_uint32 size = 0x0;

	/* Get Device Transfer Descriptor of the requested endpoint */
	dtd_address = USBS_EP_GET_dTD(EP2,IN);
    
	/* Get Device Queue head of the requested endpoint */
	dqh_address = USBS_EP_GET_dQH(EP2,IN);	
	
	/* allocate memory for data transfer */
	buffer_addrs_page0 = endpoint->buffer;

	if(buffer_addrs_page0 == 0)
	{
		USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep2_start_tx: NULL tx buffer \n");
		return;
	}

	total_bytes = (cyg_uint32)(endpoint->buffer_size);
	size = (total_bytes < BULK_TD_BUFFER_TOTAL_SIZE )?total_bytes:(BULK_TD_BUFFER_TOTAL_SIZE);
	    
	td.dtd_base 		= dtd_address; 
	td.next_link_ptr  	= dtd_address + 0x20 ;
	td.terminate 		= TERMINATE;
	td.total_bytes  	= size;
	td.ioc 				= IOC_SET;
	td.status 			= ACTIVE;
	td.buffer_ptr0  	= buffer_addrs_page0 ;
	td.current_offset 	= (buffer_addrs_page0 & 0xFFF)+ g_td_buffer_offset; 
	td.buffer_ptr1  	= 0;
	td.buffer_ptr2  	= 0;
	td.buffer_ptr3  	= 0;
	td.buffer_ptr4  	= 0;

	/* re-define the buffer page pointers based on the total_bytes*/
	if(size > BULK_TD_BUFFER_PAGE_SIZE)
		td.buffer_ptr1  	= (td.buffer_ptr0 + BULK_TD_BUFFER_PAGE_SIZE);
	if(size > BULK_TD_BUFFER_PAGE_SIZE*2)
		td.buffer_ptr2 		= (td.buffer_ptr1 + BULK_TD_BUFFER_PAGE_SIZE);
	if(size > BULK_TD_BUFFER_PAGE_SIZE*3)
		td.buffer_ptr3  	= (td.buffer_ptr2 + BULK_TD_BUFFER_PAGE_SIZE);

	/* Set the Transfer Descriptor  */ 
	usbs_setup_transdesc(&td);

	/* 1. write dQH next ptr and dQH terminate bit to 0  */
	*(volatile cyg_uint32 *)(dqh_address+0x8)= (dtd_address); 
	
	/* 2. clear active & halt bit in dQH */
	*(volatile cyg_uint32 *)(dqh_address+0xC) &= ~0xFF;

	/* 3. prime endpoint by writing '1' in ENDPTPRIME */
	usbs_imx_otg_base->endptprime = ( EPIN_PRIME << EP2 );
	
	/* wait for complete set and clear */
	while (!((usbs_imx_otg_base->endptcomplete) & (EPIN_COMPLETE<<EP2)));
	
	usbs_imx_otg_base->endptcomplete |= (EPIN_COMPLETE << EP2);

	ep2.transmitted = size;

	ep2_tx_complete(-EPIPE);
	
	USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep2 tx done\n");// ep2.transmitted);
	
}
/*=============================================================================
// The exported interface to halt the EP2
=============================================================================*/
static void
ep2_set_halted(usbs_tx_endpoint* endpoint, cyg_bool new_value)
{
	if (ep2.common.halted == new_value) {
        return;
    }
    if (new_value) {
        // The endpoint should be stalled. There is a potential race
        // condition here with the current transfer and DSR invocation.
        // Updating the stalled flag means that the DSR will do nothing.
		usbs_endpoint_stall(EP2,IN);
		ep2.common.halted = 1;
    } 
	else {
        // Take care of the hardware so that a new transfer is allowed. 
        usbs_endpoint_unstall(EP2,IN);
        ep2.common.halted = 0;
    }
}
/*=============================================================================
// The dsr will be invoked when the transmit-packet-complete bit is
// set. Typically this happens when a packet has been completed
=============================================================================*/

static void
usbs_imx_otg_dev_ep2_dsr(void)
{
	USBDBGMSG(DEBUG_TRANS,"+USBDBGMSG: ep2 dsr\n");
	/* EP2 DSR will be called as soon as a transfer complete to clear status*/
	usbs_imx_otg_base->endptcomplete |= (EPIN_COMPLETE << EP2);

	
	if(ep2.common.buffer_size==0)
	{
		ep2_tx_complete(-EPIPE);
		ep2.transmitted = 0;	//clear the field to wait for the next transmit
	}
	
}

/*=============================================================================
// Endpoint 2 initialization.
// Bulk-IN Endpoint
// This may be called during system start-up or following a reset
// from the host.
=============================================================================*/
static void
usbs_imx_otg_dev_ep2_init(void)
{
	//at initialization, ep2.common.buffer is NULL. The buffer should be initialized 
	//by upper layer caller.

	// Endpoints should never be halted after a reset
    ep2.common.halted   = false;

    // If there has been a reset and there was a receive in progress,
    // abort it. This also takes care of clearing the endpoint
    // structure fields.
    ep2_tx_complete(-EPIPE);
}

// ****************************************************************************
// -----------------------MX37 USB Device Driver API Functions-----------------
// ****************************************************************************

/*=============================================================================
// The DSR. This can be invoked directly by poll(), or via the usual
// interrupt subsystem. It acts as per the current value of
// g_isr_status_bits. If another interrupt goes off while this
// DSR is running, there will be another invocation of the DSR and
// the status bits will be updated.
=============================================================================*/
static void
usbs_imx_otg_dev_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    int status = 0;
    cyg_uint32 temp;
    CYG_ASSERT(MX51_IRQ_USB_SERVICE_REQUEST == vector, "USB DSR should only be invoked for USB interrupts" );
    CYG_ASSERT(0 == data, "The i.MX37 USB DSR needs no global data pointer");
	//USBDBGMSG("+USBDBGMSG: enter mx37 dsr\n");
    // There is no atomic swap support, so interrupts have to be
    // blocked. It might be possible to do this via the USBS_CONTROL
    // register, but at the risk of messing up the status register
    // if another interrupt comes in. Blocking interrupts at the
    // processor level is less intrusive on the USB code.
    //cyg_drv_isr_lock();
    status = g_isr_status_bits;
    g_isr_status_bits = 0;
    //cyg_drv_isr_unlock();
		
    // Reset is special, since it invalidates everything else.
    // If the reset is still ongoing then do not attempt any
    // further processing, there will just be another interrupt.
    // Otherwise handle_reset() does the hard work. Unmasking
    // the interrupt means that another interrupt will occur
    // immediately if reset is still asserted, i.e. no threads
    // will run, but there is no easy way of triggering action
    // at the end of reset.
    if (status & IMX_USB_STS_RESET) 
    {
        int new_status = usbs_imx_otg_base->usbsts; 
        if (0 == (new_status & IMX_USB_STS_RESET)) 
        {
			usbs_imx_otg_dev_handle_bus_reset();
			USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: !!USB BUS RESET\n");
        } 
		
        // This unmask is likely to cause another interrupt immediately
        #if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
        cyg_interrupt_unmask(MX51_IRQ_USB_SERVICE_REQUEST);
		#endif
    } 
	else if(status & IMX_USB_STS_USBINT)
	{
		
		if(usbs_imx_otg_base->endptsetupstat & BIT0)
      	{// if Setup Packet arrived      		
           	usbs_imx_otg_dev_ep0_dsr();
       	}

		else if((usbs_imx_otg_base->endptcomplete) & ( EPIN_COMPLETE << EP2))	
        {//	EP2 Queue Header buffer completes sending data
        	//complete bit is cleared in ep2_start_tx
        }
		
		
		else if((usbs_imx_otg_base->endptcomplete) & ( EPOUT_COMPLETE << EP1))
        {// EP1 Queue Header buffer get data        	
 			usbs_imx_otg_dev_ep1_dsr();
			
        }

		else if((usbs_imx_otg_base->endptcomplete) & ( EPOUT_COMPLETE << EP0))
		{
			//usbs_imx_otg_dev_ep0_dsr();
			usbs_imx_otg_base->endptcomplete = ( EPOUT_COMPLETE << EP0);
		}
        else
        {//do nothing, only for constructure integrity
			temp = usbs_imx_otg_base->endptcomplete;
			usbs_imx_otg_base->endptcomplete = temp;
			USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: usbsts int - unknown.\n");
		}

		#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
		// This unmask is likely to cause another interrupt immediately
		cyg_interrupt_unmask(MX51_IRQ_USB_SERVICE_REQUEST);
		#endif
	}
    else 
    {
    	#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
		// This unmask is likely to cause another interrupt immediately
       	cyg_interrupt_unmask(MX51_IRQ_USB_SERVICE_REQUEST);
		#endif
    }	

		
}
/*=============================================================================
// The DSR thread
=============================================================================*/
#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
#define CYGNUM_DEVS_USB_OTG_DEV_THREAD_STACK_SIZE	1024
#define CYGNUM_DEVS_USB_OTG_DEV_THREAD_PRIORITY	29
static unsigned char usbs_imx_otg_dev_thread_stack[CYGNUM_DEVS_USB_OTG_DEV_THREAD_STACK_SIZE];
static cyg_thread    usbs_imx_otg_dev_thread;
static cyg_handle_t  usbs_imx_otg_dev_thread_handle;
static cyg_sem_t     usbs_imx_otg_dev_sem;


static void
usbs_imx_otg_dev_thread_fn(cyg_addrword_t param)
{
	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: usb driver thread\n");
	for (;;) {
        cyg_semaphore_wait(&usbs_imx_otg_dev_sem);
		usbs_imx_otg_dev_dsr(IMX_IRQ_USB_DEV_SERVICE_REQUEST, 0, 0);
    }
    CYG_UNUSED_PARAM(cyg_addrword_t, param);
}

static void
usbs_imx_otg_dev_thread_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    CYG_ASSERT( 0 != isr_status_bits, "DSR's should only be scheduled when there is work to do");
    cyg_semaphore_post(&usbs_imx_otg_dev_sem);

	
    CYG_UNUSED_PARAM(cyg_vector_t, vector);
    CYG_UNUSED_PARAM(cyg_ucount32, count);
    CYG_UNUSED_PARAM(cyg_addrword_t, data);
}
#endif
/*=============================================================================
// The interrupt handler. This does as little as possible.
=============================================================================*/
static cyg_uint32
usbs_imx_otg_dev_isr(cyg_vector_t vector, cyg_addrword_t data)
{
	cyg_uint32 old_status_bits = g_isr_status_bits;
	cyg_uint32 status_bits;

    CYG_ASSERT(IMX_IRQ_USB_DEV_SERVICE_REQUEST == vector, "USB ISR should only be invoked for USB interrupts" );
    CYG_ASSERT(0 == data, "The MX51 USB ISR needs no global data pointer" );

	//USBDBGMSG("+USBDBGMSG: enter mx51 isr\n");
    // Read the current status. Reset is special, it means that the
    // whole chip has been reset apart from the one bit in the status
    // register. Nothing should be done about this until the DSR sets
    // the endpoints back to a consistent state and re-enables
    // interrupts in the control register.
    status_bits         = usbs_imx_otg_base->usbsts;
	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: usb intr 0x%08X\n",status_bits);
    if (status_bits & IMX_USB_STS_RESET) 
    {
    	
        g_isr_status_bits |= IMX_USB_STS_RESET;
        usbs_imx_otg_base->usbsts |= IMX_USB_STS_RESET;
        cyg_interrupt_mask(IMX_IRQ_USB_DEV_SERVICE_REQUEST);
    } 
	else if(status_bits & IMX_USB_STS_USBINT)
	{
		g_isr_status_bits |= IMX_USB_STS_USBINT;
        usbs_imx_otg_base->usbsts |= IMX_USB_STS_USBINT;
        cyg_interrupt_mask(IMX_IRQ_USB_DEV_SERVICE_REQUEST);
	}
    else 
    {
        usbs_imx_otg_base->usbsts  = status_bits;	//clear the status bit of USBSTS
        g_isr_status_bits &= ~status_bits;
        USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: unknown usb intr\n");
	}

    // Now keep the rest of the system happy.
    cyg_interrupt_acknowledge(vector);			//reenable the USB interrupt
    return (old_status_bits != g_isr_status_bits) ? CYG_ISR_CALL_DSR : CYG_ISR_HANDLED;
}
/*=============================================================================
// Polling support. This acts mostly like the interrupt handler: it
// sets the isr status bits and causes the dsr to run. Reset has to be
// handled specially: polling does nothing as long as reset is asserted.
=============================================================================*/
static void
usbs_imx_otg_dev_poll(usbs_control_endpoint* endpoint)
{
	CYG_ASSERT( endpoint == &ep0.common, "USB poll involves the wrong endpoint");
    
    if (g_isr_status_bits & IMX_USB_STS_RESET) 
    {
        // Reset was detected the last time poll() was invoked. If
        // reset is still active, do nothing. Once the reset has
        // completed things can continue.
        if (0 == (IMX_USB_STS_RESET & usbs_imx_otg_base->usbsts)) 
        {
            g_isr_status_bits = 0;
            usbs_imx_otg_dev_handle_bus_reset();
        }
    } 
    else 
    {
        g_isr_status_bits = usbs_imx_otg_base->usbsts;
        if (IMX_USB_STS_PTCHANGE & g_isr_status_bits) 
        {
            //process Port Change Detect
            usbs_imx_otg_dev_handle_port_change();
        } 
        else if (IMX_USB_STS_USBINT & g_isr_status_bits)
        {
            usbs_imx_otg_dev_dsr(IMX_IRQ_USB_DEV_SERVICE_REQUEST, 0, (cyg_addrword_t) 0);
        }
        else
        {
        	usbs_imx_otg_base->usbsts = g_isr_status_bits;	//clear the don't-care status
        }
    }
}
/*=============================================================================
// Perform reset operations on all endpoints that have been
// configured in. It is convenient to keep this in a separate
// routine to allow for polling, where manipulating the
// interrupt controller mask is a bad idea.
=============================================================================*/
static void
usbs_imx_otg_dev_handle_bus_reset(void)
{
	cyg_uint32 temp;
 	
	usbs_imx_otg_base->usbcmd &= ~BIT0; //detach device from bus temprorarily
	usbs_imx_otg_base->usbsts |= BIT6;	//clear reset bit in USBSTS

	//temp = usbs_imx_otg_base->usbsts;
	//usbs_imx_otg_base->usbsts = temp;
  
	/*1. Reading and writing back the ENDPTSETUPSTAT register
      clears the setup token semaphores */
	temp = usbs_imx_otg_base->endptsetupstat;
	usbs_imx_otg_base->endptsetupstat = temp;

	/*2. Reading and writing back the ENDPTCOMPLETE register
      clears the endpoint complete status bits */
	temp = usbs_imx_otg_base->endptcomplete;
	usbs_imx_otg_base->endptcomplete = temp;
	
	/*3. Cancel all primed status by waiting until all bits in ENDPTPRIME are 0
       and then write 0xFFFFFFFF to ENDPTFLUSH */
	while(usbs_imx_otg_base->endptprime); 	
	usbs_imx_otg_base->endptflush = 0xFFFFFFFF;	


	/*4. Initialize EP0 Queue Head again*/
	usbs_ep0_init_dqh();
    	
	usbs_imx_otg_base->endptlistaddr = g_bulkbuffer_map.ep_dqh_base_addrs; 
     
	usbs_imx_otg_base->usbcmd |= BIT0; //re-attach device to the bus

	g_usb_dev_state = USB_DEV_DEFAULT_STATE;

	
}
/*=============================================================================
// Perform port change operations on all endpoints that have been
// configured in. It is convenient to keep this in a separate
// routine to allow for polling, where manipulating the
// interrupt controller mask is a bad idea.
=============================================================================*/
static void
usbs_imx_otg_dev_handle_port_change(void)
{
	/*Port Change happens when USB device enters/exits FS or HS mode
	When exiting from FS or HS due to Bus reset or DCSuspend, the notification
	mechanisms are Reset Received and DCSuspend.
	This function only processes the port change on entering FS or HS
	Don't enable Port Change Detect interrupt, it's no sense for operation.*/
	usbs_imx_otg_base->usbsts |= IMX_USB_STS_PTCHANGE;	//clear Port change status

}

// ****************************************************************************
// ----------------------------------------------------------------------------
// ****************************************************************************
/*=============================================================================
FUNCTION: usbs_imx_otg_dev_set_configuration
DESCRIPTION:		This function Handle the SET CONFIGRATION Request.
ARGUMENTS PASSED:	usb_end_pt_info_t* config_data;
RETURN VALUE:		None	
IMPORTANT NOTES:	None		
=============================================================================*/
static void
usbs_imx_otg_dev_set_configuration(usb_end_pt_info_t* config_data)
{
    struct dtd_t td;
    cyg_uint32 total_bytes = 0x0;
    cyg_uint32 buffer_addrs_page0 = 0;
    cyg_uint32 dqh_address = 0;
    cyg_uint32 dtd_address = 0;
    cyg_uint8  endpt_num,direction;
    
    struct dqh_t qhead;

	
    /* get endpoint number to be configured and its direction */
    endpt_num= config_data->end_pt_no; 
    direction= config_data->direction;
    USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: set config - ep%d\n",endpt_num);
    /* Check if the endpoint number and direction is withing the permitted range or not */
    if (( endpt_num != EP0 ) && (endpt_num <= ( g_max_ep_supported - 1)) && 
		    ( direction == OUT || direction == IN))
    {
		/* get the device q head and deice TD */
		dqh_address = USBS_EP_GET_dQH(endpt_num,direction); 
		dtd_address = USBS_EP_GET_dTD(endpt_num,direction);

		if ( direction ==  OUT )
		{
		total_bytes = BULK_BUFFER_SIZE ;
		
		qhead.dqh_base		= dqh_address;
		qhead.zlt 			= ZLT_DISABLE;
		qhead.mps 			= config_data->max_pkt_size;
		qhead.ios 			= IOS_SET;
		qhead.next_link_ptr	= dtd_address ;
		qhead.terminate		= TERMINATE;
		qhead.total_bytes	= total_bytes;
		qhead.ioc			= IOC_SET;
		qhead.status 		= NO_STATUS;
		qhead.buffer_ptr0	= 0;
		qhead.current_offset= 0;
		qhead.buffer_ptr1	= 0;
		qhead.buffer_ptr2 	= 0;
		qhead.buffer_ptr3	= 0;
		qhead.buffer_ptr4 	= 0;

		usbs_setup_queuehead(&qhead);
		
		/* Endpoint 1 : MPS = 64, OUT (Rx endpoint) */
		usbs_imx_otg_base->endptctrl[endpt_num] = 0x00080048;
		/* Enable EP1 OUT */
		usbs_imx_otg_base->endptctrl[endpt_num] |= EPOUT_ENABLE;
		
		/* allocate buffer for receiving data */
		/* free the usb buffer after re-enumeration*/
		//g_bulkbuffer_map.buffer1_status = BUFFER_FREE;
	   	//g_bulkbuffer_map.buffer2_status = BUFFER_FREE;
	   	g_bulkbuffer_a.stat = BUFFER_FREED;
	   	g_bulkbuffer_b.stat = BUFFER_FREED;	
		
	    //buffer_addrs_page0 = util_alloc_buffer();
	    ep1.common.buffer      = g_bulkbuffer_a.buffer;
		ep1.common.buffer_size = total_bytes;
		g_bulkbuffer_a.stat = BUFFER_ALLOCATED;
		buffer_addrs_page0 = (cyg_uint32)(ep1.common.buffer);
		
		
		USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: set config - ep1 dtd buffer 0x%08X\n",buffer_addrs_page0);
		
		/* OUT setup dTD */
		td.dtd_base			= dtd_address;
		td.next_link_ptr	= dtd_address + 0x20;
		td.terminate		= TERMINATE;
		td.total_bytes		= total_bytes;
		td.ioc				= IOC_SET;
		td.status			= ACTIVE;
		td.buffer_ptr0  	= buffer_addrs_page0;
		td.current_offset 	= (buffer_addrs_page0 & 0xFFF) + g_td_buffer_offset;
		td.buffer_ptr1  	= 0x0;
		td.buffer_ptr2 		= 0x0;
		td.buffer_ptr3  	= 0x0;
		td.buffer_ptr4  	= 0x0;
            
		/* Set the Transfer Descriptor	*/
		usbs_setup_transdesc(&td);

		/* 1. write dQH next ptr and dQH terminate bit to 0 */
		*(volatile cyg_uint32*)(dqh_address+0x8)= dtd_address; 
	    
		/* 2. clear active & halt bit in dQH */
		*(volatile cyg_uint32*)(dqh_address+0xC) &= ~0xFF;
	
		/* 3. prime endpoint by writing '1' in ENDPTPRIME */
		usbs_imx_otg_base->endptprime |= (  EPOUT_PRIME << endpt_num );
		/* Endpoint Configured for output */
		g_out_endpoint= endpt_num;

		
	    }

		else
		{
		total_bytes = 0x4 ;
		
		qhead.dqh_base		= USBS_EP_GET_dQH(endpt_num,direction);
		qhead.zlt			= ZLT_DISABLE;
		qhead.mps			= config_data->max_pkt_size;
		qhead.ios			= IOS_SET;
		qhead.next_link_ptr	= USBS_EP_GET_dQH(endpt_num,direction);
		qhead.terminate 	= TERMINATE;
		qhead.total_bytes	= total_bytes;
		qhead.ioc			= IOC_SET;
		qhead.status 		= NO_STATUS;
		qhead.buffer_ptr0	= 0;
		qhead.current_offset= 0;
		qhead.buffer_ptr1	= 0;
		qhead.buffer_ptr2 	= 0;
		qhead.buffer_ptr3	= 0;
		qhead.buffer_ptr4 	= 0;

		usbs_setup_queuehead(&qhead);
		    
		/* Endpoint Configured for Input */
		g_in_endpoint= endpt_num;
    
		/* Endpoint 2: MPS = 64, IN (Tx endpoint) */
		usbs_imx_otg_base->endptctrl[endpt_num] = 0x00480008;

		/* Enable EP2 IN */
		usbs_imx_otg_base->endptctrl[endpt_num] |= EPIN_ENABLE;
    
		/* 3. prime endpoint by writing '1' in ENDPTPRIME */
		usbs_imx_otg_base->endptprime |= (EPIN_PRIME << g_in_endpoint);
		
	    }
    }
    else 
    {
        /* TODO: error handling TBD */
    }    

}

static void usbs_imx_otg_config_utmi_clock(void)
{
	#if defined(CYGHWR_USB_DEVS_MX37_OTG)
	USB_MX37_SET_PHY_CLK_24MHZ();
	#endif

	#if defined(CYGHWR_USB_DEVS_MX51_OTG)
	cyg_uint32 temp;
	/*Configure USB_PHYCLOCK_ROOT source as 24MHz OSC*/ 
	CCM_CSCMR1_REGVAL = CCM_CSCMR1_REGVAL & (~CSCMR1_USBOH3_PHY_CLK_SEL_VALUE); //configure USB CRM
	/*Configure plldivvalue of USB_PHY_CTRL_1_REG for 24 Mhz*/         
	temp  = *(volatile cyg_uint32 *)USB_PHY_CTRL_1_REG;        
	temp &= ~USB_PHY_CTRL_PLLDIVVALUE_MASK;        
	temp |= USB_PHY_CTRL_PLLDIVVALUE_24_MHZ;                
	*(volatile cyg_uint32 *)USB_PHY_CTRL_1_REG = temp;
	#endif
}

/*=============================================================================
// The USB OTG hardware relevant initialization.
=============================================================================*/
static void
usbs_imx_otg_hardware_init(void)
{
	cyg_uint32 temp;
	cyg_uint32 timeout = 0x1D0000;
	usb_plat_config_data_t config_data_ptr;	
	cyg_uint8 i;
	
	/*Enable USB Internal PHY Clock as 24MHz on-board Ocsillator*/
	usbs_imx_otg_config_utmi_clock();
	
	{/*Setup USB Buffer Map*/
    config_data_ptr.buffer_address = (cyg_uint32)usb_buffer;
    config_data_ptr.buffer_size  = BUFFER_SIZE;
    
    /* Base address of the buffer allocated to IP Layer */
    g_bulkbuffer_address_base =  config_data_ptr.buffer_address;
    
    /* length of the buffer */
    g_bulkbuffer_length = config_data_ptr.buffer_size;	 
    
    /* Maximum Number of EPs to be confiured */
    g_max_ep_supported = (( g_bulkbuffer_length - TOTAL_DATA_BUFFER_SIZE)/(BUFFER_USED_PER_EP)); //=(2048-1088)/256~=3.75->3
    
    /* Base of queue Head Pointer */
    g_bulkbuffer_map.ep_dqh_base_addrs = g_bulkbuffer_address_base; 
    
    /* Total size of qhead */
    temp = (SIZE_OF_QHD * (g_max_ep_supported * 2));	//total size of QH is 384byte

    /* Base Address of dTDs */
    g_bulkbuffer_map.ep_dtd_base_addrs = (g_bulkbuffer_map.ep_dqh_base_addrs + temp);
  
    /* Total size of transfer descriptor */ 
    temp =  ((dTD_SIZE_EPIN * g_max_ep_supported) + (dTD_SIZE_EPOUT * g_max_ep_supported )); //total size of TD is 384 byte
    
    /* Base Address of EP0 Buffer */
    g_bulkbuffer_map.ep0_buffer_addrs = (g_bulkbuffer_map.ep_dtd_base_addrs + temp  );	//256byte
    
    /*Bulk Buffer Areas, 512byte per buffer*/
	/*Actually, the dual 512 byte bulk buffers are not used, because two larger 16kB bulk buffers are used*/
    /* transfer buffer 1 */	
    g_bulkbuffer_map.buffer1_address=(g_bulkbuffer_address_base + g_bulkbuffer_length -(BULK_BUFFER_SIZE*NUM_OF_BULK_BUFFER));
    g_bulkbuffer_map.buffer1_status  = BUFFER_FREE;

    /* transfer buffer 2 */
    g_bulkbuffer_map.buffer2_address = g_bulkbuffer_map.buffer1_address + BULK_BUFFER_SIZE;
    g_bulkbuffer_map.buffer2_status  = BUFFER_FREE;
	}
	
	{/*Set USB OTG at device only mode*/
		usbs_imx_otg_base->usbmode = 0x2;					//set OTG as a device controller
		temp = 0xA5A55A5A;
		while (!(usbs_imx_otg_base->usbmode == 0x2))
		{
			if(temp != (usbs_imx_otg_base->usbmode))
			{
				temp = (usbs_imx_otg_base->usbmode);
				USBDBGMSG(DEBUG_BASIC,"usbmode is 0x%08X\n",temp);
			}
			timeout--;
			if(timeout==0) break;
		}		//check that device controller was configured to device mode only
	}

	{
		usbs_imx_otg_base->endptlistaddr = g_bulkbuffer_map.ep_dqh_base_addrs; // Configure ENDPOINTLISTADDR Pointer
		usbs_imx_otg_base->otgsc |= BIT3;					// Set OTG termination, controls the pulldown on DM
		usbs_imx_otg_base->endptnak = 0x00010001;			// Enable Endpoint NAK
		usbs_imx_otg_base->usbmode |= BIT3;				// Disable Setup Lockout by writing '1' to SLOM in USBMODE
		//usbs_imx_otg_base->usbcmd |= BIT0;				// Set Run/Stop bit to Run Mode, make USB run in usbs_imx_otg_dev_ep0_start()
	}
	
	{
		/* set it to be utmi interface */
		temp  = usbs_imx_otg_base->portsc1;
		temp &= ~USB_OTG_TRANS_MASK;
		temp |= USB_OTG_TRANS_UTMI;
		temp &= ~USB_OTG_FS_ONLY;								//enable high speed
		temp |= USB_OTG_TRANS_WIDTH;

		usbs_imx_otg_base->portsc1 = temp;
	}

	{// The USB OTG transaction relevant initialization.
		/* Select the common descriptors , these descriptor are independent of speed and security mode */ 
		g_usb_desc.device_desc	= &g_usb_device_desc ;
		g_usb_desc.config_desc	= &g_usb_config_desc;
		g_usb_desc.sn_desc		= &g_usb_serialnumber_desc;
		g_usb_desc.str_desc0 	= &g_usb_otg_str0_desc;		//language desc
		g_usb_desc.str_desc1 	= &g_usb_otg_string_desc1;	//Manufacturer desc
		g_usb_desc.str_desc2 	= &g_usb_otg_string_desc2;	//USB Name Desc
		g_usb_desc.str_desc3 	= &g_usb_otg_string_desc3;	//Device Name Desc
	
		/* Get Number of Endpoints supported from Configuration Descriptor*/
		g_number_of_endpoints = g_usb_desc.config_desc->usb_interface_desc.number_endpoints;
   
		/* Store the Endpoint specific information in local variable structure to this Layer */
		for ( i = 0 ; i< g_number_of_endpoints ; i++)
		{
			g_end_pt_info[i].end_pt_no = ((g_usb_desc.config_desc->usb_endpoint_desc[i].endpoint) & ENDPT_NUMBER_MASK);
			g_end_pt_info[i].direction = (((g_usb_desc.config_desc->usb_endpoint_desc[i].endpoint) & ENDPT_DIR_MASK )>>ENDPT_DIR_SHIFT);
			g_end_pt_info[i].transfer_type = (g_usb_desc.config_desc->usb_endpoint_desc[i].attributes & ENDPT_TRNS_TYPE_MASK);
			g_end_pt_info[i].max_pkt_size = ((g_usb_desc.config_desc->usb_endpoint_desc[i].max_packet_lo)	\
										| (( g_usb_desc.config_desc->usb_endpoint_desc[i].max_packet_hi ) << 8 ));
		}
	
		g_usb_dev_state = USB_DEV_DEFAULT_STATE;
	}
}
// ****************************************************************************
// ----------------------------------------------------------------------------
// ****************************************************************************
/*=============================================================================
// Initialization i.MX37(Marley) USB OTG Hardware
// This function is the only extern function of this device driver, and it 
// registers the driver ISR and DSRs to the kernel. 
=============================================================================*/
void
usbs_imx_otg_device_init(void)	//works like usb port open when 
{	
	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: USB Device Driver Start Initializing...\n");
	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: USB OTG REG BASE@0x%08X\n",USB_BASE_ADDRESS);
	g_usb_setup_data = ep0.common.control_buffer;
	
	g_td_buffer_offset = 0;
	#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
	USB_IMX_SET_TD_OFFSET(g_td_buffer_offset,1);
	#endif

	/*ping-pang buffer A*/
	g_bulkbuffer_a.buffer = bulk_buffer;
	g_bulkbuffer_a.stat   = BUFFER_FREED;
	
	/*ping-pang buffer B*/
	g_bulkbuffer_b.buffer = bulk_buffer + BULK_TD_BUFFER_TOTAL_SIZE;
	g_bulkbuffer_b.stat   = BUFFER_FREED;
	
	usbs_imx_otg_hardware_init();	
	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: Usb Hardware Initialize Complete.\n");
	usbs_imx_otg_dev_ep0_init();
	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: Usb Ep0 Initialize Complete.\n");
	usbs_imx_otg_dev_ep1_init();
	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: Usb Ep1 Initialize Complete.\n");
	usbs_imx_otg_dev_ep2_init();
	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: Usb Ep2 Initialize Complete.\n"); 

	#if !defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
	cyg_semaphore_init(&usbs_imx_otg_dev_sem, 0);
    cyg_thread_create(CYGNUM_DEVS_USB_OTG_DEV_THREAD_PRIORITY,
                      &usbs_imx_otg_dev_thread_fn,
                      0,
                      "i.MX37/51 USB Device",
                      usbs_imx_otg_dev_thread_stack,
                      CYGNUM_DEVS_USB_OTG_DEV_THREAD_STACK_SIZE,
                      &usbs_imx_otg_dev_thread_handle,
                      &usbs_imx_otg_dev_thread
        );
    cyg_thread_resume(usbs_imx_otg_dev_thread_handle);
	// It is also possible and desirable to install the interrupt
	// handler here, even though there will be no interrupts for a
	// while yet.
	cyg_interrupt_create(IMX_IRQ_USB_DEV_SERVICE_REQUEST,
                             IMX_IRQ_USB_DEV_PRIORITY,        // priority
                             0,         // data
                             &usbs_imx_otg_dev_isr,                             
						 	 &usbs_imx_otg_dev_thread_dsr,
                             &g_usbs_dev_intr_handle,
                             &g_usbs_dev_intr_data);
	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: cyg_interrupt_create@vector %d.\n",IMX_IRQ_USB_DEV_SERVICE_REQUEST);
	cyg_interrupt_attach(g_usbs_dev_intr_handle);		//fill interrupt handler table for USB 
	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: cyg_interrupt_attach.\n");
	cyg_interrupt_unmask(IMX_IRQ_USB_DEV_SERVICE_REQUEST);	//enable USB interrrupt
	USBDBGMSG(DEBUG_BASIC,"+USBDBGMSG: cyg_interrupt_unmask.\n");
	#endif
	ep0.common.start_fn(&(ep0.common));

}

void 
usbs_imx_otg_device_deinit(void) //works like usb port close
{
	usbs_imx_otg_base->usbcmd &= (~BIT0);				// Set Run/Stop bit to Stop Mode
	g_usb_dev_state = USB_DEV_DUMMY_STATE;
}

#if defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)

static cyg_uint32 get_free_bulk_buffer(void)
{
	cyg_uint32 buff_addr = 0;
	int i = 0;
	while(buff_addr == 0)
	{
		if(g_bulkbuffer_a.stat == BUFFER_FREED)
		{
			buff_addr = (cyg_uint32)(g_bulkbuffer_a.buffer);
			break;
		}
		else if(g_bulkbuffer_b.stat == BUFFER_FREED)
		{
			buff_addr = (cyg_uint32)(g_bulkbuffer_b.buffer);
			break;
		}
		/*
		else
		{
			i++;
			if(i==0xD0000) 
			{
				diag_printf("no bulk buffer free\n");
				break;
			}
			
		}
		*/
	}
	return buff_addr;
}

cyg_bool set_status_bulk_buffer(cyg_uint32 buff_addr, int buff_stat)
{
	cyg_bool ret = true;
	if(buff_addr == (cyg_uint32)(g_bulkbuffer_a.buffer))
		g_bulkbuffer_a.stat = buff_stat;
	else if (buff_addr == (cyg_uint32)(g_bulkbuffer_b.buffer))
		g_bulkbuffer_b.stat = buff_stat;
	else
		ret = false;

	return ret;
}
static usb_status_t usb_bulk_receive_data(usb_buffer_descriptor_t * bd)
{
	usb_status_t status;
	int res;

	/* Check if Bus Reset Received */
    if((usbs_imx_otg_base->usbsts) & IMX_USB_STS_RESET)
    {
    	/* Handle Bus Reset */
        usbs_imx_otg_dev_handle_bus_reset(); 
    }	
    /* Check if Reset is already received and Setup Token Received */
    if((usbs_imx_otg_base->endptsetupstat) & BIT0)
    {
		/* Handle Setup Token */
    	usbs_imx_otg_dev_ep0_dsr();
    }
	
	if((usbs_imx_otg_base->endptcomplete) & ( EPOUT_COMPLETE << EP1))
	{
		ep1_rx_complete(res);
		if(ep1.common.complete_data)
		{
			bd->bytes_transfered = ep1.fetched;
			memcpy(bd->buffer,ep1.common.complete_data,ep1.fetched);
			ep1.fetched = 0;
			//D("+USBDBGMSG:bd->bytes_transfered %d\n",bd->bytes_transfered);
			set_status_bulk_buffer((cyg_uint32)(ep1.common.complete_data), BUFFER_FREED);
			ep1.common.buffer = (unsigned char *)get_free_bulk_buffer();
			ep1.common.buffer_size = BULK_TD_BUFFER_TOTAL_SIZE;
			ep1_start_rx(&(ep1.common));
			usbs_imx_otg_base->endptprime |= ( EPOUT_PRIME << EP1 );//prime ep1 td
			status = USB_SUCCESS;
		}

		else
			status = USB_FAILURE;
	}
	return status;
}

static usb_status_t usb_bulk_transmit_data(usb_buffer_descriptor_t * bd)
{
	//usb_state_t status;

	while(bd->size)
	{
		ep2.common.buffer = (unsigned char *)get_free_bulk_buffer();
		set_status_bulk_buffer((cyg_uint32)(ep2.common.buffer), BUFFER_ALLOCATED);
		ep2.common.buffer_size = (BULK_TD_BUFFER_TOTAL_SIZE<(bd->size))?BULK_TD_BUFFER_TOTAL_SIZE:(bd->size);
		memcpy((ep2.common.buffer),(bd->buffer),(ep2.common.buffer_size));
		ep2_start_tx(&(ep2.common));

		bd->size -= (ep2.common.buffer_size);
	}

	return USB_SUCCESS;
}
static cyg_uint32 usb_rx_processing(cyg_uint8* read_ptr, usb_status_t* status, cyg_uint32 data_length)
{
	cyg_uint32 bytes_received = 0;
    if ( (status != NULL) && (read_ptr != NULL) )
    {
        usb_status_t trans_status = USB_FAILURE;

        usb_buffer_descriptor_t  buf_desc;

        /* Prepare the buffer descriptor for USB transfer */
        //(cyg_uint8*)(buf_desc.buffer) = read_ptr;
		buf_desc.buffer = (void *)read_ptr;
        while(data_length != 0)
        { 
            buf_desc.size = data_length;
            buf_desc.bytes_transfered = 0;

            /* Receive data from USB */
            trans_status = (usb_status_t )usb_bulk_receive_data(&buf_desc);
            if(trans_status == USB_SUCCESS)
            {
                data_length -= buf_desc.bytes_transfered;
                bytes_received += buf_desc.bytes_transfered;
                //(cyg_uint8*)
				(buf_desc.buffer) += buf_desc.bytes_transfered;
            }
            else
            {
                *status = USB_FAILURE;
            }

			g_timeout_value++;
			if(g_timeout_value%0x1000000==0) D("C");
			if(g_timeout_value == USB_DOWNLOAD_TIMEOUT_LIMIT) return 0;
        }
    }
   
    return ( bytes_received );
}
static usb_status_t usb_tx_processing(cyg_uint8* write_ptr, cyg_uint32 data_len)
{
    usb_status_t trans_status = USB_FAILURE;

    /* Prepare the buffer descriptor for USB transfer */
    usb_buffer_descriptor_t  buf_desc;

    /* Prepare transfer buffer descriptor*/ 
    buf_desc.buffer = (void *)write_ptr;
    buf_desc.size = data_len;
    buf_desc.bytes_transfered = 0;

    /* Send data over USB */
    trans_status = usb_bulk_transmit_data(&buf_desc);

	return trans_status;
}
static cyg_bool pl_get_command(void)
{
	cyg_uint8 i = 0;
	usb_status_t status;
    cyg_uint32 bytes_recvd = 0;
    cyg_uint8 start_command = 0xFF;

    while(start_command == 0xFF)
    {
		//g_timeout_value++;
		//if(g_timeout_value%1000==0) D("C");
		//D("%d\n",g_timeout_value);
		bytes_recvd = usb_rx_processing(sdp_payload_data, &status, SDP_CMD_MAX_LEN);
        start_command = pl_command_start();		
		if(g_timeout_value == USB_DOWNLOAD_TIMEOUT_LIMIT) return false;
    }
	//D("+USBDBGMSG: start_command = 0x%02X\n",start_command);
    if(start_command == 0xF0)
    {
        //copy rest of the bytes
        for(i=1; i < SDP_CMD_MAX_LEN; i++)
        {
            sdp_command[i] = sdp_payload_data[i-1];
        }
    }
    else 
    {
        //copy starting bytes
        for(i=0; i < (SDP_CMD_MAX_LEN - start_command) ; i++)
        {
            sdp_command[i] = sdp_payload_data[i + start_command];
        }

        if(start_command != 0)
        {
            //receive rest of the bytes
            bytes_recvd = usb_rx_processing(sdp_payload_data, &status, start_command);
        
            if(bytes_recvd == start_command)
            {
                for(i=0; i <start_command; i++)
                {
                    sdp_command[SDP_CMD_MAX_LEN - start_command + i] = sdp_payload_data[i];
                }
            }
        }
    }

    return true;
}
static cyg_uint8 pl_command_start(void)
{
	cyg_uint8 i=0;
    static cyg_uint8 last_byte = 0;
   
    if(last_byte != 0x0)
    {
        if(last_byte == sdp_payload_data[0])
        {
            sdp_command[0] = last_byte;
            last_byte = sdp_payload_data[SDP_CMD_MAX_LEN -1];
            return 0xF0;
        }
    }

    for(i=0; i < SDP_CMD_MAX_LEN -1; i++)
    {
        if((sdp_payload_data[i] == 0x01) && (sdp_payload_data[i+1] == 0x01) ||
           (sdp_payload_data[i] == 0x02) && (sdp_payload_data[i+1] == 0x02) ||
           (sdp_payload_data[i] == 0x03) && (sdp_payload_data[i+1] == 0x03) ||
           (sdp_payload_data[i] == 0x04) && (sdp_payload_data[i+1] == 0x04) ||
           (sdp_payload_data[i] == 0x05) && (sdp_payload_data[i+1] == 0x05) ||
           (sdp_payload_data[i] == 0x06) && (sdp_payload_data[i+1] == 0x06) ||
           (sdp_payload_data[i] == 0x07) && (sdp_payload_data[i+1] == 0x07) ||
           (sdp_payload_data[i] == 0x08) && (sdp_payload_data[i+1] == 0x08) ||
           (sdp_payload_data[i] == 0x09) && (sdp_payload_data[i+1] == 0x09) ||
           (sdp_payload_data[i] == 0x0A) && (sdp_payload_data[i+1] == 0x0A)) 
         {
             return i;
         }
    }

    //handle last byte
    last_byte = sdp_payload_data[SDP_CMD_MAX_LEN -1];
    if(!(last_byte == 0x1 || last_byte == 0x2 || last_byte == 0x3 || last_byte == 0x4 ||
       last_byte == 0x5 || last_byte == 0x6 || last_byte == 0x7 || last_byte == 0x8 || last_byte == 0x8 ||
       last_byte == 0x9))
    {
        last_byte = 0;
    }
    
    return 0xFF;
}
static cyg_uint8 pl_handle_command(cyg_uint8 g_error_status)
{
	cyg_uint16 Header = 0;
    cyg_uint32 Address = 0;
    cyg_uint32 ByteCount = 0;
    cyg_uint32 g_error_statusAck = 0;
    cyg_uint8 status = 0;
	//int i;
    /* Command Packet Format: Header(2)+Address(4)+Format(1)+ByteCount(4)+Data(4)+Execute(1) */
    Header = ((sdp_command[0]<<8) | (sdp_command[1]));
    Address = ((sdp_command[2]<<24) | (sdp_command[3]<<16) | (sdp_command[4] << 8) | (sdp_command[5]));
    ByteCount = ((sdp_command[7]<<24) | (sdp_command[8]<<16) | (sdp_command[9] << 8) | (sdp_command[10]));

    /* Save g_error_status ack */
    g_error_statusAck = (cyg_uint32)((g_error_status<<24) | (g_error_status <<16) | (g_error_status<<8) | (g_error_status));
	//D("+USBDBGMSG: Command Header 0x%04X\n",Header);
    switch (Header)
    {
        case WRITE_FILE:
			//D("+USBDBGMSG: usb download file to address 0x%08X, length %d\n",usb_download_address,ByteCount);
            pl_handle_write_file(usb_download_address, ByteCount);	
            //pl_handle_write_file(Address, ByteCount);
			//if(g_load_cycle==0) usb_download_address=Address;
			//g_load_cycle ++;
			usb_download_address += ByteCount;
			usb_download_length +=ByteCount;
			D(".");
			if(ByteCount<BULK_TD_BUFFER_TOTAL_SIZE) 
			{
				status = COMPLETE;
			}
            break;
        case ERROR_STATUS_HEADER:
            pl_command_ack(g_error_statusAck);
			status = COMPLETE;
            break;
		case READ_FILE:
        case WRITE_HEADER:
    	case READ_HEADER:
        default:
            break;
    }

    return status;

}
static void pl_handle_write_file(cyg_uint32 address, cyg_uint32 total_bytes)
{
	usb_status_t status;    
    usb_rx_processing((cyg_uint8*)address, &status, total_bytes);
}

static void pl_command_ack(cyg_uint32 ack)
{
    usb_tx_processing((cyg_uint8*)&ack, SDP_CMD_ACK_LEN);
}	

void 
usbs_imx_otg_download(unsigned char * buffer, unsigned int length)
{
	cyg_bool bytes_recvd = false;
	//D("+usbdownload: enter usbs_imx_otg_download\n");
	//D("+USBDBGMSG: re-enumerate USB device\n");
	/*enumeration*/
	/*TODO*/
	while(g_usb_dev_state!=USB_DEV_CONFIGURED_STATE)
	{
		
		/* Check if Bus Reset Received */
        if((usbs_imx_otg_base->usbsts) & IMX_USB_STS_RESET)
        {
    	    /* Handle Bus Reset */
        	usbs_imx_otg_dev_handle_bus_reset(); 
        }	
        /* Check if Reset is already received and Setup Token Received */
        if((g_usb_dev_state != USB_DEV_DUMMY_STATE) && (usbs_imx_otg_base->endptsetupstat & BIT0))
        {
	    	/* Handle Setup Token */
        	usbs_imx_otg_dev_ep0_dsr();
        }		
	}

	if(g_usb_dev_state==USB_DEV_CONFIGURED_STATE)
	{
		//D("+USBDBGMSG: enumeration done\n");
		/*file download*/
		D("USB file download start\n");
		g_timeout_value = 0;
		usb_download_length = 0;
		//usb_download_address = 0;
		//g_load_cycle = 0;
		while(1)
		{
			
			bytes_recvd = pl_get_command();
    		if(bytes_recvd == true)
    		{
            	g_usb_download_state = pl_handle_command(g_error_status);  
    		} 
	
			if((g_usb_download_state==COMPLETE)||(g_timeout_value == USB_DOWNLOAD_TIMEOUT_LIMIT)) 
				break;
			
		}
		diag_printf("\n");
		if(g_timeout_value == USB_DOWNLOAD_TIMEOUT_LIMIT) //timeout value
			D("USB download timeout to wait none file to download\n");
		else
		{
			D("USB file download complete\n");
			//D("+usbdownload: image base 0x%08X, length %d\n",usb_download_address,usb_download_length);
		}
		
	}
}
#endif

//EOF
