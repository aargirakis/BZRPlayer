/**
 * @ingroup   debug68_devel
 * @file      SC68debug_breakpoint.h
 * @author    Ben(jamin) Gerard <ben@sashipa.com>
 * @date      1999/07/14
 * @brief     debug68 breakpoints
 * @version   $Id: SC68debug_breakpoint.h,v 2.0 2003/08/21 04:58:35 benjihan Exp $
 */

/*
 *                       sc68 - debug68 breakpoints
 *         Copyright (C) 2001 Ben(jamin) Gerard <ben@sashipa.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef _SC68DEBUG_BREAKPOINT_H_
#define _SC68DEBUG_BREAKPOINT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "emu68/type68.h"

/** Kill and reset breakpoint.
 *
 * @param   addr  Address of breakpoint to kill, ~0 for all
 *
 * @return  breakpoint internal number.
 * @retval  -1:breakpoint does not exist
 */
int SC68debugger_breakp_kill(u32 addr);

/** Display breakpoint info.
 *
 * @param   addr  Address of breakpoint to display, ~0 for all
 * @return  error-code
 * @retval  0:exist
 */
int SC68debugger_breakp_display(u32 addr);

/** Set breakpoint.
 *
 * @param addr    Address of breakpoint to set
 * @param count   # pass before break (see breakpoint_t)
 * @param reset   count reset value after break
 *
 * @return  breakpoint internal number.
 * @retval  -1:breakpoint does not exist
 */
int SC68debugger_breakp_set(u32 addr, int count, int reset);

/** General break test.
 *
 * This function must be call with addr=PC at each instruction emulation.
 *
 * @return internal breakpoint number
 * @retval -1:do not break
 *
 */
int SC68debugger_breakp_test(u32 addr);

#ifdef __cplusplus
}
#endif

#endif /*  ifndef _SC68DEBUG_BREAKPOINT_H_ */
