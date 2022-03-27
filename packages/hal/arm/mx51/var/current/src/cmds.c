//==========================================================================
//
//      cmds.c
//
//      SoC [platform] specific RedBoot commands
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
//
// modification information
// ------------------------
// 2009/07/27 : from redboot_200925
//
//==========================================================================
#include <redboot.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/plf_mmap.h>
#include <cyg/hal/hal_soc.h>         // Hardware definitions
#include <cyg/hal/hal_cache.h>

int gcd(int m, int n);

typedef unsigned long long  u64;
typedef unsigned int        u32;
typedef unsigned short      u16;
typedef unsigned char       u8;

#define SZ_DEC_1M       1000000
#define PLL_PD_MAX      16      //actual pd+1
#define PLL_MFI_MAX     15
#define PLL_MFI_MIN     5
#define ARM_DIV_MAX     8
#define IPG_DIV_MAX     4
#define AHB_DIV_MAX     8
#define EMI_DIV_MAX     8
#define NFC_DIV_MAX     8

#define REF_IN_CLK_NUM  4
struct fixed_pll_mfd {
    u32 ref_clk_hz;
    u32 mfd;
};
const struct fixed_pll_mfd fixed_mfd[REF_IN_CLK_NUM] = {
    {0,                   0},      // reserved
    {0,                   0},      // reserved
    {FREQ_24MHZ,          24 * 16},    // 384
    {0,                   0},      // reserved
};

struct pll_param {
    u32 pd;
    u32 mfi;
    u32 mfn;
    u32 mfd;
};

#define PLL_FREQ_MAX(_ref_clk_)    (4 * _ref_clk_ * PLL_MFI_MAX)
#define PLL_FREQ_MIN(_ref_clk_)    ((2 * _ref_clk_ * (PLL_MFI_MIN - 1)) / PLL_PD_MAX)
#define MAX_DDR_CLK      220000000
#define AHB_CLK_MAX     133333333
#define IPG_CLK_MAX     (AHB_CLK_MAX / 2)
#define NFC_CLK_MAX     25000000
// IPU-HSP clock is independent of the HCLK and can go up to 177MHz but requires
// higher voltage support. For simplicity, limit it to 133MHz
#define HSP_CLK_MAX     133333333

#define ERR_WRONG_CLK   -1
#define ERR_NO_MFI      -2
#define ERR_NO_MFN      -3
#define ERR_NO_PD       -4
#define ERR_NO_PRESC    -5
#define ERR_NO_AHB_DIV  -6

u32 pll_clock(enum plls pll);
u32 get_main_clock(enum main_clocks clk);
u32 get_peri_clock(enum peri_clocks clk);

static volatile u32 *pll_base[] =
{
    REG32_PTR(PLL1_BASE_ADDR),
    REG32_PTR(PLL2_BASE_ADDR),
    REG32_PTR(PLL3_BASE_ADDR),
};

#define NOT_ON_VAL  0xDEADBEEF

static void clock_setup(int argc, char *argv[]);

RedBoot_cmd("clock",
            "Setup/Display clock\nSyntax:",
            "[<core clock in MHz> :<DDR clock in MHz>] \n\n\
   Examples:\n\
   [clock]         -> Show various clocks\n\
   [clock 665]     -> Core=665  \n\
   [clock 800:133]  -> Core=800  DDR=133 \n\
   [clock :166]   -> Core=no change  DDR=166 \n",
            clock_setup
           );

/*!
 * This is to calculate various parameters based on reference clock and
 * targeted clock based on the equation:
 *      t_clk = 2*ref_freq*(mfi + mfn/(mfd+1))/(pd+1)
 * This calculation is based on a fixed MFD value for simplicity.
 *
 * @param ref       reference clock freq in Hz
 * @param target    targeted clock in Hz
 * @param p_pd      calculated pd value (pd value from register + 1) upon return
 * @param p_mfi     calculated actual mfi value upon return
 * @param p_mfn     calculated actual mfn value upon return
 * @param p_mfd     fixed mfd value (mfd value from register + 1) upon return
 *
 * @return          0 if successful; non-zero otherwise.
 */
int calc_pll_params(u32 ref, u32 target, struct pll_param *pll)
{
    u64 pd, mfi = 1, mfn, mfd, n_target = target, n_ref = ref, i;

    // make sure targeted freq is in the valid range. Otherwise the
    // following calculation might be wrong!!!
    if (n_target < PLL_FREQ_MIN(ref) || n_target > PLL_FREQ_MAX(ref))
        return ERR_WRONG_CLK;
    for (i = 0; ; i++) {
        if (i == REF_IN_CLK_NUM)
            return ERR_WRONG_CLK;
        if (fixed_mfd[i].ref_clk_hz == ref) {
            mfd = fixed_mfd[i].mfd;
            break;
        }
    }

    // Use n_target and n_ref to avoid overflow
    for (pd = 1; pd <= PLL_PD_MAX; pd++) {
        mfi = (n_target * pd) / (4 * n_ref);
        if (mfi > PLL_MFI_MAX) {
            return ERR_NO_MFI;
        } else if (mfi < 5) {
            continue;
        }
        break;
    }
    // Now got pd and mfi already
    mfn = (((n_target * pd) / 4 - n_ref * mfi) * mfd) / n_ref;
#ifdef CMD_CLOCK_DEBUG
    diag_printf("%d: ref=%d, target=%d, pd=%d, mfi=%d,mfn=%d, mfd=%d\n",
                __LINE__, ref, (u32)n_target, (u32)pd, (u32)mfi, (u32)mfn, (u32)mfd);
#endif
    i = 1;
    if (mfn != 0)
        i = gcd(mfd, mfn);
    pll->pd = (u32)pd;
    pll->mfi = (u32)mfi;
    pll->mfn = (u32)(mfn / i);
    pll->mfd = (u32)(mfd / i);
    return 0;
}

