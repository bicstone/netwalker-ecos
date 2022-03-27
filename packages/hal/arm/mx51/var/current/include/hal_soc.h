//==========================================================================
//
//      hal_soc.h
//
//      SoC chip definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// 2009/08/02 : from REDBOOT_200929.
//               CLKCTL_CCGR6 add.
//
//========================================================================*/

#ifndef __HAL_SOC_H__
#define __HAL_SOC_H__

#ifdef __ASSEMBLER__

#define REG8_VAL(a)          (a)
#define REG16_VAL(a)         (a)
#define REG32_VAL(a)         (a)

#define REG8_PTR(a)          (a)
#define REG16_PTR(a)         (a)
#define REG32_PTR(a)         (a)

#else /* __ASSEMBLER__ */

extern char HAL_PLATFORM_EXTRA[];
#define REG8_VAL(a)          ((unsigned char)(a))
#define REG16_VAL(a)         ((unsigned short)(a))
#define REG32_VAL(a)         ((unsigned int)(a))

#define REG8_PTR(a)          ((volatile unsigned char *)(a))
#define REG16_PTR(a)         ((volatile unsigned short *)(a))
#define REG32_PTR(a)         ((volatile unsigned int *)(a))
#define readb(a)             (*(volatile unsigned char *)(a))
#define readw(a)             (*(volatile unsigned short *)(a))
#define readl(a)             (*(volatile unsigned int *)(a))
#define writeb(v,a)          (*(volatile unsigned char *)(a) = (v))
#define writew(v,a)          (*(volatile unsigned short *)(a) = (v))
#define writel(v,a)          (*(volatile unsigned int *)(a) = (v))

#endif /* __ASSEMBLER__ */

/*
 * Default Memory Layout Definitions
 */

/*
 * UART Chip level Configuration that a user may not have to edit. These
 * configuration vary depending on how the UART module is integrated with
 * the ARM core
 */
#define MXC_UART_NR 3
/*!
 * This option is used to set or clear the RXDMUXSEL bit in control reg 3.
 * Certain platforms need this bit to be set in order to receive Irda data.
 */
#define MXC_UART_IR_RXDMUX      0x0004
/*!
 * This option is used to set or clear the RXDMUXSEL bit in control reg 3.
 * Certain platforms need this bit to be set in order to receive UART data.
 */
#define MXC_UART_RXDMUX         0x0004

/*
 * IRAM
 */
#define IRAM_BASE_ADDR		0x1FFE8000	/* 96K internal ram */

/*
   * ROM address
   */
#define ROM_BASE_ADDRESS                    0x0
#define ROM_BASE_ADDRESS_VIRT          0x20000000

#define ROM_SI_REV_OFFSET                   0x48

/*
 * NFC internal RAM
 */
#define NFC_BASE_ADDR_AXI	0xCFFF0000
#define NFC_BASE                        NFC_BASE_ADDR_AXI

#define PLATFORM_BASE_ADDR      0x83FA0000
#define PLATFORM_ICGC                 0x14
/*
 * Graphics Memory of GPU
 */
#define GPU_BASE_ADDR					0x20000000

#define TZIC_BASE_ADDR				0x8FFFC000

#define DEBUG_BASE_ADDR				0x60000000
#define DEBUG_ROM_ADDR				(DEBUG_BASE_ADDR + 0x0)
#define ETB_BASE_ADDR							(DEBUG_BASE_ADDR + 0x00001000)
#define ETM_BASE_ADDR							(DEBUG_BASE_ADDR + 0x00002000)
#define TPIU_BASE_ADDR						(DEBUG_BASE_ADDR + 0x00003000)
#define CTI0_BASE_ADDR						(DEBUG_BASE_ADDR + 0x00004000)
#define CTI1_BASE_ADDR						(DEBUG_BASE_ADDR + 0x00005000)
#define CTI2_BASE_ADDR						(DEBUG_BASE_ADDR + 0x00006000)
#define CTI3_BASE_ADDR						(DEBUG_BASE_ADDR + 0x00007000)
#define CORTEX_DBG_BASE_ADDR			(DEBUG_BASE_ADDR + 0x00008000)

/*
 * SPBA global module enabled #0
 */
#define SPBA0_BASE_ADDR 			0x70000000

#define MMC_SDHC1_BASE_ADDR				(SPBA0_BASE_ADDR + 0x00004000)
#define ESDHC1_REG_BASE                                      MMC_SDHC1_BASE_ADDR
#define MMC_SDHC2_BASE_ADDR				(SPBA0_BASE_ADDR + 0x00008000)
#define UART3_BASE_ADDR 					(SPBA0_BASE_ADDR + 0x0000C000)
//eCSPI1
#define CSPI1_BASE_ADDR 					(SPBA0_BASE_ADDR + 0x00010000)
#define SSI2_BASE_ADDR						(SPBA0_BASE_ADDR + 0x00014000)
#define MMC_SDHC3_BASE_ADDR				(SPBA0_BASE_ADDR + 0x00020000)
#define MMC_SDHC4_BASE_ADDR				(SPBA0_BASE_ADDR + 0x00024000)
#define SPDIF_BASE_ADDR						(SPBA0_BASE_ADDR + 0x00028000)
#define ATA_DMA_BASE_ADDR					(SPBA0_BASE_ADDR + 0x00030000)
#define SLIM_BASE_ADDR				(SPBA0_BASE_ADDR + 0x00034000)
#define HSI2C_BASE_ADDR				(SPBA0_BASE_ADDR + 0x00038000)
#define SPBA_CTRL_BASE_ADDR				(SPBA0_BASE_ADDR + 0x0003C000)

/*!
 * defines for SPBA modules
 */
