/*
 *                     sourcer68 - 68000 sourcer
 *                Copyright (C) 2001-2003 Benjamin Gerard
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

/* $Id: sourcer68.c,v 2.2 2003/09/24 19:31:08 benjihan Exp $ */

#include <config68.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "desa68/desa68.h"
#include "emu68/emu68.h"

#include "file68/error68.h"
#include "file68/alloc68.h"
#include "file68/file68.h"
#include "file68/istream68_file.h"

#define TOS_RELOC_NEVER   0
#define TOS_RELOC_AUTO    1
#define TOS_RELOC_ALWAYS  2

#define SRC68_COLOR (1<<7) /**< Color processed memory cels */
#define SRC68_INST  (1<<6) /**< Color valid instruction cels */
#define SRC68_ENTRY (1<<5) /**< Color valid instruction cels */
#define SRC68_RTS   (1<<4) /**< Color rts instructions */

static char * user_replay= 0;    /**< sc68 replay directory */
static char * shared_replay= 0;  /**< sc68 replay directory */

static DESA68parm_t desa;
static char desastr[512];

static char * tabs = "\t";

static int SpoolError(int code)
{
  const char * s = SC68error_get();
  if (!s) {
    return 0;
  } else {
    do {
      fprintf(stderr, "%s\n",s);
    } while (s = SC68error_get(), s);
    return code;
  }
}

static void CleanErrorStack(void)
{
  while(SC68error_get());
}

static unsigned int FindNextColored(unsigned int from, unsigned to)
{
  for (; from<to; from += 2) {
    if ( (reg68.chk[from] | reg68.chk[from+1]) & SRC68_COLOR) {
      return from;
    }
  }
  return 0x01000000;
}


static void PrintLabel(unsigned int addr)
{
  printf("L%06X: \n", addr);
}

static void PrintRelLabel(unsigned addr, unsigned int base)
{
  printf("L%06X: = *+%d \n", addr, addr-base);
}

static void PrintDCW(unsigned int from, unsigned int to)
{
  const int collect = 8;
  int n = to - from;
  int i = 0;

  if (n<0 || n&1) {
    BREAKPOINT68;
  }
  n >>= 1;

  do {
    unsigned int v = (reg68.mem[from]<<8) | (reg68.mem[from+1]);

    if ((reg68.chk[from]|reg68.chk[from+1]) & SRC68_ENTRY) {
      if (i) {
        printf("\n");
      }
      i = 0;
      if (reg68.chk[from] & SRC68_ENTRY) {
        PrintLabel(from);
      }
      if (reg68.chk[from+1] & SRC68_ENTRY) {
        PrintRelLabel(from+1, from);
      }
    }

    if (i==0) {
      printf("%s",tabs);
    }
    printf((i==0)?"DC.W $%04x":",$%04x",v);
    if (++i == collect) {
      i = 0;
      printf("\n");
    }
    from += 2;
  } while(--n);
  if (i) {
    printf("\n");
  }
}

static int FinalDesa(char *fname,
		     unsigned int from, unsigned int to,
		     int *entries)
{
  unsigned int pc, opc;
/*   unsigned int end = reg68.memmsk; */
  int *e, pipo[2];

  e = entries;
  if (!e || *e==-1) {
    e = pipo;
    pipo[0] = from;
    pipo[1] = -1;
  }

  printf(
    "; file: \"%s\"\n"
    "; from:",
    fname);
  for ( ;*e!=-1; e++) {
    printf(" $%X",*e);
  }
  printf(
    "\n"
    "; to:   %x\n"
    ";\n"
    "; sourcer68 - Copyright (C) 2001-2003 Benjamin Gerard\n"
    ";\n",
    to);

  desa.flags = DESA68_SYMBOL_FLAG;
  desa.immsym_min = from ;
  desa.immsym_max = to;

  for (opc=pc=from; pc < to; ) {
    if (reg68.chk[pc] & SRC68_INST) {

      if (opc != pc) {
        reg68.chk[opc] |= SRC68_ENTRY;
        PrintDCW(opc,pc);
        printf("\n");
      }

      if (reg68.chk[pc] & SRC68_ENTRY) {
        PrintLabel(pc);
      }
      desa.pc = pc;
      desa68(&desa);

      /* Check ENTRY inside instruction */
      for (opc = pc+1; opc<desa.pc; opc++) {
        if (reg68.chk[opc] & SRC68_ENTRY) {
          PrintRelLabel(opc, pc);
        }
      }

      printf("%s%s\n", tabs, desa.str);
      if (reg68.chk[pc] & SRC68_RTS) {
        printf("\n");
      }

      pc = opc = desa.pc;

    } else {
      pc += 2;
    }
  }

  if (opc != pc) {
    reg68.chk[opc] |= SRC68_ENTRY;
    PrintDCW(opc,pc);
  }
  PrintLabel(pc);

  return 0;
}

