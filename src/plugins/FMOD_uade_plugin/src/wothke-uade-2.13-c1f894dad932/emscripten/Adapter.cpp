/*
* This file adapts UADE to the interface expected by my generic JavaScript player..
*
* Copyright (C) 2014 Juergen Wothke
*
* Note: UADE has no support for "seeking". Different to other players, a song's meta info
* cannot be "read" after a song has been loaded but it is instead pushed asynchronously
* by from within the Amiga emulation during playback.
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

#include <stdbool.h>
#include <stdint.h>
#include <string>

#include <emscripten.h>

#include "uade.h"
#include "standalonesupport.h"


#define MAX_SCOPE_BUFFERS 4
#define SILENCE_SPEEDUP 10

namespace uade {

	class Adapter {
	public:
		Adapter() :_initFs(false), _sampleRate(0), _isReady(false), _time(0), _uadeBasePath("")
		{
		}
		~Adapter()
		{
		}
		int init(char *uadeBasePath)
		{
			_uadeBasePath = std::string(uadeBasePath);

			if (!_initFs)
			{
				char script[1024];
				const char *in = 	"Module.FS_createPath(\"/\", \"%s/players/ENV/EaglePlayer\", true, true);"
									"Module.FS_createPath(\"/%s/players/ENV\", \"S\", true, true);"
									"Module.FS_createPath(\"/%s/\", \"songs\", true, true);"
									"Module.FS_createPath(\"/%s/\", \"amigasrc/score\", true, true);";

				snprintf(script, sizeof(script), in, uadeBasePath, uadeBasePath, uadeBasePath, uadeBasePath);

				emscripten_run_script(script);
				_initFs = true;
			}
			return 0;
		}

		void teardown()
		{
			_isReady = false;
			_time = 0;
			uade_teardown();
		}

		int loadFile(int sampleRate, char *basedir, char *songmodule, int disableModConverter)
		{
			_sampleRate = sampleRate;
			uade_disable_mod_converter = disableModConverter;

			int r = uade_boot(_uadeBasePath.c_str());
			if (r != 0)	return r;	// error or pending load

			// problem: UADE may only return track info when the song emu has been
			// run for a while.. or it my never do so for some types of songs

			// this "dry run" call will run the emulation as long as the info is available
			// (but this means the emu must be reset before actually playing this song - see below)

			r= uade_reset(sampleRate, basedir, songmodule, 1);
			return r;
		}

		int setSubsong(int subsong)
		{
			// after the earlier emu_load_file the info track info should now be available

			// previous dry run succeeded.. now repeat clean reset (or some
			// songs will not work correctly after the "dry run")
			uade_teardown();
			uade_reset(_sampleRate, 0, 0, 0);	// reboot (using path info used during emu_load_file)

			// FIXME: might just use a "track change" and avoid the "expensive" uade_reset()?

			if (subsong < 0)
			{
				// keep default
			}
			else
			{
				if (subsong < song_mins) subsong= song_mins;
				if (subsong > song_maxs) subsong= song_maxs;

				//  seems "change" is used for "initialized core"
				//change_subsong(subsong);

				// while this is used for some "uninitialized" scenario...? also seems to work fine..
				set_subsong(subsong);	// this works for silkwork.vss whereas change_subsong doesn't
			}

			_isReady = true;

			return 0;
		}

		int genSamples()
		{
			if (!_isReady) return 0;

			sample_data.is_new = 0;	// should be unnecessary

			// errors which need to be handled:
			//   - amiga may still be initializing and respective "file loads" may fail - in which
			//     case we need to abort and restart the initialization
			//   - program may terminate before the buffer is full
			struct uade_sample_data *data;


			// NOTE: the 'uade_reboot' used to be triggered via a command from the GUI
			// illegal ops may trigger quit_program so a reboot cannot fix all..)

			int speedup = SILENCE_SPEEDUP;

			while (!quit_program) {
				m68k_run_1();		// run emulator step	(see m68k_go in original server)

				// check if the above processing provided us with a result
				data = get_new_samples();

				if (data)
				{
					_time += getSampleBufferLength();	// current time in samples

					uade_apply_effects((int16_t *)sample_data.buf, getSampleBufferLength());

					if (g_state.song->is_playing || (speedup-- <= 1))
					{
						return 0;
					}
					else
					{
						// skip/speedup the silence at the start of a song
					}
				}
				else
				{
					// keep going until we have a buffer worth of samples..
				}

				// program quit before we have our samples
				if (quit_program)
				{
					if (is_amiga_file_not_ready())
					{
						// async file loading issue.. we need to retry when the file has loaded
						quit_program = 0;

						return -1;		// there is nothing in the buffer yet.. we are still in 'init phase'
					}
					else
					{
						// program is done.. and will exit loop
					}
				}
			}
			return 1;		// song end or error
		}

		char* getSampleBuffer()
		{
			sample_data.is_new = 0;		// mark as consumed
			return sample_data.buf;
		}

		int getSampleBufferLength()
		{
			return sample_data.buflen >> 2 ;	// in bytes (4 bytes per frame)
		}


		/*
		* @param val use range -1.0 to 1.0
		*/
		int setPanning(float val)
		{
			// uade's panning range is 0.0-2.0 (where 1.0 means mono)
			uade_set_panning(val+1.0);
			return 0;
		}

		int getNumberTraceStreams()
		{
			return MAX_SCOPE_BUFFERS;
		}

		const char** getTraceStreams()
		{
			_scopeBuffers[0] = sample_data.bufChan0;
			_scopeBuffers[1] = sample_data.bufChan1;
			_scopeBuffers[2] = sample_data.bufChan2;
			_scopeBuffers[3] = sample_data.bufChan3;

			return (const char**)_scopeBuffers;	// ugly cast to make emscripten happy
		}

		uint64_t getCurrentMicros()
		{
			// the "steps" of this clock are severely limited
			return _time * 1000000 / _sampleRate;
		}
	private:
		bool _initFs;
		bool _isReady;
		int _sampleRate;
		uint64_t _time;

		char* _scopeBuffers[MAX_SCOPE_BUFFERS];

		std::string _uadeBasePath;
	};
};

