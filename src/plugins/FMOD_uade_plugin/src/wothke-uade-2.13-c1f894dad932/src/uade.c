/* UADE - Unix Amiga Delitracker Emulator
 * Copyright 2000-2006, Heikki Orsila
 *
 * Patched version of uade.c - removed separate frontend & all IPC stuff
 * 2014, Juergen Wothke
 */

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <time.h>
#include <limits.h>
#include <stddef.h>
#ifdef EMSCRIPTEN
#include <unistd.h>
#include <string.h>
#endif
#include "sysconfig.h"
#include "sysdeps.h"

#include "options.h"
#include "events.h"
#include "uae.h"
#include "memory.h"
#include "custom.h"
#include "readcpu.h"
#include "newcpu.h"
#ifndef EMSCRIPTEN
#include "debug.h"
#else
#include "uadeconfstructure.h"
#include "uadestate.h"
#include "uadeconf.h"
#include "eagleplayer.h"
#include "songinfo.h"
#include "songdb.h"
#endif
#include "gensound.h"
#include "cia.h"
#include "sd-sound.h"
#include "audio.h"

#include "uade.h"
#include "amigamsg.h"
#include "ossupport.h"
#include "sysincludes.h"

#include "audiodevice.h"

// original amiga stuff from amigasrc folder
#include "exec/ports.h"
#include "exec/resident.h"
#include "exec/libraries.h"


enum print_help {
  OPTION_HELP = 1,
  OPTION_ILLEGAL_PARAMETERS = 2,
  OPTION_NO_SONGS = 3
};
#ifdef EMSCRIPTEN
int warn_illegal_PC = 1;
void ems_notify_song_update(const char *info_text, const char *inf_mins, const char *inf_maxs, const char *inf_curs);
void ems_notify_format_update(const char *format);
void ems_notify_player_update(const char *player);

// soundbuffer read size in bytes (buffer uses 16-bit stereo)
// FIXME: smaller size leads to weird speed changes - might be flaw in EMSCRIPTEN soundgen loop
#define SNDBUF_READ_SIZE (1024 * 4)

#endif
#ifndef EMSCRIPTEN
static
#endif
void change_subsong(int subsong);

static int uade_calc_reloc_size(uae_u32 *src, uae_u32 *end);
static int uade_get_u32(int addr);
static void uade_print_help(enum print_help problemcode, char *progname);
static void uade_put_long(int addr,int val);
static int uade_safe_load(int dst, FILE *file, int maxlen);
static int uade_valid_string(uae_u32 address);


static const int SCORE_MODULE_ADDR   = 0x100;
static const int SCORE_MODULE_LEN    = 0x104;
static const int SCORE_PLAYER_ADDR   = 0x108;
static const int SCORE_RELOC_ADDR    = 0x10C;
static const int SCORE_USER_STACK    = 0x110;
static const int SCORE_SUPER_STACK   = 0x114;
static const int SCORE_FORCE         = 0x118;
static const int SCORE_SET_SUBSONG   = 0x11c;
static const int SCORE_SUBSONG       = 0x120;
static const int SCORE_NTSC          = 0x124;
static const int SCORE_MODULE_NAME_ADDR = 0x128;
static const int SCORE_HAVE_SONGEND  = 0x12C;
static const int SCORE_POSTPAUSE     = 0x180;
static const int SCORE_PREPAUSE      = 0x184;
static const int SCORE_DELIMON       = 0x188;
static const int SCORE_EXEC_DEBUG    = 0x18C;
static const int SCORE_VOLUME_TEST   = 0x190;
static const int SCORE_DMA_WAIT      = 0x194;
static const int SCORE_MODULECHANGE  = 0x198;

static const int SCORE_HAVE_REPLYMSG  = 0x1fc;

static const int SCORE_INPUT_MSG     = 0x200;
static const int SCORE_MIN_SUBSONG   = 0x204;
static const int SCORE_MAX_SUBSONG   = 0x208;
static const int SCORE_CUR_SUBSONG   = 0x20C;

static const int SCORE_OUTPUT_MSG    = 0x300;

#ifndef EMSCRIPTEN
struct uade_ipc uadeipc;
#else
// code adapted from uade123.c: uses EaglePlayer to determine the player name used for a song
struct uade_state g_state;
#endif

int uade_audio_skip;
int uade_audio_output;
int uade_debug;
int uade_read_size;
int uade_reboot;
int uade_time_critical;

static int disable_modulechange;
static int old_ledstate;
static int uade_big_endian;
static int uade_dmawait;
static int uade_execdebugboolean;
static int uade_highmem;
static char uade_player_dir[PATH_MAX];
#ifdef EMSCRIPTEN
static struct uade_song2 song;
#else
static struct uade_song song;
#endif
static int uade_speed_hack;
static int voltestboolean;

static char epoptions[256];
static size_t epoptionsize;


static void add_ep_option(const char *s)
{
  size_t bufsize, l, i;

  bufsize = sizeof epoptions;
  l = strlen(s) + 1;
  i = epoptionsize;

  if (strlcpy(&epoptions[i], s, bufsize - i) >= (bufsize - i)) {
    fprintf(stdout, "Warning: uade eagleplayer option overflow: %s\n", s);
    return;
  }

  epoptionsize += l;
}


/* This is called when an eagleplayer queries for attributes. The query result
   is returned through 'dst', and the result is at most maxlen bytes long.
   'src' contains the full query. */
static int get_info_for_ep(char *dst, char *src, int maxlen)
{
  int ret = -1;
  if (strcasecmp(src, "eagleoptions") == 0) {
    if (epoptionsize > 0) {
      if (epoptionsize <= maxlen) {
	ret = epoptionsize;
	memcpy(dst, epoptions, ret);
      } else {
	fprintf(stdout, "uadecore: too long options: %s maxlen = %d\n",
		epoptions, maxlen);
      }
    } else {
      ret = 0;
    }
  } else {
    uade_send_debug("Unknown eagleplayer attribute queried: %s", src);
  }
  return ret;
}

#ifndef EMSCRIPTEN
static
#else
void set_subsong(int subsong)
{
  uade_put_long(SCORE_SET_SUBSONG, 1);
  uade_put_long(SCORE_SUBSONG, subsong);
}

#endif
void change_subsong(int subsong)
{
  song.cur_subsong = subsong;
  uade_put_long(SCORE_SUBSONG, subsong);
  uade_send_amiga_message(AMIGAMSG_SETSUBSONG);
  flush_sound();
}


// XXX get rid of this redundant hunk loading "knowledge"
static int uade_calc_reloc_size(uae_u32 *src, uae_u32 *end)
{
  uae_u32 offset;
  int i, nhunks;

  if (ntohl(*src) != 0x000003f3)
    return 0;
  src++;

  if (src >= end)
    return 0;
  if (ntohl(*src))
    return 0;
  src++;

  if (src >= end)
    return 0;
  /* take number of hunks, and apply the undocumented 16-bit mask feature */
  nhunks = ntohl(*src) & 0xffff;
  if (nhunks == 0)
    return 0;
  src += 3;          /* skip number of hunks, and first & last hunk indexes */

  offset = 0;

  for (i = 0; i < nhunks; i++) {
    if (src >= end)
      return 0;
    offset += 4 * (ntohl(*src) & 0x00FFFFFF);
    src++;
  }
  if (((int) offset) <= 0 || ((int) offset) >= uade_highmem)
    return 0;
  return ((int) offset);
}

#ifdef EMSCRIPTEN
struct uade_sample_data sample_data;

