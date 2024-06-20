/**
 * @ingroup   debug68_devel
 * @file      SC68debug_io.c
 * @author    Ben(jamin) Gerard <ben@sashipa.com>
 * @date      1999/08/14
 * @brief     debug68 fake IO emulation
 * @version   $Id: SC68debug_io.c,v 2.2 2003/09/23 17:36:39 benjihan Exp $
 */

/*
 *                    sc68 - debug68 fake IO emulation
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

#include "debug68/SC68debug_io.h"

static u8 vposr;
int paula_debug_access = 0;

/* Macro for word and long access based on byte access. */
#define READ_WORD(NAME) \
static u32 NAME##_readW(u32 addr, u32 cycle) \
{ return (NAME##_readB(addr,cycle)<<8) | NAME##_readB(addr+1,cycle); }

#define READ_LONG(NAME) \
static u32 NAME##_readL(u32 addr, u32 cycle) \
{ return (NAME##_readW(addr,cycle)<<16) | NAME##_readW(addr+2,cycle+4); }

#define WRITE_WORD(NAME) \
static void NAME##_writeW(u32 addr, u32 v, u32 cycle) \
{ NAME##_writeB(addr,v>>8,cycle); NAME##_writeB(addr+1,v,cycle); }

#define WRITE_LONG(NAME) \
static void NAME##_writeL(u32 addr, u32 v, u32 cycle) \
{ NAME##_writeW(addr,v>>16,cycle); NAME##_writeW(addr+2,v,cycle+4); }


/* Common to all chips */

static int68_t *any_int(cycle68_t cycle)
{
  cycle = cycle;
  return 0L;
}

static cycle68_t any_nextint(cycle68_t cycle)
{
  cycle = cycle;
  return IO68_NO_INT;
}

static void any_subcycle(cycle68_t sub)
{
  sub = sub;
}

/* ----------------------------------------------------------------------
   Paula Part
   ---------------------------------------------------------------------- */

static u32 paula_readB(u32 addr, u32 cycle)
{
  addr=addr; cycle=cycle;
  addr &= 0xFF;
  paula_debug_access |= 1;
  /* Big Cheat : Increment vertical raster pos at each read */
  if (addr==6) {
    return vposr++;
  }
  return 0;
}

READ_WORD(paula)
READ_LONG(paula)

static void paula_writeB(u32 addr, u32 v, u32 cycle)
{
  paula_debug_access |= 2;
  addr=addr; v=v; cycle=cycle;
}
WRITE_WORD(paula)
WRITE_LONG(paula)

static int paula_reset(void)
{
  vposr = 0;
  paula_debug_access = 0;
  return 0;
}


io68_t paula_io =
{
  NULL,
  "AMIGA Paula",
  0xFFDFF000, 0xFFDFF0DF,
  {paula_readB,  paula_readW,  paula_readL},
  {paula_writeB, paula_writeW, paula_writeL},
  any_int, any_nextint,
  any_subcycle,
  paula_reset,
  0,0
};

/* ----------------------------------------------------------------------
   YM-2149 Part
   ---------------------------------------------------------------------- */

static int ym_cur_reg;
static u8 ym_debug_reg[256];
int ym_debug_access = 0;


static u32 ym_readB(u32 addr, u32 cycle)
{
  addr=addr; cycle=cycle;
  addr &= 0xFF;
  ym_debug_access |= 1;
  if (addr&3) return 0;
  return ym_debug_reg[ym_cur_reg&255];
}

READ_WORD(ym)
READ_LONG(ym)

static void ym_writeB(u32 addr, u32 v, u32 cycle)
{
  cycle=cycle;
  addr &= 2;
  /* control register */
  if(!addr) ym_cur_reg = v&255;
  /* Data register [ control ] */
  else ym_debug_reg[ym_cur_reg&255] = v;
  ym_debug_access |= 2;
}
WRITE_WORD(ym)
WRITE_LONG(ym)

static int ym_reset(void)
{
  int i;
  for (i=0; i<sizeof(ym_debug_reg); ++i) {
    ym_debug_reg[i] = 0;
  }
  ym_cur_reg = 0;
  ym_debug_access = 0;
  return 0;
}


io68_t ym_io =
{
  NULL,
  "YM-2149",
  0xFFFF8800, 0xFF88FF,
  {ym_readB,  ym_readW,  ym_readL},
  {ym_writeB, ym_writeW, ym_writeL},
  any_int, any_nextint,
  any_subcycle,
  ym_reset,
  0,0
};

/* ----------------------------------------------------------------------
   Microwire and LMC Part
   ---------------------------------------------------------------------- */

int mw_debug_access = 0;

static u32 mw_readB(u32 addr, cycle68_t cycle)
{
  cycle=cycle;
  addr=addr;
  mw_debug_access |= 1;
  return 0;
}

READ_WORD(mw)
READ_LONG(mw)

static void mw_writeB(u32 addr, u32 v, cycle68_t cycle)
{
  addr=addr;
  v=v;
  cycle=cycle;
  mw_debug_access |= 2;
}

WRITE_WORD(mw)
WRITE_LONG(mw)

static int mw_reset(void)
{
  mw_debug_access = 0;
  return 0;
}

io68_t mw_io = {
  0,
  "STE-MicroWire",
  0xFFFF8900, 0xFFFF8925,
  {mw_readB,  mw_readW,  mw_readL},
  {mw_writeB, mw_writeW, mw_writeL},
  any_int, any_nextint,
  any_subcycle,
  mw_reset,
  0,0
};
