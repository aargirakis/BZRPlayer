/**
 * @ingroup   debug68_devel
 * @file      debug68/SC68debug_debugcom.h
 * @author    Benjamin Gerard <ben@sashipa.com>
 * @date      1999/07/11
 * @brief     debug68 debugger
 *
 * $Id: SC68debug_debugcom.h,v 2.0 2003/08/21 04:58:35 benjihan Exp $
 */

/*
 *                          sc68 - debug68 debugger
 *         Copyright (C) 2001 Benjamin Gerard <ben@sashipa.com>
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

#ifndef _SC68DEBUG_DEBUGCOM_H_
#define _SC68DEBUG_DEBUGCOM_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Enter debugger mode.
 */
int SC68debugger_entering(void);

/** Run a new debugger command.
 *
 * @param   na  number of argument, 0:previous command
 * @param   a   array of argument.
 *
 */
int SC68debugger_newcom(int na, char **a);

/** Display debugger shell prompt as "diskname track desa>".
 *
 *  @param  diskname  Used to build prompt.
 *  @param  track_num Used to build prompt.
 *
 *  @return prompt string.
 *
 * @warning Return pointer to a static buffer.
 */
char * SC68debugger_prompt(char *diskname, int track_num);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _SC68DEBUG_DEBUGCOM_H_ */
