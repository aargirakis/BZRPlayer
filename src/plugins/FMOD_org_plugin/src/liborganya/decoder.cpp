/*
 *  decoder.c
 *  liborganya
 *
 *  Created by Vincent Spader on 6/20/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

//#include <foobar2000.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#include "decoder.h"
#include "endian.h"
#include <sstream>
// Advance the decoder by one beat
void _org_advance_beat(org_decoder_t *decoder) {
	// Update the current beat
	if (decoder->state.primed) {
		decoder->state.current_beat++;
		
		// Should we loop?
		if (decoder->state.current_beat >= decoder->file->header.loop_end) {
			uint8_t should_loop = (decoder->state.loop_count == 0 || decoder->state.current_loop < decoder->state.loop_count);
			if (should_loop) {
				decoder->state.current_loop++;
				decoder->state.current_beat = decoder->file->header.loop_start;
				decoder->state.current_sample = decoder->state.current_beat * (uint32_t)(((uint64_t)decoder->file->header.tempo * (uint64_t)decoder->state.sample_rate)/(uint64_t)1000); // reset the sample
				for (int i = 0; i < 16; i++) {
					decoder->state.tracks[i].playing = 0;
					for (int j = 0; j < decoder->file->instruments[i].note_count; j++) {
						if (decoder->file->instruments[i].notes[j].start >= decoder->file->header.loop_start) {
							decoder->state.tracks[i].current_note = j;
							decoder->state.tracks[i].note = decoder->file->instruments[i].notes[j];
							break;
						}
					}
				}
			}
		}
	}
	else {
		for (int i = 0; i < 16; i++) {
            if (0 == decoder->file->instruments[i].note_count || decoder->state.tracks[i].muted) { //muted check added by blazer
				continue;
			}
			
			decoder->state.tracks[i].note = decoder->file->instruments[i].notes[0];
		}
		
		decoder->state.primed = 1;
	}
	
	// Update the notes, if neccessary
	for (uint8_t i = 0; i < 16; i++) {
        if (0 == decoder->file->instruments[i].note_count || decoder->state.tracks[i].muted) { //muted check added by blazer
			continue;
		}

		uint16_t current_note = decoder->state.tracks[i].current_note;
		
		uint8_t is_melody = (i < 8);
		
		// Check if we should move on to the next note
		if (current_note + 1 < decoder->file->instruments[i].note_count &&  decoder->file->instruments[i].notes[current_note+1].start <= decoder->state.current_beat) {
			decoder->state.tracks[i].current_note++;
			current_note++;
			
			org_note_t note = decoder->file->instruments[i].notes[current_note];
			
			if (0xff != note.key) { // If key == 0xff, then the key should remain the same. New key == new note, so we update length and whatnot accordingly as well.
				decoder->state.tracks[i].note.key = note.key;
				decoder->state.tracks[i].note.start = note.start;
				decoder->state.tracks[i].note.length = note.length;
			}
			if (0xff != note.volume) { // If volume == 0xff, then the volume should remain the same
				decoder->state.tracks[i].note.volume = note.volume;
			}
			if (0xff != note.pan) { // If pan == 0xff, then the key should remain the same
				decoder->state.tracks[i].note.pan = note.pan;
			}

			decoder->state.tracks[i].last_clock = 0;
		}
		
        if (decoder->state.tracks[i].note.start <= decoder->state.current_beat) { //Blazer added muted
            decoder->state.tracks[i].playing = 1;
		}
		if (is_melody && decoder->state.tracks[i].note.start + decoder->state.tracks[i].note.length <= decoder->state.current_beat) { 
			decoder->state.tracks[i].playing = 0;
		}
	}
}

// Decodes a given number of samples
size_t org_decode_samples(org_decoder_t *decoder, int16_t *buffer, size_t num_samples)
{
	//const uint8_t bits_per_sample = 16;
	//const uint8_t channels = 2;
	uint32_t samples_per_beat = (uint32_t)((uint64_t)decoder->file->header.tempo * (uint64_t)decoder->state.sample_rate / (uint64_t)1000);
	uint32_t samples_rendered = 0;
    for (int i = 0; i < num_samples; i++) {
		if (decoder->state.current_sample % samples_per_beat == 0) { // we are done sampling the current beat, on to the next
			_org_advance_beat(decoder);
			
			if (i > samples_rendered) {
				blip_end_frame(decoder->state.blip_buffer[0], (i - samples_rendered) * 65536);
				blip_end_frame(decoder->state.blip_buffer[1], (i - samples_rendered) * 65536);
				blip_read_samples(decoder->state.blip_buffer[0], buffer, i - samples_rendered, 1);
				blip_read_samples(decoder->state.blip_buffer[1], buffer + 1, i - samples_rendered, 1);
				buffer += (i - samples_rendered) * 2;
				samples_rendered = i;
			}

			// Check if we are done decoding the file.
			if (decoder->state.current_beat > decoder->file->header.loop_end) {
				return i;
			}
		}

		for (int j = 0; j < 16; j++) {
			uint8_t is_melody = j < 8; // First 8 instruments are melodies, last 8 are drums
			if (0 == decoder->state.tracks[j].playing)  {
				continue;
			}
			org_sample_data_t sample_data = decoder->samples[j];
			if (sample_data.length == 0) { // Set a samples length to 0 to disable it Used when debugging)
				continue;
			}
			
			org_note_t note = decoder->state.tracks[j].note;
			if (note.key == 0xff || note.volume == 0xff || note.pan == 0xff) { // Partially initialized notes shouldnt be played.
				continue;
			}
			
			// Determine frequency adjustment
			double freq = (double)decoder->file->instruments[j].pitch - 1000.0;
			if (is_melody) {
				freq += 8363*pow(2.0,note.key/12.0)/2;
			}
			else {
				freq += (800*note.key + 100);
			}
			freq /= (double)decoder->state.sample_rate;
			
			// Calculate the offset into the sample data
			uint32_t note_start_beat = note.start;
			uint32_t note_sample_pos = decoder->state.current_sample - note_start_beat * samples_per_beat;
			uint32_t note_sample_advance = (uint32_t)((1.0 / freq) * 65536.0);
			uint32_t note_sample_clock = decoder->state.tracks[j].last_clock;
			uint32_t note_sample_beat_clock = note_sample_clock + (i - samples_rendered) * 65536;
			note_sample_pos = (uint32_t)(note_sample_pos * freq);

			while (note_sample_clock < 65536) {
				int32_t left_delta;
				int32_t right_delta;
				int32_t sample = 0;
				if (is_melody && !decoder->file->instruments[j].disable_sustain) { // Loop the sample
					note_sample_pos %= sample_data.length;
					sample = sample_data.wave[note_sample_pos];
				}
				else { // Do not loop the sample
					if (note_sample_pos < sample_data.length) {
						sample = sample_data.wave[note_sample_pos];
					}
				}

				// Adjust volume
				double volume = 1;
				volume = (note.volume/252.0);;

				// Adjust for panning
				double left_pan = 0.5;
				double right_pan = 0.5;
				double pan = note.pan/12.0;
				left_pan = 1 - pan;
				right_pan = pan;

				// Actually combine the sample data
				left_delta = (sample*volume*left_pan) - decoder->state.tracks[j].last_amp[0];
				right_delta = (sample*volume*right_pan) - decoder->state.tracks[j].last_amp[1];

				blip_add_delta(decoder->state.blip_buffer[0], note_sample_beat_clock, left_delta);
				blip_add_delta(decoder->state.blip_buffer[1], note_sample_beat_clock, right_delta);

				decoder->state.tracks[j].last_amp[0] += left_delta;
				decoder->state.tracks[j].last_amp[1] += right_delta;

				note_sample_pos++;

                note_sample_clock += note_sample_advance;
				note_sample_beat_clock += note_sample_advance;
			}

			note_sample_clock -= 65536;
			decoder->state.tracks[j].last_clock = note_sample_clock;
        }
		decoder->state.current_sample++;
	}

    if (num_samples > samples_rendered) {
        blip_end_frame(decoder->state.blip_buffer[0], (num_samples - samples_rendered) * 65536);
        blip_end_frame(decoder->state.blip_buffer[1], (num_samples - samples_rendered) * 65536);
        blip_read_samples(decoder->state.blip_buffer[0], buffer, num_samples - samples_rendered, 1);
        blip_read_samples(decoder->state.blip_buffer[1], buffer + 1, num_samples - samples_rendered, 1);
    }
	
	return num_samples;
}

// Loads instrument samples for each track
uint8_t _org_decoder_load_samples(org_decoder_t *decoder, const char *resource_path)
{
	// Set some sane initial values so we can return if there is an error and what was done can still be cleaned up. Caller could instead use the return value, but meh.
	for (uint8_t i = 0; i < 16; i++) {
		decoder->samples[i].wave = NULL;
		decoder->samples[i].length = 0;
	}
	
        for (uint8_t i = 0; i < 16; i++) {
		if (decoder->file->instruments[i].note_count == 0) {
			continue;
		}
		
		// Drum sample data is stored in files 100-111, even though the instrument still indexes 0-11. We just add 100.
		uint8_t instrument = decoder->file->instruments[i].instrument;
		if (i >= 8) { // drums
			instrument += 100;
		}
		
		// Files for the sample data are just <instrument>.dat. drums have 100 added, as noted above.
                std::string dat_path = resource_path;
                dat_path = dat_path +'\\';
                dat_path += ZeroPadNumber(instrument);
		dat_path += ".dat";

                int16_t *wave = NULL;
		size_t size = 0;
                ifstream fin;
                fin.open (dat_path.c_str(), ios::binary );

		// Get the number of samples
                fin.seekg (0, ios::end);
                size = fin.tellg();
                fin.seekg (0, ios::beg);
		// Allocate space for the PCM data and read it in.
                wave = (int16_t *) malloc(size);

                if ( !wave ) throw std::bad_alloc();
                fin.read((char*)wave,size*sizeof(int16_t));
		// PCM data is big endian. We want host.
		for (int j = 0; j < size/2; j++) {
                        wave[j] = btoh16(wave[j]);
		}
		// Store it in the decoder
                decoder->samples[i].wave = wave;
                decoder->samples[i].length = size/2;
	}
	return 16;
}

// Returns the total samples the decoder will decode over its lifetime
size_t org_decoder_get_total_samples(org_decoder_t *decoder)
{
	uint32_t loop_start = decoder->file->header.loop_start;
	uint32_t loop_end = decoder->file->header.loop_end;
	uint32_t loop_count = decoder->state.loop_count;

	// Return 0 if the decoder is set to loop indefinitely
	if (0 == loop_count) {
		return 0;
	}
	
	uint32_t total_beats = loop_start + loop_count * (loop_end - loop_start);
	size_t total_samples = total_beats * (uint32_t)(((uint64_t)decoder->file->header.tempo * (uint64_t)decoder->state.sample_rate) / (uint64_t)1000);
	
	return total_samples;
}
// Added by blazer
int16_t org_decoder_vumeter_channel(org_decoder_t *decoder, size_t channel)
{
    return abs(decoder->state.tracks[channel].last_amp[0])+abs(decoder->state.tracks[channel].last_amp[1]);
}

// Added by blazer
void org_decoder_mute_channel(org_decoder_t *decoder, size_t channel, bool mute)
{
    decoder->state.tracks[channel].muted=mute;
    //reset and fast forward to current position
    org_decoder_seek_sample(decoder,decoder->state.current_sample);
}

// Really might as well create a new decoder and throw away samples.
void org_decoder_seek_sample(org_decoder_t *decoder, size_t sample)
{
	uint32_t samples_per_beat = (uint32_t)((uint64_t)decoder->state.sample_rate*(uint64_t)decoder->file->header.tempo/(uint64_t)1000);
	uint32_t beat = sample/samples_per_beat;
	
	// Reset the decoder state
	decoder->state.primed = 0;
	decoder->state.current_loop = 1; // Loop count starts at 1
	decoder->state.current_beat = 0;
	decoder->state.current_sample = 0;

	blip_clear(decoder->state.blip_buffer[0]);
	blip_clear(decoder->state.blip_buffer[1]);
	
	// Reset tracks
	for (int i = 0; i < 16; i++) {
		decoder->state.tracks[i].current_note = 0;
		decoder->state.tracks[i].playing = 0;
		decoder->state.tracks[i].last_amp[0] = 0;
		decoder->state.tracks[i].last_amp[1] = 0;
		decoder->state.tracks[i].last_clock = 0;
	}
	
	// Advance to the correct beat
	for (int i = 0; i < beat; i++) {
		_org_advance_beat(decoder);
	}
	// In == 0 case, the first call to decode samples will advance the beat
	if (sample % samples_per_beat != 0) {
		_org_advance_beat(decoder);
	}

	// Set the correct sample state. Remember we reset the sample state when it loops.
	if (beat < decoder->file->header.loop_end) {
		decoder->state.current_sample = sample;
	}
	else {
		// The beat the current loop would be if played from the beginning
		uint32_t looped_beats = (decoder->state.current_loop - 1) * (decoder->file->header.loop_end - decoder->file->header.loop_start);
		decoder->state.current_sample = sample - (looped_beats * samples_per_beat); // reset the sample
	}
}

// Returns the total samples the decoder will decode over its lifetime
org_decoder_t *org_decoder_create(ifstream & org_file, const char *resource_path, uint32_t loop_count) {
    // Allocate the decoder
    org_decoder_t *decoder = ( org_decoder_t * ) calloc(1, sizeof(org_decoder_t));
    if ( !decoder ) throw std::bad_alloc();


    // Parse the org file
    decoder->file = _org_file_create(org_file);
    if(decoder->file)
    {

        // Set initial state
        decoder->state.primed = 0;
        decoder->state.current_beat = 0;
        decoder->state.current_sample = 0;
        decoder->state.loop_count = loop_count;
        decoder->state.sample_rate = 44100;
        decoder->state.current_loop = 1; // The first time through is the first loop.

        // Load samples the decoder will use
        uint8_t samples_loaded = _org_decoder_load_samples(decoder, resource_path);
        if (16 != samples_loaded) {
            org_decoder_destroy(decoder);

            return NULL;
        }

        uint32_t samples_per_beat = (uint32_t)((uint64_t)decoder->state.sample_rate*(uint64_t)decoder->file->header.tempo/(uint64_t)1000);

        decoder->state.blip_buffer[0] = blip_new(samples_per_beat * 4);
        decoder->state.blip_buffer[1] = blip_new(samples_per_beat * 4);
        if (!decoder->state.blip_buffer[0] || !decoder->state.blip_buffer[1]) {
            org_decoder_destroy(decoder);

            return NULL;
        }
        blip_set_rates(decoder->state.blip_buffer[0], 65536, 1);
        blip_set_rates(decoder->state.blip_buffer[1], 65536, 1);

        // Set initial track state
        for (uint8_t i = 0; i < 16; i++) {
            decoder->state.tracks[i].playing = 0;
            decoder->state.tracks[i].current_note = 0;
            decoder->state.tracks[i].last_amp[0] = 0;
            decoder->state.tracks[i].last_amp[1] = 0;
            decoder->state.tracks[i].last_clock = 0;
        }

        return decoder;
    }
    else
    {
        org_decoder_destroy(decoder);
        return NULL;
    }
}

void org_decoder_destroy(org_decoder_t *decoder) {
	// Toss the file
	if ( decoder->file ) _org_file_destroy(decoder->file);
	
	// Free up the sample data
	for (uint8_t i = 0; i < 16; i++) {
		if (NULL != decoder->samples[i].wave) {
			free(decoder->samples[i].wave);
		}
	}

	blip_delete(decoder->state.blip_buffer[0]);
	blip_delete(decoder->state.blip_buffer[1]);
	
	// Clean up the rest
	free(decoder);
}

std::string ZeroPadNumber(int num)
{
        stringstream ss;

        // the number is converted to string with the help of stringstream
        ss << num;
        string ret;
        ss >> ret;

        // Append zero chars
        int str_length = ret.length();
        for (int i = 0; i < 3 - str_length; i++)
                ret = "0" + ret;
        return ret;
}
