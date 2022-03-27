/*
 * Copyright 2004-2006 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/* Modified from Belcarra Linux USB driver by Yi Li */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <redboot.h>
#include <stdlib.h>
#include <cyg/hal/hal_io.h>

#include "../include/mxc-hardware.h"
#include "../include/usbp-chap9.h"

#define CONFIG_OTG_ZASEVB_DIFFERENTIAL_UNIDIRECTIONAL 1
#define MXC_CONFIG_MC13783	1
//#define MXC_USB_DEBUG	1
#ifdef MXC_USB_DEBUG
#define dbug_usb diag_printf
#else 
#define dbug_usb(fmt, ...) do{}while(0) 
#endif

extern unsigned int pmic_reg(unsigned int reg, unsigned int val, unsigned int write);

static void do_usb(int argc, char *argv[]);
RedBoot_cmd("usb",
            "Load data from usb into RAM", 
            "XXX",
            do_usb
           );

#define CONFIG_REDBOOT_VENDORID 	0x15A2
#define CONFIG_REDBOOT_PRODUCTID 	0x001F
#define CONFIG_REDBOOT_BCDDEVICE 	0x0100
#define CONFIG_REDBOOT_MANUFACTURER 0
#define CONFIG_REDBOOT_PRODUCT_NAME 0

u8 mxc_new_address, mxc_usb_address;	/* used to save and set USB address */
struct usbd_endpoint_instance endpoint_zero;
struct usbd_endpoint_instance endpoint_bulk_in;
struct usbd_endpoint_instance endpoint_bulk_out;

u8 *ram_buffer = (u8 *)0x100000;
int ram_transfered = 0;
int transfer_done = 0;

static void inline mxc_reset_toggles(void)
{
	fs_wl(OTG_FUNC_EP_TOGGLE, fs_rl(OTG_FUNC_EP_TOGGLE));
}

static void inline mxc_reset_toggle_ep(int ep_num)
{
	if (fs_rl(OTG_FUNC_EP_TOGGLE) & ep_num)
		fs_wl(OTG_FUNC_EP_TOGGLE, ep_num);
}

static void inline mxc_set_toggle_ep(int ep_num)
{
	dbug_usb("mxc_set_toggle_ep\n");
	if ((fs_rl(OTG_FUNC_EP_TOGGLE) & ep_num) == 0)
	    fs_wl(OTG_FUNC_EP_TOGGLE, ep_num);
}

static void mxc_reset_ep_rdy(int ep_num)
{
	if (fs_rl(OTG_FUNC_EP_RDY) & ep_num) 
		fs_wl(OTG_FUNC_EP_RDY_CLR, ep_num);
}

static void mxc_set_ep_rdy(int ep_num)
{
	if ((fs_rl(OTG_FUNC_EP_RDY) & ep_num) == 0)
	    fs_wl(OTG_FUNC_EP_RDY, ep_num);   
}

static void 
mxc_ep_config(int epn, int dir, u32 ep_num, int wMaxPacketSize, int buffersize,
	int format, int ttlbtecnt)
{
	u32 ep0 = (wMaxPacketSize << 16) | ((format & 3) << 14);
	u32 ep1 = ttlbtecnt ? (data_y_buf(epn, dir) << 16) | data_x_buf(epn, dir) : 0;
	u32 ep3 = ((buffersize - 1) << 21) | (ttlbtecnt & 0x1fffff);

	dbug_usb("mxc_ep_config epn %x dir %x %x %x %x\n", epn, dir, ep0, ep1, ep3);
	fs_wl(ep_word(epn, dir, 0), ep0);
	fs_wl(ep_word(epn, dir, 1), ep1);
	fs_wl(ep_word(epn, dir, 3), ep3);

	fs_orl(OTG_FUNC_EP_EN, ep_num);
}

static void
mxc_ep_config_nodma(int epn, int dir, int wMaxPacketSize, int format, int ttlbtecnt)
{
	u32 ep_num = ep_num_dir(epn, dir);

	mxc_ep_config(epn, dir, ep_num, wMaxPacketSize, wMaxPacketSize, format, ttlbtecnt);

	mxc_set_ep_rdy(ep_num);
}

