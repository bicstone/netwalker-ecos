//==========================================================================
//
//      include/usbs_imx.h
//
//      The interface exported by the i.MX37 or i.MX51 USB OTG device driver
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
// Date:         2008-07-22
// Purpose:
//
//####DESCRIPTIONEND####
//==========================================================================
#ifndef CYGONCE_USBS_IMX_H
#define CYGONCE_USBS_IMX_H

#include <cyg/io/usb/usbs.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define USBS_DEBUG	0

/*
 * This function is an exported API for application to initialize 
 * MX37 or MX51 USB OTG in device mode from hardware to driver.
 */
void usbs_imx_otg_device_init(void);
void usbs_imx_otg_device_deinit(void);
#if defined(CYGBLD_IMX_USB_DOWNLOAD_SUPPORT)
void usbs_imx_otg_download(unsigned char * buffer, unsigned int length);
#endif
/*
 * The i.MX37 and 51 family comes with on-chip USB OTG support. This
 * provides three endpoints. Endpoint 0 can only be used for control
 * messages. Endpoints 1 and 2 can only be used for bulk transfers,
 * host->slave for endpoint 1 and slave->host for endpoint 2.
 */
extern usbs_control_endpoint    usbs_imx_otg_ep0;
extern usbs_rx_endpoint         usbs_imx_otg_ep1;
extern usbs_tx_endpoint         usbs_imx_otg_ep2;


/************************************************************************/
#define BUFFER_SIZE 	0x800

#if defined(CYGHWR_USB_DEVS_MX37_OTG)
#define USB_BASE_ADDRESS 0xC3FD4000
#define MX37_IRQ_USB_SERVICE_REQUEST	18	//i.MX37 USB OTG Interrupt 
#define MX37_IRQ_USB_PRIORITY			99	//i.MX37 USB Interrupt Priority 
#define IMX_IRQ_USB_DEV_SERVICE_REQUEST	MX37_IRQ_USB_SERVICE_REQUEST
#define IMX_IRQ_USB_DEV_PRIORITY		MX37_IRQ_USB_PRIORITY

#define CCM_BASE_ADDR 0xE3F8C000
#define CCM_CSCMR1_OFFSET	0x34	
#define CCM_CSCMR1 (CCM_BASE_ADDR + CCM_CSCMR1_OFFSET)
#define REGVAL_CCM_CSCMR1 (*((volatile cyg_uint32*)CCM_CSCMR1))
#define USB_MX37_SET_PHY_CLK_24MHZ() (REGVAL_CCM_CSCMR1&=(~((0x1) <<26)))
	
#define USB_IMX_SET_TD_OFFSET(offset,num) offset=num
#endif


#if defined(CYGHWR_USB_DEVS_MX51_OTG)
#define USB_BASE_ADDRESS 0x73F80000
#define MX51_IRQ_USB_SERVICE_REQUEST	18	//i.MX51 USB OTG Interrupt 
#define MX51_IRQ_USB_PRIORITY			99	//i.MX51 USB Interrupt Priority 
#define IMX_IRQ_USB_DEV_SERVICE_REQUEST	MX51_IRQ_USB_SERVICE_REQUEST
#define IMX_IRQ_USB_DEV_PRIORITY		MX51_IRQ_USB_PRIORITY

#define  USB_OTG_BASE_ADDR	((cyg_uint32)USB_BASE_ADDRESS + 0x000) 
#define  USB_H1_BASE_ADDR	((cyg_uint32)USB_BASE_ADDRESS + 0x200) 
#define  USB_H2_BASE_ADDR	((cyg_uint32)USB_BASE_ADDRESS + 0x400) 
#define  USB_H3_BASE_ADDR	((cyg_uint32)USB_BASE_ADDRESS + 0x400) 
#define  USB_CONTROL_REG	((cyg_uint32)USB_BASE_ADDRESS + 0x800) 
#define  USB_OTG_MIRROR_REG ((cyg_uint32)USB_BASE_ADDRESS + 0x804) 
#define  USB_PHY_CTRL_0_REG ((cyg_uint32)USB_BASE_ADDRESS + 0x808) 
#define  USB_PHY_CTRL_1_REG ((cyg_uint32)USB_BASE_ADDRESS + 0x80c) 
#define  USB_CTRL_1_REG		((cyg_uint32)USB_BASE_ADDRESS + 0x810) 