static void Color(unsigned int from, unsigned int to, int color)
{
  for (; from<to; ++from) {
    reg68.chk[from] |= color;
  }
}

static int rPreSource(void)
{
  unsigned int pc;
  int test_jmp_table=0;
  int status;

  pc = desa.pc;
  if ( (pc & 1) || pc > desa.memmsk) {
    /*BREAKPOINT68*/
    fprintf(stderr,"; ERROR: address %06X\n", pc);
    return -1;
  }
  reg68.chk[pc] |= SRC68_ENTRY;

  do {
    pc = desa.pc;

/*    if (pc == 0x8248) {
      BREAKPOINT68;
    }*/

    /* Can't disassemble odd memory */
    if (pc & 1) {
      fprintf(stderr,"; ERROR: address %06X\n", pc);
      return -1;
    }
    /* pc outside memory */
    if (pc > desa.memmsk) {
      return -1;
    }

    if (reg68.chk[pc] & SRC68_COLOR) {
      /* already done */
      /*printf("; ABORT: %06X\n",pc);*/
      return 0;
    }

    /* Disassemble */
    desa68(&desa);
    status = desa.status;
    /*printf("; %06X (%-28s)%s%s:%s:%s:%s\n",
      pc,
      desa.str,
      test_jmp_table?" [jtbl] ":"",
      (desa.status & DESA68_INST)?"I":"",
      (desa.status & DESA68_BSR)?"BSR":"",
      (desa.status & DESA68_BRA)?"BRA":"",
      (desa.status & DESA68_RTS)?"RTS":"");*/

    /* If just testing jump table, exit if not BRA or RTS */
    if (test_jmp_table && (desa.status&(DESA68_BRA|DESA68_RTS|DESA68_NOP))==0) {
      /*printf("!!%06X End if jump table\n", pc);*/
      return 0;
    }

    /* Color this cels */
    Color(pc, desa.pc, SRC68_COLOR | ((status&DESA68_INST) ? SRC68_INST : 0));

    if (status & DESA68_INST) {

      /* Check memory access entry point */
      if (desa.ea_src <= desa.memmsk) {
        reg68.chk[desa.ea_src] |= SRC68_ENTRY;
      }
      if (desa.ea_dst <= desa.memmsk) {
        reg68.chk[desa.ea_dst] |= SRC68_ENTRY;
      }

      if (status & DESA68_BRA) {
        pc = desa.pc;
        desa.pc = desa.branch;
        /* Do not follow JMP (An) ... */
        if (desa.branch < desa.memmsk) {
          rPreSource();
        }
        /*printf(";<< %06X -> %06X\n",desa.pc, pc);*/
        desa.pc = pc;
        test_jmp_table = ! (status & DESA68_BSR); /* Set jump table if BRA, remove other */
      } else if (desa.status & DESA68_RTS) {
        if (!test_jmp_table) {
          reg68.chk[pc] |= SRC68_RTS;
          return 0;
        }
      }
    } else {
      return 0;
    }

  } while(1);

}


static int PreSource(unsigned int from, unsigned int to, int *entries)
{
  if (!entries || *entries==-1) {
    desa.pc = from;
  } else {
    desa.pc = *entries++;
  }

  do {
    desa.flags = DESA68_SYMBOL_FLAG;
    desa.immsym_min = from ;
    desa.immsym_max = to;
    rPreSource();
  } while (desa.pc = *entries++, desa.pc != -1);
  return 0;
}