static void 
mxc_pcd_set_address(u8 address)
{
	mxc_usb_address = 0;
	mxc_new_address = address;	/* this will be used in the interrupt handler */
    dbug_usb("mxc_pcd_set_address to %d\n", address);
}

static int 
usbd_get_descriptor(u8 *buffer, int wLength, int descriptor_type)
{
	int actual_length = 0;
    struct usbd_device_descriptor *device_descriptor;
    u8 iManufacturer;
    u8 iProduct;
    u8 iSerialNumber = 0;
    struct usbd_configuration_descriptor configuration_descriptor;
	struct usbd_interface_descriptor interface_descriptor;
	struct usbd_endpoint_descriptor endpoint_descriptor;

    switch (descriptor_type) {
    case USB_DT_DEVICE: {
        dbug_usb("USB_DT_DEVICE\n");
        iManufacturer = CONFIG_REDBOOT_MANUFACTURER;
        iProduct = CONFIG_REDBOOT_PRODUCT_NAME;

        device_descriptor = (struct usbd_device_descriptor *) buffer;

        device_descriptor->bLength = 18;
        device_descriptor->bDescriptorType = USB_DT_DEVICE;
        device_descriptor->bcdUSB = 0x0110;
        device_descriptor->bDeviceClass = 0;
        device_descriptor->bDeviceSubClass = 0;
        device_descriptor->bDeviceProtocol = 0;
        device_descriptor->bMaxPacketSize0 = 64;

        if (wLength > 8) {
            device_descriptor->idVendor = cpu_to_le16(CONFIG_REDBOOT_VENDORID); 
            device_descriptor->idProduct = cpu_to_le16(CONFIG_REDBOOT_PRODUCTID);
            device_descriptor->bcdDevice = cpu_to_le16(CONFIG_REDBOOT_BCDDEVICE);
            device_descriptor->iManufacturer = iManufacturer;
            device_descriptor->iProduct = iProduct;
            device_descriptor->iSerialNumber = iSerialNumber;
            device_descriptor->bNumConfigurations = 1;
            actual_length += 18;
        }
        else
            actual_length += wLength;
        break;
    }
    case USB_DT_CONFIGURATION: {
        dbug_usb("USB_DT_CONFIGURATION\n");
        configuration_descriptor.bLength = 9;
        configuration_descriptor.bDescriptorType = USB_DT_CONFIGURATION;
        configuration_descriptor.bNumInterfaces = 1;
        configuration_descriptor.bConfigurationValue = 1;
        configuration_descriptor.iConfiguration = '\0';
        configuration_descriptor.bmAttributes = 0;
        configuration_descriptor.bMaxPower = 0;
        actual_length = 9;

        interface_descriptor.bLength = 9;
        interface_descriptor.bDescriptorType = USB_DT_INTERFACE;
        interface_descriptor.bInterfaceNumber = 1;
        interface_descriptor.bAlternateSetting = 0;
        interface_descriptor.bNumEndpoints = 1;
        interface_descriptor.bInterfaceClass = 0;
        interface_descriptor.bInterfaceSubClass = 0;
        interface_descriptor.bInterfaceProtocol = 0;
        interface_descriptor.iInterface = '\0';
        actual_length += 9;

        endpoint_descriptor.bLength = 7;
        endpoint_descriptor.bDescriptorType = USB_DT_ENDPOINT;
        endpoint_descriptor.bEndpointAddress = 1;
        endpoint_descriptor.bmAttributes = USB_ENDPOINT_BULK;
        endpoint_descriptor.wMaxPacketSize = 64;
        endpoint_descriptor.bInterval = 0;
        actual_length += 7;

        configuration_descriptor.wTotalLength = actual_length;
		memcpy(buffer, &configuration_descriptor, 9); 
		if (wLength > 9) {
			memcpy(buffer + 9, &interface_descriptor, 9); 
			memcpy(buffer + 18, &endpoint_descriptor, 7); 
		}
		actual_length = (wLength > actual_length) ? actual_length : wLength; 
        break;
    }
    case USB_DT_STRING: {
        *buffer = '\0';
        break;
    }
    default:
        return -1;
    }

    dbug_usb("usbd_get_descriptor actual_length %d\n", actual_length);
    return actual_length;
}