/*!
 * This function returns the low power audio clock.
 */
u32 get_lp_apm(void)
{
    u32 ret_val = 0;
    u32 ccsr = readl(CCM_BASE_ADDR + CLKCTL_CCSR);

    if (((ccsr >> 9) & 1) == 0) {
        ret_val = FREQ_24MHZ;
    } else {
        ret_val = FREQ_32768HZ;
    }
    return ret_val;
}

/*!
 * This function returns the periph_clk.
 */
u32 get_periph_clk(void)
{
    u32 ret_val = 0, clk_sel;

    u32 cbcdr = readl(CCM_BASE_ADDR + CLKCTL_CBCDR);
    u32 cbcmr = readl(CCM_BASE_ADDR + CLKCTL_CBCMR);

    if (((cbcdr >> 25) & 1) == 0) {
        ret_val = pll_clock(PLL2);
    } else {
        clk_sel = (cbcmr >> 12) & 3;
        if (clk_sel == 0) {
            ret_val = pll_clock(PLL1);
        } else if (clk_sel == 1) {
            ret_val = pll_clock(PLL3);
        } else if (clk_sel == 2) {
            ret_val = get_lp_apm();
        }
    }
    return ret_val;
}

/*!
 * This function assumes the expected core clock has to be changed by
 * modifying the PLL. This is NOT true always but for most of the times,
 * it is. So it assumes the PLL output freq is the same as the expected
 * core clock (presc=1) unless the core clock is less than PLL_FREQ_MIN.
 * In the latter case, it will try to increase the presc value until
 * (presc*core_clk) is greater than PLL_FREQ_MIN. It then makes call to
 * calc_pll_params() and obtains the values of PD, MFI,MFN, MFD based
 * on the targeted PLL and reference input clock to the PLL. Lastly,
 * it sets the register based on these values along with the dividers.
 * Note 1) There is no value checking for the passed-in divider values
 *         so the caller has to make sure those values are sensible.
 *      2) Also adjust the NFC divider such that the NFC clock doesn't
 *         exceed NFC_CLK_MAX.
 *      3) IPU HSP clock is independent of AHB clock. Even it can go up to
 *         177MHz for higher voltage, this function fixes the max to 133MHz.
 *      4) This function should not have allowed diag_printf() calls since
 *         the serial driver has been stoped. But leave then here to allow
 *         easy debugging by NOT calling the cyg_hal_plf_serial_stop().
 *
 * @param ref       pll input reference clock (24MHz)
 * @param core_clk  core clock in Hz
 * @param emi_clk   emi clock in Hz
 # @return          0 if successful; non-zero otherwise
 */
