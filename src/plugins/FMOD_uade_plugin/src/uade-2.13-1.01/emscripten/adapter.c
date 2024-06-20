/*
* This file adapts UADE to the interface expected by my generic JavaScript player..
*
* Copyright (C) 2014 Juergen Wothke
*
* LICENSE
* 
* This library is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2.1 of the License, or (at
* your option) any later version. This library is distributed in the hope
* that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include "uade.h"
#include "standalonesupport.h"

#include <emscripten.h>

#ifdef EMSCRIPTEN
#define EMSCRIPTEN_KEEPALIVE __attribute__((used))
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

extern int quit_program;
extern struct uade_sample_data sample_data;
extern char *song_info[4];

extern void uade_teardown (void);
extern void m68k_run_1 (void);


int emu_is_exit(void) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_is_exit(void) {
	return quit_program;
}

char* EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer(void) __attribute__((noinline));
char* EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer(void) {
	sample_data.is_new= 0;		// mark as consumed
	
	return sample_data.buf;
}

long EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer_length(void) __attribute__((noinline));
long EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer_length(void) {
	return sample_data.buflen;
}

// run emulation until we get a buffer full of audio samples
int emu_compute_audio_samples() __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_compute_audio_samples() {
	// errors which need to be handled: 
	//   - amiga may still be initializing and respecive "file loads" may fail - in which
	//     case we need to abort and restart the initialization
	//   - program may terminate before the buffer is full 
	struct uade_sample_data *sample_data; 
	
	// NOTE: the 'uade_reboot' used to be at via a command from the GUI.. it
	// is no longer used..
	
	while (quit_program == 0) {
		m68k_run_1 ();		// run emulator step
		
		// check if the above processing provided us with a result
		sample_data= get_new_samples();	
		if (sample_data) {
			return 0;
		}
	
		// program quit before we have our samples
		if (quit_program != 0) {
			if (is_amiga_file_not_ready()) {
				// async file loading issue.. we need to retry when the file has loaded
				quit_program= 0;

				return -1;			// there is nothing in the buffer yet.. we are still in 'init phase'
			} else {
				return 1;			// program is done..
			}
		}
	}
	return 1;
}

char script[1024];
char initFs= 0;

int emu_prepare(char *basedir) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_prepare(char *basedir)
{
	if (!initFs) {
		const char *in = 	"Module.FS_createPath(\"/\", \"%s/players/ENV/EaglePlayer\", true, true);"
							"Module.FS_createPath(\"/%s/players/ENV\", \"S\", true, true);"
							"Module.FS_createPath(\"/%s/\", \"songs\", true, true);"
							"Module.FS_createPath(\"/%s/\", \"amigasrc/score\", true, true);";

		snprintf(script, sizeof(script), in, basedir, basedir, basedir, basedir);

		emscripten_run_script(script);
		initFs= 1;
	}
}

int emu_init(int sample_rate, char *basedir, char *songmodule) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_init(int sample_rate, char *basedir, char *songmodule)
{
	return uade_reset(sample_rate, basedir, songmodule);
}

static void emu_set_subsong(int subsong) __attribute__((noinline));
static void EMSCRIPTEN_KEEPALIVE emu_set_subsong(int subsong)
{
	change_subsong(subsong);
}

void emu_teardown (void)  __attribute__((noinline));
void EMSCRIPTEN_KEEPALIVE emu_teardown (void) {
    uade_teardown();
}