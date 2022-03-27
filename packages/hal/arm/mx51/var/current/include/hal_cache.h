#ifndef CYGONCE_HAL_CACHE_H
#define CYGONCE_HAL_CACHE_H

//=============================================================================
//
//      hal_cache.h
//
//      HAL cache control API
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
//=============================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_soc.h>         // Variant specific hardware definitions

//-----------------------------------------------------------------------------
// Global control of data cache

// Enable the data cache
#define HAL_DCACHE_ENABLE_L1()                                          \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc p15, 0, r1, c1, c0, 0;"                                    \
        "orr r1, r1, #0x0007;" /* enable DCache (also ensures */        \
                               /* the MMU, alignment faults, and */       \
        "mcr p15, 0, r1, c1, c0, 0"                                     \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Clean+invalidate the both D+I caches at L1 and L2 levels
#define HAL_CACHE_FLUSH_ALL()                                         \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "stmfd	sp!, {r0-r5, r7, r9-r11};"  \
	    "mrc	p15, 1, r0, c0, c0, 1;"	/*@ read clidr*/ \
	    "ands	r3, r0, #0x7000000;"	/*@ extract loc from clidr */ \
	    "mov	r3, r3, lsr #23;"	/*@ left align loc bit field*/ \
	    "beq	555f;" /* finished;" */		/*@ if loc is 0, then no need to clean*/ \
	    "mov	r10, #0;"		/*@ start clean at cache level 0*/ \
    "111:" /*"loop1: */								\
	    "add	r2, r10, r10, lsr #1;"	/*@ work out 3x current cache level*/ \
	    "mov	r1, r0, lsr r2;"	/*@ extract cache type bits from clidr*/ \
	    "and	r1, r1, #7;"		/*@ mask of the bits for current cache only*/ \
	    "cmp	r1, #2;"		/*@ see what cache we have at this level*/ \
	    "blt	444f;" /* skip;" */			/*@ skip if no cache, or just i-cache*/ \
	    "mcr	p15, 2, r10, c0, c0, 0;" /*@ select current cache level in cssr*/ \
		"mcr	p15, 0, r10, c7, c5, 4;" /*	@ isb to sych the new cssr&csidr */ \
	    "mrc	p15, 1, r1, c0, c0, 0;"	/*@ read the new csidr*/ \
	    "and	r2, r1, #7;"	/*@ extract the length of the cache lines*/ \
	    "add	r2, r2, #4;"	/*@ add 4 (line length offset) */ \
	    "ldr	r4, =0x3ff;"	\
	    "ands	r4, r4, r1, lsr #3;"	/*@ find maximum number on the way size*/ \
	    ".word 0xE16F5F14;" /*"clz	r5, r4;"	@ find bit position of way size increment*/ \
	    "ldr	r7, =0x7fff;"	\
	    "ands	r7, r7, r1, lsr #13;"	/*@ extract max number of the index size*/ \
    "222:" /* loop2:"	*/ \
	    "mov	r9, r4;"	/*@ create working copy of max way size*/	\
    "333:" /* loop3:"	*/	\
	    "orr	r11, r10, r9, lsl r5;"	/*@ factor way and cache number into r11*/ \
	    "orr	r11, r11, r7, lsl r2;" 	/*@ factor index number into r11*/	\
	    "mcr	p15, 0, r11, c7, c14, 2;" /*@ clean & invalidate by set/way */ \
	    "subs	r9, r9, #1;"	/*@ decrement the way */ \
	    "bge	333b;" /* loop3;" */		\
	    "subs	r7, r7, #1;"	/*@ decrement the index */ \
	    "bge	222b;" /* loop2;" */		\
    "444:" /* skip:"	*/			 \
	    "add	r10, r10, #2;"	/*@ increment cache number */ \
	    "cmp	r3, r10;"	\
	    "bgt	111b;" /*loop1;"	*/ \
    "555:" /* "finished:" */		\
	    "mov	r10, #0;"		 /*@ swith back to cache level 0 */ 	\
	    "mcr	p15, 2, r10, c0, c0, 0;" /*@ select current cache level in cssr */ \
		"mcr	p15, 0, r10, c7, c5, 4;" /*	@ isb to sych the new cssr&csidr */ \
		"ldmfd	sp!, {r0-r5, r7, r9-r11};"  \
    "666:" /* iflush:" */ \
        "mov    r0, #0x0;"  \
		"mcr	p15, 0, r0, c7, c5, 0;" /*	@ invalidate I+BTB */ \
		"mcr	p15, 0, r0, c7, c10, 4;" /*	@ drain WB */ \
        :                                                               \
        :                                                               \
        : "r0" /* Clobber list */                                       \
    );                                                                  \
CYG_MACRO_END

// Disable the data cache
#define HAL_DCACHE_DISABLE_C1()                                         \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc p15, 0, r1, c1, c0, 0;"                                    \
        "bic r1, r1, #0x0004;" /* disable DCache by clearing C bit */   \
                             /* but not MMU and alignment faults */     \
        "mcr p15, 0, r1, c1, c0, 0"                                     \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
    );                                                                  \
CYG_MACRO_END

// Query the state of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)                                  \
CYG_MACRO_START                                                         \
    register int reg;                                                   \
    asm volatile (                                                      \
        "nop; "                                                         \
        "nop; "                                                         \
        "nop; "                                                         \
        "nop; "                                                         \
        "nop; "                                                         \
        "mrc p15, 0, %0, c1, c0, 0;"                                    \
                  : "=r"(reg)                                           \
                  :                                                     \
        );                                                              \
    (_state_) = (0 != (4 & reg)); /* Bit 2 is DCache enable */          \
