 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Prototypes for main.c
  *
  * Copyright 1996 Bernd Schmidt
  */

#ifdef EMSCRIPTEN
extern void uade_teardown (void);
extern int uade_boot(char *basedir);
extern int uade_main (const char*);
extern int uade_reset (int,char*,char*,char);
extern int uade_compute_audio_samples();

extern char* uade_get_audio_buffer(void);
extern long uade_get_audio_buffer_length(void);
extern void	uade_request_next_buffer(void);

extern int uade_is_reboot(void);
extern int uade_is_exit(void);
extern void m68k_reset (void);
extern void customreset (void);
	
int uade_is_reboot(void);
int uade_is_exit(void);

extern int uade_reboot;
#else
extern int uade_main (int argc, char **argv);
#endif

extern void uae_quit (void);

extern int quit_program;

extern char warning_buffer[256];

/* This structure is used to define menus. The val field can hold key
 * shortcuts, or one of these special codes:
 *   -4: deleted entry, not displayed, not selectable, but does count in
 *       select value
 *   -3: end of table
 *   -2: line that is displayed, but not selectable
 *   -1: line that is selectable, but has no keyboard shortcut
 *    0: Menu title
 */
struct bstring {
    const char *data;
    int val;
};

extern char *colormodes[];