struct usbd_urb *
usbd_alloc_urb(int length, struct usbd_endpoint_instance *endpoint)
{
    struct usbd_urb *urb;

    urb = (struct usbd_urb *) malloc(sizeof(struct usbd_urb));
    urb->endpoint = endpoint;
    urb->buffer_length = length;

    if (length) {
        urb->buffer = (u8 *) malloc(length);
    }
	endpoint->urb = urb;
    return urb;
}

static void usbd_free_urb(struct usbd_urb *urb)
{
	if (urb) {
		if (urb->endpoint)
			urb->endpoint->urb = NULL;
		if (urb->buffer) 
			free(urb->buffer);
		free(urb);
	}
}

static void 
mxc_sendzlp(int epn, struct usbd_endpoint_instance *endpoint, int wMaxPacketSize)
{
	u32 ep_num_in = ep_num_in(epn);

	if (!(fs_rl(OTG_FUNC_XFILL_STAT) & ep_num_in))
		fs_wl(OTG_FUNC_XFILL_STAT, ep_num_in);
	else if (!(fs_rl(OTG_FUNC_YFILL_STAT) & ep_num_in))
		fs_wl(OTG_FUNC_YFILL_STAT, ep_num_in);

	mxc_ep_config_nodma(epn, USB_DIR_IN, endpoint->wMaxPacketSize,
	    endpoint->bmAttributes & 0x3, 0);
}

static void 
mxc_stop_ep(int epn, int dir)
{
	int ep_num = ep_num_dir(epn, dir);

	mxc_reset_ep_rdy(ep_num);
	fs_wl(OTG_DMA_EP_CH_CLR, ep_num);
	fs_wl(OTG_FUNC_IINT_CLR, ep_num);
	fs_wl(OTG_FUNC_XINT_STAT, ep_num);
	fs_wl(OTG_FUNC_YINT_STAT, ep_num);
	if (epn == 0)
		return;

	fs_wl(OTG_FUNC_XFILL_STAT, ep_num);
	fs_wl(OTG_FUNC_YFILL_STAT, ep_num);
	fs_wl(OTG_FUNC_XFILL_STAT, ep_num);
	fs_wl(OTG_FUNC_YFILL_STAT, ep_num);
}

static void
mxc_pcd_start_ep_setup(struct usbd_endpoint_instance *endpoint)
{
	mxc_ep_config_nodma(0, USB_DIR_OUT, endpoint->wMaxPacketSize,
	    EP_FORMAT_CONTROL, endpoint->wMaxPacketSize);
	mxc_ep_config_nodma(0, USB_DIR_IN, endpoint->wMaxPacketSize,
	    EP_FORMAT_CONTROL, 0);
}

static void
mxc_pcd_start_ep_out(struct usbd_endpoint_instance *endpoint)
{
	struct usbd_urb *urb;
	int epn = endpoint->bEndpointAddress & 0x7f;
	u32 ep_num = ep_num_dir(epn, USB_DIR_OUT);

	urb = endpoint->urb; 
	endpoint->planed = urb->buffer_length;
	mxc_ep_config(epn, USB_DIR_OUT, ep_num, endpoint->wMaxPacketSize,
		endpoint->wMaxPacketSize, endpoint->bmAttributes & 0x3, 
		endpoint->planed);

	mxc_set_ep_rdy(ep_num);
}

