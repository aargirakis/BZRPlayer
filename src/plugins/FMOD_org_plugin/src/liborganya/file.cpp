/*
 *  file.c
 *  liborganya
 *
 *  Created by Vincent Spader on 6/20/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "swap.h"

// File reading helpers 
uint8_t _org_read_8(ifstream & fin) {
	uint8_t i = 0;
        fin.read((char*)&i,sizeof(uint8_t));
	return i;
}

uint16_t _org_read_16(ifstream & fin) {
	uint16_t i = 0;
        fin.read((char*)&i,sizeof(uint16_t));
        return org_ltoh_16(i);
}

uint32_t _org_read_32(ifstream & fin) {
	uint32_t i = 0;
        fin.read((char*)&i,sizeof(uint32_t));
        return org_ltoh_32(i);
}

// Read the usual org header
bool _org_read_header(org_header_t *header, ifstream & fin)
{
	// Read the magic. All orgyana files start with Org-02.
	int8_t buf[6];
        fin.read((char*)&buf,sizeof(int8_t)*6);
	if(0 != memcmp(buf, "Org-02", 6)) {
                return false;
	}
	
        header->tempo = _org_read_16(fin);
        header->steps_per_bar = _org_read_8(fin);
        header->beats_per_step = _org_read_8(fin);
        header->loop_start = _org_read_32(fin);
        header->loop_end = _org_read_32(fin);
        return true;
}

// Read properties for the instrument
void _org_read_instrument(org_instrument_t *instrument, ifstream & fin)
{
        instrument->pitch = _org_read_16(fin);
        instrument->instrument = _org_read_8(fin);
        instrument->disable_sustain = _org_read_8(fin);
        instrument->note_count = _org_read_16(fin);
}

// Read properties for each note
void _org_read_notes(org_note_t notes[], ifstream & fin, uint16_t note_count)
{
	for (uint16_t i = 0; i < note_count; i++) {
                notes[i].start = _org_read_32(fin);
	}
	for (uint16_t i = 0; i < note_count; i++) {
                notes[i].key = _org_read_8(fin);
	}
	for (uint16_t i = 0; i < note_count; i++) {
                notes[i].length = _org_read_8(fin);
	}
	for (uint16_t i = 0; i < note_count; i++) {
                notes[i].volume = _org_read_8(fin);
	}
	for (uint16_t i = 0; i < note_count; i++) {
                notes[i].pan = _org_read_8(fin);
	}
}

// Rather straightforward just follows the file format.
org_file_t *_org_file_create(ifstream & fin)
{
    org_file_t *org = ( org_file_t * ) calloc(1, sizeof(org_file_t));
    if ( !org ) throw std::bad_alloc();

    if(!_org_read_header(&org->header, fin))
    {
        _org_file_destroy( org );
        return NULL;
    }
    else
    {

        // Read instrument properties
        for (uint8_t i = 0; i < 16; i++) {
            _org_read_instrument(&org->instruments[i], fin);

            // Allocate space for notes
            if (org->instruments[i].note_count) {
                org->instruments[i].notes = ( org_note_t * ) malloc(sizeof(org_note_t) * org->instruments[i].note_count);
                if ( !org->instruments[i].notes ) throw std::bad_alloc();
            }
            else {
                org->instruments[i].notes = NULL;
            }
        }

        // Read notes for each instrument
        for (uint8_t i = 0; i < 16; i++) {
            _org_read_notes(org->instruments[i].notes, fin, org->instruments[i].note_count);
        }

        return org;
    }
}

void _org_file_destroy(org_file_t *org) {
	// Free up memory
	for (uint8_t i = 0; i < 16; i++) {
		if (org->instruments[i].notes) free(org->instruments[i].notes);
	}
	
	free(org);
}