#define SPBA_SDHC1	0x04
#define SPBA_SDHC2	0x08
#define SPBA_UART3	0x0C
#define SPBA_CSPI1	0x10
#define SPBA_SSI2		0x14
#define SPBA_SDHC3	0x20
#define SPBA_SDHC4	0x24
#define SPBA_SPDIF	0x28
#define SPBA_ATA		0x30
#define SPBA_SLIM		0x34
#define SPBA_HSI2C	0x38
#define SPBA_CTRL		0x3C


/*
 * AIPS 1
 */
#define AIPS1_BASE_ADDR 			0x73F00000
#define AIPS1_CTRL_BASE_ADDR    AIPS1_BASE_ADDR
#define USBOH3_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00080000)
#define GPIO1_BASE_ADDR						(AIPS1_BASE_ADDR + 0x00084000)
#define GPIO2_BASE_ADDR						(AIPS1_BASE_ADDR + 0x00088000)
#define GPIO3_BASE_ADDR						(AIPS1_BASE_ADDR + 0x0008C000)
#define GPIO4_BASE_ADDR						(AIPS1_BASE_ADDR + 0x00090000)
#define KPP_BASE_ADDR							(AIPS1_BASE_ADDR + 0x00094000)
#define WDOG1_BASE_ADDR						(AIPS1_BASE_ADDR + 0x00098000)
#define WDOG_BASE_ADDR                                          WDOG1_BASE_ADDR
#define WDOG2_BASE_ADDR						(AIPS1_BASE_ADDR + 0x0009C000)
#define GPT_BASE_ADDR							(AIPS1_BASE_ADDR + 0x000A0000)
#define SRTC_BASE_ADDR						(AIPS1_BASE_ADDR + 0x000A4000)
#define IOMUXC_BASE_ADDR					(AIPS1_BASE_ADDR + 0x000A8000)
#define EPIT1_BASE_ADDR						(AIPS1_BASE_ADDR + 0x000AC000)
#define EPIT2_BASE_ADDR						(AIPS1_BASE_ADDR + 0x000B0000)
#define PWM1_BASE_ADDR						(AIPS1_BASE_ADDR + 0x000B4000)
#define PWM2_BASE_ADDR						(AIPS1_BASE_ADDR + 0x000B8000)
#define UART1_BASE_ADDR						(AIPS1_BASE_ADDR + 0x000BC000)
#define UART2_BASE_ADDR						(AIPS1_BASE_ADDR + 0x000C0000)
#define SRC_BASE_ADDR							(AIPS1_BASE_ADDR + 0x000D0000)
#define CCM_BASE_ADDR							(AIPS1_BASE_ADDR + 0x000D4000)
#define GPC_BASE_ADDR							(AIPS1_BASE_ADDR + 0x000D8000)

/*
 * AIPS 2
 */
#define AIPS2_BASE_ADDR				0x83F00000
#define AIPS2_CTRL_BASE_ADDR    AIPS2_BASE_ADDR
#define PLL1_BASE_ADDR					(AIPS2_BASE_ADDR + 0x00080000)
#define PLL2_BASE_ADDR					(AIPS2_BASE_ADDR + 0x00084000)
#define PLL3_BASE_ADDR					(AIPS2_BASE_ADDR + 0x00088000)
#define AHBMAX_BASE_ADDR				(AIPS2_BASE_ADDR + 0x00094000)
#define MAX_BASE_ADDR                                   AHBMAX_BASE_ADDR
#define IIM_BASE_ADDR						(AIPS2_BASE_ADDR + 0x00098000)
#define CSU_BASE_ADDR						(AIPS2_BASE_ADDR + 0x0009C000)
#define ARM_ELBOW_BASE_ADDR					(AIPS2_BASE_ADDR + 0x000A0000)
#define OWIRE_BASE_ADDR 				(AIPS2_BASE_ADDR + 0x000A4000)
#define FIRI_BASE_ADDR					(AIPS2_BASE_ADDR + 0x000A8000)
// eCSPI2
#define CSPI2_BASE_ADDR 				(AIPS2_BASE_ADDR + 0x000AC000)
#define SDMA_BASE_ADDR 					(AIPS2_BASE_ADDR + 0x000B0000)
#define SCC_BASE_ADDR 					(AIPS2_BASE_ADDR + 0x000B4000)
#define ROMCP_BASE_ADDR 				(AIPS2_BASE_ADDR + 0x000B8000)
#define RTIC_BASE_ADDR 					(AIPS2_BASE_ADDR + 0x000BC000)
// actually cspi1
#define CSPI3_BASE_ADDR					(AIPS2_BASE_ADDR + 0x000C0000)
#define I2C2_BASE_ADDR					(AIPS2_BASE_ADDR + 0x000C4000)
#define I2C1_BASE_ADDR					(AIPS2_BASE_ADDR + 0x000C8000)
#define I2C_BASE_ADDR                                   I2C1_BASE_ADDR
#define SSI1_BASE_ADDR					(AIPS2_BASE_ADDR + 0x000CC000)
#define AUDMUX_BASE_ADDR				(AIPS2_BASE_ADDR + 0x000D0000)
#define M4IF_BASE_ADDR					(AIPS2_BASE_ADDR + 0x000D8000)
#define ESDCTL_BASE_ADDR				(AIPS2_BASE_ADDR + 0x000D9000)
#define WEIM_BASE_ADDR					(AIPS2_BASE_ADDR + 0x000DA000)
#define NFC_IP_BASE						(AIPS2_BASE_ADDR + 0x000DB000)
#define EMI_BASE_ADDR						(AIPS2_BASE_ADDR + 0x000DBF00)
#define MIPI_HSC_BASE_ADDR			(AIPS2_BASE_ADDR + 0x000DC000)
#define ATA_BASE_ADDR					  (AIPS2_BASE_ADDR + 0x000E0000)
#define SIM_BASE_ADDR						(AIPS2_BASE_ADDR + 0x000E4000)
#define SSI3_BASE_ADDR						(AIPS2_BASE_ADDR + 0x000E8000)
#define FEC_BASE_ADDR						(AIPS2_BASE_ADDR + 0x000EC000)
#define SOC_FEC_BASE						FEC_BASE_ADDR
#define TVE_BASE_ADDR						(AIPS2_BASE_ADDR + 0x000F0000)
#define VPU_BASE_ADDR						(AIPS2_BASE_ADDR + 0x000F4000)
#define SAHARA_BASE_ADDR				(AIPS2_BASE_ADDR + 0x000F8000)