struct uade_sample_data sample_data= {.alloclen=0, .buflen= 0, .buf= 0, .bufChan0= 0, .bufChan1= 0, .bufChan2= 0, .bufChan3= 0, .is_new = 0};
struct uade_sample_data * get_new_samples() {
	if (sample_data.is_new != 0) {
		return &sample_data;
	}
	return 0;
}
#endif
/* last part of the audio system pipeline */
void uade_check_sound_buffers(int bytes)
{
#ifdef EMSCRIPTEN
  if (uade_big_endian != 0) // need little endian
#else
  uint8_t space[UADE_MAX_MESSAGE_SIZE];
  struct uade_msg *um = (struct uade_msg *) space;

  /* transmit in big endian format, so swap if little endian */
  if (uade_big_endian == 0)
#endif
    uade_swap_buffer_bytes(sndbuffer, bytes);

  /* LED state changes are reported here because we are in send state and
     this place is heavily rate limited. */
  if (old_ledstate != gui_ledstate) {
    old_ledstate = gui_ledstate;
  //  uade_send_debug("LED is %s", gui_ledstate ? "ON" : "OFF");
  }

#ifndef EMSCRIPTEN
  um->msgtype = UADE_REPLY_DATA;
  um->size = bytes;
  memcpy(um->data, sndbuffer, bytes);
  if (uade_send_message(um, &uadeipc)) {
    fprintf(stdout, "uadecore: Could not send sample data.\n");
    exit(-1);
  }
#else
	if (uade_test_silence(sndbuffer, bytes, &g_state)) {
	  fprintf(stdout, "silence detected (%d seconds)\n", g_state.config.silence_timeout);
		quit_program = 1;
	}

  // signal that we have a new buffer full of sample data.. somebody should better go and fetch it..
  if (sample_data.alloclen < bytes) {
	if (sample_data.buf) {
		free(sample_data.buf);
		// "scope" buffers
		free(sample_data.bufChan0);
		free(sample_data.bufChan1);
		free(sample_data.bufChan2);
		free(sample_data.bufChan3);
	}
	sample_data.buf= malloc(sizeof(char) * bytes);

		// "scope" buffers
	sample_data.bufChan0= malloc(sizeof(char) * bytes);
	sample_data.bufChan1= malloc(sizeof(char) * bytes);
	sample_data.bufChan2= malloc(sizeof(char) * bytes);
	sample_data.bufChan3= malloc(sizeof(char) * bytes);
  }
  memcpy(sample_data.buf, sndbuffer, bytes);

// FIXME make this optional?
  memcpy(sample_data.bufChan0, chn0buffer, bytes);
  memcpy(sample_data.bufChan1, chn1buffer, bytes);
  memcpy(sample_data.bufChan2, chn2buffer, bytes);
  memcpy(sample_data.bufChan3, chn3buffer, bytes);

  sample_data.buflen= bytes;
  sample_data.is_new= 1;
#endif

  uade_read_size -= bytes;
  assert(uade_read_size >= 0);

  if (uade_read_size == 0) {
#ifdef EMSCRIPTEN
  // just keep producing samples...
	uade_read_size= SNDBUF_READ_SIZE;
#else
    /* if all requested data has been sent, move to S state */
    if (uade_send_short_message(UADE_COMMAND_TOKEN, &uadeipc)) {
      fprintf(stdout, "uadecore: Could not send token (after samples).\n");
      exit(-1);
    }
    uade_handle_r_state();
#endif
  }
}

/* Send debug messages back to uade frontend, which either prints
   the message for user or not. "-v" option can be used in uade123 to see all
   these messages. */
void uade_send_debug(const char *fmt, ...)
{
  char dmsg[256];
  va_list ap;
  va_start (ap, fmt);
  vsnprintf(dmsg, sizeof(dmsg), fmt, ap);
#ifdef EMSCRIPTEN
  	struct uade_state *state= &g_state;
	if (state->config.verbose) {
		if(strlen(dmsg)) {
			// use stdout to avoid the useless stacktrace info
			fprintf(stdout, "%s\n", dmsg);	// with the different msg source it seems better to handle NL here
		} else {
			fprintf(stdout, "empty amiga message\n");
		}
	}
#else
  if (uade_send_string(UADE_REPLY_MSG, dmsg, &uadeipc)) {
    fprintf(stdout, "uadecore %s:%d: Could not send debug message.\n", __FILE__, __LINE__);
  }
#endif
}

#ifdef EMSCRIPTEN

static int icon_addr= 0;
static int icon_len= 0;

uae_u8 *strbuf(uae_u8 *haystack, int len, uae_u8 *key) {	// file might contain 0 so string funcs are useless
	int klen = strlen((const char*)key);

	for (int i= 0; i<(len-klen); i++) {
		for (int j= 0; j<klen; j++) {
			if (haystack[i+j] != key[j]) break;

			if (j == klen-1) {
				// matched complete key
				return haystack + i + strlen(key) + 1;	// skip "=" to point directly to the value
			}
		}
	}
	return 0;
}

// we could use uade_song2 struct instead... but for the JavaScipt side
// handling the below is simpler:
#define MAX_INFO_TXT 8192
char info_text[MAX_INFO_TXT +1];	// unfortunately there is no well structured info..

char inf_mins[10];
char inf_maxs[10];
char inf_curs[10];

unsigned int song_mins, song_maxs, song_curs;	// for easier handling on adapter.c side

char *song_info[4] = {info_text,inf_mins,inf_maxs,inf_curs};
char _meta_data_ready= 0;
#endif