static void
mxc_pcd_start_ep_in(struct usbd_endpoint_instance *endpoint)
{
	struct usbd_urb *urb;
	int epn = endpoint->bEndpointAddress & 0x7f;
	u32 ep_num = ep_num_dir(epn, USB_DIR_IN);
	int wMaxPacketSize = endpoint->wMaxPacketSize;

	urb = endpoint->urb;

	/* check if xfil/yfil bits are set, clear if necessary */
	if (epn && fs_rl(OTG_FUNC_XFILL_STAT) & ep_num) {
		fs_wl(OTG_FUNC_XFILL_STAT, ep_num);
	}
	if (epn && fs_rl(OTG_FUNC_YFILL_STAT) & ep_num) {
		fs_wl(OTG_FUNC_YFILL_STAT, ep_num);
	}

	endpoint->planed = urb->actual_length;

	mxc_ep_config(epn, USB_DIR_IN, ep_num, wMaxPacketSize, 
		(endpoint->planed > wMaxPacketSize) ? wMaxPacketSize : endpoint->planed,
		endpoint->bmAttributes & 0x3, endpoint->planed);

	memcpy((void *)data_x_address(epn, USB_DIR_IN), 
		urb->buffer, endpoint->planed);
	fs_wl(OTG_FUNC_XFILL_STAT, ep_num);
	fs_wl(OTG_FUNC_YFILL_STAT, ep_num);

	mxc_set_ep_rdy(ep_num);
}

static int do_device_request(struct usbd_device_request *request)
{
    struct usbd_urb *urb;
    u8 bmRequestType = request->bmRequestType;
    u8 bRequest = request->bRequest;
    u16 wValue = le16_to_cpu(request->wValue);
    u16 wLength = le16_to_cpu(request->wLength);

	dbug_usb("do_device_request %x %x %x %x\n", 
		bmRequestType, bRequest, wValue, wLength);
    if ((bmRequestType & USB_REQ_DIRECTION_MASK)) {
        urb = usbd_alloc_urb(64, &endpoint_zero);
        dbug_usb("allocating urb length: %d at %x\n", wLength, urb);

        switch (bRequest) {
            case USB_REQ_GET_DESCRIPTOR:
                dbug_usb("USB_REQ_GET_DESCRIPTOR\n");
                urb->actual_length = 
                	usbd_get_descriptor(urb->buffer, wLength, wValue >> 8);
				dbug_usb("buffer %x %x %x %x %x %x %x %x %x\n", 
					*(urb->buffer), *(urb->buffer+1), *(urb->buffer+2), 
					*(urb->buffer+3), *(urb->buffer+4), *(urb->buffer+5), 
					*(urb->buffer+6), *(urb->buffer+7), *(urb->buffer+8)); 
                break;
            case USB_REQ_GET_CONFIGURATION:
                urb->actual_length = 1;
                urb->buffer[0] = 1;
                break;
            case USB_REQ_GET_INTERFACE:
                urb->buffer[0] = 0;
                urb->actual_length = 1;
                break;
            default:
                dbug_usb("bad descriptor type");
                return -1;
        }

        mxc_pcd_start_ep_in(urb->endpoint);
		return 0;
    }
    else {
        switch (bRequest) {
            case USB_REQ_SET_ADDRESS:
                mxc_pcd_set_address(wValue);
                break;
            case USB_REQ_SET_CONFIGURATION:
            	urb = usbd_alloc_urb(64 * 2, &endpoint_bulk_out);
			    urb->actual_length = 0;
			    mxc_pcd_start_ep_out(urb->endpoint);
				break;
            case USB_REQ_SET_INTERFACE:
            default: 
                return -1;
        }

		mxc_sendzlp(0, &endpoint_zero, endpoint_zero.wMaxPacketSize);
		hal_delay_us(5000);
		return 0;
    }
    return -1;
}

