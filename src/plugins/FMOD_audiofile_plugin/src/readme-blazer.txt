to compile with mingw32:

0.3.6
skapa en config.h och lägg in #define HAVE_UNISTD_H

ta bort filen modules/UT_RebufferModule.cpp
if filen CAF.cpp byt ut
	bzero(m_codecData->data(), m_codecData->size());
	memset(m_codecData->data(), 0,m_codecData->size());
	på rad 707
renamed modules/PCM.cpp and .h to modules/PCMrenamed.cpp
#include "PCMrenamed.h" i modules/PCMrenamed
#include "PCMrenamed.h" i units.cpp
renamed modules/G711.cpp and .h to modules/G711renamed.cpp
#include "G711renamed.h" i modules/G711	renamed
#include "G711renamed.h" i units.cpp


I WAVE.cpp kommenterade bort assertion och returnar fel istället (så ej vissa wav krashar)
	/* numCoefficients should be at least 7. */
            //assert(numCoefficients >= 7 && numCoefficients <= 255);
            if(!(numCoefficients >= 7 && numCoefficients <= 255))
            {
                return AF_FAIL;
            }

0.3.4
skapa en config.h och lägg in #define HAVE_UNISTD_H
lägg till #include "File.h" i IMA.cpp
renamed modules/G711.c and .h to modules/G711renamed.c
renamed modules/PCM.c and .h to modules/PCMrenamed.c
#include "PCMrenamed.h" i modules/PCMrenamed
#include "G711renamed.h" i modules/G711	renamed
#include "PCMrenamed.h" i units.cpp
#include "G711renamed.h" i units.cpp

0.3.2 (nu cpp istället för c)
skapa en config.h och lägg in #define HAVE_UNISTD_H
Kommentera bort assert(fh->tell() == track->fpos_first_frame); i IMA.cpp
renamed modules/g711.c and .h to modules/g711renamed.c
renamed modules/pcm.c and .h to modules/pcmrenamed.c
#include "PCMrenamed.h" i modules/PCMrenamed
#include "G711renamed.h" i modules/G711	renamed
#include "PCMrenamed.h" i units.cpp
#include "G711renamed.h" i units.cpp