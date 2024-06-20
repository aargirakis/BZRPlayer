/**
 * @ingroup   debug68_devel
 * @file      debug68/SC68debug_comline.h
 * @author    Benjamin Gerard <ben@sashipa.com>
 * @date      1999/06/06
 * @brief     debug68 command line.
 *
 * $Id: SC68debug_comline.h,v 2.0 2003/08/21 04:58:35 benjihan Exp $
 */

/*
 *                       sc68 - debug68 command line
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

#ifndef _SC68DEBUG_COMLINE_H_
#define _SC68DEBUG_COMLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Command line. */
typedef struct {
  int na;             /**< Number of args.                  */
  char * coms[128];   /**< Arguments (pointers to comline). */
  char * comline;     /**< Command line buffer.             */
} debug68_comline_t;

/** Read command line.
 *
 *  The SC68comline_read() function display prompt and read command line.
 *  The previous command line is freed by SC68comline_free().
 *  Optionnaly the function can use readline library. In that case it handles
 *  an history and any readline basic functionnalities..
 *
 * @param  prompt   Displayed prompt if not 0.
 * @param  comline  Command line struct filled by the function.
 *
 * @return number of arguments in command line.
 * @retval -1  error
 * @retval 0   no arguments (empty commad line)
 *
 * @warning Be sure to clear the comlime struct before the firs call to
 *          prevents SC68comline_free() to try to free invalid buffer.
 */
int SC68comline_read(const char * prompt, debug68_comline_t * comline);

/** Free command line.
 *
 *  @param  comline  Free internal buffer and clean structure.
 *
 */
void SC68comline_free(debug68_comline_t * comline);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _SC68DEBUG_COMLINE_H_ */
