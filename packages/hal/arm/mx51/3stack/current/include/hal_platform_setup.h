#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

//=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
//
//=============================================================================
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
//===========================================================================

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_soc.h>            // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/fsl_board.h>          // Platform specific hardware definitions

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
#define PLATFORM_SETUP1 _platform_setup1
#define CYGHWR_HAL_ARM_HAS_MMU

#ifdef CYG_HAL_STARTUP_ROMRAM
#define CYGSEM_HAL_ROM_RESET_USES_JUMP
#endif

//#define NFC_2K_BI_SWAP
#define SDRAM_FULL_PAGE_BIT     0x100
#define SDRAM_FULL_PAGE_MODE    0x37
#define SDRAM_BURST_MODE        0x33

#define CYGHWR_HAL_ROM_VADDR    0x0

#if 0
#define UNALIGNED_ACCESS_ENABLE
#define SET_T_BIT_DISABLE
#define BRANCH_PREDICTION_ENABLE
#endif

#define DCDGEN(i,type, addr, data) \
dcd_##i:                         ;\
    .long type                   ;\
    .long addr                   ;\
    .long data

#define PLATFORM_PREAMBLE flash_header
//flash header & DCD @ 0x400
.macro flash_header
    b reset_vector
    .org 0x400
app_code_jump_v:    .long reset_vector
app_code_barker:    .long 0xB1
app_code_csf:       .long 0
dcd_ptr_ptr:        .long dcd_ptr
super_root_key:	    .long 0
dcd_ptr:            .long dcd_data
app_dest_ptr:       .long 0x97f00000

dcd_data:      	    .long 0xB17219E9   // Fixed. can't change.
#ifdef IMX51_TO_2

#ifdef IMX51_MDDR
// DCD
dcd_len:            .long (38*12)
// DDR IOMUX configuration
// Control, Data, Address pads are in their default state: HIGH DS, FAST SR.
DCDGEN(1, 4, IOMUXC_BASE_ADDR + 0x4b8, 0x000000e7)  // IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCLK       MAX DS
DCDGEN(2, 4, IOMUXC_BASE_ADDR + 0x4d4, 0x000000e4)  // DQM0 DS high, slew rate slow
DCDGEN(3, 4, IOMUXC_BASE_ADDR + 0x4d8, 0x000000e4)  // DQM1 DS high, slew rate slow
DCDGEN(4, 4, IOMUXC_BASE_ADDR + 0x4dc, 0x000000e4)  // DQM2 DS high, slew rate slow
DCDGEN(5, 4, IOMUXC_BASE_ADDR + 0x4e0, 0x000000e4)  // DQM3 DS high, slew rate slow
DCDGEN(6, 4, IOMUXC_BASE_ADDR + 0x4bc, 0x000000c4)  // SDQS0 DS high, slew rate slow
DCDGEN(7, 4, IOMUXC_BASE_ADDR + 0x4c0, 0x000000c4)  // SDQS1 DS high, slew rate slow
DCDGEN(8, 4, IOMUXC_BASE_ADDR + 0x4c4, 0x000000c4)  // SDQS2 DS high, slew rate slow
DCDGEN(9, 4, IOMUXC_BASE_ADDR + 0x4c8, 0x000000c4)  // SDQS3 DS high, slew rate slow
DCDGEN(10, 4, IOMUXC_BASE_ADDR + 0x8a4, 0x00000004)  // DRAM_B0
DCDGEN(11, 4, IOMUXC_BASE_ADDR + 0x8ac, 0x00000004)  // DRAM_B1
DCDGEN(12, 4, IOMUXC_BASE_ADDR + 0x8b8, 0x00000004)  // DRAM_B2
DCDGEN(13, 4, IOMUXC_BASE_ADDR + 0x82c, 0x00000004)  // DRAM_B3
DCDGEN(14, 4, IOMUXC_BASE_ADDR + 0x878, 0x00000000)  // DRAM_B0_SR
DCDGEN(15, 4, IOMUXC_BASE_ADDR + 0x880, 0x00000000)  // DRAM_B1_SR
DCDGEN(16, 4, IOMUXC_BASE_ADDR + 0x88c, 0x00000000)  // DRAM_B2_SR
DCDGEN(17, 4, IOMUXC_BASE_ADDR + 0x89c, 0x00000000)  // DRAM_B3_SR
// Configure CS0
DCDGEN(18, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCTL0, 0x83220000) // ESDCTL0: Enable controller
DCDGEN(19, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x04008008) // ESDSCR: Precharge command
DCDGEN(20, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008010) // ESDSCR: Refresh command
DCDGEN(21, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008010) // ESDSCR: Refresh command
DCDGEN(22, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00338018) // ESDSCR: LMR with CAS=3 and BL=3 (Burst Length = 8)
DCDGEN(23, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0060801a) // ESDSCR: EMR with Half Drive strength
DCDGEN(24, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008000) // ESDSCR
DCDGEN(25, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCTL0, 0xC3220000) // ESDCTL0: 14 ROW, 10 COL, 32Bit, SREF=8
// ESDCFG0: tRFC:22clks, tXSR:28clks, tXP:2clks, tWTR:2clk, tRP:3clks, tMRD:2clks
//          tRAS:8clks, tRRD:2clks, tWR:3clks, tRCD:3clks, tRC:11clks
DCDGEN(26, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCFG0, 0xC33574AA)
DCDGEN(27, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDMISC, 0x000a1700) // ESDMISC: AP=10, Bank interleaving on, MIF3 en, RALAT=2
// Configure CS1
DCDGEN(28, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCTL1, 0x83220000) // ESDCTL1: Enable controller
DCDGEN(29, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0400800c) // ESDSCR: Precharge command
DCDGEN(30, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008014) // ESDSCR: Refresh command
DCDGEN(31, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008014) // ESDSCR: Refresh command
DCDGEN(32, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0033801c) // ESDSCR: LMR with CAS=3 and BL=3 (Burst Length = 8)
DCDGEN(33, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0060801e) // ESDSCR: EMR with Half Drive strength
DCDGEN(34, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008004) // ESDSCR
DCDGEN(35, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCTL1, 0xC3220000) // ESDCTL1: 14 ROW, 10 COL, 32Bit, SREF=8
// ESDCFG0: tRFC:22clks, tXSR:28clks, tXP:2clks, tWTR:2clk, tRP:3clks, tMRD:2clks
//          tRAS:8clks, tRRD:2clks, tWR:3clks, tRCD:3clks, tRC:11clks
DCDGEN(36, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCFG1, 0xC33574AA)
DCDGEN(37, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00000000) // ESDSCR - clear "configuration request" bit
DCDGEN(38, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCDLY5, 0x00f58000) //Delay line write - -11