static int Init68k(unsigned int size)
{
  void * data;
  unsigned int alloc_size;

  /* Calculate 68k memory buffer size. See EMU68_init() doc for more info. */
  alloc_size = size;

  if (EMU68_debugmode()) {
    alloc_size <<= 1;
  } else {
    alloc_size += 3;
  }

  data=malloc(alloc_size);
  if (!data) {
    SC68error_add("Init68k(%d) : %s", alloc_size, strerror(errno));
    return -1;
  }
    
  if (EMU68_init(data,size)) {
    free(data);
    SC68error_add(EMU68error_get());
    return -2;
  }

  desa.mem = reg68.mem;
  desa.memmsk = reg68.memmsk;
  desa.pc = reg68.pc;

  desa.str = desastr;
  desa.strmax = sizeof(desastr)-1;
  desa.str[0] = desa.str[sizeof(desastr)-1] = 0;

  desa.flags = 0;
  return 0;
}


static void WriteL(unsigned char *b, int v)
{
  b[0] = v>>24;
  b[1] = v>>16;
  b[2] = v>>8;
  b[3] = v;
}

static int ReadL(unsigned char *b)
{
  return
    (((int)*(signed char*)b)<<24) +
    (b[1]<<16) +
    (b[2]<<8) +
    b[3];
}

static int ReadW(unsigned char *b)
{
  return
    (((int)*(signed char*)b)<<8) +
    b[1];
}

/**
 * @param b         Base of 68000 memory
 * @param b_sz      Size of 68000 memory
 * @param addr      Offset to TOS program to reloc
 * @param n         Size of TOS program file.
 * @param clear_bss Clear BSS flag (0:not cleared)
 */

static int TOSreloc(unsigned char *b, int b_sz, int addr, int n, int clear_bss, int check_only)
{
  int const header_size = 0x1c;
  int text, text_sz;
  int data, data_sz;
  int symbol, symbol_sz;
  int bss, bss_sz;
  int reloc, reloc_sz;
  int end, size;

  int magic;

  if (n < header_size) {
    return SC68error_add("; Too few bytes (%d) to be a TOS executable.", n);
  }

  magic = ReadW(b+addr);
  if (magic != 0x601a) {
    return SC68error_add("Invalid TOS executable header. "
			 "Expected 0x601a, found %02X.", magic);
  }

  text_sz   = ReadL(b+addr+2);
  data_sz   = ReadL(b+addr+2+4);
  bss_sz    = ReadL(b+addr+2+8);
  symbol_sz = ReadL(b+addr+2+12);
  size      = text_sz + data_sz + symbol_sz;
  reloc_sz  = (n-header_size)-size;

  if (reloc_sz < 0) {
    return SC68error_add("Sections size greater than file size (%d>%d).",
			 size, n-header_size);
  }

  text    = addr+header_size; /* TEXT section begins just after the header. */
  data    = text+text_sz;     /* DATA section stand after TEXT section */
  bss     = data+data_sz;     /* BSS stands after DATA section */
  symbol  = bss;              /* SYMBOL stands at the BSS */
  reloc   = symbol+symbol_sz; /* RELOCATION table stands after SYMBOL section */

  end       = text+text_sz+data_sz+bss_sz;
  if (end > b_sz) {
    return SC68error_add("Not emougth space in 68K memory reloc this program. "
			 "Missing %d bytes.", end-b_sz);
  }

  if (reloc_sz) {
    int reloc_ptr, d5;
    if (reloc_sz < 4) {
      return SC68error_add("RELOCATION section size (%d) must > than 4. (%d).",
			   reloc_sz);
    }

    reloc_ptr = text;
    d5 = ReadL(b+reloc); reloc+=4;
    while (d5) {
      reloc_ptr += d5;
      if (d5 == 1) {
        reloc_ptr += 253;
      } else if (d5&1) {
        return SC68error_add("RELOCATION table corrupt: "
			     "expecting odd value (%d).", d5);
      } else {
        int v;
        if (reloc_ptr >= end) {
          return SC68error_add("RELOCATION goes outside 68000 memeory.");
        }
        v = ReadL(b+reloc_ptr);
        v += text;
        if (!check_only) {
          WriteL(b+reloc_ptr, v);
        }
        d5 = 0;
      }
      if (reloc_ptr >= addr+n) {
        return SC68error_add("RELOCATION table must be corrupt.");
      }
      d5 = b[reloc++];
    }
  }

  if (!check_only && clear_bss && bss_sz>0) {
    memset(b+bss,0,bss_sz);
  }

  return 0;
}

