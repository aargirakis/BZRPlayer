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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "file68/error68.h"
#include "file68/string68.h"
#include "file68/file68.h"
#include "file68/istream68_file.h"
#include "file68/alloc68.h"
#include "debug68/SC68debug_debugcom.h"
#include "debug68/SC68debug_breakpoint.h"
#include "debug68/SC68debug_eval.h"

#include "emu68/emu68.h"
#include "emu68/mem68.h"
#include "desa68/desa68.h"

/* Entering status backup */
static reg68_t savereg68;
static u8 *savemem68=NULL;

/* Command line backup */
#define DEBUGGER_MAX_ARG 32
static int argc = 0;
static char *argv[DEBUGGER_MAX_ARG];
static int last_com;

/* disassembler */
static DESA68parm_t desaparm;
static int default_ndesa=10;
static char desastr[256];

#define DEFAULT_MAX_EXEC 5000000


/* disassambler run and format output */
static void line()
{
  printf("------ ----  ------------\n");
}

static int mydesa(DESA68parm_t *d, u8 *chk, u32 curpc, char *tail)
{
  u32 pc = d->pc & d->memmsk;
  desa68(d);
  printf("$%06X [%c%c%c%c]%c%-30s%s",
	 pc,
	 chk[pc]&READ_68     ? 'R' : ' ',
	 chk[pc]&WRITTEN_68  ? 'W' : ' ',
	 chk[pc]&EXECUTED_68 ? 'X' : ' ',
	 chk[pc]&BREAKED_68  ? 'B' : ' ',
	 pc==curpc           ? '>' : ' ',
	 d->str,
	 tail==NULL ? "\n" : tail
	 );
  return 0;
}

static char * mydesa_str(DESA68parm_t *d, u8 *chk, u32 curpc, char *tail)
{
  static char tmp[128];
  u32 pc = d->pc & d->memmsk;

  desa68(d);
  sprintf(tmp,"$%06X [%c%c%c%c]%c%-30s%s",
	  pc,
	  chk[pc]&READ_68     ? 'R' : ' ',
	  chk[pc]&WRITTEN_68  ? 'W' : ' ',
	  chk[pc]&EXECUTED_68 ? 'X' : ' ',
	  chk[pc]&BREAKED_68  ? 'B' : ' ',
	  pc==curpc           ? '>' : ' ',
	  d->str,
	  tail==NULL ? "\n" : tail
	  );
  return tmp;
}


static unsigned last_desa = (unsigned)~0;

static int desa(u32 pc, int ndesa)
{
  int i;
  desaparm.pc     = (pc==(unsigned)~0)? reg68.pc : pc;
  desaparm.mem    = reg68.mem;
  desaparm.memmsk = reg68.memmsk;
  desaparm.str    = desastr;
  desaparm.flags  = 0;
  if(!ndesa) ndesa = default_ndesa;
  for(i=0; i<ndesa; i++)
    {
      last_desa = desaparm.pc;
      mydesa(&desaparm,reg68.chk,reg68.pc,NULL);
    }
  return 0;
}

static int desa1(u32 pc)
{
  desaparm.pc     = (pc==(unsigned)~0)? reg68.pc : pc;
  desaparm.mem    = reg68.mem;
  desaparm.memmsk = reg68.memmsk;
  desaparm.str    = desastr;
  desaparm.flags  = 0;
  mydesa(&desaparm,reg68.chk,reg68.pc,"!");
  return 0;
}

static char * desa1_str(u32 pc)
{
  desaparm.pc     = (pc==(unsigned)~0)? reg68.pc : pc;
  desaparm.mem    = reg68.mem;
  desaparm.memmsk = reg68.memmsk;
  desaparm.str    = desastr;
  desaparm.flags  = 0;
  return mydesa_str(&desaparm,reg68.chk,reg68.pc,"!");
}


static u32 next_inst(u32 pc)
{
  u32 spc = desaparm.pc, ni;
  desaparm.pc     = (pc==(unsigned)~0) ? reg68.pc : pc;;
  desaparm.mem    = savereg68.mem;
  desaparm.memmsk = savereg68.memmsk;
  desaparm.str    = desastr;
  desa68(&desaparm);
  ni = desaparm.pc;
  desaparm.pc = spc;
  return ni;
}