int configure_clock(u32 ref, u32 core_clk, u32 emi_clk)
{

    u32 pll, clk_src;
    struct pll_param pll_param;
#if 1	// redboot_200925
    int ret, clk_sel, div = 1, shift = 0;
#else
    int ret, clk_sel, div = 1, div_core = 1, div_per = 1, shift = 0;
    u32 icgc = readl(PLATFORM_BASE_ADDR + PLATFORM_ICGC);
#endif
    u32 cbcdr = readl(CCM_BASE_ADDR + CLKCTL_CBCDR);
    u32 cbcmr = readl(CCM_BASE_ADDR + CLKCTL_CBCMR);
    u32 ccsr = readl(CCM_BASE_ADDR + CLKCTL_CCSR);

    if (core_clk != 0) {
        // assume pll default to core clock first
        pll = core_clk;
        if ((ret = calc_pll_params(ref, pll, &pll_param)) != 0) {
             diag_printf("can't find pll parameters: %d\n", ret);
             return ret;
        }
#ifdef CMD_CLOCK_DEBUG
        diag_printf("ref=%d, pll=%d, pd=%d, mfi=%d,mfn=%d, mfd=%d\n",
                    ref, pll, pll_param.pd, pll_param.mfi, pll_param.mfn, pll_param.mfd);
#endif

        /* Applies for TO 2 only */
        if (((cbcdr >> 30) & 0x1) == 0x1) {
            /* Disable IPU and HSC dividers */
            writel(0x60000, CCM_BASE_ADDR + CLKCTL_CCDR);
            /* Switch DDR to different source */
            writel(cbcdr & ~0x40000000, CCM_BASE_ADDR + CLKCTL_CBCDR);
            while (readl(CCM_BASE_ADDR + CLKCTL_CDHIPR) != 0);
            writel(0x0, CCM_BASE_ADDR + CLKCTL_CCDR);
        }

        /* Switch ARM to PLL2 clock */
        writel(ccsr | 0x4, CCM_BASE_ADDR + CLKCTL_CCSR);

#if 1 // redboot_200925
        if (core_clk > 800000000) {
            increase_core_voltage(true);
        } else {
            increase_core_voltage(false);
        }
#else
        if ((core_clk > 665000000) && (core_clk <= 800000000)) {
            div_per = 5;
        } else if ((core_clk > 800000000) && (core_clk <= 900000000)) {
            div_per = 6;
        } else if (core_clk > 900000000) {
            div_per = 7;
        } else {
            div_per = 4;
        }

        if (core_clk > 800000000) {
            div_core = 3;
            increase_core_voltage(true);
        } else {
            div_core = 2;
            increase_core_voltage(false);
        }

        icgc &= ~(0x77);
        icgc |= (div_core << 4);
        icgc |= div_per;
        /* Set the platform clock dividers */
        writel(icgc, PLATFORM_BASE_ADDR + PLATFORM_ICGC);
#endif

        // adjust pll settings
        writel(((pll_param.pd - 1) << 0) | (pll_param.mfi << 4),
                   PLL1_BASE_ADDR + PLL_DP_OP);
        writel(pll_param.mfn, PLL1_BASE_ADDR + PLL_DP_MFN);
        writel(pll_param.mfd - 1, PLL1_BASE_ADDR + PLL_DP_MFD);
        writel(((pll_param.pd - 1) << 0) | (pll_param.mfi << 4),
               PLL1_BASE_ADDR + PLL_DP_HFS_OP);
        writel(pll_param.mfn, PLL1_BASE_ADDR + PLL_DP_HFS_MFN);
        writel(pll_param.mfd - 1, PLL1_BASE_ADDR + PLL_DP_HFS_MFD);

        /* Switch ARM back to PLL1 */
        writel((ccsr & ~0x4), CCM_BASE_ADDR + CLKCTL_CCSR);
        /* Applies for TO 2 only */
        if (((cbcdr >> 30) & 0x1) == 0x1) {
            /* Disable IPU and HSC dividers */
            writel(0x60000, CCM_BASE_ADDR + CLKCTL_CCDR);
            /* Switch DDR back to PLL1 */
            writel(cbcdr | 0x40000000, CCM_BASE_ADDR + CLKCTL_CBCDR);
            while (readl(CCM_BASE_ADDR + CLKCTL_CDHIPR) != 0);
            writel(0x0, CCM_BASE_ADDR + CLKCTL_CCDR);
            if (emi_clk == 0) {
                /* Keep EMI clock to the max if not specified */
                emi_clk = 200000000;
            }
        }
    }

    if (emi_clk != 0) {
        /* Applies for TO 2 only */
        if (((cbcdr >> 30) & 0x1) == 0x1) {
            clk_src = pll_clock(PLL1);
            shift = 27;
        } else {
            clk_src = get_periph_clk();
            /* Find DDR clock input */
            clk_sel = (cbcmr >> 10) & 0x3;
            if (clk_sel == 0) {
                shift = 16;
            } else if (clk_sel == 1) {
                shift = 19;
            } else if (clk_sel == 2) {
                shift = 22;
            } else if (clk_sel == 3) {
                shift = 10;
            }
        }
        if ((clk_src % emi_clk) == 0)
            div = clk_src / emi_clk;
        else
            div = (clk_src / emi_clk) + 1;
        if (div > 8)
            div = 8;

        cbcdr = cbcdr & ~(0x7 << shift);
        cbcdr |= ((div - 1) << shift);
        /* Disable IPU and HSC dividers */
        writel(0x60000, CCM_BASE_ADDR + CLKCTL_CCDR);
        writel(cbcdr, CCM_BASE_ADDR + CLKCTL_CBCDR);
        while (readl(CCM_BASE_ADDR + CLKCTL_CDHIPR) != 0);
        writel(0x0, CCM_BASE_ADDR + CLKCTL_CCDR);
    }
    return 0;
}