#else
dcd_len:            .long (56*12)

//DCD
//DDR2 IOMUX configuration
DCDGEN(1, 4, IOMUXC_BASE_ADDR + 0x8a0, 0x200)
DCDGEN(2, 4, IOMUXC_BASE_ADDR + 0x50c, 0x20c5)
DCDGEN(3, 4, IOMUXC_BASE_ADDR + 0x510, 0x20c5)
DCDGEN(4, 4, IOMUXC_BASE_ADDR + 0x83c, 0x2)
DCDGEN(5, 4, IOMUXC_BASE_ADDR + 0x848, 0x2)
DCDGEN(6, 4, IOMUXC_BASE_ADDR + 0x4b8, 0xe7)
DCDGEN(7, 4, IOMUXC_BASE_ADDR + 0x4bc, 0x45)
DCDGEN(8, 4, IOMUXC_BASE_ADDR + 0x4c0, 0x45)
DCDGEN(9, 4, IOMUXC_BASE_ADDR + 0x4c4, 0x45)
DCDGEN(10, 4, IOMUXC_BASE_ADDR + 0x4c8, 0x45)
DCDGEN(11, 4, IOMUXC_BASE_ADDR + 0x820, 0x0)
DCDGEN(12, 4, IOMUXC_BASE_ADDR + 0x4a4, 0x3)
DCDGEN(13, 4, IOMUXC_BASE_ADDR + 0x4a8, 0x3)
DCDGEN(14, 4, IOMUXC_BASE_ADDR + 0x4ac, 0xe3)
DCDGEN(15, 4, IOMUXC_BASE_ADDR + 0x4b0, 0xe3)
DCDGEN(16, 4, IOMUXC_BASE_ADDR + 0x4b4, 0xe3)
DCDGEN(17, 4, IOMUXC_BASE_ADDR + 0x4cc, 0xe3)
DCDGEN(18, 4, IOMUXC_BASE_ADDR + 0x4d0, 0xe2)
//Set drive strength to MAX
DCDGEN(19, 4, IOMUXC_BASE_ADDR + 0x82c, 0x6)
DCDGEN(20, 4, IOMUXC_BASE_ADDR + 0x8a4, 0x6)
DCDGEN(21, 4, IOMUXC_BASE_ADDR + 0x8ac, 0x6)
DCDGEN(22, 4, IOMUXC_BASE_ADDR + 0x8b8, 0x6)
//13 ROW, 10 COL, 32Bit, SREF=4 Micron Model
//CAS=3,  BL=4
DCDGEN(23, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCTL0, 0x82a20000)
DCDGEN(24, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCTL1, 0x82a20000)
DCDGEN(25, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDMISC, 0x000ad0d0)
DCDGEN(26, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCFG0, 0x333574aa)
DCDGEN(27, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCFG1, 0x333574aa)
// Init DRAM on CS0
DCDGEN(28, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x04008008)
DCDGEN(29, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0000801a)
DCDGEN(30, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0000801b)
DCDGEN(31, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00448019)
DCDGEN(32, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x07328018)
DCDGEN(33, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x04008008)
DCDGEN(34, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008010)
DCDGEN(35, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008010)
DCDGEN(36, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x06328018)
DCDGEN(37, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x03808019)
DCDGEN(38, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00408019)
DCDGEN(39, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008000)