#define CCM_BASE_ADDR 0x73FD4000
#define CCM_CSCMR1_OFFSET 0x1C
#define CCM_CSCDR1_OFFSET 0x24
#define CCM_CSCMR1 (CCM_BASE_ADDR + CCM_CSCMR1_OFFSET)
#define CCM_CSCDR1 (CCM_BASE_ADDR + CCM_CSCDR1_OFFSET)
#define CCM_CSCMR1_REGVAL (*(cyg_uint32 *)(CCM_BASE_ADDR + CCM_CSCMR1_OFFSET))
#define CCM_CSCDR1_REGVAL (*(cyg_uint32 *)(CCM_BASE_ADDR + CCM_CSCDR1_OFFSET))
/* CSCMR1 register */
#define CSCMR1_USBOH3_PHY_CLK_SEL_MASK		0x04000000
#define CSCMR1_USBOH3_PHY_CLK_SEL_VALUE 	0x04000000
#define CSCMR1_USBOH3_CLK_SEL_MASK			0x00c00000
#define CSCMR1_USBOH3_CLK_SEL_VALUE			0x00400000
/*CSCDR1 register config*/
#define CSCDR1_USBOH3_CLK_PRED_SEL_MASK		0x00000700
#define CSCDR1_USBOH3_CLK_PRED_SEL_VALUE	0x00000400  /* divide by 5 */
#define CSCDR1_USBOH3_CLK_PODF_SEL_MASK		0x000000C0
#define CSCDR1_USBOH3_CLK_PODF_SEL_VALUE	0x00000040  /* divide by 2 */
/* CDCDR register */
#define CDCDR_USB_CLK_PREDF_MASK   0x00000070
#define CDCDR_USB_CLK_PREDF_VALUE  0x00000010  /* divide by 2 */
#define CDCDR_USB_CLK_PODF_MASK    0x0000000E
#define CDCDR_USB_CLK_PODF_VALUE   0x00000002  /* divide by 2 */

/*Hash Defines for PHY_CTRL_REG_1*/
#define  USB_PHY_CTRL_PLLDIVVALUE_MASK  0x00000003
#define  USB_PHY_CTRL_PLLDIVVALUE_19_2_MHZ  0x00000000
#define  USB_PHY_CTRL_PLLDIVVALUE_24_MHZ  0x00000001
#define  USB_PHY_CTRL_PLLDIVVALUE_26_MHZ  0x00000002
#define  USB_PHY_CTRL_PLLDIVVALUE_27_MHZ  0x00000003


#define USB_IMX_SET_TD_OFFSET(offset,num) offset=num//do{}while(0) 
#endif
/************************************************************************/
#define IMX_USB_INTR_DEV_SLE		(1<<8)	//DCSuspend - Sleep Enable
#define IMX_USB_INTR_DEV_SRE		(1<<7)	//SOF Received Enable
#define IMX_USB_INTR_DEV_RESET		(1<<6)	//USB Reset Enable
#define IMX_USB_INTR_DEV_SEE		(1<<4)	//System Error Enable		
#define IMX_USB_INTR_DEV_PCE		(1<<2)	//Port Change Detect Enable
#define IMX_USB_INTR_DEV_USBINT	(1<<0)	//USBINT Enable, IOS@dQH, IOC@dTD will be available

#define IMX_USB_STS_DCSPD			(1<<8)	//DCSuspend Interrupt
#define IMX_USB_STS_SOFRSV			(1<<7)	//SOF Received Interrupt
#define IMX_USB_STS_RESET			(1<<6)	//USB Reset Received Interrupt
#define IMX_USB_STS_SYSERR			(1<<4)	//System Error Interrupt, not implemented in Marley, always '0'
#define IMX_USB_STS_PTCHANGE		(1<<2)	//Port Change Detect Interrupt
#define IMX_USB_STS_USBINT			(1<<0)	//USB Interrupt
/************************************************************************/
#define BIT0 	0x00000001
#define BIT1 	0x00000002
#define BIT2 	0x00000004
#define BIT3 	0x00000008
#define BIT4	0x00000010
#define BIT5 	0x00000020
#define BIT6	0x00000040
#define BIT7	0x00000080
#define BIT8	0x00000100
#define BIT9 	0x00000200
#define BIT10 	0x00000400
#define BIT11 	0x00000800
#define BIT12 	0x00001000
#define BIT13 	0x00002000
#define BIT14 	0x00004000
#define BIT15 	0x00008000
#define BIT16 	0x00010000
#define BIT17 	0x00020000
#define BIT18 	0x00040000
#define BIT19 	0x00080000
#define BIT20 	0x00100000
#define BIT21 	0x00200000
#define BIT22 	0x00400000
#define BIT23 	0x00800000
#define BIT24 	0x01000000
#define BIT25 	0x02000000
#define BIT26 	0x04000000
#define BIT27 	0x08000000
#define BIT28 	0x10000000
#define BIT29 	0x20000000
#define BIT30 	0x40000000
#define BIT31 	0x80000000
/* Device Queue Head and Device Transfer Descriptor Related Defination */
#define SIZE_OF_QHD		0x40
#define SIZE_OF_DTD0 	0x20
#define SIZE_OF_DTD1 	0x20
#define dTD_SIZE_EPIN 	(SIZE_OF_DTD0 + SIZE_OF_DTD1)	//0x40
#define dTD_SIZE_EPOUT 	(SIZE_OF_DTD0 + SIZE_OF_DTD1)	//0x40

#define BUFFER_USED_PER_EP ((SIZE_OF_QHD + dTD_SIZE_EPIN) +(SIZE_OF_QHD + dTD_SIZE_EPOUT)) //0x100