CYG_MACRO_END

//-----------------------------------------------------------------------------
// Global control of Instruction cache

// Enable the instruction cache
#define HAL_ICACHE_ENABLE_L1()                                          \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc p15, 0, r1, c1, c0, 0;"                                    \
        "orr r1, r1, #0x1000;"                                          \
        "orr r1, r1, #0x0003;"  /* enable ICache (also ensures   */     \
                                /* that MMU and alignment faults */     \
                                /* are enabled)                  */     \
        "mcr p15, 0, r1, c1, c0, 0"                                     \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Query the state of the instruction cache
#define HAL_ICACHE_IS_ENABLED(_state_)                                  \
CYG_MACRO_START                                                         \
    register cyg_uint32 reg;                                            \
    asm volatile (                                                      \
        "mrc p15, 0, %0, c1, c0, 0"                                     \
        : "=r"(reg)                                                     \
        :                                                               \
        );                                                              \
                                                                        \
    (_state_) = (0 != (0x1000 & reg)); /* Bit 12 is ICache enable */    \
CYG_MACRO_END

// Disable the instruction cache
#define HAL_ICACHE_DISABLE_L1()                                         \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc p15, 0, r1, c1, c0, 0;"                                    \
        "bic r1, r1, #0x1000;" /* disable ICache (but not MMU, etc) */  \
        "mcr p15, 0, r1, c1, c0, 0;"                                    \
        "mov r1, #0;"                                                   \
        "nop;" /* next few instructions may be via cache    */          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop"                                                           \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL_L1()
#ifdef TODO
#define HAL_ICACHE_INVALIDATE_ALL_L1()                                  \
CYG_MACRO_START                                                         \
    /* this macro can discard dirty cache lines (N/A for ICache) */     \
    asm volatile (                                                      \
        "mov r1, #0;"                                                   \
        "mcr p15, 0, r1, c7, c5, 0;"  /* flush ICache */                \
        "mcr p15, 0, r1, c8, c5, 0;"  /* flush ITLB only */             \
        "mcr p15, 0, r1, c7, c5, 4;"  /* flush prefetch buffer */       \
        "nop;" /* next few instructions may be via cache    */          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END
#endif

// Synchronize the contents of the cache with memory.
// (which includes flushing out pending writes)
#define HAL_ICACHE_SYNC()
#ifdef TODO
#define HAL_ICACHE_SYNC()                                       \
CYG_MACRO_START                                                 \
    HAL_DCACHE_SYNC(); /* ensure data gets to RAM */            \
    HAL_ICACHE_INVALIDATE_ALL(); /* forget all we know */       \
CYG_MACRO_END
#endif

#ifdef L2CC_ENABLED
// Query the state of the L2 cache
#define HAL_L2CACHE_IS_ENABLED(_state_)                         \
CYG_MACRO_START                                                         \
    register int reg;                                                   \
    asm volatile (                                                      \
        "nop; "                                                         \
        "nop; "                                                         \
        "nop; "                                                         \
        "nop; "                                                         \
        "mrc p15, 0, %0, c1, c0, 1;"                                    \
                  : "=r"(reg)                                           \
                  :                                                     \
        );                                                              \
    (_state_) = (0 != (2 & reg)); /* Bit 1 is L2 Cache enable */          \
CYG_MACRO_END

#define HAL_ENABLE_L2()                             \
{                                                   \
    asm("mrc 15, 0, r0, c1, c0, 1");    \
    asm("orr r0, r0, #0x2");            \
    asm("mcr 15, 0, r0, c1, c0, 1");    \
}

#define HAL_DISABLE_L2()                            \
{                                                   \
        asm("mrc 15, 0, r0, c1, c0, 1");    \
        asm("bic r0, r0, #0x2");            \
        asm("mcr 15, 0, r0, c1, c0, 1");    \
}

#else //L2CC_ENABLED

#define HAL_ENABLE_L2()
#define HAL_DISABLE_L2()
#endif //L2CC_ENABLED

/*********************** Exported macros *******************/

#define HAL_DCACHE_ENABLE() {           \
        HAL_ENABLE_L2();                \
        HAL_DCACHE_ENABLE_L1();         \
}

#define HAL_DCACHE_DISABLE() {          \
        HAL_CACHE_FLUSH_ALL();        \
        HAL_DCACHE_DISABLE_C1();               \
}

#define HAL_DCACHE_INVALIDATE_ALL() {   \
        HAL_CACHE_FLUSH_ALL(); \
}

// not needed
#define HAL_DCACHE_SYNC()

#define HAL_ICACHE_INVALIDATE_ALL() {   \
        HAL_CACHE_FLUSH_ALL(); \
}

#define HAL_ICACHE_DISABLE() {          \
        HAL_ICACHE_DISABLE_L1();        \
}

#define HAL_ICACHE_ENABLE() {           \
        HAL_ICACHE_ENABLE_L1();         \
}

#define CYGARC_HAL_MMU_OFF(__paddr__)  \
        "mrc p15, 0, r0, c1, c0, 0;" /* read c1 */                      \
        "bic r0, r0, #0x7;" /* disable DCache and MMU */                \
        "bic r0, r0, #0x1000;" /* disable ICache */                     \
        "mcr p15, 0, r0, c1, c0, 0;" /*  */                             \
        "nop;" /* flush i+d-TLBs */                                     \
        "nop;" /* flush i+d-TLBs */                                     \
        "nop;" /* flush i+d-TLBs */

#define HAL_MMU_OFF() \
CYG_MACRO_START          \
    asm volatile (                                                      \
        CYGARC_HAL_MMU_OFF()   \
    );      \
CYG_MACRO_END

#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