/*
 * Memory regions and CS
 */
#define GPU_CTRL_BASE_ADDR	    0x30000000
#define IPU_CTRL_BASE_ADDR	    0x40000000
#define CSD0_BASE_ADDR          0x90000000
#define CSD1_BASE_ADDR          0xA0000000
#define CS0_BASE_ADDR           0xB0000000
#define CS1_BASE_ADDR           0xB8000000
#define CS2_BASE_ADDR           0xC0000000
#define CS3_BASE_ADDR           0xC8000000
#define CS4_BASE_ADDR           0xCC000000
#define CS5_BASE_ADDR           0xCE000000

/*
 * DMA request assignments
 */
#define DMA_REQ_SSI3_TX1			47
#define DMA_REQ_SSI3_RX1			46
#define DMA_REQ_SPDIF					45
#define DMA_REQ_UART3_TX			44
#define DMA_REQ_UART3_RX			43
#define DMA_REQ_SLIM_B_TX			42
#define DMA_REQ_SDHC4					41
#define DMA_REQ_SDHC3					40
#define DMA_REQ_CSPI_TX				39
#define DMA_REQ_CSPI_RX				38
#define DMA_REQ_SSI3_TX2			37
#define DMA_REQ_IPU						36
#define DMA_REQ_SSI3_RX2			35
#define DMA_REQ_EPIT2					34
#define DMA_REQ_CTI2_1				33
#define DMA_REQ_EMI_WR				32
#define DMA_REQ_CTI2_0				31
#define DMA_REQ_EMI_RD				30
#define DMA_REQ_SSI1_TX1			29
#define DMA_REQ_SSI1_RX1			28
#define DMA_REQ_SSI1_TX2			27
#define DMA_REQ_SSI1_RX2			26
#define DMA_REQ_SSI2_TX1			25
#define DMA_REQ_SSI2_RX1			24
#define DMA_REQ_SSI2_TX2			23
#define DMA_REQ_SSI2_RX2			22
#define DMA_REQ_SDHC2_I2C2		21
#define DMA_REQ_SDHC1_I2C1		20
#define DMA_REQ_UART1_TX			19
#define DMA_REQ_UART1_RX			18
#define DMA_REQ_UART2_TX			17
#define DMA_REQ_UART2_RX			16
#define DMA_REQ_GPU_GPIO1_0				15
#define DMA_REQ_GPIO1_1				14
#define DMA_REQ_FIRI_TX				13
#define DMA_REQ_FIRI_RX				12
#define DMA_REQ_HS_I2C_RX			11
#define DMA_REQ_HS_I2C_TX			10
#define DMA_REQ_CSPI2_TX			9
#define DMA_REQ_CSPI2_RX			8
#define DMA_REQ_CSPI1_TX			7
#define DMA_REQ_CSPI1_RX			6
#define DMA_REQ_SLIM_B				5
#define DMA_REQ_ATA_TX_END		4
#define DMA_REQ_ATA_TX				3
#define DMA_REQ_ATA_RX				2
#define DMA_REQ_GPC						1
#define DMA_REQ_VPU						0

/*
 * Interrupt numbers
 */
