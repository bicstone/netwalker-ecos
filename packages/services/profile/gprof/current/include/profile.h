#ifndef CYGONCE_PROFILE_H
#define CYGONCE_PROFILE_H

//==========================================================================
//
//      profile.h
//
//      Application profiling support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 eCosCentric Limited
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: bartv
// Date:         2002-11-14
// Purpose:      Define profiling support
// Description:  
//              
// Usage:
//               #include <cyg/profile/profile.h>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/profile_gprof.h>
#include <cyg/infra/cyg_type.h>

// Enable profiling
__externC void profile_on(void *start_addr, void *end_addr, 
                          int bucket_size, int sample_resolution);

// Disable and reset profiling
__externC void profile_off(void);

// Callback used by timer routine
__externC void __profile_hit(CYG_ADDRWORD pc);

// Callgraph support
__externC void __profile_mcount(CYG_ADDRWORD /* caller_pc */, CYG_ADDRWORD /* callee_pc */);

// Timer setup routine, used when enabling profiling
__externC int hal_enable_profile_timer(int resolution);

#endif // CYGONCE_PROFILE_H