uade::Adapter _adapter;

extern "C" uint64_t getCurrentMicros()
{
	return _adapter.getCurrentMicros();
}



// old style EMSCRIPTEN C function export to JavaScript.
// todo: code might be cleaned up using EMSCRIPTEN's "new" Embind feature:
// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
#define EMBIND(retType, func)  \
	extern "C" retType func __attribute__((noinline)); \
	extern "C" retType EMSCRIPTEN_KEEPALIVE func


EMBIND(int, emu_load_file(int sampleRate, char *basedir, char *songmodule, int disableModConverter)) { return _adapter.loadFile(sampleRate, basedir, songmodule, disableModConverter); }
EMBIND(void, emu_teardown())						{ _adapter.teardown(); }
EMBIND(int, emu_set_subsong(int track))				{ return _adapter.setSubsong(track); }
EMBIND(char*, emu_get_audio_buffer())				{ return _adapter.getSampleBuffer(); }
EMBIND(int, emu_get_audio_buffer_length())			{ return _adapter.getSampleBufferLength(); }
EMBIND(int, emu_compute_audio_samples())			{ return _adapter.genSamples(); }
EMBIND(int, emu_number_trace_streams())				{ return _adapter.getNumberTraceStreams(); }
EMBIND(const char**, emu_get_trace_streams())		{ return _adapter.getTraceStreams(); }

// --- add-ons
EMBIND(int, emu_prepare(char *basedir))				{ return _adapter.init(basedir); }
EMBIND(int, emu_set_panning(float val))				{ return _adapter.setPanning(val); }