#define MXC_INT_BASE					0
#define MXC_INT_RESV0					0
#define MXC_INT_MMC_SDHC1			1
#define MXC_INT_MMC_SDHC2			2
#define MXC_INT_MMC_SDHC3			3
#define MXC_INT_MMC_SDHC4			4
#define MXC_INT_RESV5					5
#define MXC_INT_SDMA					6
#define MXC_INT_IOMUX					7
#define MXC_INT_NFC						8
#define MXC_INT_VPU						9
#define MXC_INT_IPU_ERR				10
#define MXC_INT_IPU_SYN				11
#define MXC_INT_GPU						12
#define MXC_INT_RESV13				13
#define MXC_INT_USB_H1				14
#define MXC_INT_EMI						15
#define MXC_INT_USB_H2				16
#define MXC_INT_USB_H3				17
#define MXC_INT_USB_OTG				18
#define MXC_INT_SAHARA_H0			19
#define MXC_INT_SAHARA_H1			20
#define MXC_INT_SCC_SMN				21
#define MXC_INT_SCC_STZ				22
#define MXC_INT_SCC_SCM				23
#define MXC_INT_SRTC_NTZ			24
#define MXC_INT_SRTC_TZ				25
#define MXC_INT_RTIC					26
#define MXC_INT_CSU						27
#define MXC_INT_SLIM_B				28
#define MXC_INT_SSI1					29
#define MXC_INT_SSI2					30
#define MXC_INT_UART1					31
#define MXC_INT_UART2					32
#define MXC_INT_UART3					33
#define MXC_INT_RESV34				34
#define MXC_INT_RESV35				35
#define MXC_INT_CSPI1					36
#define MXC_INT_CSPI2					37
#define MXC_INT_CSPI					38
#define MXC_INT_GPT						39
#define MXC_INT_EPIT1					40
#define MXC_INT_EPIT2					41
#define MXC_INT_GPIO1_INT7		42
#define MXC_INT_GPIO1_INT6		43
#define MXC_INT_GPIO1_INT5		44
#define MXC_INT_GPIO1_INT4		45
#define MXC_INT_GPIO1_INT3		46
#define MXC_INT_GPIO1_INT2		47
#define MXC_INT_GPIO1_INT1		48
#define MXC_INT_GPIO1_INT0		49
#define MXC_INT_GPIO1_LOW			50
#define MXC_INT_GPIO1_HIGH		51
#define MXC_INT_GPIO2_LOW			52
#define MXC_INT_GPIO2_HIGH		53
#define MXC_INT_GPIO3_LOW			54
#define MXC_INT_GPIO3_HIGH		55
#define MXC_INT_GPIO4_LOW			56
#define MXC_INT_GPIO4_HIGH		57
#define MXC_INT_WDOG1					58
#define MXC_INT_WDOG2					59
#define MXC_INT_KPP						60
#define MXC_INT_PWM1					61
#define MXC_INT_I2C1					62
#define MXC_INT_I2C2					63
#define MXC_INT_HS_I2C				64
#define MXC_INT_RESV65				65
#define MXC_INT_RESV66				66
#define MXC_INT_SIM_IPB				67
#define MXC_INT_SIM_DAT				68
#define MXC_INT_IIM						69
#define MXC_INT_ATA						70
#define MXC_INT_CCM1					71
#define MXC_INT_CCM2					72
#define MXC_INT_GPC1					73
#define MXC_INT_GPC2					74
#define MXC_INT_SRC						75
#define MXC_INT_NM						76
#define MXC_INT_PMU						77
#define MXC_INT_CTI_IRQ				78
#define MXC_INT_CTI1_TG0			79
#define MXC_INT_CTI1_TG1			80
#define MXC_INT_MCG_ERR				81
#define MXC_INT_MCG_TMR				82
#define MXC_INT_MCG_FUNC			83
#define MXC_INT_RESV84				84
#define MXC_INT_RESV85				85
#define MXC_INT_RESV86				86
#define MXC_INT_FEC						87
#define MXC_INT_OWIRE					88
#define MXC_INT_CTI1_TG2			89
#define MXC_INT_SJC						90
#define MXC_INT_SPDIF					91
#define MXC_INT_TVE						92
#define MXC_INT_FIFI					93
#define MXC_INT_PWM2					94
#define MXC_INT_SLIM_EXP			95
#define MXC_INT_SSI3					96
#define MXC_INT_RESV97				97
#define MXC_INT_CTI1_TG3			98
#define MXC_INT_SMC_RX				99
#define MXC_INT_VPU_IDLE			100
#define MXC_INT_RESV101				101
#define MXC_INT_GPU_IDLE			102

/*!
 * Number of GPIO port as defined in the IC Spec
 */
#define GPIO_PORT_NUM           4
/*!
 * Number of GPIO pins per port
 */
#define GPIO_NUM_PIN            32

/* CCM */
#define CLKCTL_CCR              0x00
#define CLKCTL_CCDR             0x04
#define CLKCTL_CSR              0x08
#define CLKCTL_CCSR             0x0C
#define CLKCTL_CACRR            0x10
#define CLKCTL_CBCDR            0x14
#define CLKCTL_CBCMR            0x18
#define CLKCTL_CSCMR1           0x1C
#define CLKCTL_CSCMR2           0x20
#define CLKCTL_CSCDR1           0x24
#define CLKCTL_CS1CDR           0x28
#define CLKCTL_CS2CDR           0x2C
#define CLKCTL_CDCDR            0x30
#define CLKCTL_CHSCCDR          0x34
#define CLKCTL_CSCDR2           0x38
#define CLKCTL_CSCDR3           0x3C
#define CLKCTL_CSCDR4           0x40
#define CLKCTL_CWDR             0x44
#define CLKCTL_CDHIPR           0x48
#define CLKCTL_CDCR             0x4C
#define CLKCTL_CTOR             0x50
#define CLKCTL_CLPCR            0x54
#define CLKCTL_CISR             0x58
#define CLKCTL_CIMR             0x5C
#define CLKCTL_CCOSR            0x60
#define CLKCTL_CGPR             0x64
#define CLKCTL_CCGR0            0x68
#define CLKCTL_CCGR1            0x6C
#define CLKCTL_CCGR2            0x70
#define CLKCTL_CCGR3            0x74
#define CLKCTL_CCGR4            0x78
#define CLKCTL_CCGR5            0x7C
#define CLKCTL_CCGR6            0x80
#define CLKCTL_CMEOR            0x84

#define FREQ_24MHZ                      24000000
#define FREQ_32768HZ                    (32768 * 1024)
#define FREQ_38400HZ                    (38400 * 1024)
#define FREQ_32000HZ                    (32000 * 1024)
#define PLL_REF_CLK                     FREQ_24MHZ
#define CKIH                                  22579200
//#define PLL_REF_CLK  FREQ_32768HZ
//#define PLL_REF_CLK  FREQ_32000HZ

/* WEIM registers */
#define CSGCR1                          0x00
#define CSGCR2                          0x04
#define CSRCR1                          0x08
#define CSRCR2                          0x0C
#define CSWCR1                          0x10

/* M4IF */
#define M4IF_FBPM0                        0x40
#define M4IF_FBPM1                        0x44
#define M4IF_FIDBP                         0x48
#define M4IF_MIF4                           0x48
#define M4IF_FPWC                          0x9C