static int trace(u32 addr, int display, int do_bp)
{
  int bp,i;
  reg68_t oldreg68;

  if(addr!=(unsigned)~0)
    reg68.pc = addr;

  last_desa = reg68.pc&reg68.memmsk;

  /* Evaluate breakpoint */
  if(do_bp && (bp=SC68debugger_breakp_test(reg68.pc), bp>=0))
    return bp;

  oldreg68 = reg68;
  EMU68_step();
  if(display)
    for(i=0; i<16; i++)
      if(reg68.d[i]!=oldreg68.d[i])
	printf("%c%d : %08X => %08X\n",i<8 ? 'D' : 'A', i&7, oldreg68.d[i], reg68.d[i]);

  last_desa = reg68.pc&reg68.memmsk;
  return -1;

}

void SC68debugger_help(void)
{
  printf(
	 "SC68 debug commands:\n"
	 "Syntax :\n"
	 " [0123456789]         : unsigned decimal (UDEC)\n"
	 " $UDEC|[ABCDEFabcdef] : unsigned hexadecimal (UHEX)\n"
	 " UDEC|UHEX            : unsigned inetger (UINT)\n"
	 " [-]UINT              : signed integer (INT)\n\n"
	 "t [@]          : Trace instruction\n"
	 "s [@]          : Skip instruction\n"
	 "m [@]          : Memory dump\n"
	 "D#|A#|PC|SR=#  : register value change\n"
	 "reg            : show registers\n"
	 "e expression   : Evaluate expression\n"
	 "b [@] [#] [r#] : Set Breakpoint\n"
	 "d [@] [lines]  : disassemble N instruction\n"
	 "m @  # bytes]  : Memory dump\n"
	 "r @ # inst     : run at @ # inst OR until rts\n"
	 "u @ # inst     : run at current pc until pc=@ OR # inst executed\n"
	 "sbin fname @ # bytes : Save memory into file\n"
	 );
}


/* Display debugger shell prompt as diskname track disasm >
 */
char * SC68debugger_prompt(char *diskname, int track_num)
{
  return desa1_str(reg68.pc);
}

static int outofbound(u32 addr)
{
  return addr > (u32)savereg68.memmsk;
}

/* Clean/Set bits in chk mem
 * use with anded=0 and ored=0 to clear
 */
static void and_or_chk(u32 start, u32 end, u8 anded, u8 ored)
{
  u8 *chk = savereg68.chk;
  if(chk==NULL || outofbound(start) || outofbound(end) || start>end)
    return;
  for(;start<=end;start++)
    chk[start] = (chk[start]&anded)|ored;
  return;
}

/********************************
 * command line arguments things *
 ********************************/
static void free_arg(void)
{
  int i;
  for(i=0; i<argc; i++)
    {
      SC68free(argv[i]);
      argv[i] = NULL;
    }
  argc = 0;
}

static int copy_arg(int na, char **a)
{
  int i;
  free_arg();
  if(na>DEBUGGER_MAX_ARG) na=DEBUGGER_MAX_ARG;
  for(i=0; i<na; i++)
    {
      int len;
      if(a[i]==NULL) continue;
      len = strlen(a[i]);
      if(argv[i]=(char *)SC68alloc(len+1), argv[i]==NULL)
	{
	  argc=i;
	  return SC68error_add("Failed copying command line args");
	}
      strcpy(argv[i],a[i]);
    }
  argc = na;
  return 0;
}

/********************************
 * 68K environment backup things *
 ********************************/
static void free_restore(void)
{
  SC68free(savemem68);
  savemem68=NULL;
  memset(&savereg68,0,sizeof(savereg68));
}

/*  Restore entering status
 */
static int restore_entering_status( void )
{
  memcpy(savereg68.mem, savemem68,                 savereg68.memsz);
  memcpy(savereg68.chk, savemem68+savereg68.memsz, savereg68.memsz);
  reg68 = savereg68;
  return 0;
}

/* Debugger mode clean enter
 */