// Init DRAM on CS1
DCDGEN(40, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0400800c)
DCDGEN(41, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0000801e)
DCDGEN(42, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0000801f)
DCDGEN(43, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0000801d)
DCDGEN(44, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0732801c)
DCDGEN(45, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0400800c)
DCDGEN(46, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008014)
DCDGEN(47, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008014)
DCDGEN(48, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0632801c)
DCDGEN(49, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0380801d)
DCDGEN(50, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0040801d)
DCDGEN(51, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008004)

DCDGEN(52, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCTL0, 0xb2a20000)
DCDGEN(53, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCTL1, 0xb2a20000)
DCDGEN(54, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDMISC, 0x000ad6d0)
DCDGEN(55, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCDLYGD, 0x90000000)
DCDGEN(56, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00000000)
#endif
#else
dcd_len:            .long (9*12)

//DCD
    //    ldr r0, ESDCTL_BASE_W
    //    /* Set CSD0 */
    //    ldr r1, =0x80000000
    //    str r1, [r0, #ESDCTL_ESDCTL0]
DCDGEN(1, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCTL0, 0x80000000)
    //    /* Precharge command */
    //    ldr r1, SDRAM_0x04008008
    //    str r1, [r0, #ESDCTL_ESDSCR]
DCDGEN(2, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x04008008)
    //    /* 2 refresh commands */
    //    ldr r1, SDRAM_0x00008010
    //    str r1, [r0, #ESDCTL_ESDSCR]
DCDGEN(3, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008010)
    //    str r1, [r0, #ESDCTL_ESDSCR]
DCDGEN(4, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00008010)
    //    /* LMR with CAS=3 and BL=3 */
    //    ldr r1, SDRAM_0x00338018
    //    str r1, [r0, #ESDCTL_ESDSCR]
DCDGEN(5, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x00338018)
    //    /* 13 ROW, 10 COL, 32Bit, SREF=4 Micron Model */
    //    ldr r1, SDRAM_0xB2220000
    //    str r1, [r0, #ESDCTL_ESDCTL0]
DCDGEN(6, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCTL0, 0xB2220000)
    //    /* Timing parameters */
    //    ldr r1, SDRAM_0xB02567A9
    //    str r1, [r0, #ESDCTL_ESDCFG0]
DCDGEN(7, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDCFG0, 0xB02567A9)
    //    /* MDDR enable, RLAT=2 */
    //    ldr r1, SDRAM_0x000A0104
    //    str r1, [r0, #ESDCTL_ESDMISC]
DCDGEN(8, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDMISC, 0x000A0104)
    //    /* Normal mode */
    //    ldr r1, =0x00000000
    //    str r1, [r0, #ESDCTL_ESDSCR]
DCDGEN(9, 4, ESDCTL_BASE_ADDR + ESDCTL_ESDSCR, 0x0)
#endif
image_len:           .long 256*1024

.endm

//#define ENABLE_IMPRECISE_ABORT

// This macro represents the initial startup code for the platform
    .macro  _platform_setup1
FSL_BOARD_SETUP_START:

#ifdef IMX51_MDDR
        ldr r0, =GPC_BASE_ADDR
        ldr r1, =0x1FC00000
        str r1, [r0, #4]
#endif

#ifdef ENABLE_IMPRECISE_ABORT
        mrs r1, spsr            // save old spsr
        mrs r0, cpsr            // read out the cpsr
        bic r0, r0, #0x100      // clear the A bit
        msr spsr, r0            // update spsr
        add lr, pc, #0x8        // update lr
        movs pc, lr             // update cpsr
        nop
        nop
        nop
        nop
        msr spsr, r1            // restore old spsr
#endif

    // explicitly disable L2 cache
    mrc 15, 0, r0, c1, c0, 1
    bic r0, r0, #0x2
    mcr 15, 0, r0, c1, c0, 1

    // reconfigure L2 cache aux control reg
    mov r0, #0xC0		// tag RAM
    add r0, r0, #0x4	// data RAM
    orr r0, r0, #(1 << 25)    // disable write combine
    orr r0, r0, #(1 << 24)    // disable write allocate delay
    orr r0, r0, #(1 << 23)    // disable write allocate combine
    orr r0, r0, #(1 << 22)    // disable write allocate

    mcr 15, 1, r0, c9, c0, 2

    /* Reload data from spare area to 0x400 of main area if booting from NAND */
    ldr r0, NFC_BASE_W
    cmp pc, r0
    blo 1f
    cmp pc, r1
    bhi 1f
1:
    /* Store the boot type, from NAND or SDRAM */
    mov r11, #SDRAM_NON_FLASH_BOOT

init_spba_start:
    init_spba
init_aips_start:
    init_aips
init_max_start:
    init_max
init_m4if_start:
    init_m4if
init_iomux_start:
//    init_iomux

    // disable wdog
    ldr r0, =0x30
    ldr r1, WDOG1_BASE_W
    strh r0, [r1]

    /* Skip clock setup if already booted up */
    ldr r0, =IRAM_BASE_ADDR
    ldr r0, [r0]
    ldr r1, =FROM_SPI_NOR_FLASH
    cmp r0, r1
    beq Normal_Boot_Continue
    ldr r1, =FROM_MMC_FLASH
    cmp r0, r1
    beq Normal_Boot_Continue
    ldr r1, =FROM_NAND_FLASH
    cmp r0, r1
    beq Normal_Boot_Continue

    /* If SDRAM has been setup, bypass clock/WEIM setup */
    cmp pc, #SDRAM_BASE_ADDR
    blo external_boot_cont
    cmp pc, #(SDRAM_BASE_ADDR + SDRAM_SIZE)
    blo internal_boot_cont

external_boot_cont:

init_sdram_start:
    setup_sdram


internal_boot_cont:
init_clock_start:
    init_clock

HWInitialise_skip_SDRAM_setup:
    ldr r0, NFC_BASE_W
    add r2, r0, #0x1000      // 4K window
    cmp pc, r0
    blo Normal_Boot_Continue
    cmp pc, r2
    bhi Normal_Boot_Continue

NAND_Boot_Start:
    /* Copy image from flash to SDRAM first */
    ldr r1, MXC_REDBOOT_ROM_START
1:  ldmia r0!, {r3-r10}
    stmia r1!, {r3-r10}
    cmp r0, r2
    blo 1b

    /* Jump to SDRAM */
    ldr r1, CONST_0x0FFF
    and r0, pc, r1     /* offset of pc */
    ldr r1, MXC_REDBOOT_ROM_START
    add r1, r1, #0x10
    add pc, r0, r1
    nop
    nop
    nop
    nop
    nop
    nop

NAND_Copy_Main:
    ldr r11, NFC_IP_BASE_W  //r11: NFC IP register base. Doesn't change
    ldr r0, [r11, #0x24]
    and r0, r0, #3
    cmp r0, #1
    mov r0, #4096
    moveq r0, #2048
    movlt r0, #512
    ldr r1, =_nand_pg_sz // r1 -> _nand_pg_sz
    str r0, [r1]    // store the page size into the global variable _nand_pg_sz

    ldr r0, NFC_BASE_W   //r0: nfc base. Reloaded after each page copying
    ldr r1, _nand_pg_sz //r1: starting flash addr to be copied. Updated constantly
    add r2, r0, #0x800   //r2: end of 3rd RAM buf. Doesn't change ?? dynamic
    cmp r1, #2048
    addgt r2, r2, #2048 // for 4K page, copy 4K instead of 2K

    add r12, r0, #0x1E00  //r12: NFC AXI register base. Doesn't change
    ldr r14, MXC_REDBOOT_ROM_START
    add r13, r14, #REDBOOT_IMAGE_SIZE //r13: end of SDRAM address for copying. Doesn't change
    add r14, r14, r1     //r14: starting SDRAM address for copying. Updated constantly

    //unlock internal buffer
    mov r3, #0xFF000000
    add r3, r3, #0x00FF0000
    str r3, [r11, #0x4]
    str r3, [r11, #0x8]
    str r3, [r11, #0xC]
    str r3, [r11, #0x10]
    str r3, [r11, #0x14]
    str r3, [r11, #0x18]
    str r3, [r11, #0x1C]
    str r3, [r11, #0x20]
    mov r4, #0x7
    mov r3, #0x84
1:  add r5, r3, r4, lsr #3
    str r5, [r11, #0x0]
    subs r4, r4, #0x1
    bne 1b

    mov r3, #0
    str r3, [r11, #0x2C]

Nfc_Read_Page:
    //start_nfc_addr_ops1(pg_no, pg_off);
    ldr r3, _nand_pg_sz
    cmp r3, #2048
    // TODO: need to deal with 512B page
    movgt r3, r1, lsr #12  // get the page number for 4K page nand
    moveq r3, r1, lsr #11  // get the page number for 2K page nand
    mov r3, r3, lsl #16
    str r3, [r12, #0x4] // set the addr

    // writel((FLASH_Read_Mode1_LG << 8) | FLASH_Read_Mode1, NAND_CMD_REG);
    mov r3, #0x3000
    str r3, [r12, #0x0]

    // writel(0x00000000, NAND_CONFIGURATION1_REG);
    mov r3, #0x0
    str r3, [r12, #0x34]

    // start auto-read
    //writel(NAND_LAUNCH_AUTO_READ, NAND_LAUNCH_REG);
    mov r3, #NAND_LAUNCH_AUTO_READ
    str r3, [r12, #0x40]

    do_wait_op_done

Copy_Good_Blk:
    //copying page
1:  ldmia r0!, {r3-r10}
    stmia r14!, {r3-r10}
    cmp r0, r2
    blo 1b
    cmp r14, r13
    bge NAND_Copy_Main_done
    ldr r3, _nand_pg_sz
    add r1, r1, r3
    ldr r0, NFC_BASE_W
    b Nfc_Read_Page

NAND_Copy_Main_done:

Normal_Boot_Continue:

#ifdef CYG_HAL_STARTUP_ROMRAM     /* enable running from RAM */
    /* Copy image from flash to SDRAM first */
    ldr r0, =0xFFFFF000
    and r0, r0, pc
    ldr r1, MXC_REDBOOT_ROM_START
    cmp r0, r1
    beq HWInitialise_skip_SDRAM_copy

    add r2, r0, #REDBOOT_IMAGE_SIZE

1:  ldmia r0!, {r3-r10}
    stmia r1!, {r3-r10}
    cmp r0, r2
    ble 1b
    /* Jump to SDRAM */
    ldr r1, =0xFFFF
    and r0, pc, r1         /* offset of pc */
    ldr r1, =(SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000 + 0x8)
    add pc, r0, r1
    nop
    nop
    nop
    nop
#endif /* CYG_HAL_STARTUP_ROMRAM */

HWInitialise_skip_SDRAM_copy:

init_cs1_start:
//    init_cs1 -- moved to plf_hardware_init()

/*
 * Note:
 *     IOMUX/PBC setup is done in C function plf_hardware_init() for simplicity
 */
STACK_Setup:
    // Set up a stack [for calling C code]
    ldr r1, =__startup_stack
    ldr r2, =RAM_BANK0_BASE
    orr sp, r1, r2

    // Create MMU tables
    bl hal_mmu_init

    /* Workaround for arm errata #709718 */
    //Setup PRRR so device is always mapped to non-shared
    mrc MMU_CP, 0, r1, c10, c2, 0 // Read Primary Region Remap Register
    bic r1, #(3 << 16)
    mcr MMU_CP, 0, r1, c10, c2, 0 // Write Primary Region Remap Register

    // Enable MMU
    ldr r2, =10f
    ldr r0, =ROM_BASE_ADDRESS
    ldr r3, [r0, #ROM_SI_REV_OFFSET]
    cmp r3, #0x1
    bne skip_L1_workaround
    // Workaround for L1 cache issue
    mrc MMU_CP, 0, r1, c10, c2, 1  // Read normal memory remap register
    bic r1, r1, #(3 << 14)       // Remap inner attribute for TEX[0],C,B = b111 as noncacheable
    bic r1, r1, #(3 << 6)       // Remap inner attribute for TEX[0],C,B = b011 as noncacheable
    bic r1, r1, #(3 << 4)       // Remap inner attribute for TEX[0],C,B = b010 as noncacheable
    mcr MMU_CP, 0, r1, c10, c2, 1  // Write normal memory remap register
skip_L1_workaround:
    mrc MMU_CP, 0, r1, MMU_Control, c0
    orr r1, r1, #7                          // enable MMU bit
    orr r1, r1, #0x800                      // enable z bit
    orr r1, r1, #(1 << 28)              // Enable TEX remap
    mcr MMU_CP, 0, r1, MMU_Control, c0

    /* Workaround for arm errata #621766 */
    mrc MMU_CP, 0, r1, MMU_Control, c0, 1
    orr r1, r1, #(1 << 5)                // enable L1NEON bit
    mcr MMU_CP, 0, r1, MMU_Control, c0, 1

    mov pc,r2    /* Change address spaces */
    nop
    nop
    nop
10:

    // Save shadow copy of BCR, also hardware configuration
    ldr r1, =_board_BCR
    str r2, [r1]
    ldr r1, =_board_CFG
    str r9, [r1]                // Saved far above...

    .endm                       // _platform_setup1

#else // defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
#define PLATFORM_SETUP1
#endif

    /* Do nothing */
    .macro  init_spba
    .endm  /* init_spba */

    /* AIPS setup - Only setup MPROTx registers. The PACR default values are good.*/
    .macro init_aips
        /*
         * Set all MPROTx to be non-bufferable, trusted for R/W,
         * not forced to user-mode.
         */
        ldr r0, AIPS1_CTRL_BASE_ADDR_W
        ldr r1, AIPS1_PARAM_W
        str r1, [r0, #0x00]
        str r1, [r0, #0x04]
        ldr r0, AIPS2_CTRL_BASE_ADDR_W
        str r1, [r0, #0x00]
        str r1, [r0, #0x04]

    .endm /* init_aips */

    /* MAX (Multi-Layer AHB Crossbar Switch) setup */
    .macro init_max
    .endm /* init_max */

    .macro    init_clock
        ldr r0, CCM_BASE_ADDR_W
        /* Disable IPU and HSC dividers */
        mov r1, #0x60000
        str r1, [r0, #CLKCTL_CCDR]

#ifdef IMX51_TO_2
        /* Make sure to switch the DDR away from PLL 1 */
        ldr r1, CCM_VAL_0x19239100
        str r1, [r0, #CLKCTL_CBCDR]
        /* make sure divider effective */
    1:  ldr r1, [r0, #CLKCTL_CDHIPR]
        cmp r1, #0x0
        bne 1b
#endif
        /* Switch ARM to step clock */
        mov r1, #0x4
        str r1, [r0, #CLKCTL_CCSR]
        setup_pll PLL1, 800

        setup_pll PLL3, 665
        /* Switch peripheral to PLL 3 */
        ldr r0, CCM_BASE_ADDR_W
        ldr r1, CCM_VAL_0x0000D3C0
        str r1, [r0, #CLKCTL_CBCMR]
        ldr r1, CCM_VAL_0x033B9145
        str r1, [r0, #CLKCTL_CBCDR]
        setup_pll PLL2, 665
        /* Switch peripheral to PLL 2 */
        ldr r0, CCM_BASE_ADDR_W
        ldr r1, CCM_VAL_0x013B9145
        str r1, [r0, #CLKCTL_CBCDR]
        ldr r1, CCM_VAL_0x0000E3C0
        str r1, [r0, #CLKCTL_CBCMR]

        setup_pll PLL3, 216

        /* Set the platform clock dividers */
        ldr r0, PLATFORM_BASE_ADDR_W
        ldr r1, PLATFORM_CLOCK_DIV_W
        str r1, [r0, #PLATFORM_ICGC]

        /* Switch ARM back to PLL 1. */
        ldr r0, CCM_BASE_ADDR_W
        mov r1, #0x0
        str r1, [r0, #CLKCTL_CCSR]

        /* setup the rest */
        mov r1, #0
        str r1, [r0, #CLKCTL_CACRR]

        /* Use lp_apm (24MHz) source for perclk */
#ifdef IMX51_TO_2
        ldr r1, CCM_VAL_0x000020C2
        str r1, [r0, #CLKCTL_CBCMR]
        // ddr clock from PLL 1, all perclk dividers are 1 since using 24MHz
#ifdef IMX51_MDDR
        ldr r1, CCM_VAL_0x61239100
#else
        ldr r1, CCM_VAL_0x59239100
#endif
        str r1, [r0, #CLKCTL_CBCDR]
#else
        ldr r1, CCM_VAL_0x0000E3C2
        str r1, [r0, #CLKCTL_CBCMR]
        // emi=ahb, all perclk dividers are 1 since using 24MHz
        ldr r1, CCM_VAL_0x013B9100
        str r1, [r0, #CLKCTL_CBCDR]
#endif
        /* Use PLL 2 for UART's, get 66.5MHz from it */
        ldr r1, CCM_VAL_0xA5A2A020
        str r1, [r0, #CLKCTL_CSCMR1]
        ldr r1, CCM_VAL_0x00C30321
        str r1, [r0, #CLKCTL_CSCDR1]

        /* make sure divider effective */
    1:  ldr r1, [r0, #CLKCTL_CDHIPR]
        cmp r1, #0x0
        bne 1b

        mov r1, #0x00000
        str r1, [r0, #CLKCTL_CCDR]

        // for cko - for ARM div by 8
        mov r1, #0x000A0000
        add r1, r1, #0x00000F0
        str r1, [r0, #CLKCTL_CCOSR]
    .endm /* init_clock */

    .macro setup_pll pll_nr, mhz
        ldr r0, BASE_ADDR_W_\pll_nr
        ldr r1, PLL_VAL_0x1232
        str r1, [r0, #PLL_DP_CTL]     /* Set DPLL ON (set UPEN bit); BRMO=1 */
        ldr r1, =0x2
        str r1, [r0, #PLL_DP_CONFIG]  /* Enable auto-restart AREN bit */

        ldr r1, W_DP_OP_\mhz
        str r1, [r0, #PLL_DP_OP]
        str r1, [r0, #PLL_DP_HFS_OP]

        ldr r1, W_DP_MFD_\mhz
        str r1, [r0, #PLL_DP_MFD]
        str r1, [r0, #PLL_DP_HFS_MFD]

        ldr r1, W_DP_MFN_\mhz
        str r1, [r0, #PLL_DP_MFN]
        str r1, [r0, #PLL_DP_HFS_MFN]

        /* Now restart PLL */
        ldr r1, PLL_VAL_0x1232
        str r1, [r0, #PLL_DP_CTL]
wait_pll_lock\pll_nr\mhz:
        ldr r1, [r0, #PLL_DP_CTL]
        ands r1, r1, #0x1
        beq wait_pll_lock\pll_nr\mhz
    .endm

    /* M3IF setup */
    .macro init_m4if
        ldr r1, M4IF_BASE_W
#ifdef IMX51_TO_2
        ldr r0, M4IF_0x00000203
        str r0, [r1, #M4IF_FBPM0]

        ldr r0, =0x0
        str r0, [r1, #M4IF_FBPM1]

        ldr r0, M4IF_0x00120125
        str r0, [r1, #M4IF_FPWC]

        ldr r0, M4IF_0x001901A3
        str r0, [r1, #M4IF_MIF4]
#else
        /* IPU accesses with ID=0x1 given highest priority (=0xA) */
        ldr r0, M4IF_0x00000a01
        str r0, [r1, #M4IF_FIDBP]
        /* Configure M4IF registers, VPU and IPU given higher priority (=0x4) */
        ldr r0, M4IF_0x00000404
        str r0, [r1, #M4IF_FBPM0]
#endif

    .endm /* init_m4if */

    .macro setup_sdram
        ldr r0, ESDCTL_BASE_W
        /* Set CSD0 */
        ldr r1, =0x80000000
        str r1, [r0, #ESDCTL_ESDCTL0]
        /* Precharge command */
        ldr r1, SDRAM_0x04008008
        str r1, [r0, #ESDCTL_ESDSCR]
        /* 2 refresh commands */
        ldr r1, SDRAM_0x00008010
        str r1, [r0, #ESDCTL_ESDSCR]
        str r1, [r0, #ESDCTL_ESDSCR]
        /* LMR with CAS=3 and BL=3 */
        ldr r1, SDRAM_0x00338018
        str r1, [r0, #ESDCTL_ESDSCR]
        /* 13 ROW, 10 COL, 32Bit, SREF=4 Micron Model */
        ldr r1, SDRAM_0xB2220000
        str r1, [r0, #ESDCTL_ESDCTL0]
        /* Timing parameters */
//        ldr r1, SDRAM_0x899F6BBA
        ldr r1, SDRAM_0xB02567A9
        str r1, [r0, #ESDCTL_ESDCFG0]
        /* MDDR enable, RLAT=2 */
        ldr r1, SDRAM_0x000A0104
        str r1, [r0, #ESDCTL_ESDMISC]
        /* Normal mode */
        ldr r1, =0x00000000
        str r1, [r0, #ESDCTL_ESDSCR]
    .endm

    .macro do_wait_op_done
    1:
        ldr r3, [r11, #0x2C]
        ands r3, r3, #NFC_IPC_INT
        beq 1b
        mov r3, #0x0
        str r3, [r11, #0x2C]
    .endm   // do_wait_op_done

    .macro  init_iomux
        // do nothing
    .endm /* init_iomux */

#define PLATFORM_VECTORS         _platform_vectors
    .macro  _platform_vectors
        .globl  _board_BCR, _board_CFG
_board_BCR:     .long   0       // Board Control register shadow
_board_CFG:     .long   0       // Board Configuration (read at RESET)
    .endm

WDOG1_BASE_W:           .word   WDOG1_BASE_ADDR
IIM_SREV_REG_VAL:       .word   IIM_BASE_ADDR + IIM_SREV_OFF
AIPS1_CTRL_BASE_ADDR_W: .word   AIPS1_CTRL_BASE_ADDR
AIPS2_CTRL_BASE_ADDR_W: .word   AIPS2_CTRL_BASE_ADDR
AIPS1_PARAM_W:          .word   0x77777777
MAX_BASE_ADDR_W:        .word   MAX_BASE_ADDR
MAX_PARAM1:             .word   0x00302154
ESDCTL_BASE_W:          .word   ESDCTL_BASE_ADDR
M4IF_BASE_W:            .word   M4IF_BASE_ADDR
M4IF_0x00000a01:       .word   0x00000a01
M4IF_0x00000404:       .word   0x00000404
#ifdef IMX51_TO_2
M4IF_0x00120125:       .word   0x00120125
M4IF_0x001901A3:       .word   0x001901A3
M4IF_0x00000203:       .word   0x00000203
#endif
NFC_BASE_W:             .word   NFC_BASE_ADDR_AXI
NFC_IP_BASE_W:          .word   NFC_IP_BASE
SDRAM_0x04008008:       .word   0x04008008
SDRAM_0x00008010:       .word   0x00008010
SDRAM_0x00338018:       .word   0x00338018
SDRAM_0xB2220000:       .word   0xB2220000
SDRAM_0x899F6BBA:       .word   0x899F6BBA
SDRAM_0xB02567A9:       .word   0xB02567A9
SDRAM_0x000A0104:       .word   0x000A0104
IOMUXC_BASE_ADDR_W:     .word   IOMUXC_BASE_ADDR
MXC_REDBOOT_ROM_START:  .word   SDRAM_BASE_ADDR + SDRAM_SIZE - 0x100000
CONST_0x0FFF:           .word   0x0FFF
CCM_BASE_ADDR_W:        .word   CCM_BASE_ADDR
CCM_VAL_0x0000E3C2:     .word   0x0000E3C2
CCM_VAL_0x000020C2:     .word   0x000020C2
CCM_VAL_0x013B9100:     .word   0x013B9100
CCM_VAL_0x59239100:     .word   0x59239100
CCM_VAL_0x61239100:     .word   0x61239100
CCM_VAL_0x19239100:     .word   0x19239100
CCM_VAL_0xA5A2A020:     .word   0xA5A2A020
CCM_VAL_0x00C30321:     .word   0x00C30321
CCM_VAL_0x0000D3C0:     .word   0x0000D3C0
CCM_VAL_0x033B9145:     .word   0x033B9145
CCM_VAL_0x013B9145:     .word   0x013B9145
CCM_VAL_0x0000E3C0:     .word   0x0000E3C0
PLL_VAL_0x222:          .word   0x222
PLL_VAL_0x232:          .word   0x232
BASE_ADDR_W_PLL1:       .word   PLL1_BASE_ADDR
BASE_ADDR_W_PLL2:       .word   PLL2_BASE_ADDR
BASE_ADDR_W_PLL3:       .word   PLL3_BASE_ADDR
PLL_VAL_0x1232:         .word   0x1232
W_DP_OP_800:            .word   DP_OP_800
W_DP_MFD_800:           .word   DP_MFD_800
W_DP_MFN_800:           .word   DP_MFN_800
W_DP_OP_700:            .word   DP_OP_700
W_DP_MFD_700:           .word   DP_MFD_700
W_DP_MFN_700:           .word   DP_MFN_700
W_DP_OP_400:            .word   DP_OP_400
W_DP_MFD_400:           .word   DP_MFD_400
W_DP_MFN_400:           .word   DP_MFN_400
W_DP_OP_532:            .word   DP_OP_532
W_DP_MFD_532:           .word   DP_MFD_532
W_DP_MFN_532:           .word   DP_MFN_532
W_DP_OP_665:            .word   DP_OP_665
W_DP_MFD_665:           .word   DP_MFD_665
W_DP_MFN_665:           .word   DP_MFN_665
W_DP_OP_216:            .word   DP_OP_216
W_DP_MFD_216:           .word   DP_MFD_216
W_DP_MFN_216:           .word   DP_MFN_216
PLATFORM_BASE_ADDR_W:   .word   PLATFORM_BASE_ADDR
PLATFORM_CLOCK_DIV_W:   .word   0x00000725
_nand_pg_sz:            .word 0

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