static void
mxc_pcd_stop_ep_out(int epn, struct usbd_endpoint_instance *endpoint)
{
	u32 transfer;
	u32 ep_num = ep_num_dir(epn, USB_DIR_OUT);
    char *endstring = "redboot dummy";

	transfer = endpoint->planed - (fs_rl(ep_word(epn, USB_DIR_OUT, 3)) & 0x1fffff);
	dbug_usb("mxc_pcd_stop_ep_out received %d\n", transfer);
	//mxc_stop_ep(epn, USB_DIR_OUT);

	if (epn == 0) {
#if 0
		usbd_free_urb(endpoint->urb);
#endif
	}
	else {
		if (transfer > endpoint->wMaxPacketSize) {
//diag_printf("transfer  %d\n", transfer);
			memcpy(ram_buffer + ram_transfered, 
				(void *)data_x_address(epn, USB_DIR_OUT), endpoint->wMaxPacketSize);
			memcpy(ram_buffer + ram_transfered + endpoint->wMaxPacketSize, 
				(void *)data_y_address(epn, USB_DIR_OUT), transfer - endpoint->wMaxPacketSize);
		}
		else {
			memcpy(ram_buffer + ram_transfered, 
				(void *)data_x_address(epn, USB_DIR_OUT), transfer);
            if (transfer == sizeof(endstring)) {
                if (memcmp(endstring, ram_buffer + ram_transfered, sizeof(endstring))
                    == 0) {
                    transfer = 0;
                }
            }
		}
		ram_transfered += transfer;
		dbug_usb("ram_transfered %d, %x %x %x %x %x %x %x %x %x %x %x %x\n", 
		ram_transfered, 
		*(ram_buffer+0), *(ram_buffer+1), *(ram_buffer+2), *(ram_buffer+3),
		*(ram_buffer+4), *(ram_buffer+5), *(ram_buffer+6), *(ram_buffer+7),
		*(ram_buffer+8), *(ram_buffer+9), *(ram_buffer+10), *(ram_buffer+11));

		/* check if xfil/yfil bits are set, clear if necessary */
		if (epn && fs_rl(OTG_FUNC_XFILL_STAT) & ep_num) {
			fs_wl(OTG_FUNC_XFILL_STAT, ep_num);
		}
		if (epn && fs_rl(OTG_FUNC_YFILL_STAT) & ep_num) {
			fs_wl(OTG_FUNC_YFILL_STAT, ep_num);
		}

		/* re-arm */
		mxc_ep_config(epn, USB_DIR_OUT, ep_num, endpoint->wMaxPacketSize,
			endpoint->wMaxPacketSize, endpoint->bmAttributes & 0x3, 
			endpoint->planed);

		mxc_set_ep_rdy(ep_num);

		if (transfer < endpoint->planed) {
			transfer_done = 1;
			return;
		}	
	}
}

static void
mxc_pcd_stop_ep_setup(struct usbd_endpoint_instance *endpoint)
{
	static struct usbd_device_request request;

	u32 ep0 = fs_rl(ep_word(0, USB_DIR_OUT, 0));

	dbug_usb("mxc_pcd_stop_ep_setup ep0 %x\n", ep0);
	if ((ep0 & EP0_SETUP) == 0) {
		dbug_usb("ep0 out received.\n");
		mxc_pcd_stop_ep_out(0, endpoint);
		return;
	}

	dbug_usb("setup received OTG_FUNC_XINT_STAT %x.\n", fs_rl(OTG_FUNC_XINT_STAT));
	mxc_set_toggle_ep(2);

	dbug_usb("copy x buffer\n");
	fs_memcpy((u8 *) &request, (u8 *) data_x_address(0, USB_DIR_OUT), 8);
	fs_wl(OTG_FUNC_XINT_STAT, ep_num_out(0));
	fs_wl(OTG_FUNC_XFILL_STAT, ep_num_out(0));

	do_device_request(&request);

	if ((request.bmRequestType & USB_REQ_DIRECTION_MASK) ==	USB_REQ_HOST2DEVICE) {
		if (mxc_new_address != mxc_usb_address) {
			dbug_usb("Actual set new address to %d\n", mxc_new_address);
			mxc_usb_address = mxc_new_address;
			fs_wl(OTG_FUNC_DEV_ADDR, mxc_usb_address);
		}
	}
}

static void
mxc_pcd_stop_ep_in(int epn, struct usbd_endpoint_instance *endpoint)
{
	mxc_stop_ep(epn, USB_DIR_IN);
	usbd_free_urb(endpoint->urb);

	/* for the ZLP */
	if (!epn) {
		mxc_ep_config_nodma(0, USB_DIR_OUT, endpoint->wMaxPacketSize,
		    EP_FORMAT_CONTROL, endpoint->wMaxPacketSize);
	}
}