/* ESDCTL */
#define ESDCTL_ESDCTL0                  0x00
#define ESDCTL_ESDCFG0                  0x04
#define ESDCTL_ESDCTL1                  0x08
#define ESDCTL_ESDCFG1                  0x0C
#define ESDCTL_ESDMISC                  0x10
#define ESDCTL_ESDSCR                   0x14
#define ESDCTL_ESDCDLY1                 0x20
#define ESDCTL_ESDCDLY2                 0x24
#define ESDCTL_ESDCDLY3                 0x28
#define ESDCTL_ESDCDLY4                 0x2C
#define ESDCTL_ESDCDLY5                 0x30
#define ESDCTL_ESDCDLYGD                0x34

/* DPLL */
#define PLL_DP_CTL          0x00
#define PLL_DP_CONFIG       0x04
#define PLL_DP_OP           0x08
#define PLL_DP_MFD          0x0C
#define PLL_DP_MFN          0x10
#define PLL_DP_MFNMINUS     0x14
#define PLL_DP_MFNPLUS      0x18
#define PLL_DP_HFS_OP       0x1C
#define PLL_DP_HFS_MFD      0x20
#define PLL_DP_HFS_MFN      0x24
#define PLL_DP_TOGC         0x28
#define PLL_DP_DESTAT       0x2C

#define CHIP_REV_1_0            0x0      /* PASS 1.0 */
#define CHIP_REV_1_1            0x1      /* PASS 1.1 */
#define CHIP_REV_2_0            0x2      /* PASS 2.0 */
#define CHIP_LATEST             CHIP_REV_1_1

#define IIM_STAT_OFF            0x00
#define IIM_STAT_BUSY           (1 << 7)
#define IIM_STAT_PRGD           (1 << 1)
#define IIM_STAT_SNSD           (1 << 0)
#define IIM_STATM_OFF           0x04
#define IIM_ERR_OFF             0x08
#define IIM_ERR_PRGE            (1 << 7)
#define IIM_ERR_WPE         (1 << 6)
#define IIM_ERR_OPE         (1 << 5)
#define IIM_ERR_RPE         (1 << 4)
#define IIM_ERR_WLRE        (1 << 3)
#define IIM_ERR_SNSE        (1 << 2)
#define IIM_ERR_PARITYE     (1 << 1)
#define IIM_EMASK_OFF           0x0C
#define IIM_FCTL_OFF            0x10
#define IIM_UA_OFF              0x14
#define IIM_LA_OFF              0x18
#define IIM_SDAT_OFF            0x1C
#define IIM_PREV_OFF            0x20
#define IIM_SREV_OFF            0x24
#define IIM_PREG_P_OFF          0x28
#define IIM_SCS0_OFF            0x2C
#define IIM_SCS1_P_OFF          0x30
#define IIM_SCS2_OFF            0x34
#define IIM_SCS3_P_OFF          0x38

#define IIM_PROD_REV_SH         3
#define IIM_PROD_REV_LEN        5
#define IIM_SREV_REV_SH         4
#define IIM_SREV_REV_LEN        4
#define PROD_SIGNATURE_MX51     0x1

#define EPIT_BASE_ADDR          EPIT1_BASE_ADDR
#define EPITCR                  0x00
#define EPITSR                  0x04
#define EPITLR                  0x08
#define EPITCMPR                0x0C
#define EPITCNR                 0x10

#define GPTCR                   0x00
#define GPTPR                   0x04
#define GPTSR                   0x08
#define GPTIR                   0x0C
#define GPTOCR1                 0x10
#define GPTOCR2                 0x14
#define GPTOCR3                 0x18
#define GPTICR1                 0x1C
#define GPTICR2                 0x20
#define GPTCNT                  0x24

/* Assuming 24MHz input clock with doubler ON */
/*                            MFI         PDF */
#define DP_OP_850           ((8 << 4) + ((1 - 1)  << 0))
#define DP_MFD_850      (48 - 1)
#define DP_MFN_850      41

#define DP_OP_800           ((8 << 4) + ((1 - 1)  << 0))
#define DP_MFD_800      (3 - 1)
#define DP_MFN_800      1

#define DP_OP_700           ((7 << 4) + ((1 - 1)  << 0))
#define DP_MFD_700      (24 - 1)
#define DP_MFN_700      7

#define DP_OP_400           ((8 << 4) + ((2 - 1)  << 0))
#define DP_MFD_400      (3 - 1)
#define DP_MFN_400      1

#define DP_OP_532           ((5 << 4) + ((1 - 1)  << 0))
#define DP_MFD_532      (24 - 1)
#define DP_MFN_532      13

#define DP_OP_665           ((6 << 4) + ((1 - 1)  << 0))
#define DP_MFD_665      (96 - 1)
#define DP_MFN_665      89

#define DP_OP_216           ((6 << 4) + ((3 - 1)  << 0))
#define DP_MFD_216      (4 - 1)
#define DP_MFN_216      3

#define PROD_SIGNATURE_SUPPORTED  PROD_SIGNATURE_MX51

#define CHIP_VERSION_NONE               0xFFFFFFFF      // invalid product ID
#define CHIP_VERSION_UNKNOWN        0xDEADBEEF      // invalid chip rev

#define PART_NUMBER_OFFSET          (12)
#define BOARD_VER_OFFSET                (8)
#define MAJOR_NUMBER_OFFSET         (4)
#define MINOR_NUMBER_OFFSET         (0)

#define IOMUXC_SW_MUX_CTL_PAD_NANDF_WE_B    (IOMUXC_BASE_ADDR + 0x108)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_RE_B    (IOMUXC_BASE_ADDR + 0x10C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_ALE    (IOMUXC_BASE_ADDR + 0x110)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CLE    (IOMUXC_BASE_ADDR + 0x114)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_WP_B    (IOMUXC_BASE_ADDR + 0x118)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_RB0    (IOMUXC_BASE_ADDR + 0x11C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_RB1    (IOMUXC_BASE_ADDR + 0x120)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_RB2    (IOMUXC_BASE_ADDR + 0x124)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_RB3    (IOMUXC_BASE_ADDR + 0x128)

