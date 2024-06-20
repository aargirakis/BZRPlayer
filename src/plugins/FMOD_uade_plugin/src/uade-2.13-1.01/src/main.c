#include <uae.h>

#ifndef EMSCRIPTEN
int main(int argc, char **argv)
{
    return uade_main(argc, argv);
}
#else
#include "ossupport.h"

char *buffer;
int bufLen;


// FIXME this will not work anymore .. (since there are
// currently no C impls for the JavaScript callbacks
// that the respective EMSCRIPTEN compiler is including..
// i.e. this file is not really meant to be compiled in 
// EMSCRIPTEN mode..)
int main(int argc, char **argv)
{
	while (!emu_is_exit()) {
		// song files etc will be loaded here FIMXE handle status -1, 
		//i.e. when not all files could be loaded; 1= error 0= ready
	//	int status = emu_reset (".", "songs/mdat.title");		// all init in here..
		int status = emu_init(44100, ".", argv[1]);		// all init in here..

//		fprintf(stderr, "SONG: %s\n", emu_get_current_song_info()[0]);
		
		while (/*!emu_is_reboot() &&*/ !emu_is_exit()) {	// FIXME build times or use existing feature to stop endless song
			status= emu_compute_audio_samples();
			if (status < 0) break; // retry "reset"
			else if (status > 0)	; // react to "no samples"

			buffer= emu_get_audio_buffer();
			bufLen= emu_get_audio_buffer_length();
			
			if (fwrite(buffer, bufLen, 1, stdout) != 1) {}
		}	
	}

	emu_teardown();
	return 0;
}
#endif