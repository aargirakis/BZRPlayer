#ifndef _UADE_MAIN_H_
#define _UADE_MAIN_H_

#include <limits.h>
#include <stdlib.h>


#ifndef EMSCRIPTEN
#include "uadeipc.h"

struct uade_song {
  char playername[PATH_MAX];       /* filename of eagleplayer */
  char modulename[PATH_MAX];       /* filename of song */
  char scorename[PATH_MAX];        /* filename of score file */

  int min_subsong;
  int max_subsong;
  int cur_subsong;
};
#else
#include "uadestate.h"

struct uade_song2 {
  char playername[PATH_MAX];       // filename of eagleplayer 
  char modulename[PATH_MAX];       // filename of song 
  char scorename[PATH_MAX];        // filename of score file 

  int min_subsong;
  int max_subsong;
  int cur_subsong;
};

struct uade_sample_data {
  int alloclen; 
  int buflen; 
  char *buf; 
  
  // added for raw scope data
  char *bufChan0; 
  char *bufChan1; 
  char *bufChan2; 
  char *bufChan3; 
  
  int is_new; 
};


struct uade_sample_data sample_data;

struct uade_sample_data *get_new_samples(void);

#endif

void uade_change_subsong(int subs);
void uade_check_sound_buffers(int bytes);
void uade_send_debug(const char *fmt, ...);
void uade_get_amiga_message(void);
void uade_handle_r_state(void);
#ifdef EMSCRIPTEN
void set_subsong(int subsong);
void change_subsong(int subsong);
int uade_option(const char *basedir);
int uade_reset(int,char*,char*, char);
char** uade_get_song_info(void);
int alloc_dummy_song(struct uade_state *state, const char *filename);
int uade_set_silence_timeout(struct uade_config *uc, const char *value);
#else
void uade_option(int, char**); /* handles command line parameters */
void uade_reset(void);
#endif
void uade_send_amiga_message(int msgtype);
void uade_set_automatic_song_end(int song_end_possible);
void uade_set_ntsc(int usentsc);
void uade_song_end(char *reason, int kill_it);
void uade_swap_buffer_bytes(void *data, int bytes);

extern int uade_audio_output;
extern int uade_audio_skip;
extern int uade_debug;
extern int uade_local_sound;
extern int uade_read_size;
extern int uade_reboot;
extern int uade_time_critical;

#ifndef EMSCRIPTEN 
extern struct uade_ipc uadeipc;
#endif

#endif