/* IOMUX defines */
#ifdef IMX51_TO_2

#define IOMUXC_SW_MUX_CTL_PAD_NANDF_RB5    (IOMUXC_BASE_ADDR + 0x12C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS0    (IOMUXC_BASE_ADDR + 0x130)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS1    (IOMUXC_BASE_ADDR + 0x134)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS2    (IOMUXC_BASE_ADDR + 0x138)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS3    (IOMUXC_BASE_ADDR + 0x13C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS4    (IOMUXC_BASE_ADDR + 0x140)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS5    (IOMUXC_BASE_ADDR + 0x144)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS6    (IOMUXC_BASE_ADDR + 0x148)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS7    (IOMUXC_BASE_ADDR + 0x14C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_RDY_INT    (IOMUXC_BASE_ADDR + 0x150)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D15    (IOMUXC_BASE_ADDR + 0x154)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D14    (IOMUXC_BASE_ADDR + 0x158)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D13    (IOMUXC_BASE_ADDR + 0x15C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D12    (IOMUXC_BASE_ADDR + 0x160)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D11    (IOMUXC_BASE_ADDR + 0x164)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D10    (IOMUXC_BASE_ADDR + 0x168)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D9    (IOMUXC_BASE_ADDR + 0x16C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D8    (IOMUXC_BASE_ADDR + 0x170)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D7    (IOMUXC_BASE_ADDR + 0x174)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D6    (IOMUXC_BASE_ADDR + 0x178)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D5    (IOMUXC_BASE_ADDR + 0x17C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D4    (IOMUXC_BASE_ADDR + 0x180)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D3    (IOMUXC_BASE_ADDR + 0x184)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D2    (IOMUXC_BASE_ADDR + 0x188)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D1    (IOMUXC_BASE_ADDR + 0x18C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D0    (IOMUXC_BASE_ADDR + 0x190)

#define IOMUXC_SW_PAD_CTL_PAD_NANDF_WE_B      (IOMUXC_BASE_ADDR + 0x4e4)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RE_B      (IOMUXC_BASE_ADDR + 0x4e8)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_ALE      (IOMUXC_BASE_ADDR + 0x4ec)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CLE      (IOMUXC_BASE_ADDR + 0x4f0)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_WP_B      (IOMUXC_BASE_ADDR + 0x4f4)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB0      (IOMUXC_BASE_ADDR + 0x4f8)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB1      (IOMUXC_BASE_ADDR + 0x4fc)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB2      (IOMUXC_BASE_ADDR + 0x500)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB3      (IOMUXC_BASE_ADDR + 0x504)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB5     (IOMUXC_BASE_ADDR + 0x514)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS0      (IOMUXC_BASE_ADDR + 0x518)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS1      (IOMUXC_BASE_ADDR + 0x51C)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS2      (IOMUXC_BASE_ADDR + 0x520)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS3      (IOMUXC_BASE_ADDR + 0x524)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS4      (IOMUXC_BASE_ADDR + 0x528)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS5      (IOMUXC_BASE_ADDR + 0x52C)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS6      (IOMUXC_BASE_ADDR + 0x530)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS7      (IOMUXC_BASE_ADDR + 0x534)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RDY_INT      (IOMUXC_BASE_ADDR + 0x538)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D15      (IOMUXC_BASE_ADDR + 0x53C)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D14      (IOMUXC_BASE_ADDR + 0x540)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D13      (IOMUXC_BASE_ADDR + 0x544)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D12      (IOMUXC_BASE_ADDR + 0x548)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D11      (IOMUXC_BASE_ADDR + 0x54C)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D10      (IOMUXC_BASE_ADDR + 0x550)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D9      (IOMUXC_BASE_ADDR + 0x554)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D8      (IOMUXC_BASE_ADDR + 0x558)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D7      (IOMUXC_BASE_ADDR + 0x55C)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D6      (IOMUXC_BASE_ADDR + 0x560)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D5      (IOMUXC_BASE_ADDR + 0x564)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D4      (IOMUXC_BASE_ADDR + 0x568)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D3      (IOMUXC_BASE_ADDR + 0x56C)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D2      (IOMUXC_BASE_ADDR + 0x570)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D1      (IOMUXC_BASE_ADDR + 0x574)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D0      (IOMUXC_BASE_ADDR + 0x578)

#else

#define IOMUXC_SW_MUX_CTL_PAD_NANDF_RB5    (IOMUXC_BASE_ADDR + 0x130)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_RB6    (IOMUXC_BASE_ADDR + 0x134)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_RB7    (IOMUXC_BASE_ADDR + 0x138)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS0    (IOMUXC_BASE_ADDR + 0x13C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS1    (IOMUXC_BASE_ADDR + 0x140)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS2    (IOMUXC_BASE_ADDR + 0x144)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS3    (IOMUXC_BASE_ADDR + 0x148)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS4    (IOMUXC_BASE_ADDR + 0x14C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS5    (IOMUXC_BASE_ADDR + 0x150)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS6    (IOMUXC_BASE_ADDR + 0x154)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_CS7    (IOMUXC_BASE_ADDR + 0x158)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_RDY_INT    (IOMUXC_BASE_ADDR + 0x15C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D15    (IOMUXC_BASE_ADDR + 0x160)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D14    (IOMUXC_BASE_ADDR + 0x164)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D13    (IOMUXC_BASE_ADDR + 0x168)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D12    (IOMUXC_BASE_ADDR + 0x16C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D11    (IOMUXC_BASE_ADDR + 0x170)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D10    (IOMUXC_BASE_ADDR + 0x174)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D9    (IOMUXC_BASE_ADDR + 0x178)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D8    (IOMUXC_BASE_ADDR + 0x17C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D7    (IOMUXC_BASE_ADDR + 0x180)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D6    (IOMUXC_BASE_ADDR + 0x184)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D5    (IOMUXC_BASE_ADDR + 0x188)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D4    (IOMUXC_BASE_ADDR + 0x18C)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D3    (IOMUXC_BASE_ADDR + 0x190)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D2    (IOMUXC_BASE_ADDR + 0x194)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D1    (IOMUXC_BASE_ADDR + 0x198)
#define IOMUXC_SW_MUX_CTL_PAD_NANDF_D0    (IOMUXC_BASE_ADDR + 0x19C)