int mxc_pcd_int_hndlr(long buffer, long length)
{
	u32 cmd_stat, sint_stat, ep_dstat, ep_stats;
	//u8 first_time = 1;
	int temp;
	ram_buffer = (u8 *)buffer;
	
	dbug_usb("mxc_pcd_int_hndlr +\n");
	while (1) {
		if ((sint_stat = fs_rl(OTG_FUNC_SINT_STAT)))
			fs_wl(OTG_FUNC_SINT_STAT, sint_stat);

		cmd_stat = fs_rl(OTG_FUNC_CMD_STAT);

		if ((cmd_stat & COMMAND_RESETDET) || (sint_stat & SYSTEM_RESETINT)/* || first_time*/) {
			dbug_usb("reset received.\n");
			fs_wl(OTG_FUNC_DEV_ADDR, 0);
			mxc_new_address = mxc_usb_address = 0;
			fs_wl(OTG_DMA_EP_CH_CLR, 0xffffffff);
			fs_wl(OTG_FUNC_IINT_CLR, 0xffffffff);
			fs_wl(OTG_FUNC_EP_DEN_CLR, 0xffffffff);	
			mxc_reset_toggles();
			fs_wl(OTG_FUNC_EP_RDY_CLR, fs_rl(OTG_FUNC_EP_RDY));
			fs_wl(OTG_FUNC_EP_DSTAT, fs_rl(OTG_FUNC_EP_DSTAT));
			fs_wl(OTG_FUNC_XINT_STAT, fs_rl(OTG_FUNC_XINT_STAT));
			fs_wl(OTG_FUNC_YINT_STAT, fs_rl(OTG_FUNC_YINT_STAT));
			//mxc_pcd_start_ep_setup(&endpoint_zero);
			//first_time = 0;
		}

		ep_dstat = fs_rl(OTG_FUNC_EP_DSTAT);
		if (ep_dstat) {
			ep_stats = ep_dstat | fs_rl(OTG_FUNC_XINT_STAT) | fs_rl(OTG_FUNC_YINT_STAT);

			/* clear the done status flags */
			fs_wl(OTG_FUNC_EP_DSTAT, ep_dstat);

			if (ep_stats & EP_IN) {
				dbug_usb("ep0 in received %x\n", ep_stats);
				mxc_pcd_stop_ep_in(0, &endpoint_zero);
			}

			if (ep_stats & EP_OUT) {
				dbug_usb("ep0 out/setup received %x\n", ep_stats);
				mxc_pcd_stop_ep_setup(&endpoint_zero);
			}

			if (ep_dstat & EP_OUT << 2) {
				dbug_usb("bulk out received.\n");
				mxc_pcd_stop_ep_out(1, &endpoint_bulk_out);
			}
			if (ep_dstat & EP_IN << 2) {
				dbug_usb("bulk in received.\n");
				mxc_pcd_stop_ep_in(1, &endpoint_bulk_in);
			}
		}

		if ((transfer_done == 1) || (ram_transfered == length)) {
			temp = ram_transfered;
			ram_transfered = 0;
			return temp;
		}			
	}
}

static void 
iomux_config_mux(u32 pin, int out, int in)
{
	volatile unsigned int *base =
	    (volatile unsigned int *)(IO_ADDRESS(IOMUXC_BASE_ADDR) + 0x034);
	u32 mux_index = PIN_TO_IOMUX_INDEX(pin);
	u32 mux_field = PIN_TO_IOMUX_FIELD(pin);

	base[mux_index] =
	    (base[mux_index] & (~(0xFF << (mux_field * MUX_CTL_BIT_LEN)))) |
	    (((out << 4) | in) << (mux_field * MUX_CTL_BIT_LEN));
}

static int 
mxc_iomux_gpio_mc13783_set(int usb_mode)
{
	iomux_config_mux(PIN_USB_XRXD, 1, 2);
	iomux_config_mux(PIN_USB_VMOUT, 1, 2);
	iomux_config_mux(PIN_USB_VPOUT, 1, 2);
	iomux_config_mux(PIN_USB_VPIN, 1, 2);
	iomux_config_mux(PIN_USB_TXENB, 1, 2);
	iomux_config_mux(PIN_USB_VMIN, 1, 2);

	return 0;
}

static void 
mxc_set_transceiver_mode(int mode)
{
	fs_andl(OTG_CORE_HWMODE, ~0xf0);
	fs_orl(OTG_CORE_HWMODE, (mode << 6) | (mode << 4));	// set to software hnp
}

