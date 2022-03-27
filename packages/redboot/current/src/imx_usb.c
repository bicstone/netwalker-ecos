//==========================================================================
//
//      imx_usb.c
//
//      usb download support for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
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
// Author(s):    Fisher ZHU
// Contributors: Fisher ZHU
// Date:         2008-10-27
// Purpose:      
// Description:  this code architecture is based on mxc_usb.c
//              
// This code is part of RedBoot (tm).
// 
// Revision History:
// Date			Author			Comments
// 2008-10-27	Fisher ZHU		Initial Creation, support for i.mx37
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <pkgconf/devs_usb_imx_otg.h>
static struct {
    bool open;
    int  total_timeouts, packets_received;
    unsigned long last_good_block;
    int  avail, actual_len;
//    char data[SEGSIZE+sizeof(struct imxotghdr)];
//    char *bufp;
} imxotg_stream;

EXTERN unsigned long entry_address;
EXTERN unsigned long load_address;
EXTERN unsigned long load_address_end;

extern cyg_uint32 usb_download_address;
extern cyg_uint32 usb_download_length;

extern void usbs_imx_otg_device_init(void);
extern void usbs_imx_otg_device_deinit(void);
#if defined(CYGBLD_IMX_USB_DOWNLOAD_SUPPORT)
extern void usbs_imx_otg_download(unsigned char * buffer, unsigned int length);
#endif

int
imxotg_stream_open(connection_info_t *info,
                 int *err)
{
    //diag_printf("%s()\n", __FUNCTION__);
    usbs_imx_otg_device_init();
    return 0;
}

void
imxotg_stream_close(int *err)
{
    //diag_printf("%s()\n", __FUNCTION__);
    usbs_imx_otg_device_deinit();
}

void
imxotg_stream_terminate(bool abort,
                      int (*getc)(void))
{
    int err;
    //diag_printf("%s()\n", __FUNCTION__);
    load_address_end = load_address + usb_download_length;
	entry_address = load_address;
}

int
imxotg_stream_read(char *buf,
                 int len,
                 int *err)
{
	
	//diag_printf("%s(transfer length=%d,buffer address=0x%08x)\n", __FUNCTION__, len, buf);

	/*buf and len are not used by usb download. 
	buf is a buffer pointer created by redboot, but USB download will download the binary file directly to
	the memory pointed by load_address.
	len is the buffer length, while USB download will download all the binary file once. 
	The two variables are actually dummy.*/
	usb_download_address = load_address;
	usbs_imx_otg_download(buf,len);											
	return 0;
}

char *
imxotg_error(int err)
{
    char *errmsg = "Unknown error";

    //diag_printf("%s()\n", __FUNCTION__);
#if 0
    switch (err) {
    case MXCUSB_ENOTFOUND:
        return "file not found";
    case MXCUSB_EACCESS:
        return "access violation";
    case MXCUSB_ENOSPACE:
        return "disk full or allocation exceeded";
    case MXCUSB_EBADOP:
        return "illegal MXCUSB operation";
    case MXCUSB_EBADID:
        return "unknown transfer ID";
    case MXCUSB_EEXISTS:
        return "file already exists";
    case MXCUSB_ENOUSER:
        return "no such user";
    case MXCUSB_TIMEOUT:
        return "operation timed out";
    case MXCUSB_INVALID:
        return "invalid parameter";
    case MXCUSB_TOOLARGE:
        return "file is larger than buffer";
    }
#endif
    return errmsg;
}

//
// RedBoot interface
//
GETC_IO_FUNCS(imxotg_io, imxotg_stream_open, imxotg_stream_close,
              imxotg_stream_terminate, imxotg_stream_read, imxotg_error);
RedBoot_load(usb, imxotg_io, true, false, 0);