static void clock_setup(int argc,char *argv[])
{

    u32 i, core_clk, ddr_clk, data[3];
    unsigned long temp;
    int ret;

    if (argc == 1)
        goto print_clock;

    for (i = 0;  i < 2;  i++) {
        if (!parse_num(*(&argv[1]), &temp, &argv[1], ":")) {
            diag_printf("Error: Invalid parameter\n");
            return;
        }
        data[i] = temp;
    }

    core_clk = data[0] * SZ_DEC_1M;
    ddr_clk = data[1] *  SZ_DEC_1M;

    if (core_clk != 0) {
        if ((core_clk < PLL_FREQ_MIN(PLL_REF_CLK)) || (core_clk > PLL_FREQ_MAX(PLL_REF_CLK))) {
            diag_printf("Targeted core clock should be within [%d - %d]\n",
                            PLL_FREQ_MIN(PLL_REF_CLK), PLL_FREQ_MAX(PLL_REF_CLK));
            return;
        }
    }

    if (ddr_clk != 0) {
        if (ddr_clk > MAX_DDR_CLK) {
            diag_printf("DDR clock should be less than %d MHz, assuming max value \n", (MAX_DDR_CLK / SZ_DEC_1M));
            ddr_clk = MAX_DDR_CLK;
        }
    }

    // stop the serial to be ready to adjust the clock
    hal_delay_us(100000);
    cyg_hal_plf_serial_stop();
    // adjust the clock
    ret = configure_clock(PLL_REF_CLK, core_clk, ddr_clk);
    // restart the serial driver
    cyg_hal_plf_serial_init();
    hal_delay_us(100000);

    if (ret != 0) {
        diag_printf("Failed to setup clock: %d\n", ret);
        return;
    }
    diag_printf("\n<<<New clock setting>>>\n");

    // Now printing clocks
print_clock:

    diag_printf("\nPLL1\t\tPLL2\t\tPLL3\n");
    diag_printf("========================================\n");
    diag_printf("%-16d%-16d%-16d\n\n", pll_clock(PLL1), pll_clock(PLL2),
                pll_clock(PLL3));
    diag_printf("CPU\t\tAHB\t\tIPG\t\tEMI_CLK\n");
    diag_printf("========================================================\n");
    diag_printf("%-16d%-16d%-16d%-16d\n\n",
                get_main_clock(CPU_CLK),
                get_main_clock(AHB_CLK),
                get_main_clock(IPG_CLK),
                get_main_clock(DDR_CLK));

    diag_printf("NFC\t\tUSB\t\tIPG_PER_CLK\n");
    diag_printf("========================================\n");
    diag_printf("%-16d%-16d%-16d\n\n",
                get_main_clock(NFC_CLK),
                get_main_clock(USB_CLK),
                get_main_clock(IPG_PER_CLK));

    diag_printf("UART1-3\t\tSSI1\t\tSSI2\t\tSPI\n");
    diag_printf("===========================================");
    diag_printf("=============\n");

    diag_printf("%-16d%-16d%-16d%-16d\n\n",
                get_peri_clock(UART1_BAUD),
                get_peri_clock(SSI1_BAUD),
                get_peri_clock(SSI2_BAUD),
                get_peri_clock(SPI1_CLK));

#if 0
    diag_printf("IPG_PERCLK as baud clock for: UART1-5, I2C, OWIRE, SDHC");
    if (((readl(EPIT1_BASE_ADDR) >> 24) & 0x3) == 0x2) {
        diag_printf(", EPIT");
    }
    if (((readl(GPT1_BASE_ADDR) >> 6) & 0x7) == 0x2) {
        diag_printf("GPT,");
    }
#endif
    diag_printf("\n");

}

/*!
 * This function returns the PLL output value in Hz based on pll.
 */
u32 pll_clock(enum plls pll)
{
    u64 mfi, mfn, mfd, pdf, ref_clk, pll_out, sign;
    u64 dp_ctrl, dp_op, dp_mfd, dp_mfn, clk_sel;
    u8 dbl = 0;

    dp_ctrl = pll_base[pll][PLL_DP_CTL >> 2];
    clk_sel = MXC_GET_FIELD(dp_ctrl, 2, 8);
    ref_clk = fixed_mfd[clk_sel].ref_clk_hz;

    if ((pll_base[pll][PLL_DP_CTL >> 2] & 0x80) == 0) {
        dp_op = pll_base[pll][PLL_DP_OP >> 2];
        dp_mfd = pll_base[pll][PLL_DP_MFD >> 2];
        dp_mfn = pll_base[pll][PLL_DP_MFN >> 2];
    } else {
        dp_op = pll_base[pll][PLL_DP_HFS_OP >> 2];
        dp_mfd = pll_base[pll][PLL_DP_HFS_MFD >> 2];
        dp_mfn = pll_base[pll][PLL_DP_HFS_MFN >> 2];
    }
    pdf = dp_op & 0xF;
    mfi = (dp_op >> 4) & 0xF;
    mfi = (mfi <= 5) ? 5: mfi;
    mfd = dp_mfd & 0x07FFFFFF;
    mfn = dp_mfn & 0x07FFFFFF;

    sign = (mfn < 0x4000000) ? 0: 1;
    mfn = (mfn <= 0x4000000) ? mfn: (0x8000000 - mfn);

    dbl = ((dp_ctrl >> 12) & 0x1) + 1;

    dbl = dbl * 2;
    if (sign == 0) {
        pll_out = (dbl * ref_clk * mfi + ((dbl * ref_clk * mfn) / (mfd + 1))) /
                  (pdf + 1);
    } else {
        pll_out = (dbl * ref_clk * mfi - ((dbl * ref_clk * mfn) / (mfd + 1))) /
                  (pdf + 1);
    }

    return (u32)pll_out;
}

/*!
 * This function returns the emi_core_clk_root clock.
 */
