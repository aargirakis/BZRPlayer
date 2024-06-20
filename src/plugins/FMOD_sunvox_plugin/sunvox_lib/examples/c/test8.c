//
// * Saving the project to the file
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <signal.h>

#define SUNVOX_MAIN
#include "../../headers/sunvox.h"

int main()
{
    if( sv_load_dll() )
	return 1;

    int ver = sv_init( 0, 44100, 2, 0 );
    if( ver >= 0 )
    {
	int major = ( ver >> 16 ) & 255;
	int minor1 = ( ver >> 8 ) & 255;
	int minor2 = ( ver ) & 255;
	printf( "SunVox lib version: %d.%d.%d\n", major, minor1, minor2 );

	sv_open_slot( 0 );

	//Add some module:
	sv_lock_slot( 0 );
	int mod_num = sv_new_module( 0, "Generator", "Generator", 0, 0, 0 ); //create GENERATOR
	sv_connect_module( 0, mod_num, 0 ); //connect GENERATOR to 0.OUTPUT
	sv_unlock_slot( 0 );

	//Save the project file:
	sv_save( 0, "myproj.sunvox" );

	sv_close_slot( 0 );
	sv_deinit();
    }
    else
    {
	printf( "sv_init() error %d\n", ver );
    }

    sv_unload_dll();

    return 0;
}
