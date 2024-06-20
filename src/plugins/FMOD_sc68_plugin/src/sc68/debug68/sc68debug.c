/*
 *                        sc68 - debug68 main code
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

#ifndef EMU68DEBUG
# define EMU68DEBUG 1
#endif

#define SC68_68KMEMSIZE  512

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "file68/error68.h"
#include "file68/file68.h"
#include "file68/alloc68.h"
#include "file68/istream68_file.h"
#include "file68/string68.h"
#include "file68/rsc68.h"
#include "file68/debugmsg68.h"
#include "debug68/SC68debug_comline.h"
#include "debug68/SC68debug_debugcom.h"
#include "debug68/SC68debug_io.h"

#include "emu68/emu68.h"
#include "emu68/ioplug68.h"
#include "emu68/mem68.h"
#include "desa68/desa68.h"



typedef int (command_fct_t)(int,char**);

typedef struct
{
  char *com,  /* command name */
    *altcom,  /* alternate command name ( shortcut ) */
    *use,     /* usage strng */
    *help;    /* help command string */
  command_fct_t *fnct;
} command_t;


/* Any declarations  */
static int settrack(int newtrack);
static int setreplay(char *newname);

/* Command fonctions declarations  */
static command_fct_t loaddisk_com, mergedisk_com, savedisk_com,
  diskinfo_com, settrack_com, trackinfo_com, exit_com,
  help_com, settime_com, findtime_com, debug_com, capitalize_com,
  replay_com, author_com, composer_com, name_com, deltrack_com,
  frq_com, loaddata_com, savedata_com, dup_com, address_com;

static command_t commands[] = {
  {"exit","x",NULL,"Exit SC68 debugger",exit_com},
  {"help","?","[command]","Display command(s) help message",help_com},
  {"loaddisk","ld","disk-file-name","Load a new SC68 file as current disk",loaddisk_com},
  {"mergedisk","md","disk-file-name","Add a new SC68 file to current disk",mergedisk_com},
  {"savedisk","sd","[disk-file-name]","Save current disk with current track as default disk track, if no filename use last loaded disk",savedisk_com},
  {"diskinfo","di",NULL,"Display current disk info",diskinfo_com},
  {"settrack","t=","#","Set current track number",settrack_com},
  {"trackinfo","ti","[#] default:current-track","Display track info",trackinfo_com},
  {"settime","st","[ALL|* mm:sec] | [[mm:sec] [ [track mm:sec] ... ]]","Set track(s) time",settime_com},
  {"findtime","ft","[ALL|* [mm:sec]] | [[mm:sec] track] ...]","Search track(s) loop time within given duration",findtime_com},
  {"debug","db",NULL,"Enter debugger mode",debug_com},
  {"capitalize","cap","<ALL|*>","Capitalize Disk,Title,Author",capitalize_com},
  {"replay","r=","[ALL|Track-list]  <NONE|NAME>","Change external replay",replay_com},
  {"name","n=","[ALL|Track-list]  <NONE|NAME>","Change disk or track name",name_com},
  {"author","a=","[Track-list]  <NONE|NAME>","Change track author name",author_com},
  {"composer","c=","[Track-list]  <NONE|NAME>","Change track composer name",composer_com},
  {"del","-","track-nmber","Remove track from disk",deltrack_com},
  {"frq","f=","[track-list] replay-frq(hz)","Change replay rate",frq_com},
  {"loaddata","ldat","data-file-name -d0=# -a0=hex -f=# -[AaYySs]","Load a data file with track parms", loaddata_com},
  {"savedata","sdat","[track] data-file-name","Save track data into file", savedata_com},
  {"dup","+","track","Copy a track, set d0 to new track number", dup_com},
  {"address","a0=","[track] -|hexa-value","Set load address (- is default)", address_com},

  {NULL}
};

/* Global */
static int isinterractif=1;
static char *forced_fname=0,*lastload_fname=0;
static int forced_help=0;
static int quit, quit_code=0;

/* replay */
static u8 *replay=NULL;
static int replaysz;
static char *replayname=NULL, *forced_replay=NULL;

/* 68K */
static int memsize=SC68_68KMEMSIZE<<10;

/* Disk & Music */
static disk68_t *disk=NULL;
static music68_t *mus=NULL;
static int track=-1,forced_track=-1;
static u32 time_ms=(u32)-1,memlow=(u32)-1,memhi=(u32)-1,frames=(u32)-1;
static int forced_a0=0, a0=0, playaddr;
static unsigned int cycleperpass;

#define TRAP_14_ADDR 0x600
/* ST xbios function emulator.#
 *  Just 4 SUPEREXEC in LOGICAL & some other music files
 */
static u8 trap14[] = {
  0x0c,0x6f,0x00,0x26,0x00,0x06,0x67,0x00,0x00,0x22,0x0c,0x6f,0x00,0x1f,0x00,
  0x06,0x67,0x00,0x00,0x28,0x0c,0x6f,0x00,0x22,0x00,0x06,0x67,0x00,0x00,0x86,
  0x0c,0x6f,0x00,0x0e,0x00,0x06,0x67,0x00,0x00,0xec,0x4e,0x73,0x48,0xe7,0xff,
  0xfe,0x20,0x6f,0x00,0x44,0x4e,0x90,0x4c,0xdf,0x7f,0xff,0x4e,0x73,0x48,0xe7,
  0xff,0xfe,0x41,0xef,0x00,0x44,0x4c,0x98,0x00,0x07,0x02,0x40,0x00,0x03,0xd0,
  0x40,0x16,0x38,0xfa,0x17,0x02,0x43,0x00,0xf0,0xe5,0x4b,0x36,0x7b,0x00,0x42,
  0xd6,0xc3,0x76,0x00,0x45,0xf8,0xfa,0x1f,0xd4,0xc0,0x43,0xf8,0xfa,0x19,0xd2,
  0xc0,0xe2,0x48,0x04,0x00,0x00,0x02,0x6b,0x1a,0x57,0xc3,0x18,0x03,0x0a,0x03,
  0x00,0xf0,0x44,0x04,0x48,0x84,0xd8,0x44,0x43,0xf1,0x40,0xfe,0xd8,0x44,0x02,
  0x41,0x00,0x0f,0xe9,0x69,0xc7,0x11,0x14,0x82,0x26,0x90,0x83,0x11,0x4c,0xdf,
  0x7f,0xff,0x4e,0x73,0x00,0x34,0x00,0x20,0x00,0x14,0x00,0x10,0x48,0xe7,0x00,
  0xc0,0x43,0xfa,0x00,0x20,0x70,0x00,0x20,0x7b,0x00,0x3e,0x41,0xfb,0x80,0x5e,
  0x23,0x88,0x00,0x00,0x58,0x40,0xb0,0x7c,0x00,0x24,0x66,0xec,0x20,0x09,0x4c,
  0xdf,0x03,0x00,0x4e,0x73,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x4e,0x75,0xc1,0x88,0x41,0xfa,0x00,0x0c,0x21,0x48,0x00,0x04,0x58,
  0x48,0xc1,0x88,0x4e,0x73,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,
};


/* Misc Global */
static int debugmode = 0;

static void message( void )
{
  printf("SC68 file debugger - 68K disassembler\n"
         "(c) 1998-2003  Benjamin Gerard\n"
	 "\n"
         "Usage : debug68 [options] <input>\n"
	 "\n"
         "Options:\n"
         " -noi       Not interractif\n"
         " -t:#       Choose track number [1..99]\n"
         " -ext:name  Use this external replay\n"
         " -@:#       hexa load address (default:%X)\n"
         " -mem:#     68K memory size (Kb) (default:%dK)\n"
         "\n",
         SC68_LOADADDR,
         SC68_68KMEMSIZE);
}

