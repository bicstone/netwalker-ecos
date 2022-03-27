#ifndef _IMX_ATA_H_
#define _IMX_ATA_H_
//==========================================================================
//
//      mxc_ata.h
//
//   Support ATA on Freescale MXC platforms
//
//==========================================================================
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
// Author(s):    Mahesh Mahadevan <mahesh.mahadevan@freescale.com>
// Contributors: Mahesh Mahadevan <mahesh.mahadevan@freescale.com>
// Date:         2008-11-18
//
//==========================================================================
#define FSL_ATA_TIMING_REGS                             0x00
#define FSL_ATA_FIFO_FILL                                   0x20
#define FSL_ATA_CONTROL                                     0x24
#define FSL_ATA_INT_PEND                                   0x28
#define FSL_ATA_INT_EN                                       0x2C
#define FSL_ATA_INT_CLEAR                                 0x30
#define FSL_ATA_FIFO_ALARM                              0x34
#define FSL_ATA_ADMA_ERROR_STATUS               0x38
#define FSL_ATA_SYS_DMA_BADDR                       0x3C
#define FSL_ATA_ADMA_SYS_ADDR                       0x40
#define FSL_ATA_BLOCK_COUNT                            0x48
#define FSL_ATA_BURST_LENGTH                          0x4C
#define FSL_ATA_SECTOR_SIZE                             0x50
#define FSL_ATA_DRIVE_DATA                              0xA0
#define FSL_ATA_DFTR                                          0xA4
#define FSL_ATA_DSCR                                          0xA8
#define FSL_ATA_DSNR                                          0xAC
#define FSL_ATA_DCLR                                           0xB0
#define FSL_ATA_DCHR                                          0xB4
#define FSL_ATA_DDHR                                          0xB8
#define FSL_ATA_DCDR                                          0xBC
#define FSL_ATA_DRIVE_CONTROL                         0xD8

/* bits within FSL_ATA_CONTROL */
#define FSL_ATA_CTRL_DMA_SRST                         0x1000
#define FSL_ATA_CTRL_DMA_64ADMA                    0x800
#define FSL_ATA_CTRL_DMA_32ADMA                    0x400
#define FSL_ATA_CTRL_DMA_STAT_STOP               0x200
#define FSL_ATA_CTRL_DMA_ENABLE                     0x100
#define FSL_ATA_CTRL_FIFO_RST_B                       0x80
#define FSL_ATA_CTRL_ATA_RST_B                        0x40
#define FSL_ATA_CTRL_FIFO_TX_EN                      0x20
#define FSL_ATA_CTRL_FIFO_RCV_EN                    0x10
#define FSL_ATA_CTRL_DMA_PENDING                   0x08
#define FSL_ATA_CTRL_DMA_ULTRA                       0x04
#define FSL_ATA_CTRL_DMA_WRITE                       0x02
#define FSL_ATA_CTRL_IORDY_EN                          0x01

/* bits within the interrupt control registers */
#define FSL_ATA_INTR_ATA_INTRQ1                      0x80
#define FSL_ATA_INTR_FIFO_UNDERFLOW             0x40
#define FSL_ATA_INTR_FIFO_OVERFLOW               0x20
#define FSL_ATA_INTR_CTRL_IDLE                         0x10
#define FSL_ATA_INTR_ATA_INTRQ2                      0x08
#define FSL_ATA_INTR_DMA_ERR                           0x04
#define FSL_ATA_INTR_DMA_TRANS_OVER            0x02

/* ADMA Addr Descriptor Attribute Filed */
#define FSL_ADMA_DES_ATTR_VALID                     0x01
#define FSL_ADMA_DES_ATTR_END                        0x02
#define FSL_ADMA_DES_ATTR_INT                         0x04
#define FSL_ADMA_DES_ATTR_SET                         0x10
#define FSL_ADMA_DES_ATTR_TRAN                      0x20
#define FSL_ADMA_DES_ATTR_LINK                       0x30

#define PIO_XFER_MODE_0                                     0
#define PIO_XFER_MODE_1                                     1
#define PIO_XFER_MODE_2                                     2
#define PIO_XFER_MODE_3                                     3
#define PIO_XFER_MODE_4                                     4

#define ATA_ID_PROD                                             27
#define ATA_ID_PROD_LEN                                     40

#define ATA_BUSY                                                   (1 << 7)
#define ATA_DRQ                                                     (1 << 3)
#define ATA_ERR                                                     (1)

#define ATA_IEN                                                     (1 << 1)
#define ATA_SRST                                                   (1 << 2)

#define ATA_CMD_READ                                          0x20
#define ATA_CMD_WRITE                                        0x30
#define ATA_CMD_READ_MULTI                              0xC4
#define ATA_CMD_WRITE_MULTI                            0xC5
#define ATA_CMD_ID_ATA                                      0xEC
#define ATA_CMD_SET_FEATURES                          0xEF

#define ATA_SECTOR_SIZE                                     512
#define MAX_NUMBER_OF_SECTORS                       256
#endif // _IMX_ATA_H_