#define ZLT_ENABLE		0
#define ZLT_DISABLE 	1

#define IOS_NOTSET		0
#define IOS_SET			1

#define IOC_NOTSET		0
#define IOC_SET			1

#define TERMINATE		1
#define NOT_TERMINATE	0

#define NO_STATUS 		0
#define ACTIVE			BIT7

#define EPOUT_COMPLETE  BIT0
#define EPIN_COMPLETE	BIT16
	
#define EPOUT_PRIME		BIT0
#define EPIN_PRIME		BIT16

#define EPOUT_ENABLE	BIT7
#define EPIN_ENABLE   	BIT23

#define STALL_RX		0x00000001
#define STALL_TX		0x00010000

/* Buffer size of the buffer used for bulk data transfer */ 

#define	CONTROL_BUFFER_SIZE		0x40
#define BULK_BUFFER_SIZE		0x200
#define NUM_OF_BULK_BUFFER		0x2
#define TOTAL_DATA_BUFFER_SIZE	((BULK_BUFFER_SIZE * NUM_OF_BULK_BUFFER) + CONTROL_BUFFER_SIZE)//512*2+64=1088

#define BULK_TD_BUFFER_TOTAL_SIZE	0x4000
#define BULK_TD_BUFFER_PAGE_SIZE	0x1000
/************************************************************************/
#define USB_OTG_TRANS_MASK      0xC0000000
#define USB_OTG_TRANS_SERIAL    0xC0000000
#define USB_OTG_TRANS_ULPI      0x80000000
#define USB_OTG_TRANS_PHILIP    0x40000000
#define USB_OTG_TRANS_UTMI      0x00000000
#define USB_OTG_FS_ONLY         0x01000000
#define USB_OTG_TRANS_WIDTH     0x10000000

/***********************USB OTG Register Map*****************************/
// ----------------------------------------------------------------------------
// This device driver for i.MX37 has three endpoints.
//
// Endpoint 0 can only be used for bi-directional control messages. 
//
// Endpoint 1 can only be used for host->slave bulk OUT transfers. 
//
// Endpoint 2 can only be used for slave-host bulk IN transfers. 
//
// Start with definitions of the hardware. The use of a structure and
// a const base pointer should allow the compiler to do base/offset
// addressing and keep the hardware base address in a register. This
// is better than defining each hardware register via a separate
// address. Although the registers are only a byte wide, the peripheral
// bus only supports word accesses.
//
// The USB_OTG_ID etc. macros allow for an alternative way of
// accessing the hardware if a better approach is presented, without
// having to rewrite all the code. Macros that correspond to registers
// are actually addresses, making it easier in the code to distinguish
// them from bit values: the & and * operators will just cancel out.
typedef struct usbs_imx_otg_hardware{
  volatile cyg_uint32 id;             //0x000, Identification Register
  volatile cyg_uint32 hwgeneral;      //0x004, General HW Parameters
  volatile cyg_uint32 hwhost;         //0x008, Host HW Parameters
  volatile cyg_uint32 hwdevice;       //0x00c, Device HW Parameters
  volatile cyg_uint32 hwtxbuf;        //0x010, TX Buffer HW Parameters
  volatile cyg_uint32 hwrxbuf;        //0x014, RX Buffer HW Parameters
                    int rsv1[26];
  volatile cyg_uint32 gptimer0ld;     //0x080, GP Timer0 Load Register
  volatile cyg_uint32 gptimer0ctrl;   //0x084, GP Timer0 Control Register
  volatile cyg_uint32 gptimer1ld;     //0x088, GP Timer1 Load Register
  volatile cyg_uint32 gptimer1ctrl;   //0x08c, GP Timer1 control register
  volatile cyg_uint32 sbuscfg;        //0x090, System Bus Interface Control
                    int rsv2[27];
  volatile unsigned char caplength;     //0x100, Capability Length Register
                    char rsv3;
  volatile cyg_uint16 hciversion;   //0x102, Host Interface Version Number
  volatile cyg_uint32 hcsparams;      //0x104, Host Control Structural Parameters
  volatile cyg_uint32 hccparams;      //0x108, Host Control Capability Parameters
                    int rsv4[5];
  volatile cyg_uint16 dciversion;   //0x120, Device Interface Version Number
                    short rsv5;       
  volatile cyg_uint32 dccparams;      //0x124, Device Control Capability Parameters
                    int rsv6[6];
  volatile cyg_uint32 usbcmd;         //0x140, USB Command
  volatile cyg_uint32 usbsts;         //0x144, USB Status
  volatile cyg_uint32 usbintr;        //0x148, USB Interrupt Enable
  volatile cyg_uint32 frindex;        //0x14c, USB Frame Index
                    int rsv7;
  volatile cyg_uint32 devaddr;        //0x154, USB Device Address
  volatile cyg_uint32 endptlistaddr;  //0x158, Address of Endpoint list in memory
  volatile cyg_uint32 ttctrl;         //0x15c, TT status and control
  volatile cyg_uint32 burstsize;      //0x160, Programmable Burst Size
  volatile cyg_uint32 txfilltuning;   //0x164, Host Transmit Pre-Buffer Packet Tuning
  volatile cyg_uint32 txttfilltuning; //0x168,Host TT Transmit Pre-Buffer packet Tuning
                    int rsv8;
  volatile cyg_uint32 ulpiviewpoint;  //0x170, ULPI Viewport
                    int rsv9;
  volatile cyg_uint32 endptnak;       //0x178, Endpoint NAK
  volatile cyg_uint32 endptnaken;     //0x17c, Endpoint NAK Enable
  volatile cyg_uint32 configflag;     //0x180, Configured Flag Register
  volatile cyg_uint32 portsc1;      //0x184~0x1a0, Port Status/Control 1~8
	volatile cyg_uint32 portsc2;
	volatile cyg_uint32 portsc3;
	volatile cyg_uint32 portsc4;
	volatile cyg_uint32 portsc5;
	volatile cyg_uint32 portsc6;
	volatile cyg_uint32 portsc7;
	volatile cyg_uint32 portsc8;		
  volatile cyg_uint32 otgsc;          //0x1a4, OTG Status and Control
  volatile cyg_uint32 usbmode;        //0x1a8, USB Device Mode
  volatile cyg_uint32 endptsetupstat; //0x1ac,Endpoint Setup Status
  volatile cyg_uint32 endptprime;     //0x1b0, Endpoint Initialization
  volatile cyg_uint32 endptflush;     //0x1b4, Endpoint De-Initialization
  volatile cyg_uint32 endptstatus;    //0x1b8, Endpoint Status
  volatile cyg_uint32 endptcomplete;  //0x1bc, Endpoint Complete
  volatile cyg_uint32 endptctrl[16];  //0x1c0~0x1fc, Endpoint Control 0~15
}usbs_imx_otg_hardware;
/*************************usb structures typedefs*************************/
//-----------------------------------------------
//USB buffer data structure
typedef struct {
    cyg_uint32 buffer_address;
    cyg_uint32 buffer_size;
}usb_plat_config_data_t;

