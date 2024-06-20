/**
 * @ingroup   debug68_devel
 * @file      SC68debug_breakpoint.c
 * @author    Ben(jamin) Gerard <ben@sashipa.com>
 * @date      1999/07/14
 * @brief     debug68 breakpoints
 * @version   $Id: SC68debug_breakpoint.c,v 2.0 2003/08/21 04:58:35 benjihan Exp $
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


#include <stdio.h>
#include "debug68/SC68debug_breakpoint.h"

#define SC68MAXBREAKP 8
#define ALL_BP ((unsigned)~0)

typedef struct {
  u32 addr;			/* Breakpoint address (~0 is free) */
  int count;			/*  0 : free,
				   * >0 : # pass before activation
				   * <0:  Deactivated
				 */
  int reset;			/* count value after a break */
} breakpoint_t;

static breakpoint_t bp[SC68MAXBREAKP];


/***********************
 *Single breakpoint op *
 **********************/
static void set_bp(breakpoint_t * bp, u32 addr, int count, int reset)
{
  bp->addr = count == 0 ? 0 : addr;
  bp->count = count;
  bp->reset = count == 0 ? 0 : reset;
}

static void kill_bp(breakpoint_t * bp)
{
  set_bp(bp, 0, 0, 0);
}

static void display_bp(int i, breakpoint_t * bp)
{
  if (!bp->count) {
    printf("B%02d Free\n", i);
    return;
  }
  printf("B%02d $%08X ", i, bp->addr);
  if (bp->count > 0)
    printf("[%02d]", bp->count);
  else
    printf("{%02d}", -bp->count);
  if (bp->reset) {
    printf(" => ");
    if (bp->reset > 0)
      printf("[%02d]", bp->reset);
    else
      printf("{%02d}", -bp->reset);
  }
  printf("\n");
}

/* addr = Breakpoint to find
 * addr = ~0 find a free place
 */
static int find_bp(u32 addr)
{
  int i;
  for (i = 0; i < SC68MAXBREAKP; i++)
    if (bp[i].count) {
      if (bp[i].addr == addr)
	return i;
    } else if (addr == ALL_BP)
      return i;
  return -1;
}

/* Kill and reset breakpoint :
 * addr : Address of breakpoint to kill, ~0 for all
 * return breakpoint internal number or -1
 */
int SC68debugger_breakp_kill(u32 addr)
{
  int i;
  /* Kill ALL */
  if (addr == ALL_BP) {
    for (i = 0; i < SC68MAXBREAKP; i++)
      kill_bp(bp + i);
    return -1;
  }
  /* Search & Kill */
  if (i = find_bp(addr), i < 0)
    return -1;
  kill_bp(bp + i);
  return i;
}

/* Display breakpoint info
 * addr : Address of breakpoint to display, ~0 for all
 * return internal number or # setted breakpoints
 */
int SC68debugger_breakp_display(u32 addr)
{
  int i;
  /* Display ALL */
  if (addr == ALL_BP) {
    int nb = 0;
    for (i = 0; i < SC68MAXBREAKP; i++)
      if (bp[i].count) {
	nb++;
	display_bp(i, bp + i);
      }
    return nb;
  }
  /* Search & Display */
  if (i = find_bp(addr), i < 0)
    return -1;
  display_bp(i, bp + i);
  return i;
}

/* Set breakpoint :
 * addr  : Address of breakpoint to set
 * count : # pass before break (see breakpoint_t)
 * reset : count reset value after break
 * return breakpoint internal number or -1
 */
int SC68debugger_breakp_set(u32 addr, int count, int reset)
{
  int i;
  /* Kill ALL */
  if (addr == ALL_BP)
    return -1;

  /* Search & Kill */
  if (i = find_bp(addr), i < 0)
    if (i = find_bp(ALL_BP), i < 0)
      return -1;

  set_bp(bp + i, addr, count, reset);
  return i;
}

/* General break test :
 * call with addr=PC at each instruction emulation
 * return -1 or internal breakpoint number that break
 */
int SC68debugger_breakp_test(u32 addr)
{
  int i;
  for (i = 0; i < SC68MAXBREAKP; i++)
    if (bp[i].count > 0 && addr == bp[i].addr && !--bp[i].count) {
      bp[i].count = bp[i].reset;
      return i;
    }
  return -1;
}