/*******************
 *  MISC FUNCATIONS *
 *******************/

/*static char *plurial(int n)
  {
  return (n>1) ? "s" : "";
  }*/

/* string to Hexa number
 */
int atox(char *s)
{
  int r;
  if(!s) {
    return 0;
  }
  for (r=0; *s; s++) {
    if(*s>='0' && *s<='9') {
      r = r*16 + *s-'0';
    } else if(*s>='a' && *s<='f') {
      r = r*16 + *s-'a'+10;
    } else if(*s>='A' && *s<='F') {
      r = r*16 + *s-'A'+10;
    } else {
      return r;
    }
  }
  return r;
}


static unsigned int ms_to_sec(unsigned int ms)
{
  return (ms+999u) / 1000u;
}

static unsigned int frames_to_ms(unsigned int frames, unsigned int cpf)
{
  u64 ms;

  ms =  frames; /* total frames. */
  ms *= cpf;    /* total cycles. */
  ms /= (8000000/1000u);

  return (unsigned int) ms;
}

static unsigned int ms_to_frames(unsigned int ms, unsigned int cpf)
{
  u64 fr;

  fr =  ms;    /* total ms */
  fr *= (8000000/1000u); /* total cycles */
  fr /= cpf;

  return (unsigned int) fr;
}

static unsigned int cycle_per_frame(unsigned int frq)
{
  if (!frq) {
    /* $$$ Safety net ... */
    frq = 50;
  }
  if (frq == 50) {
    return 160256;
  } else if (frq == 60) {
    /* $$$ Can't remember the exact number of cycle for 60Hz VBL */
  }
  return 8000000u/frq;
}

/*  Display pending error message
 */
static int spoolerror(void)
{
  const char * s;

  while (s=SC68error_get(), s) {
    fprintf(stderr,"=>%s\n",s);
  }
  return -0x2;
}

/*  TRUE if file exist (can be "rb" openened)
 */
#if 0
static int fexist(char *name)
{
  int r;
  FILE *f = fopen(name,"rb");

  if (f) {
    return 0;
  }
  r = fread(&r,1,1,f);
  fclose(f);
  return r == 1;
}
#endif

/* Alloc & load external replay file
 */
static u8 * load_external_replay(istream_t * is, int *psize)
{
  int size;
  u8 *buf = 0;

  size = istream_length(is);
  if (size == -1) {
    goto error;
  }

  buf = SC68alloc(size);
  if (!buf) {
    goto error;
  }

  if (istream_read(is, buf, size) != size) {
    goto error;
  }

  *psize = size;
  return buf;

 error:
  SC68free(buf);
  return 0;
}

static char *check_a_name(char *name)
{
  int i;
  static char *list[][2] =
    {
      { "Max", "Jochen Hippel (Mad Max)" },
      { "Mad Max", "Jochen Hippel (Mad Max)" },

      { "Count Zero", "Nic Alderton (Count Zero)" },
      { "CZ", "Nic Alderton (Count Zero)" },
      { "Nick Alderton (Count Zero)", "Nic Alderton (Count Zero)" },
      { "C.Z.", "Nic Alderton (Count Zero)" },
      { "C.Z", "Nic Alderton (Count Zero)" },

      { "Big Alec", "Gunnar Gaubatz (Big Alec)" },
      { "B.A.", "Gunnar Gaubatz (Big Alec)" },
      { "B.A", "Gunnar Gaubatz (Big Alec)" },
      { "G. Gaubatz (Big Alec)", "Gunnar Gaubatz (Big Alec)" },

      { "Lap", "Emmanuel Larry (Lap)" },

      { "Docland", "Cyril Payet (Doclands)" },
      { "Doclands", "Cyril Payet (Doclands)" },
      { "Cyril Payet (Docland)", "Cyril Payet (Doclands)" },

      { "Jess", "J.S. Gerard (Jess)" },

      { "Lotus", "Laurens Tummers (Lotus)" },

      { "JMP","John M. Phillips" },
      { "J.M.P","John M. Phillips" },
      { "J.M.P.","John M. Phillips" },


      { 0, 0 }
    };

  if (name) {
    for(i=0; list[i][0]; i++) {
      if(!SC68strcmp(name,list[i][0])) {
	name = list[i][1];
	break;
      }
    }
  }
  return name;
}

static int checknames(void)
{
  int i;
  if(!disk) {
    return 0;
  }
  for(i=0; i<disk->nb_six; i++) {
    disk->mus[i].author = check_a_name(disk->mus[i].author);
    disk->mus[i].composer = check_a_name(disk->mus[i].composer);
  }
  return 0;
}

static int checkdisk(void)
{
  if(disk==NULL)
    return SC68error_add("No disk loaded");
  return 0;
}

static int checktrack(int track)
{
  int err;
  if(err=checkdisk(), err<0) {
    return err;
  }
  if (!disk->nb_six) {
    return SC68error_add("Disk has no track");
  }
  if(track<0 || track>=disk->nb_six) {
    return SC68error_add("File Corrupted : track #%d "
			 "Out of range [0..%d]",
			 track, disk->nb_six-1);
  }
  if(disk->default_six < 0) {
    disk->default_six = 0;
  } else if (disk->default_six >= disk->nb_six) {
    disk->default_six = disk->nb_six-1;
  }
  if (track < 0 || track >= disk->nb_six) {
    track = disk->default_six;
  }
  return track;
}

/* Display various track info : depending of level
 * level 1 : track No, music name
 * level 2 : author, composer
 * level 4 : IO-chip, replay rate , cycles/pass
 * level 8 : Load address, External replay
 */
static int trackinfo(int trk, int infolevel)
{
  int err;
  music68_t *mus;

  if(err=checktrack(trk), err<0) {
    return err;
  }

  if(err != trk) {
    return SC68error_add("Track #%d out of range",trk+1);
  }

  mus = disk->mus + trk;

  /* level 1 : time, music name */
  if (infolevel&1) {
    printf("%c%-8s %-40s",
	   (trk==track) ? '*' : ' ',
	   SC68time_str(0, trk+1, mus->time_ms/1000),
	   mus->name);
  }

  /* level 2 : authors, composer */
  if (infolevel&2) {
    if(!SC68strcmp(mus->author,mus->composer)) {
      printf(" (%s)",mus->author);
    } else {
      printf(" (%s/%s)",mus->author,mus->composer);
    }
  }

  if (infolevel&3) {
    putchar('\n');
  }

  /* level 4 : IO-chip, replay rate , cycles/pass */
  if (infolevel&4) {
    if(mus->flags.ym) {
	printf("  YM-2149 (Atari STF/E sound chip) required\n");
    }
    if(mus->flags.ste) {
      printf("  MicroWire (Atari STE sound chip) required\n");
    }
    if(mus->flags.amiga) {
      printf("  Paula (Amiga 1000/500/1200... sound chip) required\n");
    }
    printf("  replay rate: %d Hz (%u cpf)\n",
	   mus->frq, cycle_per_frame(mus->frq));
  }

  /* level 8 : Load address, External replay */
  if (infolevel&8) {
    if (!mus->replay) {
      if(mus->a0 != SC68_LOADADDR) {
	printf("  Absolute position built-in replay @ $%X-%X (%d bytes)\n",
	       mus->a0,mus->a0+mus->datasz-1,mus->datasz);
      } else {
	printf("  Relocatable built-in replay (%d bytes)\n",
	       mus->datasz);
      }
    } else {
      if (mus->a0 != SC68_LOADADDR) {
	printf("  Absolute external replay <%s> @ $%X-%X (%d bytes)\n",
	       mus->replay,
	       mus->a0,mus->a0+mus->datasz-1,mus->datasz);
      } else {
	printf("  Relocatable external replay <%s> (%d bytes)\n",
	       mus->replay,
	       mus->datasz);
      }
    }
  }
  return 0;
}