int mc13783_tcd_mod_init(void)
{
#ifdef CONFIG_OTG_ZASEVB_DIFFERENTIAL_BIDIRECTIONAL
	int hwmode = XCVR_D_D;
	int newmode = XCVR_D_D;
#elif CONFIG_OTG_ZASEVB_DIFFERENTIAL_UNIDIRECTIONAL
	int hwmode = XCVR_D_SE0_NEW;
	int newmode = XCVR_D_D;
#elif CONFIG_OTG_ZASEVB_SINGLE_ENDED_UNIDIRECTIONAL
	int hwmode = XCVR_SE0_D_NEW;
	int newmode = XCVR_SE0_D_NEW;
#elif CONFIG_OTG_ZASEVB_SINGLE_ENDED_BIDIRECTIONAL
	int hwmode = XCVR_SE0_SE0;
	int newmode = XCVR_SE0_SE0;
#endif

	mxc_iomux_gpio_mc13783_set(hwmode);

	mxc_set_transceiver_mode(newmode);

#ifdef MXC_CONFIG_MC13783
	pmic_reg(49, 0x1844, 1);
	pmic_reg(50, 0xe, 1);
#endif

	return 0;
}

void mxc_pcd_exit(void)
{
	dbug_usb("mxc_pcd_exit\n");
	mxc_func_clock_off();
	mxc_main_clock_off();
#ifdef MXC_CONFIG_MC13783
	pmic_reg(49, 0xa0060, 1);
	pmic_reg(50, 0x6, 1);
#endif
	transfer_done = 0;
	ram_transfered = 0;
}
void mxc_pcd_open(void)
{
	mxc_main_clock_on();

	mc13783_tcd_mod_init();

	fs_clear_words((volatile u32 *)IO_ADDRESS(OTG_EP_BASE),	(NUM_ETDS * 16 / 4));
	fs_clear_words((volatile u32 *)IO_ADDRESS(OTG_EP_BASE), 0x200 / 4);
	fs_clear_words((volatile u32 *)IO_ADDRESS(OTG_DATA_BASE), 0x400 / 4);

	//fs_wl(OTG_DMA_MISC_CTRL, OTG_DMA_MISC_ARBMODE);	/* round-robin mode */
	fs_andl(OTG_CORE_HWMODE, ~MODULE_CRECFG_HOST);	// set to FUNCTION hnp
	fs_orl(OTG_CORE_HWMODE, MODULE_CRECFG_FUNC);	// set to software hnp

	mxc_func_clock_on();

	endpoint_zero.wMaxPacketSize = 64;
	endpoint_zero.bEndpointAddress = 0;
	endpoint_zero.bmAttributes = USB_ENDPOINT_CONTROL;
	mxc_pcd_start_ep_setup(&endpoint_zero);

	endpoint_bulk_out.wMaxPacketSize = 64;
	endpoint_bulk_out.bEndpointAddress = 1;
	endpoint_bulk_out.bmAttributes = USB_ENDPOINT_BULK;

	endpoint_bulk_in.wMaxPacketSize = 64;
	endpoint_bulk_in.bEndpointAddress = 1;
	endpoint_bulk_in.bmAttributes = USB_ENDPOINT_BULK;

	fs_andl(OTG_CORE_HNP_CSTAT, ~(MODULE_MASTER | MODULE_SLAVE |
	    MODULE_CMPEN | MODULE_BGEN | MODULE_SWAUTORST |	MODULE_ABBUSREQ));
	fs_orl(OTG_CORE_HNP_CSTAT, MODULE_SLAVE | MODULE_CMPEN | MODULE_BGEN |
		 MODULE_SWAUTORST | MODULE_ABBUSREQ);
}

static int mxc_pcd_init(void)
{
	dbug_usb("mxc_pcd_init\n");
    
    mxc_pcd_open();
	mxc_pcd_int_hndlr(0x100000, 0x200000);

	mxc_pcd_exit();
	return 0;
}

static void
do_usb(int argc, char *argv[])
{
    dbug_usb("do_usb\n");
	mxc_pcd_init();
}