#define IOMUXC_SW_PAD_CTL_PAD_NANDF_WE_B      (IOMUXC_BASE_ADDR + 0x5B0)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RE_B      (IOMUXC_BASE_ADDR + 0x5B4)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_ALE      (IOMUXC_BASE_ADDR + 0x5B8)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CLE      (IOMUXC_BASE_ADDR + 0x5BC)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_WP_B      (IOMUXC_BASE_ADDR + 0x5C0)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB0      (IOMUXC_BASE_ADDR + 0x5C4)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB1      (IOMUXC_BASE_ADDR + 0x5C8)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB2      (IOMUXC_BASE_ADDR + 0x5CC)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB3      (IOMUXC_BASE_ADDR + 0x5D0)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB4      (IOMUXC_BASE_ADDR + 0x5D4)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB5      (IOMUXC_BASE_ADDR + 0x5D8)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB6      (IOMUXC_BASE_ADDR + 0x5DC)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RB7      (IOMUXC_BASE_ADDR + 0x5E0)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS0      (IOMUXC_BASE_ADDR + 0x5E4)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS1      (IOMUXC_BASE_ADDR + 0x5E8)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS2      (IOMUXC_BASE_ADDR + 0x5EC)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS3      (IOMUXC_BASE_ADDR + 0x5F0)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS4      (IOMUXC_BASE_ADDR + 0x5F4)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS5      (IOMUXC_BASE_ADDR + 0x5F8)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS6      (IOMUXC_BASE_ADDR + 0x5FC)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_CS7      (IOMUXC_BASE_ADDR + 0x600)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_RDY_INT      (IOMUXC_BASE_ADDR + 0x604)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D15      (IOMUXC_BASE_ADDR + 0x608)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D14      (IOMUXC_BASE_ADDR + 0x60C)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D13      (IOMUXC_BASE_ADDR + 0x610)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D12      (IOMUXC_BASE_ADDR + 0x614)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D11      (IOMUXC_BASE_ADDR + 0x618)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D10      (IOMUXC_BASE_ADDR + 0x61C)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D9      (IOMUXC_BASE_ADDR + 0x620)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D8      (IOMUXC_BASE_ADDR + 0x624)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D7      (IOMUXC_BASE_ADDR + 0x628)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D6      (IOMUXC_BASE_ADDR + 0x62C)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D5      (IOMUXC_BASE_ADDR + 0x630)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D4      (IOMUXC_BASE_ADDR + 0x634)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D3      (IOMUXC_BASE_ADDR + 0x638)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D2      (IOMUXC_BASE_ADDR + 0x63C)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D1      (IOMUXC_BASE_ADDR + 0x640)
#define IOMUXC_SW_PAD_CTL_PAD_NANDF_D0      (IOMUXC_BASE_ADDR + 0x644)

#endif

//#define BARKER_CODE_SWAP_LOC            0x404
#define BARKER_CODE_VAL                 0xB1
#define NFC_V3_0                        0x30
// This defines the register base for the NAND AXI registers
#define NAND_REG_BASE                   (NFC_BASE_ADDR_AXI + 0x1E00)

#define NAND_CMD_REG                    (NAND_REG_BASE + 0x00)
#define NAND_ADD0_REG                   (NAND_REG_BASE + 0x04)
#define NAND_ADD1_REG                   (NAND_REG_BASE + 0x08)
#define NAND_ADD2_REG                   (NAND_REG_BASE + 0x0C)
#define NAND_ADD3_REG                   (NAND_REG_BASE + 0x10)
#define NAND_ADD4_REG                   (NAND_REG_BASE + 0x14)
#define NAND_ADD5_REG                   (NAND_REG_BASE + 0x18)
#define NAND_ADD6_REG                   (NAND_REG_BASE + 0x1C)
#define NAND_ADD7_REG                   (NAND_REG_BASE + 0x20)
#define NAND_ADD8_REG                   (NAND_REG_BASE + 0x24)
#define NAND_ADD9_REG                   (NAND_REG_BASE + 0x28)
#define NAND_ADD10_REG                  (NAND_REG_BASE + 0x2C)
#define NAND_ADD11_REG                  (NAND_REG_BASE + 0x30)

#define NAND_CONFIGURATION1_REG         (NAND_REG_BASE + 0x34)
#define NAND_CONFIGURATION1_NFC_RST     (1 << 2)
#define NAND_CONFIGURATION1_NF_CE       (1 << 1)
#define NAND_CONFIGURATION1_SP_EN       (1 << 0)

#define NAND_ECC_STATUS_RESULT_REG      (NAND_REG_BASE + 0x38)

#define NAND_STATUS_SUM_REG             (NAND_REG_BASE + 0x3C)