/************
 * DISK INFO *
 ************/
static int diskinfo(void)
{
  int err;
  int i;

  if (err=checkdisk(), err<0) {
    return err;
  }

  printf("\nDISK <%s>\n",disk->name);
  for(i=0; i<disk->nb_six; i++) {
    if(err=trackinfo(i, 1 + 2), err<0) {
      return err;
    }
  }
  printf(" -- -----\n %s\n",
	 SC68time_str(0,disk->nb_six,ms_to_sec(disk->time_ms)));
  return 0;
}

/************
 * KILL DISK *
 ************/
static int killdisk(void)
{
  /* Clear 68K memory */
  memset(reg68.mem,0,reg68.memsz);
  memset(reg68.chk,0,reg68.memsz);

  /* Reset replay buffer */
  SC68free(replay);
  replay=0;
  replaysz = 0;
  SC68free(replayname);
  replayname = 0;

  /* Free mem & CLear pointers */

  if (disk) {
    int i;
    SC68free(disk->name);
    for (i=0; i<disk->nb_six; ++i) {
      SC68free(disk->mus[i].replay);
      SC68free(disk->mus[i].name);
      SC68free(disk->mus[i].author);
      SC68free(disk->mus[i].composer);
      memset(disk->mus+i,0,sizeof(disk->mus[i]));
    }
  }
  SC68free(disk);
  SC68free(lastload_fname);
  lastload_fname = 0;
  disk = 0;
  mus = 0;
  track = -1;

  return 0;
}


/******************
 * LOAD A NEW DISK *
 ******************/
static int loaddisk(char *fname, int mergemode)
{
  disk68_t *newdisk = 0;

  if (!fname) {
    return SC68error_add("No disk to load");
  }

  /* Desactif merge if no disk */
  mergemode &= -(disk!=0);

  /* Load SC68 file */
  if(newdisk=SC68file_load_file(fname), !newdisk) {
    return SC68error_add("Failed to load '%s'",fname);
  }


  /* If not merge mode : Free previous disk */
  if (!mergemode) {
    killdisk();
    disk = newdisk;
    lastload_fname = SC68strdup(fname);
  } else {
    int i,j,nb = disk->nb_six + newdisk->nb_six;

    /* Clear 68K memory */
    memset(reg68.mem,0,reg68.memsz);
    memset(reg68.chk,0,reg68.memsz);

    if(nb>SC68_MAX_TRACK) {
      printf( "Too many musix : skipping %d track(s)\n",nb-SC68_MAX_TRACK);
      nb = SC68_MAX_TRACK;
    }
    for(i=disk->nb_six, j=0; i<nb; i++, j++) {
      disk->mus[i] = newdisk->mus[j];
      disk->nb_six = nb;
    }
    /* $$$ MEMORY LEAK HERE , can't free newdisk (in used) $$$ */
  }

  /* Auto transform very known names !!! */
  checknames();

  /* Set current track */
  if(settrack(disk->default_six)<0) {
    return SC68error_add("File corrupted : Default track out of disk bound.");
  }
  
  return 0;
}

static int loaddata(char *fname, int d0, int a0, int frq, int flags,
		    char *mname, char *aname, char *rname)
{
  disk68_t * newdisk;
  istream_t * is;
  int  flen,size;

  if (!fname) {
    return SC68error_add("NULL pointer as data file !!");
  }

  is = istream_file_create(fname,1);
  if (!is) {
    return SC68error_add("Can't load data file : see previous");
  }
  if (istream_open(is)) {
    istream_destroy(is);
    return SC68error_add("Can't load data file : see previous");
  }
  flen = istream_length(is);
  if (flen == -1) {
    istream_destroy(is);
    return SC68error_add("Can't load data file : see previous");
  }

  printf("Load data file : %s (%d bytes)\n"
	 " d0=%d a0=$%x frq=%d flags=%c%c%c (%x)\n",
	 fname, flen, d0, a0, frq,
	 'y'-((!!(flags&SC68_YM))<<5),
	 's'-((!!(flags&SC68_STE))<<5),
	 'a'-((!!(flags&SC68_AMIGA))<<5),
	 flags);

  size = flen + sizeof(*newdisk);
  if(newdisk=(disk68_t *)SC68alloc(size), !newdisk) {
    istream_destroy(is);
    return SC68error_add("Can't load data file : see previous");
  }
  memset(newdisk,0,size);

  if (istream_read(is, newdisk->data, flen) != flen) {
    SC68free(newdisk);
    istream_destroy(is);
    return SC68error_add("Can't load data file : see previous");
  }
  istream_destroy(is);

  killdisk();
  disk = newdisk;

  /* At least 1 track for a valid disk */
  disk->nb_six    = 1;
  disk->mus[0].d0 = d0;
  disk->mus[0].a0 = a0;
  disk->mus[0].frq = frq;
  *(int*)&disk->mus[0].flags = flags;
  disk->mus[0].data = disk->data;
  disk->mus[0].datasz = flen;

  /* Name settup */
  disk->name = SC68strdup(mname);
  disk->mus[0].replay = SC68strdup(rname);
  disk->mus[0].author = SC68strdup(aname);
  disk->mus[0].composer = SC68strdup(disk->mus[0].author);

  /* Set current track */
  if(settrack(disk->default_six) < 0) {
    return SC68error_add("File corrupted : Default track out of disk bound.");
  }

  return 0;
}

/********************
 * SAVE CURRENT DISK *
 ********************/
static int savedisk(char *fname)
{
  int err;
  if(err=checkdisk(), err<0) {
    return err;
  }
  if (!fname) {
    return SC68error_add("Missing filename");
  }
  if (err = checktrack(disk->default_six = track), err<0) {
    return err;
  }
  if (err=SC68file_save_file(fname, disk), !err) {
    printf("%s saved\n",fname);
  }
  return err;
}

/* Kill previous external replay.
 * Load new one ( NULL is none ).
 */
static int setreplay(char *newname)
{
  istream_t * is;
  char * tmpname;

  /* Kill previous replay */
  SC68free(replayname);
  replayname = 0;
  SC68free(replay);
  replay = 0;
  replaysz = 0;
  
  if (!newname) {
    return 0;
  }
  tmpname = SC68strdup(newname);
  if (!tmpname) {
    return SC68error_add("Could not alloc replay name <%s>", newname);
  }

  is = SC68rsc_open(SC68rsc_replay, newname, 1);
  if (!is) {
    SC68free(tmpname);
    return SC68error_add("Could not find external replay <%s>", newname);
  }

  replay = load_external_replay(is, &replaysz);
  istream_destroy(is);
  if (!replay) {
    SC68free(tmpname);
    SC68error_add("Failed to load external replay <%s>", newname);
    return -1;
  }
  replayname = tmpname;
  return 0;
}

/* Set new current track No, Load external replay
 */
static int settrack(int newtrack)
{
  int err;
  char *thisreplay;

  if(err=checktrack(newtrack), err<0) {
    return err;
  }

  track    = newtrack;
  mus      = disk->mus+track;
  playaddr = a0 = forced_a0 ? forced_a0 : mus->a0;
  time_ms  = mus->time_ms;
  memlow   = (u32)-1;
  memhi    = (u32)-1;
  if (mus->frq==0) mus->frq = 50;
  cycleperpass = cycle_per_frame(mus->frq);
  thisreplay = (forced_replay) ? forced_replay : mus->replay ;
  if(err=setreplay(thisreplay), err<0) {
    return SC68error_add("Failed loading track %d external replay <%s>\n",
			 track,thisreplay);
  }
  return 0;
}

/*  68K First time init;
 */