//setup data for Queue Header
typedef struct dqh_setup_t{
	cyg_uint32 dqh_word0;	
	cyg_uint32 dqh_word1;	
	cyg_uint32 dqh_word2;	
	cyg_uint32 dqh_word3;	
	cyg_uint32 dqh_word4;	
	cyg_uint32 dqh_word5;	
	cyg_uint32 dqh_word6;	
	cyg_uint32 dqh_word7;	
	cyg_uint32 dqh_word8;	
	cyg_uint32 dqh_word9;	
	cyg_uint32 dqh_word10;	
	cyg_uint32 dqh_word11;	
} dqh_setup_t;
//setup data for Transfer Descriptor
typedef struct dtd_setup_t {
	cyg_uint32 dtd_word0;	
	cyg_uint32 dtd_word1;	
	cyg_uint32 dtd_word2;	
	cyg_uint32 dtd_word3;	
	cyg_uint32 dtd_word4;	
	cyg_uint32 dtd_word5;	
	cyg_uint32 dtd_word6;	
	cyg_uint32 dtd_word7;	
} dtd_setup_t;

//structure for Queue Header
typedef struct dqh_t {
	cyg_uint32 dqh_base;
	cyg_uint32 next_link_ptr;
	cyg_uint32 buffer_ptr0; 
	cyg_uint32 buffer_ptr1; 
	cyg_uint32 buffer_ptr2; 
	cyg_uint32 buffer_ptr3; 
	cyg_uint32 buffer_ptr4;
	cyg_uint16 total_bytes;
  cyg_uint16 mps;
  cyg_uint16 current_offset;
  cyg_uint8 zlt; 
  cyg_uint8 ios;
  cyg_uint8 terminate;
  cyg_uint8 ioc;
  cyg_uint8 status; 
}dqh_t;

//structure for Transfer Descriptor
typedef struct dtd_t {
	cyg_uint32 dtd_base; 
	cyg_uint32 next_link_ptr;
	cyg_uint32 buffer_ptr0; 
	cyg_uint32 buffer_ptr1; 
	cyg_uint32 buffer_ptr2;
	cyg_uint32 buffer_ptr3;
	cyg_uint32 buffer_ptr4;
	cyg_uint16 total_bytes;
	cyg_uint16 current_offset;
	cyg_uint8 terminate;
	cyg_uint8 ioc;
	cyg_uint8 status;
}dtd_t;