u32 get_emi_core_clk(void)
{
    u32 cbcdr = readl(CCM_BASE_ADDR + CLKCTL_CBCDR);
    u32 clk_sel = 0, max_pdf = 0, peri_clk = 0, ahb_clk = 0;
    u32 ret_val = 0;

    max_pdf = (cbcdr >> 10) & 0x7;
    peri_clk = get_periph_clk();
    ahb_clk = peri_clk / (max_pdf + 1);

    clk_sel = (cbcdr >> 26) & 1;
    if (clk_sel == 0) {
        ret_val = peri_clk;
    } else {
        ret_val = ahb_clk ;
    }
    return ret_val;
}

/*!
 * This function returns the main clock value in Hz.
 */
u32 get_main_clock(enum main_clocks clk)
{
    u32 pdf, max_pdf, ipg_pdf, nfc_pdf, clk_sel;
    u32 pll, ret_val = 0;
    u32 cacrr = readl(CCM_BASE_ADDR + CLKCTL_CACRR);
    u32 cbcdr = readl(CCM_BASE_ADDR + CLKCTL_CBCDR);
    u32 cbcmr = readl(CCM_BASE_ADDR + CLKCTL_CBCMR);
    u32 cscmr1 = readl(CCM_BASE_ADDR + CLKCTL_CSCMR1);
    u32 cscdr1 = readl(CCM_BASE_ADDR + CLKCTL_CSCDR1);

    switch (clk) {
    case CPU_CLK:
        pdf = cacrr & 0x7;
        pll = pll_clock(PLL1);
        ret_val = pll / (pdf + 1);
        break;
    case AHB_CLK:
        max_pdf = (cbcdr >> 10) & 0x7;
        pll = get_periph_clk();
        ret_val = pll / (max_pdf + 1);
        break;
    case IPG_CLK:
        max_pdf = (cbcdr >> 10) & 0x7;
        ipg_pdf = (cbcdr >> 8) & 0x3;
        pll = get_periph_clk();
        ret_val = pll / ((max_pdf + 1) * (ipg_pdf + 1));
        break;
    case IPG_PER_CLK:
       clk_sel = cbcmr & 1;
       if (clk_sel == 0) {
           clk_sel = (cbcmr >> 1) & 1;
           pdf = (((cbcdr >> 6) & 3) + 1) * (((cbcdr >> 3) & 7) + 1) * ((cbcdr & 7) + 1);
           if (clk_sel == 0) {
               ret_val = get_periph_clk() / pdf;
           } else {
               ret_val = get_lp_apm();
           }
       } else {
           /* Same as IPG_CLK */
           max_pdf = (cbcdr >> 10) & 0x7;
           ipg_pdf = (cbcdr >> 8) & 0x3;
           pll = get_periph_clk();
           ret_val = pll / ((max_pdf + 1) * (ipg_pdf + 1));
       }
       break;
    case DDR_CLK:
        if (((cbcdr >> 30) & 0x1) == 0x1) {
            pll = pll_clock(PLL1);
            pdf = (cbcdr >> 27) & 0x7;
        } else {
            clk_sel = (cbcmr >> 10) & 3;
            pll = get_periph_clk();
            if (clk_sel == 0) {
                /* AXI A */
                pdf = (cbcdr >> 16) & 0x7;
            } else if (clk_sel == 1) {
                /* AXI B */
                pdf = (cbcdr >> 19) & 0x7;
            } else if (clk_sel == 2) {
                /* EMI SLOW CLOCK ROOT */
                pll = get_emi_core_clk();
                pdf = (cbcdr >> 22) & 0x7;
            } else if (clk_sel == 3) {
                /* AHB CLOCK */
                pdf = (cbcdr >> 10) & 0x7;
            }
        }

        ret_val = pll / (pdf + 1);
        break;
    case NFC_CLK:
        pdf = (cbcdr >> 22) & 0x7;
        nfc_pdf = (cbcdr >> 13) & 0x7;
        pll = get_emi_core_clk();
        ret_val = pll / ((pdf + 1) * (nfc_pdf + 1));
        break;
    case USB_CLK:
        clk_sel = (cscmr1 >> 22) & 3;
        if (clk_sel == 0) {
            pll = pll_clock(PLL1);
        } else if (clk_sel == 1) {
            pll = pll_clock(PLL2);
        } else if (clk_sel == 2) {
            pll = pll_clock(PLL3);
        } else if (clk_sel == 3) {
            pll = get_lp_apm();
        }
        pdf = (cscdr1 >> 8) & 0x7;
        max_pdf = (cscdr1 >> 6) & 0x3;
        ret_val = pll / ((pdf + 1) * (max_pdf + 1));
        break;
    default:
        diag_printf("Unknown clock: %d\n", clk);
        break;
    }

    return ret_val;
}

/*!
 * This function returns the peripheral clock value in Hz.
 */