void uade_get_amiga_message(void)
{
	// XXX caution: hardcoded default location for messages (see score.s) is $200

  uae_u8 *ptr;
  uae_u8 *nameptr;
  FILE *file;
  int x;
  unsigned int mins, maxs, curs;
  int status;
  int src, dst, off, len;
  char tmpstr[256];
  char *srcstr, *dststr;
#ifdef EMSCRIPTEN
  struct AFILE *uo;
#endif
  uint32_t *u32ptr;
  uint8_t space[256];
  struct uade_msg *um = (struct uade_msg *) space;

  x = uade_get_u32(SCORE_INPUT_MSG);  /* message type from amiga */

  switch (x) {

  case AMIGAMSG_SONG_END:
    uade_song_end("player", 0);
    break;

  case AMIGAMSG_SUBSINFO:
    mins = uade_get_u32(SCORE_MIN_SUBSONG);
    maxs = uade_get_u32(SCORE_MAX_SUBSONG);
    curs = uade_get_u32(SCORE_CUR_SUBSONG);
    /* Brain damage in TFMX BC Kid Despair */
    if (maxs < mins) {
      uade_send_debug("Odd subsongs. Eagleplayer reported (min, cur, max) == (%u, %u, %u)", mins, curs, maxs);
      maxs = mins;
    }
    /* Brain damage in Bubble bobble custom */
    if (curs > maxs) {
      uade_send_debug("Odd subsongs. Eagleplayer reported (min, cur, max) == (%u, %u, %u)", mins, curs, maxs);
      maxs = curs;
    }

#ifdef EMSCRIPTEN
	song_mins= mins;
	song_maxs= maxs;
	song_curs= curs;
	snprintf(inf_mins, sizeof inf_mins, "%d", mins);
	snprintf(inf_maxs, sizeof inf_maxs, "%d", maxs);
	snprintf(inf_curs, sizeof inf_curs, "%d", curs);

//fprintf(stdout, "amiga song info: %d %d %d\n", mins, curs, maxs);

	// easier to use JavaScript to parse that info_text crap
	// CAUTION: the EMSCRIPTEN default String handling will mess up any non-ASCII encoded data
	ems_notify_song_update(info_text, inf_mins, inf_maxs, inf_curs);
	_meta_data_ready= 1;
#else
    um->msgtype = UADE_REPLY_SUBSONG_INFO;
    um->size = 12;
    u32ptr = (uint32_t *) um->data;
    u32ptr[0] = htonl(mins);
    u32ptr[1] = htonl(maxs);
    u32ptr[2] = htonl(curs);
    if (uade_send_message(um, &uadeipc)) {
      fprintf(stdout, "uadecore: Could not send subsong info message.\n");
      exit(-1);
    }
#endif
    break;

  case AMIGAMSG_PLAYERNAME:
//    strlcpy(tmpstr, (char *) get_real_address(0x204), sizeof tmpstr);
//fprintf(stdout, "amiga playername: %s\n", tmpstr);
#ifndef EMSCRIPTEN
    uade_send_string(UADE_REPLY_PLAYERNAME, tmpstr, &uadeipc);
#else
    strlcpy(tmpstr, (char *) get_real_address(0x204), sizeof tmpstr);
	ems_notify_player_update(tmpstr);
#endif
    break;

  case AMIGAMSG_MODULENAME:
//    strlcpy(tmpstr, (char *) get_real_address(0x204), sizeof tmpstr);
//fprintf(stdout, "amiga modulename: %s\n", tmpstr);
#ifndef EMSCRIPTEN
    uade_send_string(UADE_REPLY_MODULENAME, tmpstr, &uadeipc);
#endif
    break;

  case AMIGAMSG_FORMATNAME:
//    strlcpy(tmpstr, (char *) get_real_address(0x204), sizeof tmpstr);
//fprintf(stdout, "amiga formatname: %s\n", tmpstr);
#ifndef EMSCRIPTEN
    uade_send_string(UADE_REPLY_FORMATNAME, tmpstr, &uadeipc);
#else
    strlcpy(tmpstr, (char *) get_real_address(0x204), sizeof tmpstr);
	ems_notify_format_update(tmpstr);
#endif
    break;

  case AMIGAMSG_GENERALMSG:
// XXX
	uade_send_debug((char *) get_real_address(0x204 + 0x400));
//    uade_send_debug((char *) get_real_address(0x204 ));	// hack: use separate buffer to avoid interference
    break;

  case AMIGAMSG_CHECKERROR:
    uade_song_end("module check failed", 1);
    break;

  case AMIGAMSG_SCORECRASH:
    if (uade_debug) {
      fprintf(stdout, "uadecore: Score crashed.\n");
#ifndef EMSCRIPTEN
      activate_debugger();
#endif
      break;
    }
    uade_song_end("score crashed", 1);
    break;

  case AMIGAMSG_SCOREDEAD:
     if (uade_debug) {
      fprintf(stdout, "uadecore: Score is dead.\n");
#ifndef EMSCRIPTEN
      activate_debugger();
#endif
      break;
    }
     uade_song_end("score died", 1);
    break;

#ifdef EMSCRIPTEN
	/*
	Reminder: Heikki meanwhile migrated the below "icon.library" functionality directly into
	the uade HEAD version of score.s - in order to eventually use that impl here.. the below meta-data
	extraction for PokeyNoise songs would need to be moved to the regular AMIGAMSG_LOADFILE.
	*/
  case AMIGAMSG_ICON_TOOLTYPE: {
		// get key at 0x204 (key pointer) and write result to address pointed by 0x208
		src = uade_get_u32(0x204);
		if (!uade_valid_string(src)) {
			fprintf(stdout, "uadecore: Icon key in invalid address range.\n");
			break;
		}

		uae_u8 *key = get_real_address(src);
		uae_u8 *haystack = get_real_address(icon_addr);
		uae_u8 *found= strbuf(haystack, icon_len, key);

		unsigned int src_result= 0;
		if (found) {
			int offset = ((unsigned int)found - (unsigned int)haystack);
			src_result = icon_addr + offset;
		}
		uade_put_long(0x208, src_result);

		break;
  }
  case AMIGAMSG_ICON_LOAD: {
		// poor man's impl used for "PokeyNoise"
		// load a file named at 0x204 (name pointer) to address pointed by 0x208 and write the length to 0x20C
		src = uade_get_u32(0x204);
		if (!uade_valid_string(src)) {
			fprintf(stdout, "uadecore: Load icon name in invalid address range.\n");
			break;
		}
		nameptr = get_real_address(src);

		char iconfile[PATH_MAX];
		snprintf(iconfile, PATH_MAX, "%s.info", nameptr);

		uo= uade_open_amiga_file((char *) iconfile, uade_player_dir);
		if ((file = uo->file)) {
			icon_addr = uade_get_u32(0x208);	// hack: one icon should be enough
			icon_len = uade_safe_load(icon_addr, file, uade_highmem - icon_addr);

			fclose(file); file = NULL;
			uade_put_long(0x20C, icon_len);
//			uade_send_debug("load icon success: %s ptr 0x%x size 0x%x", iconfile, icon_addr, icon_len);

			// get some meta info (the PokeyNoise player itself doesn't look for these)
			uae_u8 *haystack = get_real_address(icon_addr);
			uae_u8 *name= strbuf(haystack, icon_len, "NAME");
			uae_u8 *creator= strbuf(haystack, icon_len, "CREATOR");
			uae_u8 *copyright= strbuf(haystack, icon_len, "COPYRIGHT");	// will also match the "#COPYRIGHT" variation used by some songs

			// format already used by other crappy modules
			snprintf(info_text, sizeof info_text, "MODULENAME:\r%s\rAUTHORNAME:\r%s\rCREDITS:\r%s\r", name, creator, copyright);
		}
		break;
  }
#endif
  case AMIGAMSG_LOADFILE: {
   /* load a file named at 0x204 (name pointer) to address pointed by
       0x208 and insert the length to 0x20C */
    src = uade_get_u32(0x204);
    if (!uade_valid_string(src)) {
      fprintf(stdout, "uadecore: Load name in invalid address range.\n");
      break;
    }
    nameptr = get_real_address(src);
#ifndef EMSCRIPTEN
    if ((file = uade_open_amiga_file((char *) nameptr, uade_player_dir))) {
#else
	uo= uade_open_amiga_file((char *) nameptr, uade_player_dir);
    if ((file = uo->file)) {
#endif
      dst = uade_get_u32(0x208);
      len = uade_safe_load(dst, file, uade_highmem - dst);
      fclose(file); file = NULL;

	  // FIXME: all the below uses of "uade_put_long(0x2.., y);" lead to
	  // potentially fragile 68k ASM code on the "score.s" side: any debug output
	  // via "put_string" / "put_value" will overwrite the 0x2.. buffer and some
	  // added debug-print will potentially break the previously functioning
	  // logic FIXME: don't directly use input buffer for return values and/or
	  // use separate buffer for UADE_GENERALMSG

      uade_put_long(0x20C, len);	// this should be removed/replaced with the below


	  // put the length were it cannot be corrupted by UADE_GENERALMSG messages
	  uint32_t dstLen = uade_get_u32(0x210);
	  *((uint32_t*)get_real_address(dstLen)) = swap32(len);

      uade_send_debug("load success: %s ptr 0x%x size 0x%x", nameptr, dst, len);
    } else {
      uade_send_debug("load: file not found: %s", nameptr);
    }
    } break;

  case AMIGAMSG_READ:
    src = uade_get_u32(0x204);
    if (!uade_valid_string(src)) {
      fprintf(stdout, "uadecore: Read name in invalid address range.\n");
      break;
    }
    nameptr = get_real_address(src);
    dst = uade_get_u32(0x208);
    off = uade_get_u32(0x20C);
    len = uade_get_u32(0x210);
#ifndef EMSCRIPTEN
    if ((file = uade_open_amiga_file((char *) nameptr, uade_player_dir))) {
#else
	uo= uade_open_amiga_file((char *) nameptr, uade_player_dir);
    if ((file = uo->file)) {
#endif
      if (fseek(file, off, SEEK_SET)) {
	perror("can not fseek to position");
	x = 0;
      } else {
	x = uade_safe_load(dst, file, len);
	if (x > len)
	  x = len;
      }
      fclose(file);
//       uade_send_debug("read: %s dst 0x%x off 0x%x len 0x%x res 0x%x", nameptr, dst, off, len, x);
      uade_put_long(0x214, x);
    } else {
      uade_send_debug("read: file not found: %s", nameptr);
      uade_put_long(0x214, 0);
    }
    break;

// reminder: used by dos_open to "open an amiga file"; uade_open_amiga_file() handles the async scenario and triggers "abort"
// if necessary
  case AMIGAMSG_FILESIZE:
    src = uade_get_u32(0x204);
    if (!uade_valid_string(src)) {
      fprintf(stdout, "uadecore: Filesize name in invalid address range.\n");
      break;
    }
    nameptr = get_real_address(src);
#ifndef EMSCRIPTEN
    if ((file = uade_open_amiga_file((char *) nameptr, uade_player_dir))) {
#else
	uo= uade_open_amiga_file((char *) nameptr, uade_player_dir);
    if ((file = uo->file)) {
#endif
      fseek(file, 0, SEEK_END);
      len = ftell(file);
      fclose(file);
      uade_put_long(0x208, len);
      uade_put_long(0x20C, -1);
 //     uade_send_debug("filesize: name: %s size: 0x%x", nameptr, len);
    } else {
		// the "load pending" scenario also ends up here
      uade_put_long(0x208, 0);
      uade_put_long(0x20C, 0);
//      uade_send_debug("filesize: file not found: %s", nameptr);
    }
    break;

  case AMIGAMSG_TIME_CRITICAL:
    uade_time_critical = uade_get_u32(0x204) ? 1 : 0;
    if (uade_speed_hack < 0) {
      /* a negative value forbids use of speed hack */
      uade_time_critical = 0;
    }
    break;

  case AMIGAMSG_GET_INFO:
    src = uade_get_u32(0x204);
    dst = uade_get_u32(0x208);
    len = uade_get_u32(0x20C);
    if (!uade_valid_string(src)) {
      fprintf(stdout, "uadecore: get info: Invalid src: 0x%x\n", src);
      break;
    }
    if (len <= 0) {
      fprintf(stdout, "uadecore: get info: len = %d\n", len);
      break;
    }
    if (!valid_address(dst, len)) {
      fprintf(stdout, "uadecore: get info: Invalid dst: 0x%x\n", dst);
      break;
    }
    srcstr = (char *) get_real_address(src);
    dststr = (char *) get_real_address(dst);
    uade_send_debug("score issued an info request: %s (maxlen %d)", srcstr, len);
    len = get_info_for_ep(dststr, srcstr, len);
    /* Send printable debug */
    do {
      size_t i;
      size_t maxspace = sizeof space;
      if (len <= 0) {
	maxspace = 1;
      } else {
	if (len < maxspace)
	  maxspace = len;
      }
      for (i = 0; i < maxspace; i++) {
	space[i] = dststr[i];
	if (space[i] == 0)
	  space[i] = ' ';
      }
      if (i < maxspace) {
	space[i] = 0;
      } else {
	space[maxspace - 1] = 0;
      }
      uade_send_debug("reply to score: %s (total len %d)", space, len);
    } while (0);
    uade_put_long(0x20C, len);
    break;

  case AMIGAMSG_START_OUTPUT:
    uade_audio_output = 1;
    uade_send_debug("Starting audio output at %d", uade_audio_skip);
    break;

  case AMIGAMSG_AUDIO_DEV_BEGINIO:
	src = uade_get_u32(0x204);
	if (!uade_valid_string(src)) {
		fprintf(stdout, "uade: BeginIO - IOAudio* in invalid address range.\n");
		break;
	}
	dst = uade_get_u32(0x208);
	if (!uade_valid_string(dst)) {
		fprintf(stdout, "uade: BeginIO - IOAudio* in invalid dst address range.\n");
		break;
	}
	audiodevice_beginIO(swap32(src), swap32(dst));
	break;

  case AMIGAMSG_AUDIO_DEV_ABORTIO:
	src = uade_get_u32(0x204);
	if (!uade_valid_string(src)) {
		fprintf(stdout, "uade: AbortIO - IOAudio* in invalid address range.\n");
		break;
	}
	audiodevice_abortIO(swap32(src));
	break;

  case AMIGAMSG_AUDIO_DEV_OPEN:
	src = uade_get_u32(0x204);
	if (!uade_valid_string(src)) {
		fprintf(stdout, "uade: Open - IOAudio* in invalid address range.\n");
		break;
	}
	audiodevice_open(swap32(src));
	break;
  default:
    fprintf(stdout,"uadecore: Unknown message from score (%d)\n", x);
    break;
  }
}

void uade_handle_r_state(void)
{
#ifndef EMSCRIPTEN
  uint8_t space[UADE_MAX_MESSAGE_SIZE];
  struct uade_msg *um = (struct uade_msg *) space;
  int ret;
  uint32_t x, y;

  while (1) {

    ret = uade_receive_message(um, sizeof(space), &uadeipc);
    if (ret == 0) {
      fprintf(stdout, "uadecore: No more input. Exiting successfully.\n");
      exit(0);
    } else if (ret < 0) {
      fprintf(stdout, "uadecore: Error on input. Exiting with error.\n");
      exit(-1);
    }

    if (um->msgtype == UADE_COMMAND_TOKEN)
      break;

    switch (um->msgtype) {

    case UADE_COMMAND_ACTIVATE_DEBUGGER:
      fprintf(stdout, "uadecore: Received activate debugger message.\n");
      activate_debugger();
      uade_debug = 1;
      break;

    case UADE_COMMAND_CHANGE_SUBSONG:
      if (uade_parse_u32_message(&x, um)) {
	fprintf(stdout, "uadecore: Invalid size with change subsong.\n");
	exit(-1);
      }
      change_subsong(x);
      break;

    case UADE_COMMAND_FILTER:
      if (uade_parse_two_u32s_message(&x, &y, um)) {
	fprintf(stdout, "uadecore: Invalid size with filter command\n");
	exit(-1);
      }
      audio_set_filter(x, y);
      break;

    case UADE_COMMAND_IGNORE_CHECK:
      /* override bit for sound format checking */
      uade_put_long(SCORE_FORCE, 1);
      break;

    case UADE_COMMAND_SET_FREQUENCY:
      if (uade_parse_u32_message(&x, um)) {
	fprintf(stdout, "Invalid frequency message size: %u\n", um->size);
	exit(-1);
      }
      set_sound_freq(x);
      break;

    case UADE_COMMAND_SET_PLAYER_OPTION:
      uade_check_fix_string(um, 256);
      add_ep_option((char *) um->data);
      break;

    case UADE_COMMAND_SET_RESAMPLING_MODE:
      uade_check_fix_string(um, 16);
      audio_set_resampler((char *) um->data);
      break;

    case UADE_COMMAND_SPEED_HACK:
      uade_time_critical = 1;
      break;

    case UADE_COMMAND_READ:
      if (uade_read_size != 0) {
	fprintf(stdout, "uadecore: Read not allowed when uade_read_size > 0.\n");
	exit(-1);
      }
      if (uade_parse_u32_message(&x, um)) {
	fprintf(stdout, "uadecore: Invalid size on read command.\n");
	exit(-1);
      }
      if (x == 0 || x > MAX_SOUND_BUF_SIZE || (x & 3) != 0) {	// EMSCRIPTEN cleanup
	fprintf(stdout, "uadecore: Invalid read size: %u\n", x);
	exit(-1);
      }
      uade_read_size = x;
      break;

    case UADE_COMMAND_REBOOT:
      uade_reboot = 1;
      break;

    case UADE_COMMAND_SET_NTSC:
      fprintf(stdout, "\nuadecore: Changing to NTSC mode.\n");
      uade_set_ntsc(1);
      break;

    case UADE_COMMAND_SONG_END_NOT_POSSIBLE:
      uade_set_automatic_song_end(0);
      break;

    case UADE_COMMAND_SET_SUBSONG:
      if (uade_parse_u32_message(&x, um)) {
	fprintf(stdout, "uadecore: Invalid size on set subsong command.\n");
	exit(-1);
      }
      uade_put_long(SCORE_SET_SUBSONG, 1);
      uade_put_long(SCORE_SUBSONG, x);
      break;

    case UADE_COMMAND_USE_TEXT_SCOPE:
      audio_use_text_scope();
      break;

    default:
      fprintf(stdout, "uadecore: Received invalid command %d\n", um->msgtype);
      exit(-1);
    }
  }
#endif
}



#ifndef EMSCRIPTEN
void uade_option(int argc, char **argv)
#else
int uade_option(const char *uade_base_path)
#endif
{
#ifndef EMSCRIPTEN
  int i, j, no_more_opts;
  char **s_argv;
  int s_argc;
#endif
  int cfg_loaded = 0;
  char optionsfile[PATH_MAX];
#ifndef EMSCRIPTEN
  int ret;
  char *input = NULL;
  char *output = NULL;
#endif
  /* network byte order is the big endian order */
  uade_big_endian = (htonl(0x1234) == 0x1234);

  memset(&song, 0, sizeof(song));
#ifdef EMSCRIPTEN
  snprintf(optionsfile, PATH_MAX, "%s/uaerc", uade_base_path);


  FILE* f = uade_fopen(optionsfile, "r")->file;
  if (f == 0) return -1;	// this file must exist in a consistent installation

  if (cfgfile_load_file (&currprefs, f) == 0) {
    fprintf(stdout, "uadecore: Could not load uaerc (%s).\n", optionsfile);
    exit(-1);
  }
#else
  no_more_opts = 0;

  s_argv = malloc(sizeof(argv[0]) * (argc + 1));
  if (!s_argv) {
    fprintf (stdout, "uadecore: Out of memory for command line parsing.\n");
    exit(-1);
  }
  s_argc = 0;
  s_argv[s_argc++] = argv[0];

  for (i = 1; i < argc;) {

    j = i;

    /* if argv[i] begins with '-', see if it is a switch that we should
       handle here. */

    if (argv[i][0] == '-') {

      if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h") || !strcmp(argv[i], "-help")) {
	uade_print_help(OPTION_HELP, argv[0]);
	exit(0);

      } else if (!strcmp(argv[i], "-i")) {
	if ((i + 1) >= argc) {
	  fprintf(stdout, "uadecore: %s parameter missing\n", argv[i]);
	  uade_print_help(OPTION_ILLEGAL_PARAMETERS, argv[0]);
	  exit(-1);
	}
	input = argv[i + 1];
	i += 2;

      } else if (!strcmp(argv[i], "-o")) {
	if ((i + 1) >= argc) {
	  fprintf(stdout, "uadecore: %s parameter missing\n", argv[i]);
	  uade_print_help(OPTION_ILLEGAL_PARAMETERS, argv[0]);
	  exit(-1);
	}
	output = argv[i + 1];
	i += 2;

      } else if (!strcmp(argv[i], "--")) {
	for (i = i + 1; i < argc ; i++)
	  s_argv[s_argc++] = argv[i];
	break;
      }
    }

    if (i == j) {
      s_argv[s_argc++] = argv[i];
      i++;
    }
  }
  s_argv[s_argc] = NULL;

  uade_set_peer(&uadeipc, 0, input, output);

  ret = uade_receive_string(optionsfile, UADE_COMMAND_CONFIG, sizeof(optionsfile), &uadeipc);
  if (ret == 0) {
    fprintf(stdout, "uadecore: No config file passed as a message.\n");
    exit(-1);
  } else if (ret < 0) {
    fprintf(stdout, "uadecore: Invalid input. Expected a config file.\n");
    exit(-1);
  }

  /* use the config file provided with a message, if '-config' option
     was not given */
  if (!cfg_loaded) {
    if (cfgfile_load (&currprefs, optionsfile) == 0) {
      fprintf(stdout, "uadecore: Could not load uaerc (%s).\n", optionsfile);
      exit(-1);
    }
  }

  free(s_argv);
#endif
  uade_portable_initializations();

#ifndef EMSCRIPTEN
  uade_reboot = 1;
#endif
  return 0;
}


static void uade_print_help(enum print_help problemcode, char *progname)
{
  switch (problemcode) {
  case OPTION_HELP:
    /* just for printing help */
    break;
  case OPTION_ILLEGAL_PARAMETERS:
    fprintf(stdout, "uadecore: Invalid parameters.\n\n");
    break;
  case OPTION_NO_SONGS:
    fprintf(stdout, "uadecore: No songs given as parameters.\n\n");
    break;
  default:
    fprintf(stdout, "uadecore: Unknown error.\n");
    break;
  }
  fprintf(stdout, "UADE usage:\n");
  fprintf(stdout, " %s [OPTIONS]\n\n", progname);

  fprintf(stdout, " options:\n");
  fprintf(stdout, " -h\t\tPrint help\n");
  fprintf(stdout, " -i file\tSet input source ('filename' or 'fd://number')\n");
  fprintf(stdout, " -o file\tSet output destination ('filename' or 'fd://number'\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "This tool should not be run from the command line. This is for internal use\n");
  fprintf(stdout, "of other programs.\n");
}

#ifdef EMSCRIPTEN
int alloc_dummy_song(struct uade_state *state, const char *filename)
{
	struct uade_song *us;
	struct uade_content *content;

	if (state->song)	free(state->song);
	state->song = NULL;

	us = calloc(1, sizeof *us);
	if (us == NULL)
		goto error;

	strlcpy(us->module_filename, filename, sizeof us->module_filename);

	us->playtime = -1;
	us->min_subsong = us->max_subsong = us->cur_subsong = 0;	// will be reset in uade_reset()!

	state->song = us;
	return 1;

      error:
	if (us != NULL) {
		free(us);
	}
	return 0;
}

char abspath[PATH_MAX];

char *getAbsPath(const char*dir, const char*file) {
	snprintf(abspath, PATH_MAX, "%s/%s", dir, file);
	return abspath;
}

void emsCopyPath(char *dest, int maxsize, char*src) {
	// with the special chars that may be used in song file URLs
	// the 'snprintf' used earlier no longer works..

	int srcLen= strlen(src);
	srcLen= srcLen > maxsize ? maxsize : srcLen;
	strncpy(dest, src, srcLen); dest[srcLen]= 0 ;
}

#ifdef EMSCRIPTEN

void uade_set_panning(float val) {
	struct uade_state *state= &g_state;

	if (state) {
		struct uade_effect *effects = &state->effects;
		if (effects) {
			uade_effect_pan_set_amount(effects, val);
			uade_effect_enable(effects, UADE_EFFECT_PAN);
		}
	}
}

void uade_apply_effects(int16_t * samples, int frames) {
	struct uade_state *state= &g_state;
	if (state && &state->effects) {
		uade_effect_run(&state->effects, samples, frames);
	}
}
#endif


// @return -1=need async load; 0= ok; 1= not ok
static int get_player_name(const char *dir, char *modulename, char *playername) {
	emsCopyPath(info_text, MAX_INFO_TXT, modulename);

//	snprintf(info_text, MAX_INFO_TXT, "%s", modulename);	// doesn't work for special char paths..

	memset(&g_state, 0, sizeof g_state);	// arrgh..ugly side-effect

#ifndef EMSCRIPTEN
	uade_config_set_defaults(&g_state.config);	// is this really needed and does it actually work?
#else
	char *tmp= malloc(512);
	snprintf(tmp, 512, "%s/uade.conf", dir);

	int status= uade_load_config(&g_state.config, tmp);
	free(tmp);
	if (status < 0) {	// handle errors
//		fprintf(stdout, "some file is not ready yet: %s\n", modulename);
		return -1;
	} else {
	}
#endif
	snprintf(g_state.config.basedir.name, sizeof g_state.config.basedir.name, "%s", dir);
//	g_state.config.verbose= 1;
//	uade_set_silence_timeout(&g_state.config, "20");	// use setting from uade.conf

	g_state.song = NULL;
	g_state.ep = NULL;

	alloc_dummy_song(&g_state, modulename);

	// note: only place where the EaglePlayer "is our file" function is used...
	status= uade_is_our_file(modulename, 0, &g_state);	// 1= ok; 0= not; -1=async load
	if (status < 1) {	// handle errors
		if (status <0) {
			return -1;
		} else {
			fprintf(stdout, "Unknown format: %s\n",  strrchr(modulename,'/') + 1);
			return 1;
		}
	}

	if (strcmp(g_state.ep->playername, "custom") == 0) {	// build-in custom player
		strlcpy(playername, modulename, PATH_MAX);
		modulename[0] = 0;
	} else {											// player that needs to be loaded separately
		snprintf(playername, PATH_MAX, "players/%s",
					g_state.ep->playername);
	}
	if (strlen(playername) == 0) {
		fprintf(stdout, "Error: an empty player name given\n");
		return 1;
	}

	// who would be interested in the result?
	if (g_state.ep != NULL)
      uade_set_ep_attributes(&g_state);	// this actually set some "type:st20" for silkworm!
    uade_set_effects(&g_state);


	if (uade_song_info(info_text, sizeof info_text, info_text, UADE_MODULE_INFO) == 0) {	// UGLY SHIT using info_text as I/O param...
//		fprintf(stdout, "info: [%s]\n", info_text);
#ifdef EMSCRIPTEN
	} else {
		info_text[0]= 0; // make sure there is no text

#endif
	}
	return 0;
}

// migrated from uadecontrol.c

static void uade_filter_command(struct uade_state *state)	// see uade_send_filter_command
{
	struct uade_config *uadeconf = &state->config;

	int filter_type = uadeconf->filter_type;
	int filter_state = uadeconf->led_state;
	int force_filter = uadeconf->led_forced;

	if (uadeconf->no_filter)
		filter_type = 0;

	filter_state = force_filter ? (2 + (filter_state & 1)) : 0;

    audio_set_filter(filter_type, filter_state);
}

static void uade_resampling_command(struct uade_config *uadeconf)	// see send_resampling_command
{
	char *mode = uadeconf->resampler;
	if (mode != NULL) {
		if (strlen(mode) == 0) {
			fprintf(stdout, "Resampling mode may not be empty.\n");
			exit(-1);
		}

		audio_set_resampler((char *) mode);
	}
}

static int uade_set_ep_options(struct uade_ep_options *eo)	// see send_ep_options
{
	if (eo && eo->o && (eo->s > 0)) {
		size_t i = 0;
		while (i < eo->s) {
			char *s = &eo->o[i];
			size_t l = strlen(s) + 1;
			assert((i + l) <= eo->s);

			add_ep_option((char *) s);
			i += l;
		}
	}
	return 0;
}

// below is sequence from uade_song_initialization (without initial IPC handshakes
// with uade_reset)
static void ems_song_initialization(struct uade_state *state) {
	struct uade_config *uc = &state->config;
	struct uade_song *us = state->song;

	us->silence_count = 0;
	us->is_playing = 0;

	uade_filter_command(state);

	uade_resampling_command(uc);

	if (uc->speed_hack) {
	    uade_time_critical = 1;
	}

	if (uc->use_ntsc) {
      uade_set_ntsc(1);
	}

	if (uc->frequency != UADE_DEFAULT_FREQUENCY) {
	      set_sound_freq(uc->frequency);
	}

	if (uc->use_text_scope) {
      audio_use_text_scope();
	}

	//  this seems to be actually needed for mod-file playback
	if (uade_set_ep_options(&us->ep_options) ||	uade_set_ep_options(&uc->ep_options)) {}
}
#endif

static int uade_safe_load_name(int vaddr, char *name, const char *expl,
			       int maxlen)
{
  int bytesread;
  FILE *file;
#ifndef EMSCRIPTEN
  file = fopen(name, "rb");
  if (!file) {
    return 0;
  }
#else
  struct AFILE *uo = uade_fopen(name, "rb");
  file= uo->file;

  if (!file) {
	if (uo->async_status <0)
		return -1;
    return 0;			// we are not loading empty files .. so we can use this to signal error
  }
#endif

  bytesread = uade_safe_load(vaddr, file, maxlen);
  fclose(file);
  return bytesread;
}

#ifndef EMSCRIPTEN
/* this is called for each played song from newcpu.c/m68k_reset() */
void uade_reset(void)
{
#else

static const char* _uade_base_path;

int uade_boot(const char *uade_base_path) {
	_uade_base_path = uade_base_path; // memory managed in Adapter.cpp

	// purpose of dry_run is to retrieve the meta info (before track selection is started)
	song_mins= song_maxs= song_curs= _meta_data_ready= 0;

	// initial start sequence (base for uade_reset)
	uade_reboot = 1;
	quit_program = 0;

	if (uade_main(uade_base_path) < 0) return -1;
	/*
	* until here the setup should correspond to what m68k_go did before loop - one loop would then
	start with the "reboot" sequence (all of which not is included in uade_reset):
	    uade_reset ();
		m68k_reset ();
		customreset ();
	and afterwards the emu ran..
	*/
	snprintf(uade_player_dir, sizeof uade_player_dir, "%s", uade_base_path);

	uade_read_size= SNDBUF_READ_SIZE;	// "request" audio data
	return 0;
}

#define MAX_LAST_LEN 256
char last_basedir[MAX_LAST_LEN +1];
char last_songmodule[MAX_LAST_LEN +1];

int uade_reset(int sample_rate, char *basedir, char *songmodule, char dry_run)
{
	// ease reuse for "track switch"
	if (basedir)
		emsCopyPath(last_basedir, MAX_LAST_LEN, basedir);
	else
		basedir= last_basedir;

	if (songmodule)
		emsCopyPath(last_songmodule, MAX_LAST_LEN, songmodule);
	else
		songmodule= last_songmodule;

#endif

  /* IMPORTANT:
     It seems that certain players don't work totally reliably if memory
     contains trash from previous songs. To be certain that each song is
     played from the same initial state of emulator we clear the memory
     from 0x400 to 'uade_highmem' each time a new song is played */
  uade_highmem = 0;
  while (uade_highmem < 0x800000) {
    if (!valid_address(0, uade_highmem + 0x10000))
      break;
    uade_highmem += 0x10000;
  }
  if (uade_highmem < 0x80000) {
    fprintf(stdout, "uadecore: There must be at least 512 KiB of amiga memory (%d bytes found).\n", uade_highmem);
    exit(-1);
  }
  if (uade_highmem < 0x200000) {
    fprintf(stdout, "uadecore: Warning: highmem == 0x%x (< 0x200000)!\n", uade_highmem);
  }
  memset(get_real_address(0), 0, uade_highmem);


  /* don't load anything under 0x1000 (execbase top at $1000)
  XXX actually execbase top ends at $0f7a! see SYSBASESIZE */

  const int modnameaddr = 0x00400;
  const int scoreaddr   = 0x01000;

  FILE *file;
  int bytesread,bytesreads,bytesreadp;

  /* load sound core (score) */
#ifndef EMSCRIPTEN
  if ((file = fopen(song.scorename, "rb"))) {
#else
  snprintf(song.scorename, sizeof song.scorename, "%s/system/score", _uade_base_path);

  if ((file = uade_fopen(song.scorename, "rb")->file)) {
#endif
    bytesread = uade_safe_load(scoreaddr, file, uade_highmem - scoreaddr);
    fclose(file);
  } else {
#ifdef EMSCRIPTEN
	return -1;	// load still pending
#else
    fprintf (stdout, "uadecore: Can not load score (%s).\n", song.scorename);
    goto skiptonextsong;
#endif
  }

  const  int userstack_size		= 0x1000;		// original
  const  int superstack_size	= 0x0a00;
  const  int player_offset		= 0x0100;	// XXX makes no sense but reflects original UADE impl

  // replaced the fragile hardcoded garbage by using the actually loaded score file size
  const int userstack   = scoreaddr + bytesread + userstack_size;
  const int superstack  = userstack + superstack_size;
  const int playeraddr  = superstack + player_offset;
  bytesreads = bytesread;

  int relocaddr;
  int modaddr;
  int len;
#ifndef EMSCRIPTEN
  uint8_t command[UADE_MAX_MESSAGE_SIZE];
  struct uade_msg *um = (struct uade_msg *) command;

  int ret;

 nextsong:
#endif

  song.cur_subsong = song.min_subsong = song.max_subsong = 0;

#ifdef EMSCRIPTEN
	snprintf(song.modulename, sizeof song.modulename, "%s/%s", basedir, songmodule);
	snprintf(song.playername, sizeof song.playername, "%s", "");

	// resolve the needed player (for "custom" modules the module becomes the player)

	int stat= get_player_name(_uade_base_path , song.modulename, song.playername);
	if (stat != 0) {
		return stat;
	}
	struct uade_state* state= &g_state;	// setup in above get_player_name
#else
  ret = uade_receive_string(song.scorename, UADE_COMMAND_SCORE, sizeof(song.scorename), &uadeipc);
  if (ret == 0) {
    fprintf(stdout, "uadecore: No more songs to play.\n");
    exit(0);
  } else if (ret < 0) {
    fprintf(stdout, "uadecore: Invalid input. Expected score name.\n");
    exit(-1);
  }

  ret = uade_receive_string(song.playername, UADE_COMMAND_PLAYER, sizeof(song.playername), &uadeipc);
  if (ret == 0) {
    fprintf(stdout, "uadecore: Expected player name. Got nothing.\n");
    exit(-1);
  } else if (ret < 0) {
    fprintf(stdout, "uadecore: Invalid input. Expected player name.\n");
    exit(-1);
  }

  if (uade_dirname(uade_player_dir, song.playername, sizeof(uade_player_dir)) == NULL) {
    fprintf(stdout, "uadecore: Invalid dirname with player: %s\n", song.playername);
    exit(-1);
  }

  ret = uade_receive_message(um, sizeof command, &uadeipc);
  if (ret == 0) {
    fprintf(stdout,"uadecore: Expected module name. Got nothing.\n");
    exit(-1);
  } else if (ret < 0) {
    fprintf(stdout, "uadecore: Invalid input. Expected module name.\n");
    exit(-1);
  }
  assert(um->msgtype == UADE_COMMAND_MODULE);
  if (um->size == 0) {
    song.modulename[0] = 0;
  } else {
    assert(um->size == (strlen((char *) um->data) + 1));
    strlcpy(song.modulename, (char *) um->data, sizeof(song.modulename));
  }
#endif
  uade_set_automatic_song_end(1);

  uade_put_long(SCORE_EXEC_DEBUG, uade_execdebugboolean ? 0x12345678 : 0);
  uade_put_long(SCORE_VOLUME_TEST, voltestboolean);
  uade_put_long(SCORE_DMA_WAIT, uade_dmawait);
  uade_put_long(SCORE_MODULECHANGE, disable_modulechange);


// EMSCRIPTEN why does original UADE impl have to use KM long functions without breaking the
// impl down into more managable logical chunks...? this is so annoying!


#ifndef EMSCRIPTEN
  bytesread = uade_safe_load_name(playeraddr, song.playername, "player", uade_highmem - playeraddr);
#else
	bytesread = uade_safe_load_name(playeraddr, song.modulename[0]?getAbsPath(_uade_base_path, song.playername):song.playername, "player", uade_highmem - playeraddr);
	if (bytesread <1) {
		return (bytesread<0)?-1:1; // async load pending or error
	}
#endif
  if (bytesread > (uade_highmem - playeraddr)) {
    fprintf (stdout, "uadecore: Player %s too big a file (%d bytes).\n", song.playername, bytesread);
    goto skiptonextsong;
  }
  if (bytesread == 0) {
    goto skiptonextsong;
  }

  /* fprintf(stdout, "uadecore: player '%s' (%d bytes)\n", song.playername, bytesread); */

  /* set player executable address for relocator */
  uade_put_long(SCORE_PLAYER_ADDR, playeraddr);
  len = uade_calc_reloc_size((uae_u32 *) get_real_address(playeraddr),
			     (uae_u32 *) get_real_address(playeraddr + bytesread));
  if (!len) {
    fprintf(stdout, "uadecore: Problem with reloc calculation.\n");
    goto skiptonextsong;
  }
  bytesreadp=len;

// XXX fixme: use regular LoadSeg to avoid duplicated code, and avoid potential
// flaws of UADE's old hunk loading/relocator impl. testcase: StoneTracker - player
// has separate BSS segment and it is unclear if UADE's old impl actually handles this
// correctly

  relocaddr  = ((playeraddr + bytesread) & 0x7FFFF000) + 0x4000;
  /* + 0x4000 for hippel coso (wasseremu) */
  modaddr = ((relocaddr + len) & 0x7FFFF000) + 0x2000;

  if (modaddr <= relocaddr) {
    /* this is very bad because sound core memory allocation will fail */
    fprintf(stdout, "uadecore: Warning: modaddr <= relocaddr: 0x%x <= 0x%x\n", modaddr, relocaddr);
  }

  uade_put_long(SCORE_RELOC_ADDR, relocaddr);  /*address for relocated player*/
  uade_put_long(SCORE_MODULE_ADDR, modaddr);   /* set module address */
  uade_put_long(SCORE_MODULE_LEN, 0);          /* set module size to zero */
  uade_put_long(SCORE_MODULE_NAME_ADDR, 0);    /* mod name address pointer */

  /* load the module if available */
  if (song.modulename[0]) {
    bytesread = uade_safe_load_name(modaddr, song.modulename, "module", uade_highmem - modaddr);
#ifdef EMSCRIPTEN
	if (bytesread <1) {
		return (bytesread<0)?-1:1; // async load pending or error
	}
#endif
    if (bytesread > (uade_highmem - playeraddr)) {
      fprintf (stdout, "uadecore: Module %s too big a file (%d bytes).\n", song.modulename, bytesread);
      goto skiptonextsong;
    }
    if (bytesread == 0) {
      goto skiptonextsong;
    }

    uade_put_long(SCORE_MODULE_LEN, bytesread);

    if (!valid_address(modnameaddr, strlen(song.modulename) + 1)) {
      fprintf(stdout, "uadecore: Invalid address for modulename.\n");
      goto skiptonextsong;
    }

    strlcpy((char *) get_real_address(modnameaddr), song.modulename, 1024);
    uade_put_long(SCORE_MODULE_NAME_ADDR, modnameaddr);

  } else {

    if (!valid_address(modnameaddr, strlen(song.playername) + 1)) {
      fprintf(stdout, "uadecore: Invalid address for playername.\n");
      goto skiptonextsong;
    }

    strlcpy((char *) get_real_address(modnameaddr), song.playername, 1024);
    uade_put_long(SCORE_MODULE_NAME_ADDR, modnameaddr);

    bytesread = 0;
  }

/*
    fprintf(stdout, "--- memory layout: ---\n	score: %x - %x\n	userstack: %x - %x\n	superstack: %x - %x\n	player: %x - %x\n	module: %x - %x\n",
		scoreaddr,scoreaddr+bytesreads,
		userstack-userstack_size,userstack,
		superstack-superstack_size,superstack,
		playeraddr,playeraddr+bytesreadp,
		modaddr,modaddr+bytesread);
*/

  m68k_areg(regs,7) = scoreaddr;
  m68k_setpc(scoreaddr);

  /* obey player format checking */
  uade_put_long(SCORE_FORCE, 0);
  /* set default subsong */
  uade_put_long(SCORE_SET_SUBSONG, 0);
  uade_put_long(SCORE_SUBSONG, 0);
  /* set PAL mode */
  uade_set_ntsc(0);

  /* pause bits (don't care!), for debugging purposes only */
  uade_put_long(SCORE_PREPAUSE, 0);
  uade_put_long(SCORE_POSTPAUSE, 0);
  /* set user and supervisor stack pointers */
  uade_put_long(SCORE_USER_STACK, userstack);
  uade_put_long(SCORE_SUPER_STACK, superstack);
  /* no message for score */
  uade_put_long(SCORE_OUTPUT_MSG, 0);

//  no more need for this garbage: it is now allocated as needed..
//  if ((userstack - (scoreaddr + bytesread)) < 0x1000)
//    fprintf(stdout, "uadecore: Amiga stack overrun warning. (read: %d left: %d)\n", bytesread, userstack - (scoreaddr + bytesread));

  flush_sound();

  /* note that uade_speed_hack can be negative (meaning that uade never uses
     speed hack, even if it's requested by the amiga player)! */
  uade_time_critical = 0;
  if (uade_speed_hack > 0) {
    uade_time_critical = 1;
  }

#ifdef EMSCRIPTEN
  uade_audio_output = 0;	// amiga side reports when it is playing..
#else
  uade_reboot = 0;

  uade_audio_output = 0;
#endif
  uade_audio_skip = 0;

  old_ledstate = gui_ledstate;

#ifndef EMSCRIPTEN
  if (uade_receive_short_message(UADE_COMMAND_TOKEN, &uadeipc)) {
    fprintf(stdout, "uadecore: Can not receive token in uade_reset().\n");
    exit(-1);
  }

  if (uade_send_short_message(UADE_REPLY_CAN_PLAY, &uadeipc)) {
    fprintf(stdout, "uadecore: Can not send 'CAN_PLAY' reply.\n");
    exit(-1);
  }
  if (uade_send_short_message(UADE_COMMAND_TOKEN, &uadeipc)) {
    fprintf(stdout, "uadecore: Can not send token from uade_reset().\n");
    exit(-1);
  }
  set_sound_freq(UADE_DEFAULT_FREQUENCY);
#else
  set_sound_freq(sample_rate);
#endif
  epoptionsize = 0;

  audiodevice_reset();
#ifndef EMSCRIPTEN
  return;
#else
	// what uadecontrol does once the above handshake was successful
	ems_song_initialization(state);

	/*
	* together with the uade_reset the below 2 calls were what
	* performed a reboot in the old impl..
	*/
	m68k_reset ();
	customreset ();	// reset of the Amiga custom-chips

	uade_reboot = 0;

	if (state->config.verbose)
		fprintf(stdout, "started module [%s] using player [%s]\n", strrchr(song.modulename,'/'), song.playername);

	/*
	* when the code gets here then there may still be "player" code that has NOT
	* been loaded yet!	example: "Dyter-07/dyter07 title.osp"
	*/
	if (dry_run && !_meta_data_ready) {
		long limit= 10000000;	// 	make sure it terminates
		while(((quit_program|uade_reboot) == 0) && ((limit--) > 0)) {
			// problem: there seem to be songs that never report any meta data..
			m68k_run_1 ();		// run until song reports the meta info.. (change_subsong depends on it)
			get_new_samples();  // just ignore the output

			if ((quit_program != 0) && is_amiga_file_not_ready()) {
				quit_program= 0;	// this needs to be repeated when all the files are ready..
				return -1;
			}

			if (_meta_data_ready || uade_audio_output) break;
		}
		if(_meta_data_ready) {
			// fprintf(stdout, "track detect SUCCESS\n");
		} else {
			// fprintf(stdout, "track detect FAIL\n");
		}
	}
	return 0;
#endif
 skiptonextsong:
  fprintf(stdout, "uadecore: Can not play. Reboot.\n");

#ifndef EMSCRIPTEN
  if (uade_receive_short_message(UADE_COMMAND_TOKEN, &uadeipc)) {
    fprintf(stdout, "uadecore: Can not receive token in uade_reset().\n");
    exit(-1);
  }

  if (uade_send_short_message(UADE_REPLY_CANT_PLAY, &uadeipc)) {
    fprintf(stdout, "uadecore: Can not send 'CANT_PLAY' reply.\n");
    exit(-1);
  }
  if (uade_send_short_message(UADE_COMMAND_TOKEN, &uadeipc)) {
    fprintf(stdout, "uadecore: Can not send token from uade_reset().\n");
    exit(-1);
  }
  goto nextsong;
#else
  return 1;	// error
#endif
}


static void uade_put_long(int addr, int val)
{
  uae_u32 *p;
  if (!valid_address(addr, 4)) {
    fprintf(stdout, "uadecore: Invalid uade_put_long (0x%x).\n", addr);
    return;
  }
  p = (uae_u32 *) get_real_address(addr);

  *p = htonl(val);
}

/* XXX

// debugging utility: read a 32-bit value from the specified Amiga
// mem addr (equivalent to directly using a specified address
// in score.s code..)
uae_u32 dbg_get_u32(uaecptr addr) {
  uae_u32* ptr = (uae_u32 *) get_real_address(addr);
  return ntohl(*ptr);
}

// get the current Amiga PC
uaecptr m68k_getpc (void)
{
    return regs.pc + ((char *)regs.pc_p - (char *)regs.pc_oldp);
}

// get the current Amiga stack pointer
uaecptr m68k_SP() {
	return m68k_areg(regs, 7);
}
*/


static int uade_get_u32(int addr)
{
  uae_u32 *ptr;
  int x;
  if (!valid_address(addr, 4)) {
    fprintf(stdout, "uadecore: Invalid uade_get_u32 (0x%x).\n", addr);
    return 0;
  }
  ptr = (uae_u32 *) get_real_address(addr);
  return ntohl(*ptr);
}

static int uade_safe_load(int dst, FILE *file, int maxlen)
{
#define UADE_SAFE_BUFSIZE 4096

  char buf[UADE_SAFE_BUFSIZE];
  int nbytes, len, off;

  len = UADE_SAFE_BUFSIZE;
  off = 0;

  if (maxlen <= 0)
    return 0;

  while (maxlen > 0) {

    if (maxlen < UADE_SAFE_BUFSIZE)
      len = maxlen;

    nbytes = fread(buf, 1, len, file);
    if (!nbytes)
      break;

    if (!valid_address(dst + off, nbytes)) {
      fprintf(stdout, "uadecore: Invalid load range [%x,%x).\n", dst + off, dst + off + nbytes);
      break;
    }

    memcpy(get_real_address(dst + off), buf, nbytes);

    off += nbytes;
    maxlen -= nbytes;
  }

  /* find out how much would have been read even if maxlen was violated */
  while ((nbytes = fread(buf, 1, UADE_SAFE_BUFSIZE, file)))
    off += nbytes;

  return off;
}

static void uade_safe_get_string(char *dst, int src, int maxlen)
{
  int i = 0;
  while (1) {
    if (i >= maxlen)
      break;
    if (!valid_address(src + i, 1)) {
      fprintf(stdout, "uadecore: Invalid memory range in safe_get_string.\n");
      break;
    }
    dst[i] = * (char *) get_real_address(src + i);
    i++;
  }
  if (maxlen > 0) {
    if (i < maxlen) {
      dst[i] = 0;
    } else {
      fprintf(stdout, "uadecore: Warning: string truncated.\n");
      dst[maxlen - 1] = 0;
    }
  }
}


void uade_send_amiga_message(int msgtype)
{
  uade_put_long(SCORE_OUTPUT_MSG, msgtype);
}


void uade_set_ntsc(int usentsc)
{
  uade_put_long(SCORE_NTSC, usentsc);
}


void uade_set_automatic_song_end(int song_end_possible)
{
  uade_put_long(SCORE_HAVE_SONGEND, song_end_possible);
}

/* if kill_it is zero, uade may switch to next subsong. if kill_it is non-zero
   uade will always switch to next song (if any) */
void uade_song_end(char *reason, int kill_it)
{
#ifndef EMSCRIPTEN
  uint8_t space[sizeof(struct uade_msg) + 4 + 256];
  struct uade_msg *um = (struct uade_msg *) space;
  um->msgtype = UADE_REPLY_SONG_END;
  ((uint32_t *) um->data)[0] = htonl(((intptr_t) sndbufpt) - ((intptr_t) sndbuffer));
  ((uint32_t *) um->data)[1] = htonl(kill_it);
  strlcpy((char *) um->data + 8, reason, 256);
  um->size = 8 + strlen(reason) + 1;
  if (uade_send_message(um, &uadeipc)) {
    fprintf(stdout, "uadecore: Could not send song end message.\n");
    exit(-1);
  }
#else
  quit_program= 1;	// end and let client decide how he wants to proceed

  fprintf(stdout, "%s ended: %s\n", strrchr(song.modulename,'/'), reason);
#endif
  /* if audio_output is zero (and thus the client is waiting for the first
     sound data block from this song), then start audio output so that the
     clients first sound finishes ASAP and we can go to the next (sub)song.
     uade must finish the pending sound data request (for the client) even if
     the sound core crashed */
  uade_audio_output = 1;
}


void uade_swap_buffer_bytes(void *data, int bytes)
{
  uae_u8 *buf = (uae_u8 *) data;
  uae_u8 sample;
  int i;
  assert((bytes % 2) == 0);
  for (i = 0; i < bytes; i += 2) {
    sample = buf[i + 0];
    buf[i + 0] = buf[i + 1];
    buf[i + 1] = sample;
  }
}


/* check if string is on a safe zone */
static int uade_valid_string(uae_u32 address)
{
  while (valid_address(address, 1)) {
    if (* ((uae_u8 *) get_real_address(address)) == 0)
      return 1;
    address++;
  }
  fprintf(stdout, "uadecore: Invalid string at 0x%x.\n", address);
  return 0;
}