static int init68K(int size)
{
  static int init=0;
  void * buffer;

  if(init) {
    if (reg68.mem) {
      memset(reg68.mem,0,reg68.memsz);
    }
    if (reg68.chk) {
      memset(reg68.chk,0,reg68.memsz);
    }
    return 0;
  }

  if (!EMU68_debugmode()) {
    return SC68error_add("EMU68 not compiled in debug mode.");
  }
  buffer = SC68alloc(size * 2); /* *2 for reg68.chk buffer */
  if(EMU68_init(buffer, size)<0) {
    SC68free(buffer);
    return SC68error_add(EMU68error_get());
  }
  init = 1;
  return 0;
}

static int init(int na, char **a)
{
  int err;
  int i;

  forced_fname  = NULL;
  forced_replay = NULL;
  forced_track  = -1;
  forced_a0     = 0;
  forced_help   = 0;

  /* Parse command line */
  for(i=1; i<na;i++)
    {
      if(strstr(a[i],"-t")==a[i])
	forced_track = atoi( a[i] + 2 + (a[i][2]==':') )-1;
      else if(strstr(a[i],"-noi")==a[i])
	isinterractif=0;
      else if(strstr(a[i],"-ext")==a[i])
	forced_replay = a[i] + 4 + (a[i][2]==':');
      else if(strstr(a[i],"-@")==a[i])
	forced_a0 = atox(a[i]+2+(a[i][2]==':'));
      else if(strstr(a[i],"-mem")==a[i])
	memsize = atoi(a[i]+4+(a[i][2]==':'))<<10;
      else if(strstr(a[i],"-?")==a[i]
	      || strstr(a[i],"-h")==a[i]
	      || strstr(a[i],"--help")==a[i])
	forced_help = 1;
      else if(forced_fname==NULL) forced_fname=a[i];
      else printf("Warning : Only one file could be loaded : '%s' skipped\n",
		  a[i]);
    }

  /*********************/

  if(err=init68K(memsize),err<0)
    return err;

  return 0;
}

/* Set music time ( if newtrack out of bound , use default )
 */
static int settime(int newtrack, unsigned newtime_ms, unsigned newframe)
{
  unsigned newtime = newtime_ms/1000u;
  if(newtrack=checktrack(newtrack), newtrack<0)
    return newtrack;
  printf("Set track #%02d time to %02d:%02d\n",
	 newtrack+1,newtime/60,newtime%60);
  disk->time_ms +=  newtime_ms - disk->mus[newtrack].time_ms;
  disk->mus[newtrack].time_ms = newtime_ms;
  return 0;
}

static int prepare_reg68(void)
{
  u32 A0=a0;
  int err;
  io68_t *io;
  int i;

  if(err=checktrack(track), err<0) {
    return err;
  }
  printf("Prepare 68000:\n"
	 " - Disk [%s]\n"
	 " - Track #%02d [%s]\n"
	 " - Replay [%s]\n"
	 " - Frq:%uHz Cpf:%u\n"
	 , disk->name, track+1, mus->name,
	 replayname ? replayname : "built-in",
	 mus->frq, cycleperpass);

  /* Must be call because of some _DEBUG test */
  EMU68_reset();

  /* Clear 68 K mem */
  memset(reg68.mem, 0, reg68.memsz);
  reg68.mem[0] = 0x4e;
  reg68.mem[1] = 0x73;
  reg68.mem[0x41a] = 0;         /* Zound Dragger */
  reg68.mem[0x41b] = 0x10;      /* Zound Dragger */
  reg68.mem[TRAP_VECTOR(14) + 0] = (u8) (TRAP_14_ADDR >> 24);
  reg68.mem[TRAP_VECTOR(14) + 1] = (u8) (TRAP_14_ADDR >> 16);
  reg68.mem[TRAP_VECTOR(14) + 2] = (u8) (TRAP_14_ADDR >> 8);
  reg68.mem[TRAP_VECTOR(14) + 3] = (u8) (TRAP_14_ADDR);
  memcpy(reg68.mem + TRAP_14_ADDR, trap14, sizeof(trap14));

  EMU68ioplug(&paula_io);
  EMU68ioplug(&ym_io);
  EMU68ioplug(&mw_io);

  /* Display IO list */
  for(i=0, io=reg68.iohead; io; io=io->next, i++) {
    printf(" - io #%d/%d : '%s' %x-%x\n",
	   i+1,reg68.nio,io->name,io->addr_low,io->addr_high);
  }

  if (setreplay(mus->replay) < 0) {
    return SC68error_add("Failed loading external replay <%s>\n",mus->replay);
  }

  if (replay) {
    u32 data;
    if(EMU68_memput(A0,replay,replaysz)<0) {
      SC68error_add(EMU68error_get());
      return SC68error_add("Failed copying external replay into 68K memory");
    }
    printf(" - External replay <%s> loaded @$%X-%X\n",
	   replayname, A0, A0+replaysz-1);
    data = A0 + ((replaysz+1)&-2);
    /* Check memory overflow */
    if(EMU68_memvalid(data,mus->datasz+1024)) {
      EMU68error_get(); /* Pop error message !!! */
      data = A0 - ((mus->datasz+1025)&-2);
      if (data<0x1000) {
	return SC68error_add("No room for copying data into 68K memory");
      }
    }
    A0 = data;
  }

  /* Copy Data into 68K memory */
  if(EMU68_memput(A0,(u8*)mus->data,mus->datasz)) {
    return SC68error_add(EMU68error_get());
  }

  /* $$$ NOTE: Absolute/Indepenadnt test is broken */
  printf(" - %s position %s loaded @$%X-%X\n",
	 (a0 == SC68_LOADADDR) ? "Independant" : "Absolute",
	 replay ? "music data" : " music replay and data",
	 A0,A0+mus->datasz-1);

  /* 68K registers init */
  reg68.pc   = playaddr;
  reg68.sr   = 0x2300;
  reg68.d[0] = mus->d0;
  reg68.d[1] = !mus->flags.ste;
  reg68.d[2] = mus->datasz;
  reg68.a[0] = A0;
  reg68.a[7] = reg68.memsz-16;
  reg68.cycle = 0;
  memset(reg68.chk,0,reg68.memsz);

  printf("68000 Ready!\n");

  return 0;
}

/* Memory & Time checker
 */
