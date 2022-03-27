//==========================================================================
//
//      mc34704.c
//
//      PMIC support on i.MX25 3stack platforms
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
#include <stdlib.h>
#include <pkgconf/hal.h>
#include <pkgconf/devs_pmic_arm_imx25_3stack.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/fsl_board.h>
#include <cyg/io/mxc_i2c.h>
#include <cyg/io/mc34704.h>

#define MC34704_REG_MAX 0x59

static void mxc_pmic_init(void)
{
	if (CYGHWR_DEVS_PMIC_I2C_PORT >= i2c_num) 
		return;

	i2c_init(i2c_base_addr[CYGHWR_DEVS_PMIC_I2C_PORT], 40000); 
	
	diag_printf("Turning on PMIC regulators: 1,2,3,4,5\n");

	pmic_reg(0x02, 0x09, 1);
}

RedBoot_init(mxc_pmic_init, RedBoot_INIT_PRIO(100));

static void do_pmic(int argc, char *argv[]);
RedBoot_cmd("pmic",
	    "Read/Write internal PMIC register",
	    "<reg num> [value to be written]",
	    do_pmic);

static void do_pmic(int argc,char *argv[])
{
	unsigned int reg, temp, val = 0, write = 0;

	if (argc == 1) {
		diag_printf("\tRead:  pmic <reg num>\n");
		diag_printf("\tWrite: pmic <reg num> <value to be written>\n");
		return;
	}

	if (!parse_num(*(&argv[1]), (unsigned long *)&reg, &argv[1], ":")) {
		diag_printf("Error: Invalid parameter\n");
		return;
	}

	if (argc == 3) {
		if (!parse_num(*(&argv[2]), (unsigned long *)&val, &argv[2], ":")) {
			diag_printf("Error: Invalid parameter\n");
			return;
		}
		write = 1;
	}

	temp = pmic_reg(reg, val, write);

	diag_printf("\tval: 0x%08x\n\n", temp);
}

static unsigned int pmic_reg_i2c(unsigned int reg, unsigned int val, unsigned int write)
{
	struct mxc_i2c_request rq;
	rq.dev_addr = CYGHWR_DEVS_PMIC_I2C_ADDR;
	rq.reg_addr = reg;
	rq.reg_addr_sz = 1;
	rq.buffer = (unsigned char *)&val;
	rq.buffer_sz = 1;
	write =  write ? I2C_WRITE : I2C_READ;
	if (i2c_xfer(CYGHWR_DEVS_PMIC_I2C_PORT, &rq, write) != 0) {
		diag_printf("Error in I2C transaction\n\n");
		return 0;
    	}
	return val;	
}

/*!
 * To read/write to a PMIC register. For write, it does another read for the
 * actual register value.
 *
 * @param   reg         register number inside the PMIC
 * @param   val         data to be written to the register; don't care for read
 * @param   write       0 for read; 1 for write
 *
 * @return              the actual data in the PMIC register
 */
unsigned int pmic_reg(unsigned int reg, unsigned int val, unsigned int write)
{
	if (reg > MC34704_REG_MAX) {
		diag_printf("<reg num> = %d is invalid. Should be less than %d\n",
			reg, MC34704_REG_MAX);
		return 0;
	}
	return pmic_reg_i2c(reg, val, write);
}