//structure for Transfer Descriptor layout
typedef volatile struct TransferDescriptor {
	unsigned terminal			:1	;
	unsigned rsv1				:4	;
	unsigned nxt_pt				:27	;
		
	unsigned status				:8	;
	unsigned rsv2				:2	;
	unsigned multo				:2	;
	unsigned rsv3				:3	;
	unsigned ioc				:1	;
	unsigned totalbytes			:15	;
	unsigned rsv4				:1	;
		
	unsigned offset				:12	;
	unsigned bufferptr0			:20	;
	
	unsigned frame_num			:11	;
	unsigned rsv5				:1	;
	unsigned bufferptr1			:20	;
		
	unsigned rsv6				:12	;
	unsigned bufferptr2			:20	;
		
	unsigned rsv7				:12	;
	unsigned bufferptr3			:20	;
		
	unsigned rsv8				:12	;
	unsigned bufferptr4			:20	;
}__attribute__((packed)) TransferDescriptor;

//structure for Queue Header layout
typedef volatile struct QueueHeader {
	unsigned rsv1				:15		;
	unsigned ios				:1		;
	unsigned mps				:11		;
	unsigned rsv2				:2		;
	unsigned zlt				:1		;
	unsigned mult				:2		;
	
	unsigned rsv3				:5		;		
	unsigned current_dtd		:27		;
		
	struct TransferDescriptor 	dtd		;
	
	unsigned rsv4				:32		;
	
	unsigned setupbuf0			:8		;
	unsigned setupbuf1			:8		;
	unsigned setupbuf2			:8		;
	unsigned setupbuf3			:8		;
	
	unsigned setupbuf4			:8		;
	unsigned setupbuf5			:8		;
	unsigned setupbuf6			:8		;
	unsigned setupbuf7			:8		;												
}__attribute__((packed)) QueueHeader;

//bulk buffer status
enum {
	BUFFER_FREED,
	BUFFER_RELEASED,
	BUFFER_ALLOCATED
};

//structure of bulk buffer
typedef struct {
	unsigned char * buffer;
	cyg_uint32    stat;
}bulk_buffer_t;

//bulk buffer status
enum {
	MASS_STORAGE_CBW_TYPE = 1,
	MASS_STORAGE_DATA_TYPE
};

//structure of USB buffer map
typedef struct {
    cyg_uint32 ep_dqh_base_addrs;	/* Base Address of Queue Header */
    cyg_uint32 ep_dtd_base_addrs;	/* Base Address of Transfer Descriptor */
    cyg_uint32 ep0_buffer_addrs; 	/* Buffer Addres for EP0 IN  */
    cyg_uint32 buffer1_address;		/* Buffer1 address for bulk transfer */
    cyg_uint32 buffer1_status;		/* Status of Buffer1 */
    cyg_uint32 buffer2_address;		/* Buffer2 address for bulk transfer */
    cyg_uint32 buffer2_status;		/* Status of Buffer2 */
}buffer_map_t;

//Data Structure used for configuring the Endpoints.
typedef struct {
    cyg_uint8  end_pt_no;					/* Endpoint number */
    cyg_uint8  direction;					/* Direction of endpoint */
    cyg_uint8  transfer_type;			/* type of transfer supporting on the endpoint */
    cyg_uint16  max_pkt_size;			/* maximum packet size in bytes */
}usb_end_pt_info_t;

//Buffer descriptor used for data transfer on USB
typedef struct {
    void * buffer ;								/* Address of the buffer to/from data is to be transmitted */
    cyg_uint32 size ;          		/*  size of the buffer to  be transmitted/recieved */
    cyg_uint32 bytes_transfered;	/* actual number of bytes transfered */
}usb_buffer_descriptor_t;

/*************************important constant in usb transaction*************************/
/* Maximum packet size defination */
#define MPS_8		8
#define MPS_64		64

#define SETUP_DATA_LENGTH			0x8
#define ENDPT_NUMBER_MASK			0x0F
#define ENDPT_DIR_MASK				0x80
#define ENDPT_DIR_SHIFT				0x7 
#define ENDPT_TRNS_TYPE_MASK		0x03

#define USB_MAX_DEVICE_ADDR			127
#define USB_DEV_VALUE_OF_UNCONFIG	0x0

#define USB_DEV_CONFIG_DESC_CONFIG_VALUE	0x01
/* Default device address */
#define USB_DEFAULT_ADDR			0x00

/* DESCRIPTOR Type */
#define DEVICE_DESC					0x1
#define CONF_DESC					0x2
#define STRING_DESC					0x3
#define INTERFACE_DESC   			0x4
#define ENDPOINT_DESC    			0x5
#define DEVICE_QUALIFIER 			0x6
#define OTHER_SPEED_CONF_DESC		0x7

/* String SUB DESCRIPTOR type */
#define STR_DES0					0x0
#define STR_DES1					0x1
#define STR_DES2					0x2
#define STR_DES3					0x3
#define STR_DES4					0x4
#define STR_DES5					0x5

/* Descriptor Index */	
#define FILL_DEVICE_DESC			0x1
#define FILL_DEVICE_QF_DESC			0x2
#define FILL_CONF_DESC				0x3
#define FILL_OT_CONF_DESC			0x4
#define FILL_STR_DES0				0x5
#define FILL_STR_DES1				0x6
#define FILL_STR_DES2				0x7
#define FILL_STR_DES3				0x8
#define FILL_SN_DESC				0x9	//mandatory descriptor for mass storage device