static int find_time(u32 nbsec,
		     u32 *pt_frm,
		     u32 *pt_sec,
		     u32 *pt_memmin,
		     u32 *pt_memmax)
{
  int err;
  u32 lastframe, nbframe;
  unsigned int nb_ms;
  unsigned int min,max,i;

  if (err=prepare_reg68(), err<0) {
    return err;
  }

  /* Default utime : 3 minutes */
  if (!nbsec) {
    nbsec = 3*60;
  }
  nbframe = ms_to_frames(nbsec*1000, cycleperpass);

  /* Run init code */
  printf("Running init code:"); fflush(stdout);
  EMU68_level_and_interrupt(0);
  memset(reg68.chk,0,reg68.memsz);
  printf("OK\n");

  reg68.pc   = playaddr+8;
  lastframe=0; i=0;
  do {
    nbsec = ms_to_sec(frames_to_ms(nbframe,cycleperpass));
    if (nbsec >= 60*60) {
      printf("Reach maximum time: abort...\n");
      lastframe=0;
      break;
    }
    printf("Running music for %s (%u frames, %u cpf): Wait a while...\n",
	   SC68time_str(0,track+1, nbsec), nbframe, cycleperpass);
    /* Run emulation code for given time */
    for(; i<nbframe; i++) {
      EMU68_level_and_interrupt(cycleperpass);
      if(reg68.framechk)
	lastframe = i;
    }

    nb_ms = frames_to_ms(lastframe,cycleperpass);
    nbsec = ms_to_sec(nb_ms);
    printf("Last cycle : %d => %s\n",lastframe,
	   SC68time_str(0, -1, nbsec));

    if(lastframe+lastframe/2 < nbframe)
      break;
    nbframe *= 2;
  } while(1);

  nb_ms = frames_to_ms(lastframe, cycleperpass);
  nbsec = nb_ms/1000u;

  /* Retrieve memory access bound */
  min = reg68.memsz-1024;
  max = 0;
  for(i=0x4000; i<reg68.memsz-1024; i++) {
    if(reg68.chk[i]) {
      if(i<min) min=i;
      if(i>max) max=i;
    }
  }

  if (pt_memmin) *pt_memmin = min;
  if (pt_memmax) *pt_memmax = max;
  if (pt_frm)    *pt_frm = lastframe;
  if (pt_sec)    *pt_sec = nb_ms;

  printf("Last cycle : %d => %s\n", lastframe, SC68time_str(0,-1, nbsec));
  printf("memory hit boundary @$%x-%x (%d bytes)\n", min,max,max-min);
  printf("Paula access: %c%c\n",
	 (paula_debug_access & 1) ? 'R' : '-',
	 (paula_debug_access & 2) ? 'W' : '-');
  printf("YM-2149 access: %c%c\n",
	 (ym_debug_access & 1) ? 'R' : '-',
	 (ym_debug_access & 2) ? 'W' : '-');
  printf("MicroWire access: %c%c\n",
	 (mw_debug_access & 1) ? 'R' : '-',
	 (mw_debug_access & 2) ? 'W' : '-');

  if (!(paula_debug_access|ym_debug_access|mw_debug_access)) {
    printf("No hardware access detected !!!\n");
  } else if (paula_debug_access && (ym_debug_access|mw_debug_access)) {
    printf("Both Atari and Amiga hardware detected !!!\n");
  } else {
    int old;
    old = mus->flags.amiga;
    mus->flags.amiga = !!paula_debug_access;
    if (old != mus->flags.amiga) {
      printf("%s PAULA hardware\n", old ? "CLEAR" : "FORCE");
    }

    old = mus->flags.ym;
    mus->flags.ym = !!ym_debug_access;
    if (old != mus->flags.ym) {
      printf("%s YM-2149 hardware\n", old ? "CLEAR" : "FORCE");
    }

    old = mus->flags.ste;
    mus->flags.ste = !!mw_debug_access;
    if (old != mus->flags.ste) {
      printf("%s STE hardware\n", old ? "CLEAR" : "FORCE");
    }
  }
  
  return 0;
}

static char * prompt(void)
{
  static char tmp[1024];

  if(!debugmode) {
    if (!disk) {
	return "No Disk>";
    } else {
      sprintf(tmp, "%s (#%d)>",disk->name,track+1);
      return tmp;
    }
  }  else {
    if (!disk) {
	return "Nothing to debug ('x' to exit debug mode)>";
    } else {
      return SC68debugger_prompt(disk->name,track+1);
    }
  }
}

/*
 *
 *
 *
 */
static int interractif(void)
{
  char **a;
  int na;
  int err,i;
  debug68_comline_t comline;
  memset(&comline,0,sizeof(comline));
  
  while (!quit) {
    int hascom = SC68comline_read(prompt(), &comline) > 0;
    int testmode;
    if (!hascom && !debugmode) {
      /* Skip empty comline except for debugger mode */
      continue;
    }
    na = comline.na;
    a  = comline.coms;

    /* Debugger mode */
    if(debugmode) {
      if(err=SC68debugger_newcom(na,a), err<0) {
	spoolerror();
      } else if (err > 0) {
	debugmode = 0;
      }
      continue;
    }

    /* Find prefixed test mode (conditionnal command execution) */
    testmode = 0;
    switch (a[0][0]) {
    case '=':    case '!':    case '>':    case '<':
      testmode = 1;
    }

    /* Perform test */
    if (testmode) {
      char * s;
      int not = 0, res = 0, err = 0, c;

      for (s=a[0]; !err && ((c=*s++),c); ) {
	switch (c) {
	case '!': not = !not; break;
	case '=': res = res || (quit_code == 0); break;
	case '>': res = quit_code > 0; break;
	case '<': res = quit_code < 0; break;
	default:
	  SC68error_add("Invalid test char '%c'", c);
	  err = 1;
	  break;
	}
      }

      if (err) {
	SC68error_add("Prefix test syntax error");
	spoolerror();
	continue;
      }
      if (!(res ^ not)) {
	continue;
      }
      /* Start after test. */
      if (--na <= 0) {
	continue;
      }
      a++;
    }

    for(i=0; commands[i].com; i++) {
      if( ! SC68strcmp(commands[i].com,    a[0]) ||
	  ! SC68strcmp(commands[i].altcom, a[0])) {
	quit_code = (*commands[i].fnct)(na,a);
	if(quit_code < 0) {
	  spoolerror();
	}
	break;
      }
    }
    if (!commands[i].com) {
      SC68error_add("unknown command <%s>", a[0]);
      spoolerror();
      quit_code=-1;
    }
  }
  return quit_code;
}

int main(int na, char **a)
{

  SC68set_alloc(malloc);
  SC68set_free(free);
#ifdef _DEBUG
  debugmsg68_set_handler((debugmsg68_t)vfprintf);
  debugmsg68_set_cookie(stderr);
#endif

  if(init(na,a)<0)
    return spoolerror();

  if(forced_help)
    {
      message();
      return 1;
    }

  if(forced_fname!=NULL && loaddisk(forced_fname,0)<0)
    return spoolerror();

  diskinfo();
  spoolerror();
  if(isinterractif) {
    int code = interractif();
    if (code < 0) {
      spoolerror();
    }
    return code;
  }
  else {
    return -1;
  }
  return 0;
}


static void dcom(command_t *com)
{
  printf(com->com);
  if(com->altcom!=NULL) printf(" (%s)",com->altcom);
  if(com->help  !=NULL) printf(" : %s",com->help);
  if(com->use   !=NULL) printf("\n  usage: %s %s",com->com,com->use);
  printf("\n");
}

static int help_on(char *name)
{
  int i;
  for(i=0; commands[i].com!=NULL; i++) {
    if(!SC68strcmp(commands[i].com,name) ||
       !SC68strcmp(commands[i].altcom,name)) {
      dcom(commands+i);
      return 1;
    }
  }
  return -1;
}

static int help_com(int na, char **a)
{
  int i,j;

  /* No args : display all commands help */
  if(na==1) {
    for(i=0; commands[i].com; i++) {
      dcom(commands+i);
    }
  } else {
    for(j=1; j<na;j++) {
      if(help_on(a[j]) < 0) {
	printf("Unknown command `%s'\n",a[j]);
      }
    }
  }
  return 0;
}

static int exit_com(int na, char **a)
{
  int code = quit_code;
  quit = 1;
  if (na > 1) {
    code = atoi(a[1]);
  }
  return code;
}

static int loaddisk_com(int na, char **a)
{
  if (na<2) {
    return help_on("ld");
  }
  return loaddisk(a[1],0) || diskinfo();
}

static int mergedisk_com(int na, char **a)
{
  if (na<2) {
    return help_on("md");
  }
  return loaddisk(a[1],1) || diskinfo();
}

static int savedisk_com(int na, char **a)
{
  if (na<2) {
    if(!lastload_fname || !lastload_fname[0]) {
      return help_on("sd");
    } else {
      return savedisk(lastload_fname);
    }
  }
  return savedisk(a[1]);
}

static int diskinfo_com(int na, char **a)
{
  return diskinfo();
}

static int settrack_com(int na, char **a)
{
  if (na<2) {
    return help_on("t=");
  }
  return settrack(atoi(a[1])-1) || trackinfo(track,-1);
}