u32 get_peri_clock(enum peri_clocks clk)
{
    u32 ret_val = 0, pdf, pre_pdf, clk_sel;
    u32 cscmr1 = readl(CCM_BASE_ADDR + CLKCTL_CSCMR1);
    u32 cscdr1 = readl(CCM_BASE_ADDR + CLKCTL_CSCDR1);
    u32 cscdr2 = readl(CCM_BASE_ADDR + CLKCTL_CSCDR2);
    u32 cs1cdr = readl(CCM_BASE_ADDR + CLKCTL_CS1CDR);
    u32 cs2cdr = readl(CCM_BASE_ADDR + CLKCTL_CS2CDR);

    switch (clk) {
    case UART1_BAUD:
    case UART2_BAUD:
    case UART3_BAUD:
        pre_pdf = (cscdr1 >> 3) & 0x7;
        pdf = cscdr1 & 0x7;
        clk_sel = (cscmr1 >> 24) & 3;
        if (clk_sel == 0) {
            ret_val = pll_clock(PLL1) / ((pre_pdf + 1) * (pdf + 1));
        } else if (clk_sel == 1) {
            ret_val = pll_clock(PLL2) / ((pre_pdf + 1) * (pdf + 1));
        } else if (clk_sel == 2) {
            ret_val = pll_clock(PLL3) / ((pre_pdf + 1) * (pdf + 1));
        } else {
            ret_val = get_lp_apm() / ((pre_pdf + 1) * (pdf + 1));
        }
        break;
    case SSI1_BAUD:
        pre_pdf = (cs1cdr >> 6) & 0x7;
        pdf = cs1cdr & 0x3F;
        clk_sel = (cscmr1 >> 14) & 3;
        if (clk_sel == 0) {
            ret_val = pll_clock(PLL1) / ((pre_pdf + 1) * (pdf + 1));
        } else if (clk_sel == 0x1) {
            ret_val = pll_clock(PLL2) / ((pre_pdf + 1) * (pdf + 1));
        } else if (clk_sel == 0x2) {
            ret_val = pll_clock(PLL3) / ((pre_pdf + 1) * (pdf + 1));
        } else {
            ret_val = CKIH /((pre_pdf + 1) * (pdf + 1));
        }
        break;
    case SSI2_BAUD:
        pre_pdf = (cs2cdr >> 6) & 0x7;
        pdf = cs2cdr & 0x3F;
        clk_sel = (cscmr1 >> 12) & 3;
        if (clk_sel == 0) {
            ret_val = pll_clock(PLL1) / ((pre_pdf + 1) * (pdf + 1));
        } else if (clk_sel == 0x1) {
            ret_val = pll_clock(PLL2) / ((pre_pdf + 1) * (pdf + 1));
        } else if (clk_sel == 0x2) {
            ret_val = pll_clock(PLL3) / ((pre_pdf + 1) * (pdf + 1));
        } else {
            ret_val = CKIH /((pre_pdf + 1) * (pdf + 1));
        }
        break;
    case SPI1_CLK:
    case SPI2_CLK:
        pre_pdf = (cscdr2 >> 25) & 0x7;
        pdf = (cscdr2 >> 19) & 0x3F;
        clk_sel = (cscmr1 >> 4) & 3;
        if (clk_sel == 0) {
            ret_val = pll_clock(PLL1) / ((pre_pdf + 1) * (pdf + 1));
        } else if (clk_sel == 1) {
            ret_val = pll_clock(PLL2) / ((pre_pdf + 1) * (pdf + 1));
        } else if (clk_sel == 2) {
            ret_val = pll_clock(PLL3) / ((pre_pdf + 1) * (pdf + 1));
        } else {
            ret_val = get_lp_apm() / ((pre_pdf + 1) * (pdf + 1));
        }
        break;
    default:
        diag_printf("%s(): This clock: %d not supported yet \n",
                    __FUNCTION__, clk);
        break;
    }

    return ret_val;
}

#ifdef L2CC_ENABLED
/*
 * This command is added for some simple testing only. It turns on/off
 * L2 cache regardless of L1 cache state. The side effect of this is
 * when doing any flash operations such as "fis init", the L2
 * will be turned back on along with L1 caches even though it is off
 * by using this command.
 */
RedBoot_cmd("L2",
            "L2 cache",
            "[ON | OFF]",
            do_L2_caches
           );

void do_L2_caches(int argc, char *argv[])
{
    u32 oldints;
    int L2cache_on=0;

    if (argc == 2) {
        if (strcasecmp(argv[1], "on") == 0) {
            HAL_DISABLE_INTERRUPTS(oldints);
            HAL_ENABLE_L2();
            HAL_RESTORE_INTERRUPTS(oldints);
        } else if (strcasecmp(argv[1], "off") == 0) {
            HAL_DISABLE_INTERRUPTS(oldints);
            HAL_DCACHE_DISABLE_C1();
            HAL_CACHE_FLUSH_ALL();
            HAL_DISABLE_L2();
            HAL_DCACHE_ENABLE_L1();
            HAL_RESTORE_INTERRUPTS(oldints);
        } else {
            diag_printf("Invalid L2 cache mode: %s\n", argv[1]);
        }
    } else {
        HAL_L2CACHE_IS_ENABLED(L2cache_on);
        diag_printf("L2 cache: %s\n", L2cache_on?"On":"Off");
    }
}
#endif //L2CC_ENABLED