static char * LoadBinary(char * fname, int * fsize)
{
  int size;
  char *b = 0;

  istream_t *is = istream_file_create(fname,1);

  if (istream_open(is) == -1) {
    goto error;
  }
  size = istream_length(is);
  if (size < 0) {
    goto error;
  }

  b = SC68alloc(size);
  if (!b) {
    goto error;
  }

  if (istream_read(is, b,size) != size) {
    goto error;
  }
  if (fsize) {
    *fsize = size;
  }
  istream_destroy(is);
  return b;

error:
  istream_destroy(is);
  SC68free(b);
  return 0;
}


static char * Basename(char *name);
static int Usage (const char *name);
static char *IsOption(char *a,char *o);
static int * parse_entry(char *entries, unsigned char *mem, int from, int to);

int main(int na, char **a)
{
  int i;
  char *fname = 0, *s;
  disk68_t *d=0;
  char * buf = 0;
  int fsize=0;
  int size68=512<<10;
  int tosreloc = TOS_RELOC_AUTO;
  char *entry="+0";
  int *entries;

  SC68set_alloc(malloc);
  SC68set_free(free);

  for (i=1; i<na; ++i) {
    if (!strcmp(a[i],"--help")) {
      return Usage(Basename(a[0]));
    } else if (s = IsOption(a[i],"--replay="), s) {
      user_replay = s;
    } else if (s = IsOption(a[i],"--reloc="), s) {
      if (!strcmp(s, "no")) {
        tosreloc = TOS_RELOC_NEVER;
      } else if (!strcmp(s, "yes")) {
        tosreloc = TOS_RELOC_ALWAYS;
      } else if (!strcmp(s, "auto")) {
        tosreloc = TOS_RELOC_AUTO;
      } else {
        fprintf(stderr, "--reloc bad value. Try --help.\n");
        return 2;
      }
    } else if (s = IsOption(a[i],"--entry="), s) {
        entry = s;
    } else if (s = IsOption(a[i],"--tab="), s) {
        tabs = s;
    } else if (!fname) {
      fname = a[i];
    } else {
      fprintf(stderr, "Too many parameters. Try --help.\n");
      return 2;
    }
  }

  if (Init68k(size68)) {
    return SpoolError(5);
  }

  /* Check if input filename was given */
  if (!fname) {
    fprintf(stderr, "Missing input file. Try --help.\n");
    return 3;
  }

  /* Get user replay path, if not set. */
  if (!user_replay) {
    user_replay = getenv("HOME");
    if (user_replay) {
      char * r, * rpath = "/.sc68";
      r = malloc(strlen(user_replay) + strlen(rpath) + 1);
      if (r) {
	strcpy(r,user_replay);
	strcat(r,rpath);
      }
      user_replay = r;
    }
  }

  /* Default replay = PWD */
  if (!user_replay) {
    user_replay = "";
  }


  /* Get replay from environment */
  shared_replay = getenv("SC68_DATA");
  if (shared_replay) {
    char * r, * rpath = "/Replay";
    r = malloc(strlen(shared_replay) + strlen(rpath) + 1);
    if (r) {
      strcpy(r,shared_replay);
      strcat(r,rpath);
    }
    shared_replay = r;
  }

  if (!shared_replay) {
    shared_replay = SC68_SHARED_DATA_PATH;
  }

  /* Verify if input is an sc68 file */
  if (!SC68file_verify_file(fname)) {
    d = SC68file_load_file(fname);
    if (!d) {
      return SpoolError(4);
    }
    return -1;
  } else {
    CleanErrorStack();
    buf = LoadBinary(fname, &fsize);
    if (!buf) {
      return SpoolError(5);
    }

  }

  if (buf) {
    int addr = 0x8000;
    if (EMU68_memput(addr, buf, fsize)) {
      SC68error_add(EMU68error_get());
      return SpoolError(6);
    }

    if (tosreloc == TOS_RELOC_AUTO) {
      int err = TOSreloc(reg68.mem, reg68.memsz, addr, fsize, 0, 1);
      if (err < 0) {
        const char *s = SC68error_get();
        fprintf(stderr, "; Not a TOS file: %s\n", s ? s : "no error !");
        tosreloc = TOS_RELOC_NEVER;
      }
    }

    if (tosreloc != TOS_RELOC_NEVER) {
      if (TOSreloc(reg68.mem, reg68.memsz, addr, fsize, 0, 0) < 0) {
        return SpoolError(6);
      }
    }

//    BREAKPOINT68;

    entries = parse_entry(entry, reg68.mem, addr, addr+fsize);
    SpoolError(0);

    PreSource(addr, addr+fsize, entries);
    FinalDesa(Basename(fname), addr, addr+fsize, entries);
  }

  if (d) {
    SC68free(d);
  }

  if (buf) {
    SC68free(buf);
  }

  return 0;
}

