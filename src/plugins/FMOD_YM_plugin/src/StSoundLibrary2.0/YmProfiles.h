/*-----------------------------------------------------------------------------

	ST-Sound ( YM files player library )

	Copyright (C) 1995-1999 Arnaud Carre ( http://leonard.oxg.free.fr )
	Copyright (C) 2010 CPC SDK crew ( http://github.com/cpcsdk/ )

	Define YM Profiles for multiple possible configuration (VolTable, Mixer...)
	Change that file depending of your platform. Please respect the right size
	for each type.

-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------

	This file is part of ST-Sound

	ST-Sound is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	ST-Sound is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with ST-Sound; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

-----------------------------------------------------------------------------*/

#ifndef __YMPROFILES__
#define __YMPROFILES__

#include "YmTypes.h"

/**
 * \struct ymProfile
 * \brief Stock emulation profile parameters
 * Profile for emulation, contains volumes tables, mixing value and clock
 */
typedef struct _ymProfile
{
    ymint volumeTable[5][16];
    ymfloat volOut[3]; // Mono volume mixer
    ymfloat volLeftOut[3]; // Left volume mixer
    ymfloat volRightOut[3]; // Right volume mixer
    ymu32 masterClock;
} ymProfile;

// From Original STSound values
#define YM_VOL_TABLE { {62,161,265,377,580,774,1155,1575,2260,3088,4570,6233,9330,13187,21220,32767}, {62,161,265,377,580,774,1155,1575,2260,3088,4570,6233,9330,13187,21220,32767}, {62,161,265,377,580,774,1155,1575,2260,3088,4570,6233,9330,13187,21220,32767}, {62,161,265,377,580,774,1155,1575,2260,3088,4570,6233,9330,13187,21220,32767}, {62,161,265,377,580,774,1155,1575,2260,3088,4570,6233,9330,13187,21220,32767} }

// Measurements done on a real CPC by Targhan/Arkos
// Note : when there is noise, the table is altered. We have 5 different ones
#define AY_VOL_TABLE { {0,116,348,579,1042,1390,2084,3358,4053,6600,9147,12157,16094,20378,26400,32767}, {0,0,  348,348,1042,1042,2084,2084,4053,4053,9147,12157,16094,16094,26400,32767}, {0,0,  0,  0,  3358,3358,3358,4053,4053,4053,6600,6600,  6600,32767,32767,32767}, {0,0,  0,  0,  0,   4053,4053,4053,4053,4053,4053,32767,32767,32767,32767,32767}, {0,0,  0,  0,  0,   0,  4053,4053,4053,4053,32767,32767,32767,32767,32767,32767} }

/**
 * \brief Profile for Atari ST emulation
 * \todo Found good mixing and see for difference between Atari models
 */
static ymProfile profileAtari =
{
    YM_VOL_TABLE,
    { 0.333f, 0.333f, 0.333f },
    { 0.333f, 0.333f, 0.333f },
    { 0.333f, 0.333f, 0.333f },
    /** \TODO: Find real mixer values (for STe, STf is mono anyway?) */
    2000000,
};

/**
 * \brief Profile for Amstrad CPC emulation
 * Volume Table computed from R values in CPC schematics by Grimmy/Arkos
 */
static ymProfile profileCPC =
{
    AY_VOL_TABLE,
    { 0.333f, 0.333f, 0.333f },
    { 0.687f, 0.312f, 0.000f },
    { 0.000f, 0.312f, 0.687f },
    1000000,
};

/**
 * \brief Profile for Spectrum emulation
 * \todo Find real mixer values, and handle difference between all Spectrum clones
 */
static ymProfile profileSpectrum =
{
    AY_VOL_TABLE,
    { 0.333f, 0.333f, 0.333f },
    { 0.333f, 0.333f, 0.333f },
    { 0.333f, 0.333f, 0.333f },
    /** \TODO: Find real mixer values */
    1773400,
};

static ymProfile profileMFP =
{
    AY_VOL_TABLE,
    { 0.333f, 0.333f, 0.333f },
    { 0.333f, 0.333f, 0.333f },
    { 0.333f, 0.333f, 0.333f },
    /** \TODO: Find real mixer values */
    2457600,
};

#endif