static int trackinfo_com(int na, char **a)
{
  int i,err=0;

  if(na<2) {
    return trackinfo(track,-1);
  }

  for (i=1; i<na; i++) {
    int t;
    t = atoi(a[i]);
    err |= trackinfo(t-1, -1);
  }
  return err;
}

/* read mm:sec time format
 * Returns:
 * >=0  time in seconds
 * -1   Not a time format (missing ':')
 * -2   invalid digit
 */
static int read_time_format(char *a)
{
  char * deuxpoint,* s;
  int min=0,sec=0;

  if(deuxpoint=strchr(a,':'), !deuxpoint) {
    return -1;
  }

  for(s=a; s<deuxpoint; s++) {
    if (!isdigit(*s)) {
      return -2;
    } else {
      min = min*10 + *s-'0';
    }
  }

  for(++s; *s; s++) {
    if (!isdigit(*s)) {
      return -2;
    } else {
      sec = sec*10 + *s-'0';
    }
  }

  return min*60 + sec;
}

/* return 'perfect' unsigned decimal or -1
 */
static int read_unsigned(char *a)
{
  int n;
  for(n=0; *a; a++) {
    if(!isdigit(*a)) {
      return -1;
    }
    n = n * 10 + *a - '0';
  }
  return n;
}

static int settime_com(int na, char **a)
{
  int i,err,all;
  int newtime=-1,newtrack=track;

  /* settime */
  if(na<2) {
    return help_on("st");
  }

  /* settime ALL|* mm:sec */
  all = !SC68strcmp(a[1],"ALL") || !SC68strcmp(a[1],"*");
  if(all)
    {
      if(na!=3)
	return SC68error_add("bad parmeters : settime ALL|* mm:sec");
      if(newtime=read_time_format(a[2]), newtime<0)
	return SC68error_add("found %s, expected time format mm:sec",a[2]);
      for(newtrack=0; newtrack>=0 && checktrack(newtrack)==newtrack; newtrack++)
	if(err=settime(newtrack,newtime,-1), err<0)
	  return err;
      return 0;
    }

  /* settime [mm:sec] [ [track mm:sec] ... ] */
  for(i=1; i<na; i++)
    {
      int gettime,gettrack;
      gettrack = read_unsigned(a[i]);
      gettime  = read_time_format(a[i]);
      if(gettrack>0)
	newtrack = gettrack-1;
      else if(gettime>=0)
	newtime = gettime;
      else
	return SC68error_add("Found %s, expected track number or time mm:sec format\n",a[i]);

      if(newtime>=0)
	{
	  if(newtrack<0)
	    return SC68error_add("Found %s, expected track number\n",a[i]);
	  else if(err=settime(newtrack,newtime,-1), err<0)
	    return err;
	  newtrack = newtime = -1;
	}
    }

  if(newtrack>=0)
    return SC68error_add("Missing time for track #%02d\n",newtrack+1);

  return 0;
}

static int findtime_com(int na, char **a)
{
  int i,err=0,all;
  int newtime=0;
  int savetrack=track,newtrack=track;

  /* findtime ( current track, default time ) */
  if (na<=1) {
    if(err=find_time(0, &frames, &time_ms, &memlow, &memhi), !err) {
      settime(track,time_ms,frames);
    }
    return err;
  }

  /* findtime  ALL|* [mm:sec] */
  all = !SC68strcmp(a[1],"ALL") || !SC68strcmp(a[1],"*");
  if (all) {
    if(na==3) {
      if (newtime=read_time_format(a[2]), newtime<0) {
	return SC68error_add("bad parmeters : findtime ALL|* [mm:sec]");
      }
    } else if(na!=2) {
      return SC68error_add("too many parmeters : findtime ALL|* [mm:sec]");
    }
    for(newtrack=0;
	newtrack>=0 && checktrack(newtrack)==newtrack; newtrack++) {
      if(err=settrack(newtrack), err<0) {
	break;
      }
      if(err=find_time(newtime, &frames, &time_ms, &memlow, &memhi), err<0) {
	break;
      }
      settime(newtrack, time_ms, frames);
    }
    newtrack = -1;
  } else {
    /* findtime [[mm:sec] track] ... */
    for(i=1; i<na; i++) {
      int gettime,gettrack;

      gettrack = read_unsigned(a[i]);
      gettime  = read_time_format(a[i]);

      if (gettrack > 0) {
	newtrack = gettrack-1;
      } else if (gettime>=0) {
	newtime = gettime;
      } else {
	err=SC68error_add("Found %s, expected track number or"
			  " time mm:sec format\n",a[i]);
	break;
      }

      if (newtrack<0) {
	err=SC68error_add("Found %s, expected track number\n",a[i]);
	break;
      }	else if(err=settrack(newtrack), err<0) {
	  break;
      }	else {
	err=find_time(newtime, &frames, &time_ms, &memlow, &memhi);
	if (err < 0) {
	  break;
	}
	settime(newtrack, time_ms, frames);
	newtrack = -1;
      }
    }
  }

  /* Restore the current (default) track */
  settrack(savetrack);
  return err;
}

static int debug_com(int na, char **a)
{
  int err;
  debugmode = 0;
  if(err=checkdisk(), err<0)
    return SC68error_add("Nothing to debug");
  if(err=prepare_reg68(), err<0)
    return err;
  if(err=SC68debugger_entering(), err<0)
    return err;
  debugmode = 1;
  return 0;
}


static char *killspace(char *s)
{
  while(isspace(*s)) s++;
  return s;
}

static int myisspace(int c)
{
  return isspace(c) || c=='-';
}

static char *nextword(char *s)
{
  int prev;
  if(!*s) return s;
  for(prev=*s++; *s; prev=*s++)
    {
      if(myisspace(prev) && !myisspace(*s))
	return s;
    }
  return s;
}

static void cap_word(char *s)
{
  int c;
  char *saves = s;
  if(s==NULL) return;
  printf("%s => ",s);
  for(; *s; s++) if(*s=='_') *s=' ';
  s=killspace(saves);
  for(c=*s; c; s=nextword(s), c=*s)
    {
      int c2=toupper(c); /* Do not remove becoz of     */
      if(c2 != c) *s = c2; /* static in checknames() !!! */
    }
  printf("%s\n",saves);
}

static int capitalize_com(int na, char **a)
{
  int err,i;

  /* capitilize */
  if(na<2)
    return help_on("cap");

  if(err=checkdisk(), err<0)
    return err;

  /* capitilize <ALL|*> */
  if(!SC68strcmp(a[1],"ALL") || !SC68strcmp(a[1],"*"))
    {
      cap_word(disk->name);
      for(i=0; i<disk->nb_six;i++)
	{
	  cap_word(disk->mus[i].name);
	  cap_word(disk->mus[i].author);
	  cap_word(disk->mus[i].composer);
	}
      return 0;
    }
  return SC68error_add("This command  is implemented in mode ALL only !! sorry :-)");
}

/* Sorry but these commands alloc memory but never freed :-)
 */
static int replay_com(int na, char **a)
{
  char *r;
  int err,i;

  /* replay */
  if(na<2)
    return help_on("r=");

  /* Get name */
  if(!SC68strcmp(a[na-1],"NONE"))
    r = NULL;
  else if(r=(char*)SC68alloc(strlen(a[na-1])+1), r==NULL)
    return SC68error_add("Can't alloc new replay name");
  else strcpy(r,a[na-1]);

  /* replay NAME|NONE */
  if(na==2)
    {
      if(err=checktrack(track), err<0)
	return err;
      mus->replay = r;
      return trackinfo(track,11);
    }

  if(!SC68strcmp(a[1],"ALL") || !SC68strcmp(a[1],"*"))
    {
      if(err=checkdisk(), err<0)
	return err;
      for(i=0; i<disk->nb_six; i++)
	{
	  disk->mus[i].replay = r;
	  trackinfo(i,11);
	}
      return 0;
    }

  for(i=1; i<na-1; i++)
    {
      int t;
      if(!isdigit(a[i][0]))
	return SC68error_add("<%s> is not a track number",a[i]);
      t=atoi(a[i])-1;
      if(err=checktrack(t), err<0)
	return err;
      if(err==t)
	disk->mus[t].replay = r;
      trackinfo(t,11);
    }
  return 0;
}