int SC68debugger_entering(void)
{
  /* Safer !! */
  free_restore();
  free_arg();

  /* Save 68K registers and info */
  savereg68 = reg68;

  /* No previous command */
  last_com  =
    last_desa = (unsigned)~0;

  if(savereg68.mem!=NULL && savereg68.chk!=NULL)
    {
      /* Alloc a block for both mem and chk ... */
      if(savemem68 = (u8 *)SC68alloc(savereg68.memsz * 2), savemem68==NULL)
	return -1;
      memcpy(savemem68,                 savereg68.mem, savereg68.memsz);
      memcpy(savemem68+savereg68.memsz, savereg68.chk, savereg68.memsz);
    }

  restore_entering_status();
  return 0;
}

/*  Debugger mode clean exit
 */
static int SC68debugger_exit(int return_code)
{
  free_restore();
  free_arg();
  return return_code;
}

static void show_reg(void)
{
  int i;
  int ccr = GET_CCR(reg68.sr);
  for(i=0; i<8; i++)
    printf("  D%d=%08X  A%d=%08X\n",i,reg68.d[i],i,reg68.a[i]);
  printf("  PC=%08X USP=%08X\n"
         "  SR=%04X %c%c%c%c%c\n",
         reg68.pc,reg68.usp,
         reg68.sr&0xFFFF,
         (ccr&SR_X) ? 'X' : '-',
         (ccr&SR_N) ? 'N' : '-',
         (ccr&SR_Z) ? 'Z' : '-',
         (ccr&SR_V) ? 'V' : '-',
         (ccr&SR_C) ? 'C' : '-'
         );
}

static int *is_reg_change(char *s);

/* Run as new debugger command, if no args are given,
 * previous command is used.
 */
