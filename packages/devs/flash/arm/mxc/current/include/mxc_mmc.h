/*=================================================================================

    Module Name:  mxc_mmc.h

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
Lewis Liu                  18-Feb-2008


Portability: Portable to other compilers or platforms.

====================================================================================================*/

#ifndef __MXC_MMC_H__
#define __MXC_MMC_H__

#include <pkgconf/system.h>

#define FLASH_DEBUG_MIN 1
#define FLASH_DEBUG_MED 2
#define FLASH_DEBUG_MAX 3
#define FLASH_DEBUG_LEVEL FLASH_DEBUG_MED
#define flash_dprintf(level, args...)          \
   do {                                \
         if (FLASH_DEBUG_LEVEL >= level)     \
            diag_printf(args);          \
   } while(0)

#define CHECK_RUN_TIMES(n) { \
	static int count = 0;\
	if(++count > n){\
		diag_printf("%s: the loop gets the limitation, WRONG!\n", __FUNCTION__);break;}

typedef int mxc_mmc_check_sdhc_boot_slot(unsigned int port, unsigned int *sdhc_addr);

#define READ_PORT_FROM_FUSE     50
#endif //__MXC_MMC_H__
