/**
 * @ingroup   debug68_devel
 * @file      SC68debug_eval.h
 * @author    Ben(jamin) Gerard <ben@sashipa.com>
 * @date      1999/07/12
 * @brief     debug68 expression evaluator.
 * @version   $Id: SC68debug_eval.h,v 2.0 2003/08/21 04:58:35 benjihan Exp $
 */

/*
 *                   sc68 - debug68 expression evaluator
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

#ifndef _SC68DEBUG_EVAL_H_
#define _SC68DEBUG_EVAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Evaluate a string 0 or space terminated.
 *
 * @param   s   string to evaluate
 * @param   err_loc   pointer to error location
 */
int SC68debugger_eval( char *s, char **err_loc );

#ifdef __cplusplus
}
#endif

#endif /* _SC68DEBUG_EVAL_H_ */