static char * Basename(char *name)
{
  char *p1 = strrchr(name, '/');
  char *p2 = strrchr(name, '\\');
  if (p2 > p1) {
    p1 = p2;
  }
  return p1 ? p1+1 : name;
}

static int Usage (const char *name)
{
  printf(
    "%s - 68000 sourcer\n"
    "Copyright (C) 2001 Ben(jamin) Gerard\n"
    "\n"
    "Usage: %s [options] <file>\n"
    "\n"
    "Where <file> could be rather a sc68 or a binary file.\n"
    "\n"
    "Options:\n"
    "\n"
    "  --help                 This message.\n"
    "  --reloc=[yes|no|auto]  TOS relocation (default:auto).\n"
    "  --replay=PATH          Path to sc68 replay. \n"
    "                         Overide the SC68REPLAY environment variable.\n"
    "  --entry=[ENTRY-LIST]   Set dissassembly entry points (default:+0)\n"
    "  --tab=[STRING]         Set tabulation string.\n"
    "\n"
    "ENTRY-LIST: ENTRY[,ENTRY]*\n"
    "\n"
    "  The entry list is an ordered coma (',') separated list of entry point where\n"
    "  the disassembler starts a disassembly pass.\n"
    "\n"
    "ENTRY: [l][+]integer\n"
    "\n"
    "   l       : Get long at the effective address (indirection).\n"
    "             'l' options could by use more than one time.\n"
    "   +       : Effective address is file start + integer.\n"
    "   integer : Number (use 0x prefix for hexadecimal).\n"
    "\n"
    ,name,name);
  return 1;
}

static char *IsOption(char *a,char *o)
{
  return (strstr(a,o)==a) ? a + strlen(o) : 0;
}

/* Set a -1 terminated array of entry point from --entry= option string */
static int * parse_entry(char *entries, unsigned char *mem, int from, int to)
{
  static int e[256];
  const int max = (int)(sizeof(e)/sizeof(*e))-1;
  int i;
  char *s;

  e [0] = -1;

  if (!entries) {
    SC68error_add("No entry in entry-list. Try --help.");
    goto error;
  }

  for (i=0, s=entries; *s && i<max; ) {
    int ea;
    int v;
    char *err;
    int indirect;

    /* Get number of indiretion */
    for (indirect=0; *s == 'l'; ++indirect, ++s);

    /* Get offset indicator */
    ea = 0;
    if (*s == '+') {
      ea = from;
      ++s;
    }

    /* Read integer */
    err = 0;
    v = strtol(s, &err, 0);
    if (errno) {
      SC68error_add("Syntax error in entry list. Can't get number (%s)",
		    strerror(errno));
      goto error;
    }
    s = err;
    if (*s) {
      if (*s!=',') {
        SC68error_add("Syntax error (missing coma ',' separator.");
        goto error;
      }
      ++s;
    }

    do {
      int addr = ea+v;
      if (addr<from || addr>=to) {
        SC68error_add("Effective address ($%x) is outside file data.", addr);
        ea = -1;
        break;
      }
      if (indirect) {
        v = 0;
        addr = ReadL(mem+addr)&0xFFFFFF;
      }
      ea = addr;
      --indirect;
    } while (indirect >= 0);

    if (ea != -1) {
      e[i++] = ea;
    }
  }

  if (i==0) {
    SC68error_add("No valid entry point. Set to default.");
    goto error;
  }

  if (*s || i>max) {
    SC68error_add("Too many entry points. Maximum is set to %d.", max);
    e[max] = -1;
    return e;
  }

  e[i] = -1;
  return e;

error:
  /* Error : restore default */
  e[0] = from;
  e[1] = -1;
  return e;
}