#define NAND_LAUNCH_REG                 (NAND_REG_BASE + 0x40)
#define NAND_LAUNCH_FCMD                (1 << 0)
#define NAND_LAUNCH_FADD                (1 << 1)
#define NAND_LAUNCH_FDI                 (1 << 2)
#define NAND_LAUNCH_AUTO_PROG           (1 << 6)
#define NAND_LAUNCH_AUTO_READ           (1 << 7)
#define NAND_LAUNCH_AUTO_READ_CONT      (1 << 8)
#define NAND_LAUNCH_AUTO_ERASE          (1 << 9)
#define NAND_LAUNCH_COPY_BACK0          (1 << 10)
#define NAND_LAUNCH_COPY_BACK1          (1 << 11)
#define NAND_LAUNCH_AUTO_STAT           (1 << 12)

#define NFC_WR_PROT_REG                 (NFC_IP_BASE + 0x00)
#define UNLOCK_BLK_ADD0_REG             (NFC_IP_BASE + 0x04)
#define UNLOCK_BLK_ADD1_REG             (NFC_IP_BASE + 0x08)
#define UNLOCK_BLK_ADD2_REG             (NFC_IP_BASE + 0x0C)
#define UNLOCK_BLK_ADD3_REG             (NFC_IP_BASE + 0x10)
#define UNLOCK_BLK_ADD4_REG             (NFC_IP_BASE + 0x14)
#define UNLOCK_BLK_ADD5_REG             (NFC_IP_BASE + 0x18)
#define UNLOCK_BLK_ADD6_REG             (NFC_IP_BASE + 0x1C)
#define UNLOCK_BLK_ADD7_REG             (NFC_IP_BASE + 0x20)

#define NFC_FLASH_CONFIG2_REG           (NFC_IP_BASE + 0x24)
#define NFC_FLASH_CONFIG2_ECC_EN        (1 << 3)

#define NFC_FLASH_CONFIG3_REG           (NFC_IP_BASE + 0x28)

#define NFC_IPC_REG                     (NFC_IP_BASE + 0x2C)
#define NFC_IPC_INT                     (1 << 31)
#define NFC_IPC_AUTO_DONE               (1 << 30)
#define NFC_IPC_LPS                     (1 << 29)
#define NFC_IPC_RB_B                    (1 << 28)
#define NFC_IPC_CACK                    (1 << 1)
#define NFC_IPC_CREQ                    (1 << 0)
#define NFC_AXI_ERR_ADD_REG             (NFC_IP_BASE + 0x30)

#define MXC_MMC_BASE_DUMMY              0x00000000

#define NAND_FLASH_BOOT                 0x10000000
#define FROM_NAND_FLASH                 NAND_FLASH_BOOT

#define SDRAM_NON_FLASH_BOOT            0x20000000

#define MMC_FLASH_BOOT                  0x40000000
#define FROM_MMC_FLASH                  MMC_FLASH_BOOT

#define SPI_NOR_FLASH_BOOT              0x80000000
#define FROM_SPI_NOR_FLASH              SPI_NOR_FLASH_BOOT

#define IS_BOOTING_FROM_NAND()          (0)
#define IS_BOOTING_FROM_SPI_NOR()       (0)
#define IS_BOOTING_FROM_NOR()           (0)
#define IS_BOOTING_FROM_SDRAM()         (0)
#define IS_BOOTING_FROM_MMC()           (0)

#ifndef MXCFLASH_SELECT_NAND
#define IS_FIS_FROM_NAND()              0
#else
#define IS_FIS_FROM_NAND()              (_mxc_fis == FROM_NAND_FLASH)
#endif

#ifndef MXCFLASH_SELECT_MMC
#define IS_FIS_FROM_MMC()               0
#else
#define IS_FIS_FROM_MMC()               (_mxc_fis == FROM_MMC_FLASH)
#endif

#define IS_FIS_FROM_SPI_NOR()           (_mxc_fis == FROM_SPI_NOR_FLASH)

#define IS_FIS_FROM_NOR()               0

/*
 * This macro is used to get certain bit field from a number
 */
#define MXC_GET_FIELD(val, len, sh)          ((val >> sh) & ((1 << len) - 1))

/*
 * This macro is used to set certain bit field inside a number
 */
#define MXC_SET_FIELD(val, len, sh, nval)    ((val & ~(((1 << len) - 1) << sh)) | (nval << sh))

#define L2CC_ENABLED
#define UART_WIDTH_32         /* internal UART is 32bit access only */

#if !defined(__ASSEMBLER__)
void cyg_hal_plf_serial_init(void);
void cyg_hal_plf_serial_stop(void);
void hal_delay_us(unsigned int usecs);
#define HAL_DELAY_US(n)     hal_delay_us(n)
extern int _mxc_fis;
extern unsigned int system_rev;

enum plls {
    PLL1,
    PLL2,
    PLL3,
};

enum main_clocks {
        CPU_CLK,
        AHB_CLK,
        IPG_CLK,
        IPG_PER_CLK,
        DDR_CLK,
        NFC_CLK,
        USB_CLK,
};

enum peri_clocks {
        UART1_BAUD,
        UART2_BAUD,
        UART3_BAUD,
        SSI1_BAUD,
        SSI2_BAUD,
        CSI_BAUD,
        MSTICK1_CLK,
        MSTICK2_CLK,
        SPI1_CLK = CSPI1_BASE_ADDR,
        SPI2_CLK = CSPI2_BASE_ADDR,
};

unsigned int pll_clock(enum plls pll);

unsigned int get_main_clock(enum main_clocks clk);

unsigned int get_peri_clock(enum peri_clocks clk);

typedef unsigned int nfc_setup_func_t(unsigned int, unsigned int, unsigned int, unsigned int);

#endif //#if !defined(__ASSEMBLER__)

#endif /* __HAL_SOC_H__ */
