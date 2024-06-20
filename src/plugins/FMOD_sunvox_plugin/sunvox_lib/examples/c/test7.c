//
// * Playback synchronization on different slots
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <signal.h>
#include <math.h>

#define SUNVOX_MAIN /* We are using a dynamic lib. SUNVOX_MAIN adds implementation of sv_load_dll()/sv_unload_dll() */
#include "../../headers/sunvox.h"

int keep_running = 1;
void int_handler( int param ) 
{
    keep_running = 0;
}

int load( int slot, const char* name )
{
    int res = sv_load( slot, name );
    if( res )
	printf( "Can't load %s: error %d.\n", name, res );
    else
	printf( "%s loaded\n", name );
    return res;
}

int main()
{
    signal( SIGINT, int_handler );

    if( sv_load_dll() )
	return 1;

    int ver = sv_init( 0, 44100, 2, 0 );
    if( ver >= 0 )
    {
	int major = ( ver >> 16 ) & 255;
	int minor1 = ( ver >> 8 ) & 255;
	int minor2 = ( ver ) & 255;
	printf( "SunVox lib version: %d.%d.%d\n", major, minor1, minor2 );

	int slot1 = 0;
	int slot2 = 1;
	sv_open_slot( slot1 );
	sv_open_slot( slot2 );

	load( slot1, "song03.sunvox" );
	sv_play_from_beginning( slot1 );
	printf( "Playing slot1...\n" );

	int i = 0;
	while( keep_running )
	{
	    printf( "status: %d %d\n", !sv_end_of_song( slot1 ), !sv_end_of_song( slot2 ) );
	    if( i == 5 )
	    {
		//It's time to load and start song04.sunvox on slot2:
		load( slot2, "song04.sunvox" );
		sv_pause( slot2 ); //make slot2 suspended
		sv_play_from_beginning( slot2 ); //SLOT2: prepare to play
		printf( "Prepare to play slot2 (waiting for sync)...\n" );
		int p = sv_find_pattern( slot1, "SYNC" ); //SLOT1: find a pattern named "SYNC"
		if( p >= 0 )
		{
		    //Here we use lock/unlock, because it is important to execute the following commands at the same time
		    sv_lock_slot( slot1 );
		    //Write STOP (effect 30) command to the pattern (track 1; line 0):
		    sv_set_pattern_event(
			slot1, p,
			1, //track
			0, //line
			-1, //note (NN); skipped
			-1, //velocity (VV); skipped
			-1, //module (MM); skipped
			0x0030, //0xCCEE (controller and effect)
			-1 //0xXXYY (ctl/effect parameter); skipped
		    );
		    sv_sync_resume( slot2 ); //SLOT2: wait for sync (effect 0x33 from the "SYNC" pattern) and resume
		    sv_unlock_slot( slot1 );
		}
	    }
	    if( i == 16 )
	    {
		//Stop slot2 and play slot1:
		sv_pause( slot1 ); //make slot1 suspended
		int p = sv_find_pattern( slot1, "SYNC" ); //SLOT1: find a pattern named "SYNC"
		if( p >= 0 )
		{
		    //Remove STOP (effect 30) command from the pattern (track 1; line 0)
		    sv_set_pattern_event(
			slot1, p,
			1, //track
			0, //line
			-1, //note (NN); skipped
			-1, //velocity (VV); skipped
			-1, //module (MM); skipped
			0, //0xCCEE (controller and effect)
			-1 //0xXXYY (ctl/effect parameter); skipped
		    );
		}
		sv_play_from_beginning( slot1 ); //SLOT1: prepare to play
		printf( "Prepare to play slot1 again (waiting for sync)...\n" );
		p = sv_find_pattern( slot2, "SYNC" ); //SLOT2: find a pattern named "SYNC"
		if( p >= 0 )
		{
		    sv_lock_slot( slot2 );
		    //Write STOP (effect 30) command to the pattern (track 1; line 0)
		    sv_set_pattern_event(
			slot2, p,
			1, //track
			0, //line
			-1, //note (NN); skipped
			-1, //velocity (VV); skipped
			-1, //module (MM); skipped
			0x0030, //0xCCEE (controller and effect)
			-1 //0xXXYY (ctl/effect parameter); skipped
		    );
		    sv_sync_resume( slot1 ); //SLOT1: wait for sync (effect 0x33 from the "SYNC" pattern) and resume
		    sv_unlock_slot( slot2 );
		}
	    }
	    sleep( 1 );
	    i++;
	}

	sv_close_slot( slot1 );
        sv_close_slot( slot2 );
	sv_deinit();
    }
    else
    {
	printf( "sv_init() error %d\n", ver );
    }

    sv_unload_dll();

    return 0;
}
