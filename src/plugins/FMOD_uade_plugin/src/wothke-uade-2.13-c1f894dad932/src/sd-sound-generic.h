/* 
 * UADE sound output
 * 
 * Copyright 1997 Bernd Schmidt
 * Copyright 2000-2005 Heikki Orsila <heikki.orsila@iki.fi>
 */

#include <assert.h>
#include <stdint.h>

#include <errno.h>
#include <string.h>

#include "uade.h"
#include "uadeconstants.h"

// actual sndbuffer size in bytes
#define MAX_SOUND_BUF_SIZE (65536)

extern uae_u16 sndbuffer[];
extern uae_u16 *sndbufpt;
#ifdef EMSCRIPTEN
extern uae_u16 chn0buffer[];
extern uae_u16 *chn0bufpt;
extern uae_u16 chn1buffer[];
extern uae_u16 *chn1bufpt;
extern uae_u16 chn2buffer[];
extern uae_u16 *chn2bufpt;
extern uae_u16 chn3buffer[];
extern uae_u16 *chn3bufpt;
#endif

//extern int sndbufsize; // EMSCRIPTEN cleanup
extern int sound_bytes_per_second;

extern void finish_sound_buffer (void);

//#define DEFAULT_SOUND_MAXB 8192	EMSCRIPTEN cleanup
//#define DEFAULT_SOUND_MINB 8192
#define DEFAULT_SOUND_BITS (8 * UADE_BYTES_PER_SAMPLE)
#define DEFAULT_SOUND_FREQ UADE_DEFAULT_FREQUENCY