#define IIM_ERR_SHIFT       8
#define POLL_FUSE_PRGD      (IIM_STAT_PRGD | (IIM_ERR_PRGE << IIM_ERR_SHIFT))
#define POLL_FUSE_SNSD      (IIM_STAT_SNSD | (IIM_ERR_SNSE << IIM_ERR_SHIFT))

static void fuse_op_start(void)
{
    /* Do not generate interrupt */
    writel(0, IIM_BASE_ADDR + IIM_STATM_OFF);
    // clear the status bits and error bits
    writel(0x3, IIM_BASE_ADDR + IIM_STAT_OFF);
    writel(0xFE, IIM_BASE_ADDR + IIM_ERR_OFF);
}

/*
 * The action should be either:
 *          POLL_FUSE_PRGD
 * or:
 *          POLL_FUSE_SNSD
 */
static int poll_fuse_op_done(int action)
{

    u32 status, error;

    if (action != POLL_FUSE_PRGD && action != POLL_FUSE_SNSD) {
        diag_printf("%s(%d) invalid operation\n", __FUNCTION__, action);
        return -1;
    }

    /* Poll busy bit till it is NOT set */
    while ((readl(IIM_BASE_ADDR + IIM_STAT_OFF) & IIM_STAT_BUSY) != 0 ) {
    }

    /* Test for successful write */
    status = readl(IIM_BASE_ADDR + IIM_STAT_OFF);
    error = readl(IIM_BASE_ADDR + IIM_ERR_OFF);

    if ((status & action) != 0 && (error & (action >> IIM_ERR_SHIFT)) == 0) {
        if (error) {
            diag_printf("Even though the operation seems successful...\n");
            diag_printf("There are some error(s) at addr=0x%x: 0x%x\n",
                        (IIM_BASE_ADDR + IIM_ERR_OFF), error);
        }
        return 0;
    }
    diag_printf("%s(%d) failed\n", __FUNCTION__, action);
    diag_printf("status address=0x%x, value=0x%x\n",
                (IIM_BASE_ADDR + IIM_STAT_OFF), status);
    diag_printf("There are some error(s) at addr=0x%x: 0x%x\n",
                (IIM_BASE_ADDR + IIM_ERR_OFF), error);
    return -1;
}

static unsigned int sense_fuse(int bank, int row, int bit)
{
    int addr, addr_l, addr_h, reg_addr;

    fuse_op_start();

    addr = ((bank << 11) | (row << 3) | (bit & 0x7));
    /* Set IIM Program Upper Address */
    addr_h = (addr >> 8) & 0x000000FF;
    /* Set IIM Program Lower Address */
    addr_l = (addr & 0x000000FF);

#ifdef IIM_FUSE_DEBUG
    diag_printf("%s: addr_h=0x%x, addr_l=0x%x\n",
                __FUNCTION__, addr_h, addr_l);
#endif
    writel(addr_h, IIM_BASE_ADDR + IIM_UA_OFF);
    writel(addr_l, IIM_BASE_ADDR + IIM_LA_OFF);
    /* Start sensing */
    writel(0x8, IIM_BASE_ADDR + IIM_FCTL_OFF);
    if (poll_fuse_op_done(POLL_FUSE_SNSD) != 0) {
        diag_printf("%s(bank: %d, row: %d, bit: %d failed\n",
                    __FUNCTION__, bank, row, bit);
    }
    reg_addr = IIM_BASE_ADDR + IIM_SDAT_OFF;
    return readl(reg_addr);
}

void do_fuse_read(int argc, char *argv[])
{
    unsigned long bank, row;
    unsigned int fuse_val;

    if (argc == 1) {
        diag_printf("Useage: fuse_read <bank> <row>\n");
        return;
    } else if (argc == 3) {
        if (!parse_num(*(&argv[1]), &bank, &argv[1], " ")) {
                diag_printf("Error: Invalid parameter\n");
            return;
        }
        if (!parse_num(*(&argv[2]), &row, &argv[2], " ")) {
                diag_printf("Error: Invalid parameter\n");
                return;
            }

        diag_printf("Read fuse at bank:%ld row:%ld\n", bank, row);
        fuse_val = sense_fuse(bank, row, 0);
        diag_printf("fuses at (bank:%d, row:%d) = 0x%x\n", bank, row, fuse_val);
    } else {
        diag_printf("Passing in wrong arguments: %d\n", argc);
        diag_printf("Useage: fuse_read <bank> <row>\n");
    }
}

/* Blow fuses based on the bank, row and bit positions (all 0-based)
*/
int fuse_blow(int bank,int row,int bit)
{
    int addr, addr_l, addr_h, ret = -1;

    fuse_op_start();

    /* Disable IIM Program Protect */
    writel(0xAA, IIM_BASE_ADDR + IIM_PREG_P_OFF);

    addr = ((bank << 11) | (row << 3) | (bit & 0x7));
    /* Set IIM Program Upper Address */
    addr_h = (addr >> 8) & 0x000000FF;
    /* Set IIM Program Lower Address */
    addr_l = (addr & 0x000000FF);

#ifdef IIM_FUSE_DEBUG
    diag_printf("blowing addr_h=0x%x, addr_l=0x%x\n", addr_h, addr_l);
#endif

    writel(addr_h, IIM_BASE_ADDR + IIM_UA_OFF);
    writel(addr_l, IIM_BASE_ADDR + IIM_LA_OFF);
    /* Start Programming */
    writel(0x31, IIM_BASE_ADDR + IIM_FCTL_OFF);
    if (poll_fuse_op_done(POLL_FUSE_PRGD) == 0) {
        ret = 0;
    }

    /* Enable IIM Program Protect */
    writel(0x0, IIM_BASE_ADDR + IIM_PREG_P_OFF);
    return ret;
}