#define LEN_OF_CONFIG_VALUE			0x1
	
#define	NUM_OF_ENDPT_OFFSET 		0x4
#define	CONFIG_NUMBER_OFFSET 		0x5
#define STRING_DESC_LEN_OFFSET		0x0
#define DEVICE_DESC_LEN_OFFSET		0x0
#define CONF_DESC_LEN_OFFSET		0x0
#define INF_DESC_LEN_OFFSET			0x0
#define EP_DESC_LEN_OFFSET			0x0

/*************************usb enums typedefs*************************/
typedef enum {
	USB_DEFAULT_STATE,
	USB_ADDRESSED_STATE,
	USB_CONFIGURED_STATE,
	USB_SUSPENDED_STATE
} USB_DEVICE_STATE_T;

/* USB Device State which are handled by DCD */
typedef enum
{
    USB_DEV_DUMMY_STATE,
    USB_DEV_DEFAULT_STATE,
    USB_DEV_ADDRESSED_STATE,
    USB_DEV_CONFIGURED_STATE
}usb_state_t;

/* Status of all transaction on USB */
typedef enum
{
    USB_SUCCESS,
    USB_FAILURE,
    USB_INVALID = -1 /* Always Keep this entry in last */
}usb_status_t;

/* enum for endpoint numbers */
enum
{
    EP0,
    EP1,
    EP2,
    EP3,
    EP4,
    EP5
};

enum 
{
    OUT,
    IN
};   
/* enum for data transfer type on endpoints */
enum
{
    CONTROL,
    ISOCHRONOUS,
    BULK,
    INTERRUPT
};

/* Constants defined to represent the elements within the setup packet. */
enum 
{
    BMREQUESTTYPE,
    BREQUEST,
    WVALUE_LOWBYTE,
    WVALUE_HIGHBYTE,
    WINDEX_LOWBYTE,
    WINDEX_HIGHBYTE,
    WLENGTH_LOWBYTE,
    WLENGTH_HIGHBYTE
};

/* Enum constants for function to identify the USB Standard Request defined 
 * in USB Specification. 
 */
enum 
{
   USB_GET_STATUS,
   USB_CLEAR_FEATURE,
   USB_RESERVED_REQ_ONE,
   USB_SET_FEATURE,
   USB_RESERVED_REQ_TWO,
   USB_SET_ADDRESS,
   USB_GET_DESCRIPTOR,
   USB_SET_DESCRIPTOR,
   USB_GET_CONFIGURATION,
   USB_SET_CONFIGURATION,
   USB_GET_INTERFACE,
   USB_SET_INTERFACE,
   USB_SYNCH_FRAME
};

/* Mass Storage Class-specific request
 */
enum{
	USB_MSC_GET_MAX_LUN=0xFE,
	USB_MSC_BOT_RESET
};
#define USB_REQTYPE_RESET	0x21
#define USB_REQTYE_GETMAXLUN	0xA1

/* Status of the buffer used for bulk transfer */
enum {
    BUFFER_FREE,
    BUFFER_IN_USE
};

/***********************usb_descriptor_definitions*************************/
#define VID	0x15A2
#define	PID	0x002C

/* Constants defined to represent device descriptor elements. */
#define USB_DEV_DESC_LEN                                 0x12
#define USB_DEV_DESC_TYPE                                0x01
#define USB_DEV_DESC_SPEC_LB                             0x00
#define USB_DEV_DESC_SPEC_HB                             0x02
#define USB_DEV_DESC_DEV_CLASS                           0x00  /*ROM Code definition*///0x02	/* Fisher: CDC bDeviceClass */										    
#define USB_DEV_DESC_DEV_SUBCLASS						 0x00   //0x02  /* Fisher: Abstract Control Model*/										    
#define USB_DEV_DESC_DEV_PROTOCOL                        0x00
#define USB_DEV_DESC_EP0_MAXPACKETSIZE                   0x40
#define USB_DEV_DESC_VENDORID_LB                         (VID & 0x00FF) 
#define USB_DEV_DESC_VENDORID_HB                         ((VID & 0xFF00) >> 0x8) 
#define USB_DEV_DESC_PRODUCTID_LB                        (PID & 0x00FF)
#define USB_DEV_DESC_PRODUCTID_HB                        ((PID & 0xFF00) >> 0x8)
#define USB_DEV_DESC_DEV_RELEASE_NUM_LB                  0x01
#define USB_DEV_DESC_DEV_RELEASE_NUM_HB                  0x00
#define USB_DEV_DESC_DEV_STRING_IND_MANUFACTURE          0x01
#define USB_DEV_DESC_DEV_STRING_IND_PRODUCT              0x02
#if defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
#define USB_DEV_DESC_DEV_STRING_IND_SERIAL_NUM           0x00 
#else
#define USB_DEV_DESC_DEV_STRING_IND_SERIAL_NUM           0x05 /*for mass storage device, it must >0*/
#endif
#define USB_DEV_DESC_DEV_NUM_CONFIGURATIONS              0x01