static int name_com(int na, char **a)
{
  char *r;
  int err,i;

  /* name */
  if(na<2)
    return help_on("n=");

  /* Get name */
  if(!SC68strcmp(a[na-1],"NONE"))
    r = NULL;
  else if(r=(char*)SC68alloc(strlen(a[na-1])+1), r==NULL)
    return SC68error_add("Can't alloc new name");
  else strcpy(r,a[na-1]);

  /* replay NAME|NONE : disk name */
  if(na==2)
    {
      if(err=checkdisk(), err<0)
	return err;
      if(r==NULL)
	return SC68error_add("Can't set disk name to NONE");
      disk->name = r;
      return diskinfo();
    }

  if(!SC68strcmp(a[1],"ALL"))
    {
      if(err=checkdisk(), err<0)
	return err;
      if(r!=NULL) disk->name = r;
      for(i=0; i<disk->nb_six; i++)
	{
	  disk->mus[i].name = r==NULL ? disk->name : r;
	}
      diskinfo();
      return 0;
    }

  for(i=1; i<na-1; i++)
    {
      int t;
      if(!isdigit(a[i][0]))
	return SC68error_add("<%s> is not a track number",a[i]);
      t=atoi(a[i])-1;
      if(err=checktrack(t), err<0)
	return err;
      if(err==t)
	disk->mus[t].name = (r==NULL) ? disk->name : r;
      trackinfo(t,3);
    }
  return 0;
}

static int author_com(int na, char **a)
{
  char *r;
  int err,i;

  /* author */
  if(na<2)
    return help_on("a=");

  /* Get author name */
  if(!SC68strcmp(a[na-1],"NONE"))
    r = NULL;
  else if(r=(char*)SC68alloc(strlen(a[na-1])+1), r==NULL)
    return SC68error_add("Can't alloc new name");
  else strcpy(r,a[na-1]);

  if(na==2)
    {
      if(err=checkdisk(), err<0)
	return err;
      for(i=0; i<disk->nb_six; i++)
	{
	  disk->mus[i].author = r;
	}
      diskinfo();
      return 0;
    }

  for(i=1; i<na-1; i++)
    {
      int t;
      if(!isdigit(a[i][0]))
	return SC68error_add("<%s> is not a track number",a[i]);
      t=atoi(a[i])-1;
      if(err=checktrack(t), err<0)
	return err;
      if(err==t)
	disk->mus[t].author = r;
      trackinfo(t,3);
    }

  return 0;
}

static int composer_com(int na, char **a)
{
  char *r;
  int err,i;

  /* composer */
  if(na<2)
    return help_on("c=");

  /* Get composer name */
  if(!SC68strcmp(a[na-1],"NONE"))
    r = NULL;
  else if(r=(char*)SC68alloc(strlen(a[na-1])+1), r==NULL)
    return SC68error_add("Can't alloc new name");
  else strcpy(r,a[na-1]);

  if(na==2)
    {
      if(err=checkdisk(), err<0)
	return err;
      for(i=0; i<disk->nb_six; i++)
	{
	  disk->mus[i].composer = r;
	}
      diskinfo();
      return 0;
    }

  for(i=1; i<na-1; i++)
    {
      int t;
      if(!isdigit(a[i][0]))
	return SC68error_add("<%s> is not a track number",a[i]);
      t=atoi(a[i])-1;
      if(err=checktrack(t), err<0)
	return err;
      if(err==t)
	disk->mus[t].composer = r;
      trackinfo(t,3);
    }

  return 0;
}

static int deltrack_com(int na, char **a)
{
  int err,t;
  /* deltrack */
  if(na!=2)
    return help_on("del");

  /* deltrack # */
  if(!isdigit(a[1][0]))
    return SC68error_add("<%s> is not a track number",a[1]);
  t=atoi(a[1])-1;
  if(err=checktrack(t), err<0)
    return err;
  if(err==t)
    {
      int i;
      for(i=t; i<disk->nb_six-1; i++)
	disk->mus[i] = disk->mus[i+1];
      disk->nb_six--;
    }
  return diskinfo();
}

static int frq_com(int na, char **a)
{
  int err,i,rate=-1, all;

  /* frq */
  if (na < 2) {
    return help_on("f=");
  }

  if(err=checkdisk(), err<0) {
    return err;
  }

  /* Get frq rate */
  if(isdigit(a[na-1][0])) {
    rate = atoi(a[na-1]);
  }
  if(rate<0 || rate>20000) {
    return SC68error_add("Bad or missing rate (in hz)");
  }

  all = (na == 2)
    || ((na == 3) && (!SC68strcmp(a[1],"ALL") || !SC68strcmp(a[1],"*")));

  if(all) {
    for(i=0; i<disk->nb_six; i++) {
      disk->mus[i].frq = rate;
    }
    diskinfo();
  } else {
    for(i=1; i<na-1; i++) {
      int t;
      if (!isdigit(a[i][0])) {
	err =  SC68error_add("<%s> is not a track number",a[i]);
	break;
      }
      t=atoi(a[i])-1;
      if(err=checktrack(t), err<0) {
	break;
      }
      if(err==t) {
	disk->mus[t].frq = rate;
      }
      trackinfo(t,7);
    }
  }
  if (mus) {
    cycleperpass = cycle_per_frame(mus->frq);
  }
  return -(err<0);
}

static int savedata_com(int na, char **a)
{
  int err;
  int t=track;
  int namei = 1;
  istream_t * os;
  if(na<2)
    return help_on("sdat");
  if(err=checkdisk(), err<0)
    return err;
  if(na>2)
    {
      t = atoi(a[1]) - 1;
      namei = 2;
    }
  if(err=checktrack(t), err<0)
    return err;
  os = istream_file_create(a[namei],2);
  if (!os) return -1;
  if (!istream_open(os)) {
    istream_destroy(os);
    return -1;
  }
  printf("saving track #%d data (%p,%d)\n",
	 t+1,disk->mus[t].data,disk->mus[t].datasz);
  if(disk->mus[t].data!=NULL && disk->mus[t].datasz > 0)
    err =
      istream_write(os, disk->mus[t].data, disk->mus[t].datasz)
      != disk->mus[t].datasz;
  else
    err = SC68error_add("No dat in this track!!!");
  istream_destroy(os);
  return -!!err;
}

typedef struct
{
  char *mname;
  char **rname;
  int *flags;
  char **aname;
  int *a0;
  int *frq;
} replay_parm_t;