int SC68debugger_newcom(int na, char **a)
{
  int err;
  if(na>0 && a!=NULL)
    {
      if(err=copy_arg(na,a), err<0)
	SC68debugger_exit(err);
    }
  else if(argc<1)
    return 0;
  /*  else
      last_desa = (unsigned)~0;*/

  /* e(X)it*/
  if(!SC68strcmp(argv[0],"x") || !SC68strcmp(argv[0],"exit"))
    {
      free_restore();
      return SC68debugger_exit(1);
    }

  /* (H)elp */
  else if(!SC68strcmp(argv[0],"?")    ||
          !SC68strcmp(argv[0],"help") ||
          !SC68strcmp(argv[0],"h"))
    {
      SC68debugger_help();
      return 0;
    }

  /* (T)race [address] */
  else if(!SC68strcmp(argv[0],"t"))
    {
      u32 addr = (unsigned)~0;
      int bp;
      char *error;

      if(na>1)
	{
	  addr = SC68debugger_eval(argv[1],&error);
	  if(error!=NULL)
	    {
	      printf( "Can't trace at this address : syntax error\n");
	      return 0;
	    }
	}
      bp = trace(addr,1,0);
      if(bp>=0)
	{
	  printf("Reach: ");
	  SC68debugger_breakp_display(addr);
	}
    }
  /* show (REG)isters */
  else if(!SC68strcmp(argv[0],"reg"))
    {
      show_reg();
    }
  /* set breakpoint ( b [address] [count] [reset] ) */
  else if(!SC68strcmp(argv[0],"b"))
    {
      u32 addr = (unsigned)-1;
      int count=1, reset=-1, n;
      char *err=NULL;

      if(na>1)
	{
	  addr = SC68debugger_eval(argv[1], &err);
	  if(err!=NULL)
	    {
	      printf("Invalid breakpoint address : Syntax error\n");
	      return 0;
	    }
	}
      if(na>2)
	{
	  count = SC68debugger_eval(argv[2], &err);
	  if(err!=NULL)
	    {
	      printf("Invalid breakpoint count : Syntax error\n");
	      return 0;
	    }
	}
      if(na>3)
	{
	  reset = SC68debugger_eval(argv[3], &err);
	  if(err!=NULL)
	    {
	      printf("Invalid breakpoint count-reset : Syntax error\n");
	      return 0;
	    }
	}
      if(reset==-1) reset=count;

      /* count==0 : KILL */
      if(!count)
	{
	  n = SC68debugger_breakp_kill(addr);
	  if(addr==(unsigned)~0)
	    printf("All breakpoint exterminated\n");
	  else
	    {
	      if(n>=0)
		printf("B%02d killed\n",n);
	      else
		printf("Breakpoint not found\n");
	    }
	}

      /* SET or DISPLAY */
      else
	{
	  /* addr==-1 : DISPLAY */
	  if(addr==(unsigned)~0)
	    {
	      n=SC68debugger_breakp_display(addr);
	      if(n<=0) printf("No breakpoint\n");
	    }
	  /* SET */
	  else
	    {
	      n=SC68debugger_breakp_set(addr,count,reset);
	      if(n>=0)
		{
		  printf("New breakpoint : ");
		  SC68debugger_breakp_display(addr);
		}
	      else
		printf("Can't set this breakpoint !!! sorry !!!");
	    }
	}
      return 0;
    }

  /* (E)val expression */
  else if(!SC68strcmp(argv[0],"e"))
    {
      char *error_loc;
      int v;
      if(na>1)
	{
	  v=SC68debugger_eval(argv[1], &error_loc);
	  if(error_loc!=NULL)
	    printf("syntax error at %s\n",error_loc);
	  else
	    printf("=%d ($%X)\n",v,v);
	}
    }

  /* (D)isassemble [@{=PC}] [lines{=default_desa}] */
  else if(!SC68strcmp(argv[0],"d"))
    {
      u32 addr = last_desa/*(unsigned)~0*/;
      int ndesa = 0;
      char *err;
      if(na>1)
	{
	  addr = SC68debugger_eval(argv[1], &err);
	  if(err!=NULL)
	    {
	      printf("Invalid disassemble address : Syntax error\n");
	      return 0;
	    }
	}
      if(na>2)
	{
	  ndesa = SC68debugger_eval(argv[2], &err);
	  if(err!=NULL)
	    ndesa = 0;
	}
      line();
      desa(addr, ndesa);
      line();
    }

  /* (M)emory dump [@] [# bytes] */
  else if(!SC68strcmp(argv[0],"m"))
    {
      char *err=NULL;
      u32  addr;
      int  n=16,i;

      if(na<2)
	{
	  printf("use : m @  [# bytes]\n");
	  return 0;
	}

      if(na>1)
	addr = SC68debugger_eval(argv[1],&err);
      if(na>2 && err==NULL)
	n = SC68debugger_eval(argv[2],&err);

      if(n<=0) n = 1;

      if(err!=NULL)
	{
	  printf("Invalid memory address syntax");
	  return 0;
	}

      line();
      for(i=0; i<n ; i+=16 )
	{
	  int j,l;
	  printf("%06X ",(addr+i)&0xFFFFFF);
	  l = n-i;
	  if(l>16) l=16;
	  for(j=0; j<l; j++)
	    printf("%02X",EMU68_peek(addr+i+j));
	  for(  ; j<16; j++) printf("  ");
	  printf("|");

	  for(j=0; j<l; j++)
	    {
	      u8 c;
	      c = EMU68_peek(addr+i+j);
	      printf("%c", isgraph(c) ? c : '.');
	    }
	  for(  ; j<16; j++) printf(" ");
	  printf("\n");
	}
      line();

    }

  /* (R)un [pc] [max-instruction] */
  else if(!SC68strcmp(argv[0],"r"))
    {
      u32 addr = (unsigned)~0;
      s32 savesp;
      unsigned max = DEFAULT_MAX_EXEC, ct;
      char *err;

      if(na>1)
	{
	  addr = SC68debugger_eval(argv[1],&err);
	  if(err!=NULL)
	    {
	      printf("Invalid run address");
	      return 0;
	    }
	  reg68.pc = addr;
	}

      if(na>2)
	{
	  max = SC68debugger_eval(argv[2],&err);
	  if(err!=NULL)
	    {
	      printf("Invalid instruction limit");
	      return 0;
	    }
	}

      for(savesp=reg68.a[7], ct=0; (max==0 || ct!=max) && reg68.a[7]<=savesp; ct++)
	{
	  int bp;
	  bp = trace((unsigned)~0,0,1);
	  if(bp>=0)
	    {
	      printf("Reach: ");
	      SC68debugger_breakp_display(reg68.pc);
	      return 0;
	    }
	}
      /*if(reg68.a[7]>savesp)
	reg68.a[7]=savesp;*/
      if(ct==max)
	{
	  printf("%u instrction%s executed\n",max,max>1 ? "s" : "");
	  return 0;
	}
    }

  /* Run until pc=value <value> [max-instruction]*/
  else if(!SC68strcmp(argv[0],"u"))
    {
      u32 addr = (unsigned)~0;
      unsigned max = DEFAULT_MAX_EXEC, ct, breakat;
      char *err;

      if(na<2)
	{
	  printf("Missing break point address\n");
	  return 0;
	}

      addr = SC68debugger_eval(argv[1],&err);
      if(err!=NULL)
	{
	  printf("Invalid break point address");
	  return 0;
	}
      breakat = addr&reg68.memmsk;

      if(na>2)
	{
	  max = SC68debugger_eval(argv[2],&err);
	  if(err!=NULL)
	    {
	      printf("Invalid instructions limit");
	      return 0;
	    }
	}

      for(ct=0; (max==0 || ct!=max) && (reg68.pc&reg68.memmsk)!=(reg68.memmsk&breakat); ct++)
	{
	  int bp;
	  bp = trace((unsigned)~0,0,1);
	  if(bp>=0)
	    {
	      printf("Reach: ");
	      SC68debugger_breakp_display(reg68.pc);
	      return 0;
	    }
	}
      if((reg68.pc&reg68.memmsk)!=(reg68.memmsk&breakat))
	{
	  printf("%u instrction%s executed\n",max,max>1 ? "s" : "");
	  return 0;
	}
    }

  /* Save Bin */
  else if(!SC68strcmp(argv[0],"sbin"))
    {
      if(argc<4)
	{
	  printf("sbin filename start-@ size\n");
	  return 0;
	}
      else
	{
	  istream_t *os=0;
	  unsigned int start,size;
	  int err=0;
	  char *errstr=NULL;
	  if(start=SC68debugger_eval(argv[2],&errstr),errstr!=NULL)
	    return SC68error_add("Invalid start address");
	  if(size=SC68debugger_eval(argv[3],&errstr),errstr!=NULL)
	    return SC68error_add("Invalid size");
	  if(EMU68_memvalid(start,size)<0)
	    SC68error_add(EMU68error_get());
	  if(os=istream_file_create(argv[1],2), !os)
	    return -1;
	  if (istream_open(os)) {
	    istream_destroy(os);
	    return -1;
	  }
	  err =
	    istream_write(os, reg68.mem+start,size)
	    != size;
	  istream_destroy(os);
	  if(!err)
	    printf("[%x-%x] (%d bytes) => <%s>\n",
		   start,start+size-1,size,argv[1]);
	  return -!!err;
	}
    }

  else
    {
      int *p;
      if(p=is_reg_change(a[0]), p!=NULL)
	{
	  char *n=NULL,*err=NULL;
	  int v=0;
	  for(n=a[0]+3; *n && isspace(*n); n++);
	  if(*n==0 && argc>1)
	    for(n=a[1]; *n && isspace(*n); n++);
	  if(*n==0 || (v=SC68debugger_eval(n,&err),err!=NULL))
	    printf("Bad value for register\n");
	  else *p = v;
	}
      else
	printf("... unknown command <%s> (? for help)\n",argv[0]);
    }

  return 0;
}

static int *is_reg_change(char *s)
{
  if(s==NULL || strlen(s)<3) return NULL;
  if(s[2]!='=') return NULL;
  switch(toupper(*s))
    {
    case 'D':
      if(s[1]<'0' || s[1]>'7') return NULL;
      return  reg68.d+(s[1]-'0');
    case 'A':
      if(s[1]<'0' || s[1]>'7') return NULL;
      return  reg68.a+(s[1]-'0');
    case 'S':
      switch(toupper(s[1]))
	{
	case 'R': return (int*)&reg68.sr;
	case 'P': return &reg68.usp;
	}
      break;
    }
  return NULL;
}