/* Constants defindes to represent elements of configuration descriptor. */

#define USB_DEV_CONFIG_DESC_LEN                          0x09 /* Length of configuration descriptor. */ 
#define USB_DEV_CONFIG_DESC_TYPE                         0x02 /* Descriptor type. */
#define USB_DEV_CONFIG_DESC_TTL_LEN_LB                   0x20 /* Total length of configuration information. */
#define USB_DEV_CONFIG_DESC_TTL_LEN_HB                   0x00 /* Total length of configuration information. */
#define USB_DEV_CONFIG_DESC_NUM_0F_INF                   0x01 /* Number of interfaces in this configuration. */ 
#define USB_DEV_CONFIG_DESC_CONFIG_VALUE                 0x01 /* Configuration value. */
#if defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
#define USB_DEV_CONFIG_DESC_STRING_INDEX                 0x04 /* String index for this configuration. */
#else
#define USB_DEV_CONFIG_DESC_STRING_INDEX                 0x00 /* String index for this configuration. */
#endif
#define USB_DEV_CONFIG_DESC_ATTRIBUTES                   0xC0/*  Self powered and supported remote wakeup. */ 
																/* 0x80 Self powered and supported remote wakeup. */
															  
#define USB_DEV_CONFIG_DESC_MAX_POWER                    0x32 /* 100ma,Max power consumed by phone. */
 
#define USB_DEV_INF_DESC_LEN                             0x09 /* Interface descriptor length. */
#define USB_DEV_INF_DESC_TYPE                            0x04 /* The descriptor type, 4 interface descriptor. */
#define USB_DEV_INF_DESC_INF_INDEX                       0x00 /* Interface index. */
#define USB_DEV_INF_DESC_ALT_SETTING                     0x00 /* The alternate setting is 0. */
#define USB_DEV_INF_DESC_NUM_OF_EP                       0x02 /* Control endpoint and data endpoint 1 and 2. */
#define USB_DEV_INF_DESC_INF_CLASS_VENDOR                0xFF /* Interface class: Vendor Specific. */			
#define USB_DEV_INF_DESC_INF_CLASS_MSC                   0x08 /* Interface class: Mass Storage. */									     
#define USB_DEV_INF_DESC_INF_SUBCLASS_S_BLANK						 0x40 /* (Subclass) Motorola Flash Download. */												
#define USB_DEV_INF_DESC_INF_SUBCLASS_NS_BLANK           0x42				
#define USB_DEV_INF_DESC_INF_SUBCLASS_MSC_SCSI           0x06	/* SCSI transparent command set for mass storage*/							
#define USB_DEV_INF_DESC_INF_PROTOCOL                    0x01 /* (Interface protocol) Vendor Specific, ROM bootloader interface. */
#define USB_DEV_INF_DESC_INF_PROTOCOL_MSC_BOT						 0x50	/* Mass Storage Bulk Only Transport*/
#if defined(CYGHWR_IMX_USB_DOWNLOAD_SUPPORT)
#define USB_DEV_INF_DESC_STRING_INDEX                    0x05  /* Index of interface string descriptor. */
#else
#define USB_DEV_INF_DESC_STRING_INDEX                    0x04  /* Index of interface string descriptor. */
#endif

/* Constants defined to represent the endpoint descriptor elements. */
#define USB_MAX_PACKET_SIZE			0x0200
#define USB_MAX_PACKET_SIZE_LO		(USB_MAX_PACKET_SIZE&0xFF)
#define USB_MAX_PACKET_SIZE_HI		((USB_MAX_PACKET_SIZE>>8)&0xFF)

/* Endpoint 1 descriptor. */
#define USB_EP1_DESC_SIZE                                0x07 /* Size of descriptor in bytes. */
#define USB_EP1_DESC_TYPE                                0x05 /* Descriptor type. */
#define USB_EP1_DESC_EP_ADDR                             0x01 /* (Endpoint address) Endpoint 1, OUT. */
#define USB_EP1_DESC_ATTRIBUTES                          0x02 /* (Attributes) Bulk Endpoint. */
#define USB_EP1_DESC_MAX_PACKET_SIZE_FS_LB               0x40 /* Max Packet Size. */
#define USB_EP1_DESC_MAX_PACKET_SIZE_FS_HB               0x00 /* Max Packet Size. */
#define USB_EP1_DESC_MAX_PACKET_SIZE_HS_LB               USB_MAX_PACKET_SIZE_LO /* Max Packet Size. */
#define USB_EP1_DESC_MAX_PACKET_SIZE_HS_HB               USB_MAX_PACKET_SIZE_HI /* Max Packet Size. */
#define USB_EP1_DESC_INTERVAL                            0x00 /* Interval, ignored. */
#define USB_EP1_DESC_INTERVAL_HS                      	 0x01 /* at most 1NAK. */
/* Endpoint 2 descriptor. */
#define USB_EP2_DESC_SIZE                                0x07 /* Size of descriptor in bytes. */
#define USB_EP2_DESC_TYPE                                0x05 /* Descriptor type. */
#define USB_EP2_DESC_EP_ADDR                             0x82 /* (Endpoint address) Endpoint 2, IN. */
#define USB_EP2_DESC_ATTRIBUTES                          0x02 /* (Attributes) Bulk Endpoint. */
#define USB_EP2_DESC_MAX_PACKET_SIZE_FS_LB               0x40 /* Max Packet Size. */
#define USB_EP2_DESC_MAX_PACKET_SIZE_FS_HB               0x00 /* Max Packet Size. */
#define USB_EP2_DESC_MAX_PACKET_SIZE_HS_LB               USB_MAX_PACKET_SIZE_LO/* Max Packet Size. */
#define USB_EP2_DESC_MAX_PACKET_SIZE_HS_HB               USB_MAX_PACKET_SIZE_HI /* Max Packet Size. */

