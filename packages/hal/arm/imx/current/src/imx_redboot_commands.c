//==========================================================================
//
//      imx_redboot_cmds.c
//
//      iMX specific RedBoot commands
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

#include <redboot.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/plf_mmap.h>
#include <cyg/hal/fsl_board.h>

static void runImg(int argc, char *argv[]);

RedBoot_cmd("run",
            "Run an image at a location with MMU off",
            "[<virtual addr>]",
            runImg
           );

void __attribute__((__noinline__)) launchRunImg(unsigned long addr)
{
    asm volatile ("mov r12, r0;");

    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_DISABLE();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_MMU_OFF();

    asm volatile (
                 "mov r0, #0;"
                 "mov r1, r12;"
                 "mov r11, #0;"
                 "mov r12, #0;"
                 "mrs r10, cpsr;"
                 "bic r10, r10, #0xF0000000;"
                 "msr cpsr_f, r10;"
                 "mov pc, r1"
                 );
}

extern unsigned long entry_address;

static void runImg(int argc,char *argv[])
{
    unsigned int virt_addr, phys_addr;

    // Default physical entry point for Symbian
    if (entry_address == NO_MEMORY)
        virt_addr = 0x800000;
    else
        virt_addr = entry_address;

    if (!scan_opts(argc,argv,1,0,0,(void*)&virt_addr,
                   OPTION_ARG_TYPE_NUM, "virtual address"))
        return;

    if (entry_address != NO_MEMORY)
        diag_printf("load entry_address=0x%lx\n", entry_address);

    HAL_VIRT_TO_PHYS_ADDRESS(virt_addr, phys_addr);

    diag_printf("virt_addr=0x%x\n",virt_addr);
    diag_printf("phys_addr=0x%x\n",phys_addr);

    launchRunImg(phys_addr);
}