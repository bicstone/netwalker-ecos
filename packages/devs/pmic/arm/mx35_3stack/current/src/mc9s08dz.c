//==========================================================================
//
//      mc9s08dz.c
//
//      PMIC support on i.MX35 3stack platforms
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
#include <pkgconf/devs_pmic_arm_imx35_3stack.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/fsl_board.h>
#ifdef MXC_PMIC_I2C_ENABLED
#include <cyg/io/mxc_i2c.h>
#endif                          // MXC_PMIC_I2C_ENABLED
#include <cyg/io/mc9s08dz.h>

extern unsigned int system_rev;

static void mxc_pmic_init(void)
{
    volatile unsigned int rev_id;

#ifdef MXC_PMIC_I2C_ENABLED
    if (CYGHWR_DEVS_PMIC_I2C_PORT >= i2c_num)
        return;
// 40kHz data rate
    i2c_init(i2c_base_addr[CYGHWR_DEVS_PMIC_I2C_PORT], 40000);
#else
#error "Please select a valid interface"
#endif                          // MXC_PMIC_I2C_ENABLED

    rev_id = pmic_reg(0, 0, 0);
    diag_printf("PMIC ID: 0x%08x [Rev: ", rev_id);
    switch (rev_id & 0x1F) {
    case 0x10:
        diag_printf("1.0");
        break;
    default:
        diag_printf("unknown");
        break;
    }
    diag_printf("]\n");
}

RedBoot_init(mxc_pmic_init, RedBoot_INIT_PRIO(100));

static void do_pmic(int argc, char *argv[]);
RedBoot_cmd("pmic",
            "Read/Write internal PMIC register",
            "<reg num> [value to be written]", do_pmic);

static void do_pmic(int argc, char *argv[])
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

#ifdef MXC_PMIC_I2C_ENABLED
static unsigned int pmic_reg_i2c(unsigned int reg, unsigned int val,
                                 unsigned int write)
{
    struct mxc_i2c_request rq;
    rq.dev_addr = CYGHWR_DEVS_PMIC_I2C_ADDR;
    rq.reg_addr = reg;
    rq.reg_addr_sz = 1;
    rq.buffer = (unsigned char *)&val;
    rq.buffer_sz = 1;
    write = write ? I2C_WRITE : I2C_READ;
    if (i2c_xfer(CYGHWR_DEVS_PMIC_I2C_PORT, &rq, write) != 0) {
        diag_printf("Error I2C transfer\n\n");
        return 0;
    }
    return val;
}
#endif                          //MXC_PMIC_I2C_ENABLED
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
    if (reg > MC9S08DZ_MAX_REGS) {
        diag_printf("<reg num> = %d is invalide. Should be less then 0x28\n",
                    reg);
        return 0;
    }
#ifdef MXC_PMIC_I2C_ENABLED
    return pmic_reg_i2c(reg, val, write);
#else
    return 0;
#endif                          //MXC_PMIC_I2C_ENABLED
}

static void mxc_pmic_detect(void)
{
    struct mxc_i2c_request rq;
    unsigned char buf[4] = { 0 };

    rq.dev_addr = 0x34;
    rq.reg_addr = 0x10;
    rq.reg_addr_sz = 1;
    rq.buffer = buf;
    rq.buffer_sz = 1;

    if (i2c_xfer(0, &rq, I2C_WRITE) != 0) {
        /* v2.0 board which does not have max8660 */
        system_rev |= 0x1 << 8;
        /* workaround for WDOG reset pin */
        writel(0x11, IOMUXC_BASE_ADDR + 0xC);
        diag_printf("Board version V2.0\n");
    } else {
        diag_printf("Board version V1.0\n");
    }

#ifdef CYGPKG_DEVS_ETH_FEC
    /**
     * if we have v2.0 board, need to enable
     * APLite VGEN1 regulator
     */
    if (system_rev & 0xF00) {
        /* set VGEN voltage to 3.3v */
        rq.dev_addr = 0x08;
        rq.reg_addr = 0x1E;     /* VGEN REG0 setting */
        rq.reg_addr_sz = 1;
        rq.buffer = buf;
        rq.buffer_sz = 3;
        i2c_xfer(0, &rq, I2C_READ);
        rq.buffer_sz = 3;
        buf[2] |= 0x3;
        i2c_xfer(0, &rq, I2C_WRITE);
        /* enable FEC 3v3 */
        rq.dev_addr = 0x08;
        rq.reg_addr = 0x20;     /* VGEN REG0 */
        rq.reg_addr_sz = 1;
        rq.buffer = buf;
        rq.buffer_sz = 3;
        i2c_xfer(0, &rq, I2C_READ);
        rq.buffer_sz = 3;
        buf[2] |= 0x1;
        i2c_xfer(0, &rq, I2C_WRITE);
    }
#endif

}

RedBoot_init(mxc_pmic_detect, RedBoot_INIT_PRIO(101));