#define USB_EP2_DESC_INTERVAL                            0x00 /* Interval, ignored. */
#define USB_EP2_DESC_INTERVAL_HS						 0x01 /* at most 1NAK. */
/* String Descriptor 0 */
#define USB_STR0_DESC_SIZE                               0x04 /* Size of descriptor in bytes. */
#define USB_STR0_DESC_TYPE                               0x03 /* Descriptor type. */
#define USB_LANGUAGE_ID_LB								 0x09 /* Language id of english */
#define USB_LANGUAGE_ID_HB								 0x04 /* Language id of english */

/* String Descriptor 1 */
#define USB_STR1_DESC_SIZE                                0x3A /* Size of descriptor in bytes. */
#define USB_STR1_DESC_TYPE                                0x03 /* Descriptor type. */

/* String Descriptor 2 */
#define USB_STR2_DESC_SIZE_NS                             0x20 /* Size of descriptor in bytes for Non Secure Download*/
#define USB_STR2_DESC_SIZE_SE                             0x20 /* Size of descriptor in bytes for Secure Engg. download*/
#define USB_STR2_DESC_SIZE_S                              0x20 /* Size of descriptor in bytes for Secure production download*/
#define USB_STR2_DESC_TYPE                                0x03 /* Descriptor type. */

/* String Descriptor 3 */
#define USB_STR3_DESC_SIZE                                0x20 /* Size of descriptor in bytes. */
#define USB_STR3_DESC_TYPE                                0x03 /* Descriptor type. */

/* Serial number string descriptor */
#define USB_SN_DESC_LEN									0x1A /* Size of descriptor length*/
#define USB_SN_DESC_TYPE								0x03 /* type of descriptor*/
/*************************usb descriptor typedefs********************/
typedef struct {
	usb_configuration_descriptor usb_config_desc;
    usb_interface_descriptor  usb_interface_desc;
    usb_endpoint_descriptor  usb_endpoint_desc[USB_DEV_INF_DESC_NUM_OF_EP];		
}__attribute__((packed)) usb_conf_desc;
/* USB device serial number for mass storage requiremen*/
typedef struct {
	cyg_uint8  length;
	cyg_uint8  descriptor_type;
	cyg_uint8  string[24];
}__attribute__((packed)) usb_str4_desc;
/* USB string Descriptor structure 0 according to USB2.0 Specification */
typedef struct {
    cyg_uint8  length;
    cyg_uint8  descriptor_type;
    cyg_uint8  language_id0_l;
    cyg_uint8  language_id0_h;
}__attribute__((packed)) usb_str0_desc;

/* USB string Descriptor structure 1 according to USB2.0 Specification */
typedef struct {
    cyg_uint8  length;
    cyg_uint8  descriptor_type;
    cyg_uint8  string[56];
}__attribute__((packed)) usb_str1_desc;

/* USB string Descriptor structure 2 according to USB2.0 Specification */
typedef struct {
    cyg_uint8  length;
    cyg_uint8  descriptor_type;
    cyg_uint8  string[34];
}__attribute__((packed)) usb_str2_desc;

/* USB string Descriptor structure 3 according to USB2.0 Specification */
typedef struct {
    cyg_uint8  length;
    cyg_uint8  descriptor_type;
    cyg_uint8  string[30];
}__attribute__((packed)) usb_str3_desc;

#define usb_dev_desc usb_device_descriptor	//rename the structure

/* ALL USB Descriptors for both FS and HS */
typedef struct {
    usb_dev_desc*  device_desc; 
    usb_conf_desc* config_desc;	
	usb_str4_desc*   sn_desc;
    usb_str0_desc* str_desc0; 
    usb_str1_desc* str_desc1;
    usb_str2_desc* str_desc2; 
    usb_str3_desc* str_desc3;
	
}usb_descriptor;

#ifdef __cplusplus
} /* extern "C" { */
#endif


#endif /* CYGONCE_USBS_IMX_H */