/*
 * This command is added for burning IIM fuses
 */
RedBoot_cmd("fuse_read",
            "read some fuses",
            "<bank> <row>",
            do_fuse_read
           );

RedBoot_cmd("fuse_blow",
            "blow some fuses",
            "<bank> <row> <value>",
            do_fuse_blow
           );

#define         INIT_STRING              "12345678"

void quick_itoa(u32 num, char *a)
{
    int i, j, k;
    for (i = 0; i <= 7; i++) {
        j = (num >> (4 * i)) & 0xF;
        k = (j < 10) ? '0' : ('a' - 0xa);
        a[i] = j + k;
    }
}

static void fuse_blow_prepare(int bank, int row, unsigned long value)
{
    unsigned int reg, i;

    reg = readl(CCM_BASE_ADDR + 0x64);
    reg |= 0x10;
    writel(reg, CCM_BASE_ADDR + 0x64);

    for (i = 0; i < 8; i++) {
        if (((value >> i) & 0x1) == 0) {
            continue;
        }
        if (fuse_blow(bank, row, i) != 0) {
            diag_printf("fuse_blow(bank: %ld, row: %ld, bit: %d failed\n",
                        bank, row, i);
        }
    }
    reg &= ~0x10;
    writel(reg, CCM_BASE_ADDR + 0x64);
}

void do_fuse_blow(int argc, char *argv[])
{
    unsigned long bank, row, value;
    unsigned int fuse_val;

    if (argc != 4) {
        diag_printf("It is too dangeous for you to use this command.\n");
        return;
    }
    if (!parse_num(*(&argv[1]), &bank, &argv[1], " ")) {
        diag_printf("Error: Invalid parameter\n");
        return;
    }
    if (!parse_num(*(&argv[2]), &row, &argv[2], " ")) {
        diag_printf("Error: Invalid parameter\n");
        return;
    }
    if (!parse_num(*(&argv[3]), &value, &argv[3], " ")) {
        diag_printf("Error: Invalid parameter\n");
        return;
    }

    diag_printf("Blowing fuse at bank:%ld row:%ld value:%ld\n",
                    bank, row, value);
    fuse_blow_prepare(bank, row, value);
    fuse_val = sense_fuse(bank, row, 0);
    diag_printf("fuses at (bank:%d, row:%d) = 0x%x\n", bank, row, fuse_val);
}

/* precondition: m>0 and n>0.  Let g=gcd(m,n). */
int gcd(int m, int n)
{
    int t;
    while(m > 0) {
        if(n > m) {t = m; m = n; n = t;} /* swap */
        m -= n;
    }
    return n;
}

int read_mac_addr_from_fuse(unsigned char* data)
{
    data[0] = sense_fuse(1, 9, 0) ;
    data[1] = sense_fuse(1, 10, 0) ;
    data[2] = sense_fuse(1, 11, 0) ;
    data[3] = sense_fuse(1, 12, 0) ;
    data[4] = sense_fuse(1, 13, 0) ;
    data[5] = sense_fuse(1, 14, 0) ;

    if ((data[0] == 0) && (data[1] == 0) && (data[2] == 0) &&
         (data[3] == 0) && (data[4] == 0) && (data[5] == 0)) {
        return 0;
    }

    return 1;
}

static void set_fecmac(int argc,char *argv[])
{
    unsigned long temp;
    unsigned char data[6];
    int ret, i;
    unsigned int reg;

    if (argc == 1) {
        goto print_mac_info;
    }

    if (argc != 2) {
        ret = -1;
        goto error;
    }
    for (i = 0;  i < 6;  i++) {
        if (!parse_num(*(&argv[1]), &temp, &argv[1], ":")) {
            ret = -2;
            goto error;
        }
        if (temp > 0xFF) {
            ret = -3;
            goto error;
        }
        data[i] = temp & 0xFF;
    }

    for (i = 0;  i < 6;  i++) {
        fuse_blow_prepare(1, i + 9, data[i]);
    }

print_mac_info:
    /* Read the Mac address and print it */
    ret = read_mac_addr_from_fuse(data);

    diag_printf("FEC MAC address: ");
    diag_printf("0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n\n",
                    data[0], data[1], data[2], data[3], data[4], data[5]);

    return;

    error:
    diag_printf("Wrong value for set_fecmac. Error=%d\n\n", ret);
}

RedBoot_cmd("set_fecmac",
            "Set FEC MAC address in Fuse registers",
            "[0x##:0x##:0x##:0x##:0x##:0x##]",
            set_fecmac
           );