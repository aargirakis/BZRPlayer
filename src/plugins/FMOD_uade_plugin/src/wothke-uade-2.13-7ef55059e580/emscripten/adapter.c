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

extern int quit_program;
extern struct uade_sample_data sample_data;
extern unsigned int song_mins, song_maxs, song_curs;

extern int uade_boot(char *basedir);
extern void uade_teardown (void);
extern void m68k_run_1 (void);
extern void uade_set_panning(float val);
extern void uade_apply_effects(int16_t * samples, int frames);


#define MAX_SCOPE_BUFFERS 4
static char* scope_buffers[MAX_SCOPE_BUFFERS];	

/*
* @param val use range -1.0 to 1.0
*/
static int emu_set_panning(float val) __attribute__((noinline));
static int EMSCRIPTEN_KEEPALIVE emu_set_panning(float val)
{
	// uade's panning range is 0.0-2.0 (where 1.0 means mono)	
	uade_set_panning(val+1.0);
	return 0;
}


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
	struct uade_sample_data *data; 
	
	// NOTE: the 'uade_reboot' used to be triggered via a command from the GUI
	// illegal ops may trigger quit_program so a reboot cannot fix all..)
	
	while (quit_program == 0) {
		m68k_run_1();		// run emulator step	(see m68k_go in original server)
		
		// check if the above processing provided us with a result
		data= get_new_samples();	
		if (data) {
			uade_apply_effects((int16_t *)sample_data.buf, emu_get_audio_buffer_length()>> 2);
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
	return 0;
}

static int _last_sample_rate;	// convenience hack
int emu_init(int sample_rate, char *basedir, char *songmodule) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_init(int sample_rate, char *basedir, char *songmodule)
{
	_last_sample_rate= sample_rate;
	
	int r= uade_boot(basedir);
	if (r != 0)	return r;	// error or pending load
	
	// problem: UADE may only return track info when the song emu has been 
	// run for a while.. or it my never do so for some types of songs
	
	// this "dry run" call will run the emulation as long as the info is available
	// (but this means the emu must be reset before actually playing this song - see below)
	r= uade_reset(sample_rate, basedir, songmodule, 1);
	return r;
}

static int emu_set_subsong(int subsong) __attribute__((noinline));
static int EMSCRIPTEN_KEEPALIVE emu_set_subsong(int subsong)
{
	// after the earlier emu_init the info track info should now be available
	
	// previous dry run succeeded.. now repeat clean reset (or some 
	// songs will not work correctly after the "dry run")
    uade_teardown();
	uade_reset(_last_sample_rate, 0, 0, 0);	// reboot (using path info used during emu_init)
	
	// FIXME: might just use a "track change" and avoid the "expensive" uade_reset()? 

	if (subsong < 0) {	
		// keep default
	} else {
		if (subsong < song_mins) subsong= song_mins;
		if (subsong > song_maxs) subsong= song_maxs;

		//  seems "change" is used for "initialized core"
		//change_subsong(subsong);
		
		// while this is used for some "uninitialized" scenario...? also seems to work fine..
		set_subsong(subsong);	// this works for silkwork.vss whereas change_subsong doesn't
	}
	return 0;
}

void emu_teardown (void)  __attribute__((noinline));
void EMSCRIPTEN_KEEPALIVE emu_teardown (void) {
    uade_teardown();
}


int emu_get_number_trace_streams() __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_get_number_trace_streams() {
	return MAX_SCOPE_BUFFERS;
}

const char** emu_get_trace_streams() __attribute__((noinline));
const char** EMSCRIPTEN_KEEPALIVE emu_get_trace_streams() {
	scope_buffers[0] = sample_data.bufChan0;
	scope_buffers[1] = sample_data.bufChan1;
	scope_buffers[2] = sample_data.bufChan2;
	scope_buffers[3] = sample_data.bufChan3;
	
	return (const char**)scope_buffers;	// ugly cast to make emscripten happy
}
