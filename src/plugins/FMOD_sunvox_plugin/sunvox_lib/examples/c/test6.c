//
// * Loading different projects into different slots and playing them simultaneously
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
	printf( "Current sample rate: %d\n", sv_get_sample_rate() );

	int slot1 = 0;
	int slot2 = 1;
	sv_open_slot( slot1 );
	sv_open_slot( slot2 );

	int res = -1;
	res = sv_load( slot1, "song01.sunvox" );
	if( res == 0 )
	    printf( "Project 1 loaded.\n" );
	else
	    printf( "Load error %d.\n", res );
	res = sv_load( slot2, "song02.sunvox" );
	if( res == 0 )
	    printf( "Project 2 loaded.\n" );
	else
	    printf( "Load error %d.\n", res );

	printf( "Project 1 name: %s\n", sv_get_song_name( slot1 ) );
	printf( "Project 2 name: %s\n", sv_get_song_name( slot2 ) );

	sv_pause( slot1 );
	sv_pause( slot2 );

	sv_play_from_beginning( slot1 );
	sv_play_from_beginning( slot2 );

	int i = 0;
	while( keep_running )
	{
	    if( i & 1 )
		sv_resume( slot1 );
	    else
		sv_pause( slot1 );
	    if( i & 2 )
		sv_resume( slot2 );
	    else
		sv_pause( slot2 );
	    printf( "Slot states: %d %d\n", ( i & 1 ) != 0, ( i & 2 ) != 0 );
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