static int check_ext_list(replay_parm_t *rparm)
{
  if(	!SC68strcmp(rparm->mname,	"FC13") ||
	!SC68strcmp(rparm->mname,	"FC14") ||
	!SC68strcmp(rparm->mname,  "FC") )
    {
      *rparm->rname = "fc13_14";
      *rparm->flags = SC68_AMIGA;
      return 1;
    }
  else
    if(	!SC68strcmp(rparm->mname,  "PUMA") ||
	!SC68strcmp(rparm->mname,  "PUM") )
      {
	*rparm->rname = "puma";
	*rparm->flags = SC68_AMIGA;
	return 2;
      }
    else
      if(	!SC68strcmp(rparm->mname,  "MON") )
	{
	  *rparm->rname = "MON";
	  *rparm->flags = SC68_AMIGA;
	  *rparm->aname = "Maniacs Of Noise";
	  return 3;
	}
      else
	if(	!SC68strcmp(rparm->mname,  "DW") )
	  {
	    *rparm->rname = "DWaga";
	    *rparm->flags = SC68_AMIGA;
	    *rparm->aname = "David Whittaker";
	    return 4;
	  }
	else
	  if(	!SC68strcmp(rparm->mname,  "BP") ||
		!SC68strcmp(rparm->mname,  "BP3") )
	    {
	      *rparm->rname = "bs22";
	      *rparm->flags = SC68_AMIGA;
	      return 5;
	    }
	  else
	    if(	!SC68strcmp(rparm->mname,  "JAM") )
	      {
		*rparm->rname = "JamCrackerPro";
		*rparm->flags = SC68_AMIGA;
		return 6;
	      }
	    else
	      if(	!SC68strcmp(rparm->mname,  "DMU") )
		{
		  *rparm->rname = "DMU";
		  *rparm->flags = SC68_AMIGA;
		  *rparm->a0 = 0x74f6;
		  return 6;
		}
	      else
		if(	!SC68strcmp(rparm->mname,  "TCB") )
		  {
		    *rparm->rname = "TCBtracker";
		    *rparm->flags = SC68_YM;
		    return 7;
		  }
		else
		  if(	!SC68strcmp(rparm->mname,  "MM2") )
		    {
		      *rparm->rname = "musmon2";
		      *rparm->flags = SC68_YM;
		      *rparm->frq = 200;
		      return 8;
		    }
		  else
		    if(	!SC68strcmp(rparm->mname,  "SSD1") )
		      {
			*rparm->rname = "ssd1";
			*rparm->flags = SC68_YM;
			return 9;
		      }
		    else
		      if(	!SC68strcmp(rparm->mname,  "ELOF") )
			{
			  *rparm->rname = "blipp079";
			  *rparm->flags = SC68_YM;
			  return 10;
			}
		      else
			if(	!SC68strcmp(rparm->mname,  "MS27") )
			  {
			    *rparm->rname = "tao_ms27";
			    *rparm->flags = SC68_YM;
			    *rparm->aname = "Tao";
			    *rparm->frq   = 200;
			    return 11;
			  }
			else
			  if(	!SC68strcmp(rparm->mname,  "MS25") )
			    {
			      *rparm->rname = "tao_ms25";
			      *rparm->flags = SC68_YM;
			      *rparm->aname = "Tao";
			      return 11;
			    }
			  else
			    if(	!SC68strcmp(rparm->mname,  "MS21") )
			      {
				*rparm->rname = "tao_ms211";
				*rparm->flags = SC68_YM;
				*rparm->aname = "Tao";
				return 12;
			      }
			    else
			      if(	!SC68strcmp(rparm->mname,  "YM") )
				{
				  *rparm->flags = SC68_YM;
				  return 256;
				}
			      else
				return 0;
}

static int loaddata_com(int na, char **a)
{
  char *fname=NULL,*rname=NULL,*aname=NULL,*mname=NULL,*s;
  char tmp[1024];
  int i;
  int d0=1,a0=SC68_LOADADDR,frq=50,flags=SC68_YM;
  replay_parm_t rparm;
  if(na<2)
    return help_on("ldat");

  /* Find filename in arg list */
  for(i=1, fname=NULL; i<na; i++) {
    if(a[i][0] != '-') {
      if(!fname) {
	fname = a[i];
      } else {
	return SC68error_add("Invalid argument [%s] :Only one file name",a[i]);
      }
    }
  }

  if (!fname) {
    return SC68error_add("Loaddata failed : Missing filename\n");
  }

  /* Get all possible info from filename */

  /* Kill path (posix) : get music-name from filename */
  strncpy(tmp,fname,sizeof(tmp));
  tmp[sizeof(tmp)-1] = 0;
  if(mname=strrchr(tmp,'/'), !mname) {
    mname = tmp;
  } else {
    mname = mname+1;
  }

  rparm.a0 = &a0;
  rparm.aname = &aname;
  rparm.flags = &flags;
  rparm.frq = &frq;
  rparm.mname = mname;
  rparm.rname = &rname;

  /* Kill prefix extension (Amigaaaaa) */
  if(s=strchr(mname,'.'), (s!=NULL && (s-mname)<=4))
    {
      *s = 0;
      if(check_ext_list(&rparm))
	mname = s+1;
      else
	*s = '.';
    }

  /* Kill ext */
  if(s=strrchr(mname,'.'), s!=NULL)
    {
      rparm.mname = s+1;
      if(check_ext_list(&rparm))
	*s = 0;
      /* Assume ext for len<=4 & no digit in first lettter
	 (avoid to remove hydra.0.1) */
      else if(!isdigit(s[1]) && strlen(s+1)<=4)
	*s = 0;
    }

  /* Remove _ by white space */
  for(s=strchr(mname,'_'); s!=NULL; s=strchr(s,'_')) *s= ' ';

  /* Command line parsing : get command args */
  for(i=1; i<na; i++) {
    rparm.mname = a[i]+1;
    if(a[i][0]=='-' && !check_ext_list(&rparm)) {
      if(toupper(a[i][1])=='D' && a[i][2]=='0'  && a[i][3]=='=')
	d0 = atoi(a[i]+4);
      else if(toupper(a[i][1])=='A' && a[i][2]=='0'  && a[i][3]=='=')
	a0 = atox(a[i]+4);
      else if(toupper(a[i][1])=='F' && a[i][2]=='=')
	frq = atoi(a[i]+3);
      else {
	int j;
	for(j=1; a[i][j]; j++) {
	  if(a[i][j]=='y') flags &= ~SC68_YM;
	  else if(a[i][j]=='Y') flags |= SC68_YM;
	  else if(a[i][j]=='a') flags &= ~SC68_AMIGA;
	  else if(a[i][j]=='A') flags |= SC68_AMIGA;
	  else if(a[i][j]=='s') flags &= ~SC68_STE;
	  else if(a[i][j]=='S') flags |= SC68_STE;
	}
      }
    }
  }
  
  if(loaddata(fname,d0,a0,frq,flags,mname,aname,rname))
    return SC68error_add("loaddata : Failed");
  return diskinfo();
}

static int dup_com(int na, char **a)
{
  int err,t;
  music68_t mus;

  if(na<2)
    return help_on("+");

  if(err=checkdisk(), err<0)
    return err;

  t = atoi(a[1]) - 1;

  if(err=checktrack(t), err!=t)
    return err;

  if(disk->nb_six == SC68_MAX_TRACK)
    return SC68error_add("Too many tracks");

  mus = disk->mus[t];
  mus.d0++;
  disk->mus[disk->nb_six++] = mus;

  return diskinfo();
}


static int address_com(int na, char **a)
{
  int a0=SC68_LOADADDR, track = 0, err, i;
  char *s;

  if (na < 2 || na > 3) {
    return help_on("address");
  }

  if (err=checkdisk(), err<0) {
    return err;
  }

  if (na == 3) {
    s = a[2];
    track = atoi(a[1]);
  } else {
    s = a[1];
  }

  if (*s == '$') {
    ++s;
  }
  if (isxdigit(*s)) {
    a0 = atox(s);
  }

  if (!track) {
    for(i=0; i<disk->nb_six; i++) {
      disk->mus[i].a0 = a0;
    }
    diskinfo();
    return 0;
  } else {
    --track;
    if (track<0 || track>=disk->nb_six) {
      return SC68error_add("Track #%d out of disk bound [%d, %d]",
			   track+1,1,disk->nb_six);
    }
    disk->mus[track].a0 = a0;
    trackinfo(track, -1);
  }
  return 0;
}
