/*
 * Pro-Wizard_1.c
 *
 * 1997-2023 (c) Sylvain "Asle" Chipaux
 *
 * stripped down for use in UADE by Juergen Wothke
*/

#include "globals.h"
#include "extern.h"
#include "vars.h"

// NOTE:  The below logic seems to be somewhat fragile with regard to the removal of
// unused formats (some tests seem to depend on the context of previous tests).
// The UADE patches try to preserve the original file structure as much as possible
// (in order to ease potential future updates/merges) while getting rid of unused formats .

// NOTE: Only those formats that provide an actual "Depack*" step
// are potentially beneficial to UADE - whereas the benefit of "Rip_*" steps 
// is to rip actual music data from some larger memeory area (which doesn't help 
// for formats that UADE still doesn't know how to play).

// FIXME: it is probably counter productive (for UADE context) that the code scans the 
// complete buffer for the occurrence of potential rip candidates. Since respective input 
// has already been ripped it might reduce the risk of false positive detections to only
// detect what occurs immediately at the start of the buffer.

// replacement printf to make the converter less chatty
void disabledprint(const char*f, ...) {}

// below macros are used to end the detection logic once the
// first match has been made (the original impl would keep going 
// potentially creating multiple different output files - which isn't
// relevant in the UADE context)

	
// UADE natively already supports this format and we therefore won't use Proviz' output
#define CANCEL_SUPPORTED_FORMAT \
	if (Save_Status == GOOD) { cancel = 1; goto end;	}
	
// Proviz doesn't have a converter implementation for this format
#define CANCEL_UNIMPLEMENTED \
	{ cancel = 2; goto end;	}
	
// use whatever format was detected first
#define USE_MATCH \
	if (Save_Status == GOOD) goto end;


int proviz_get_result(uint8_t **out_data, uint32_t *outlen) {
	if ((CONVERT == GOOD) && (Save_Status == GOOD)) {
		FILE *file = PW_fopen(Depacked_OutName, "r+b");

		
		if (file) {
			struct stat st;
			fstat(fileno(file), &st);

			uint8_t *tmp_buffer = 0;
			int nbytes = 0;
			

			if (st.st_size <= 0) {
				fprintf(stdout, "proviz_get_result: error empty file %d\n", st.st_size);
			} 
			else {	
				tmp_buffer = malloc(st.st_size);
				nbytes = fread(tmp_buffer, 1, st.st_size, file);

			}
			*out_data = tmp_buffer;
			*outlen = nbytes;			

			fclose(file);
		}
		return 1;
	}
	
	// let's presume the temp files automatically overwrite 
	// and therefore should not sum up to much from a mem leak perspective
	
	*out_data = 0;
	*outlen = 0;			
		
	return 0;
}

static unsigned int usthack[] = {
//	0x7c44746e,	// Blueberry.mod this one sounds horrible..
	0xa15b897d,	// Crystal Hammer.mod
	0xc46bf520,	// Funky Up.mod
	0x3d215208,	// Nor02.mod
	0x2ca25c8e,	// Nor03.mod
	0xbc0d16be,	// Sll 4.mod
	0xce799468,	// The Firm - Mega Trainer.mod - actually "Defkam Soundtracker III / Alpha Flight" (close enough..)
	0x9d3adc62,	// Tune162.mod
	0xad01b314,	// Unit4.mod
	0
};

void proviz_convert( uint8_t *buf, uint32_t len )
{
	// UADE: patched startup sequence of original "main" impl
	PW_Start_Address = 0;
	OutputSize=0;
	Cpt_Filename = 01;
	Save_Status = GOOD;
	PW_WholeSampleSize=0;
	CONVERT = BAD;
	Amiga_EXE_Header = GOOD;
	int cancel = 0;
	LastFormatName = 0;
	DetectedFormatId = -1;
	
	
	// it seems tha the checking logic already corrupts the input buffer
	// so we better make a copy here.. (see PP20 files)
	in_data = (uint8_t *)malloc(len);	
	memcpy(in_data, buf, len);
	
	PW_in_size = len;

	calccrc(in_data, PW_in_size);
	
	
	if (checkcrc(usthack)) {	// UADE
		// poor man's "Ultimate SoundTracker 1.0-1.21" detection - for those songs that don't 
		// play with the UltimateSoundTracker player. might be different sub-versions actually... 
		
		// todo: <insert proper detection impl here>
		cancel = 1;
		DetectedFormatId = UltimateSoundTracker;
		goto end;
	}
	
  /********************************************************************/
  /**************************   SEARCH   ******************************/
  /********************************************************************/
  for ( PW_i=0 ; PW_i<(PW_in_size-MINIMAL_FILE_LENGHT) ; PW_i+=1 )
  {
    /* display where we are every 10 Kbytes */
    /*    if ( (PW_i%10240) == 0 )*/
    /*    {*/
      /* ... and rewrites on itself. */
    /*      printf ( "\r%ld", PW_i );*/
      /* force printing on stdout (i.e. the screen) */
    /*      fflush ( stdout );*/
    /*    }*/

    /*******************************************************************/
    /* ok, now the real job starts here. So, we look for ID or Volume  */
    /* values. A simple switch case follows .. based on Hex values of, */
    /* as foretold, ID or volume (not all file have ID ... few in fact */
    /* do ..).                                                         */
    /*******************************************************************/


    if ( in_data[PW_i] <= 0x40 )
    {
      /* try to get rid of those 00 hanging all the time everywhere :(
	 */
      if ( in_data[PW_i] == 0x00 )
      {
	    for ( PW_j = 0 ; PW_j<MINIMAL_FILE_LENGHT ; PW_j++)
	    {
	      if ( in_data[PW_j+PW_i] != 0x00 )
	        break;
	    }
	    if ( PW_j == MINIMAL_FILE_LENGHT )
	    {
	      PW_i += (MINIMAL_FILE_LENGHT-2);
	      continue;
	    }
      }


      /* first, let's take care of the formats with 'ID' value <= 0x40 */
      /* "!PM!" : ID of Power Music */
      if ( (in_data[PW_i]   == '!') &&
           (in_data[PW_i+1] == 'P') &&
           (in_data[PW_i+2] == 'M') &&
           (in_data[PW_i+3] == '!') )
      {
        if ( testPM() != BAD )
        {
          Rip_PM ();
          Depack_PM ();
		  
		  USE_MATCH
        }
      }
      /* Software Visions */
      /* ".M.K: id of Software Visions catalog musics */
      if ( (in_data[PW_i]   == '.') &&
           (in_data[PW_i+1] == 'M') &&
           (in_data[PW_i+2] == '.') &&
           (in_data[PW_i+3] == 'K') )
      {
        if ( testSV() != BAD )
        {
          Rip_SV ();
          Depack_SV ();
		  
		  CANCEL_SUPPORTED_FORMAT
        }
      }
      /* "[1-9]CHN" FastTracker v1 */
      if ( ((in_data[PW_i]   == '1') ||
	    (in_data[PW_i]   == '2') ||
	    (in_data[PW_i]   == '3') ||
	    (in_data[PW_i]   == '4') ||
	    (in_data[PW_i]   == '5') ||
	    (in_data[PW_i]   == '6') ||
	    (in_data[PW_i]   == '7') ||
	    (in_data[PW_i]   == '8') ||
	    (in_data[PW_i]   == '9'))&&
           (in_data[PW_i+1] == 'C') &&
           (in_data[PW_i+2] == 'H') &&
           (in_data[PW_i+3] == 'N') )
      {
        if ( testMOD(in_data[PW_i]-0x30) != BAD )
        {
          Rip_MOD (in_data[PW_i]-0x30);
		  
		  CANCEL_UNIMPLEMENTED
        }
      }
      /* "[10-32]CH" FastTracker v1/v2 */
      if ( ((((in_data[PW_i]   == '1') || (in_data[PW_i]   == '2')) && 
	     ((in_data[PW_i+1]   == '0') ||
	      (in_data[PW_i+1]   == '1') ||
	      (in_data[PW_i+1]   == '2') ||
	      (in_data[PW_i+1]   == '3') ||
	      (in_data[PW_i+1]   == '4') ||
	      (in_data[PW_i+1]   == '5') ||
	      (in_data[PW_i+1]   == '6') ||
	      (in_data[PW_i+1]   == '7') ||
	      (in_data[PW_i+1]   == '8') ||
	      (in_data[PW_i+1]   == '9'))) ||
	    ((in_data[PW_i]   == '3') &&
	     ((in_data[PW_i+1]   == '0') ||
	      (in_data[PW_i+1]   == '1')))) &&
	    (in_data[PW_i+2] == 'C') &&
	    (in_data[PW_i+3] == 'H') )
      {
        if ( testMOD((in_data[PW_i]-0x30)*10+in_data[PW_i+1]-0x30) != BAD )
        {
          Rip_MOD ((in_data[PW_i]-0x30)*10+in_data[PW_i+1]-0x30);
		  
		  CANCEL_UNIMPLEMENTED
        }
      }
#ifdef INCLUDEALL
      /* =SB= data cruncher */
      if ( (in_data[PW_i]   == 0x3D) &&
           (in_data[PW_i+1] == 'S') &&
           (in_data[PW_i+2] == 'B') &&
           (in_data[PW_i+3] == 0x3D) )
      {
        if ( testSpecialCruncherData ( 8, 4 ) != BAD )
        {
          Rip_SpecialCruncherData ( "=SB= Data Cruncher" , 12 , SB_DataCruncher );
		  
		  CANCEL_UNIMPLEMENTED
        }
      }

      /* B9AB data cruncher */
      if ( (in_data[PW_i]   == 0x0B) &&
           (in_data[PW_i+1] == 0x09) &&
           (in_data[PW_i+2] == 0x0A) &&
           (in_data[PW_i+3] == 0x0B) )
      {
        if ( testB9AB () != BAD )
        {
          Rip_SpecialCruncherData ( "B9AB Data Cruncher" , 274, B9AB );
		  
		  CANCEL_UNIMPLEMENTED
        }
      }

      /* -CJ- data cruncher (CrackerJack/Mirage)*/
      if ( (in_data[PW_i]   == 0x2D) &&
           (in_data[PW_i+1] == 'C') &&
           (in_data[PW_i+2] == 'J') &&
           (in_data[PW_i+3] == 0x2D) )
      {
        if ( testSpecialCruncherData ( 4, 8 ) != BAD )
        {
          Rip_SpecialCruncherData ( "-CJ- Data Cruncher" , 0 , CJ_DataCruncher );
		  
		  CANCEL_UNIMPLEMENTED
        }
      }
      
      /* "LOB" : ID LOB's File-Compressor 3.70 (Atari ST) */
      if ( (in_data[PW_i]   == 0x02) &&
           (in_data[PW_i+1] == 'L') &&
           (in_data[PW_i+2] == 'O') &&
           (in_data[PW_i+3] == 'B') )
      {
        if ( testSpecialCruncherData ( 8, 4 ) != BAD )
        {
          Rip_SpecialCruncherData ( "LOB's File-Compressor Data Cruncher" , 12 , LOBDataCruncher );
		  
		  CANCEL_UNIMPLEMENTED
        }
      }

      /* -GD- GnoiPacker (Skizzo)*/
      if ( (in_data[PW_i]   == 0x2D) &&
           (in_data[PW_i+1] == 'G') &&
           (in_data[PW_i+2] == 'D') &&
           (in_data[PW_i+3] == 0x2D) )
      {
        if ( testSkizzo() != BAD )
        {
          Rip_Skizzo ();
          Depack_Skizzo ();
		  
		  USE_MATCH
        }
      }

      /* Max Packer 1.2 */
      if ((in_data[PW_i]   == 0x28) &&
         (in_data[PW_i+1]  == 0x3C) &&
         (in_data[PW_i+6]  == 0x26) &&
         (in_data[PW_i+7]  == 0x7A) &&
         (in_data[PW_i+8]  == 0x01) &&
         (in_data[PW_i+9]  == 0x6C) &&
         (in_data[PW_i+10] == 0x41) &&
         (in_data[PW_i+11] == 0xFA) &&
         (in_data[PW_i+12] == 0x01) &&
         (in_data[PW_i+13] == 0x7C) &&
         (in_data[PW_i+14] == 0xD1) &&
         (in_data[PW_i+15] == 0xFA) )
      {
        if ( testMaxPacker12() != BAD )
        {
          Rip_MaxPacker12 ();
		  
		  CANCEL_UNIMPLEMENTED
        }
      }
#endif

      /* XANN packer */
      if ( in_data[PW_i] == 0x3c )
      {
        if ( testXANN() != BAD )
        {
          Rip_XANN ();
          Depack_XANN ();
		  
		  CANCEL_SUPPORTED_FORMAT 
        }
      }

      /* hum ... that's where things become interresting :) */
      /* Module Protector without ID */
      /* LEAVE IT THERE !!! ... at least before Heatseeker format since they are VERY similar ! */
      if ( testMP_noID() != BAD )
      {
        Rip_MP_noID ();
        Depack_MP ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* Digital Illusion */
      if ( testDI() != BAD )
      {
        Rip_DI ();
        Depack_DI ();
		  
		  USE_MATCH
      }

      /* eureka packer */
      if ( testEUREKA() != BAD )
      {
        Rip_EUREKA ();
        Depack_EUREKA ();
		  
		  USE_MATCH 
      }

      /* The player 5.0a ? */
      if ( testP50A() != BAD )
      {
        Rip_P50A ();
        Depack_P50A ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* The player 6.0a ? */
      if ( testP60A_nopack() != BAD )
      {
        Rip_P60A ();
        Depack_P60A ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* The player 6.0a (packed samples)? */
      if ( testP60A_pack() != BAD )
      {
       // fprintf (stdout,  "The Player 6.0A with PACKED samples found at %d ... cant rip it!\n" , PW_Start_Address );
        /*Rip_P60A ();*/
        /*Depack_P60A ();*/
		  CANCEL_UNIMPLEMENTED 
      }

      /* The player 6.1a ? */
      if ( testP61A_nopack() != BAD )
      {
        Rip_P61A ();
        Depack_P61A ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* The player 6.1a (packed samples)? */
      if ( testP61A_pack() != BAD )
      {
       // fprintf (stdout,  "The Player 6.1A with PACKED samples found at %d ... cant rip it!\n" , PW_Start_Address );
        /*Rip_P61A ();*/
        /*Depack_P61A ();*/
 		  CANCEL_UNIMPLEMENTED 
     }

      /* Propacker 1.0 */
      if ( testPP10() != BAD )
      {
        Rip_PP10 ();
        Depack_PP10 ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* Noise Packer v2 */
      /* LEAVE VERSION 2 BEFORE VERSION 1 !!!!! */
      if ( testNoisepacker2() != BAD )
      {
        Rip_Noisepacker2 ();
        Depack_Noisepacker2 ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* Noise Packer v1 */
      if ( testNoisepacker1() != BAD )
      {
        Rip_Noisepacker1 ();
        Depack_Noisepacker1 ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* Noise Packer v3 */
      if ( testNoisepacker3() != BAD )
      {
        Rip_Noisepacker3 ();
        Depack_Noisepacker3 ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* Promizer 0.1 */
      if ( testPM01() != BAD )
      {
        Rip_PM01 ();
        Depack_PM01 ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* ProPacker 2.1 */
      if ( testPP21() != BAD )
      {
        Rip_PP21 ();
        Depack_PP21 ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* ProPacker 3.0 */
      if ( testPP30() != BAD )
      {
        Rip_PP30 ();
        Depack_PP30 ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* StartTrekker pack */
	  // UADE: plays Claustrophobia.stp incorrectly
      if ( testSTARPACK() != BAD )
      {
        Rip_STARPACK ();
        Depack_STARPACK ();
		  
		  USE_MATCH 
      }

      /* Zen packer */
      if ( testZEN() != BAD )
      {
        Rip_ZEN ();
        Depack_ZEN ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* Unic tracker v1 ? */
      if ( testUNIC_withemptyID() != BAD )
      {
        Rip_UNIC_withID ();
        Depack_UNIC ();
		  
		  USE_MATCH 
      }

      /* Unic tracker v1 ? */
      if ( testUNIC_noID() != BAD )
      {
        Rip_UNIC_noID ();
        Depack_UNIC ();
		  
		  USE_MATCH 
      }

      /* Unic trecker v2 ? */
	  // old UADE seems to play this just as well
      if ( testUNIC2() != BAD )
      {
        Rip_UNIC2 ();
        Depack_UNIC2 ();
		  
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* Game Music Creator ? */
      if ( testGMC() != BAD )
      {
        Rip_GMC ();
        Depack_GMC ();
		
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* Heatseeker ? */
      if ( testHEATSEEKER() != BAD )
      {
        Rip_HEATSEEKER ();
        Depack_HEATSEEKER ();
		
		  CANCEL_SUPPORTED_FORMAT 
      }

      /* SoundTracker (15 smp) */
      if ( testSoundTracker() != BAD )
      {
        Rip_SoundTracker ();
		  
		  CANCEL_UNIMPLEMENTED
      }

      /* The Dark Demon (group name) format */
      if ( testTheDarkDemon() != BAD )
      {
        Rip_TheDarkDemon ();
        Depack_TheDarkDemon ();
		  
		  USE_MATCH
      }

      /* Newtron */
	  // UADE this is a keeper
      if ( testNewtron() != BAD )
      {
        Rip_Newtron ();
        Depack_Newtron ();
		  
		  USE_MATCH
      }

      /* Newtron Old */
	  // UADE this is a keeper
      if ( testNewtronOld() != BAD )
      {
        Rip_NewtronOld ();
        Depack_NewtronOld ();
		  
		  USE_MATCH
      }

      /* Titanics Player ? */
      if ( testTitanicsPlayer() != BAD )
      {
        Rip_TitanicsPlayer ();
        Depack_TitanicsPlayer ();
		  
		  CANCEL_SUPPORTED_FORMAT
      }
    }


    /**********************************/
    /* ok, now, the files with ID ... */
    /**********************************/
    switch ( in_data[PW_i] )
    {
      case 'B': /* 0x42 */
        /* "BNR!" Binary Packer */
        if ( (in_data[PW_i+1] == 'N') &&
             (in_data[PW_i+2] == 'R') &&
             (in_data[PW_i+3] == '!') )
        {
          if ( testBNR() != BAD )
          {
            Rip_BNR();
            Depack_BNR();
		  
		  USE_MATCH
          }
        }
        break;

      case 'F': /* 0x46 */
         /* "Fuck" : ID of Noise From Heaven chiptunes */
        if ( (in_data[PW_i+1] == 'u') &&
             (in_data[PW_i+2] == 'c') &&
             (in_data[PW_i+3] == 'k') )
        {
          if ( testNFH() != BAD )
          {
            Rip_NFH ();
            Depack_NFH ();
		  
		  USE_MATCH
          }
        }
    	  /* "FAST" : ID of Stone Arts Player */
        if ( (in_data[PW_i+1] == 'A') &&
             (in_data[PW_i+2] == 'S') &&
             (in_data[PW_i+3] == 'T') )
        {
          if ( testStoneArtsPlayer() != BAD )
          {
            Rip_StoneArtsPlayer ();
  	        Depack_StoneArtsPlayer ();
		  
		  USE_MATCH
          }
        }
        break;

      case 'G': /* 0x47 */
	/* GPMO (crunch player ?)*/
        if ( (in_data[PW_i+1] == 'P') &&
           (in_data[PW_i+2] == 'M') &&
           (in_data[PW_i+3] == 'O') )
        {
          if ( testGPMO() != BAD )
          {
            Rip_GPMO ();
            Depack_GPMO ();
		  
		  USE_MATCH
          }
        }

	/* Gnu player */
        if ( (in_data[PW_i+1] == 'n') &&
           (in_data[PW_i+2] == 'P') &&
           (in_data[PW_i+3] == 'l') )
        {
          if ( testGnuPlayer() != BAD )
          {
            Rip_GnuPlayer ();
            Depack_GnuPlayer ();
		  
		  USE_MATCH
          }
        }

        break;

      case 'M': /* 0x4D */

        if ( (in_data[PW_i+1] == '.') &&
             (in_data[PW_i+2] == 'K') &&
             (in_data[PW_i+3] == '.') )
        {
			// UADE keep all the unused below since the detecting might depend on it
			
          /* protracker ? */
          if ( testMOD(4) != BAD )
          {
            Rip_MOD(4);
		  
		  CANCEL_SUPPORTED_FORMAT
          }

          /* Unic tracker v1 ? */
          if ( testUNIC_withID() != BAD )
          {
            Rip_UNIC_withID ();
            Depack_UNIC ();
		  
		  USE_MATCH 
          }

          /* Noiserunner ? */
          if ( testNoiserunner() != BAD )
          {
            Rip_Noiserunner ();
            Depack_Noiserunner ();
		  
		  USE_MATCH
          }

          /* Mosh packer ? */
          if ( testMOSH() != BAD )
          {
            Rip_MOSH ();
            Depack_MOSH ();
		  
		  CANCEL_SUPPORTED_FORMAT
          }
          
          /* Devils' replay ? */
          if ( testDevils() != BAD )
          {
            Rip_Devils ();
            Depack_Devils ();
			
		  USE_MATCH
          }
          /* HCD-protector ? */
		  // UAFE: this is a keeper - much better than original UADE (or MPT or XMP)!
          if ( testHCD() != BAD )
          {
            Rip_HCD ();
            Depack_HCD ();
			
		  USE_MATCH
          }
        }

        break;

      case 'P': /* 0x50 */
          /* "P22A" : ID of The Player */
		  // UAFE: this is a keeper
        if ( (in_data[PW_i+1] == '2') &&
             (in_data[PW_i+2] == '2') &&
             (in_data[PW_i+3] == 'A') )
        {
          if ( testP40A() != BAD ) /* yep, same tests apply */
	      {
            Rip_P22A ();
            Depack_P22 ();
		  
		  USE_MATCH
          }
        }

          /* "PMd3/PMD3 : ID of ?!? TSCC format - 8CHN */
        if ( (in_data[PW_i+1] == 'M') &&
             ((in_data[PW_i+2] == 'd') || (in_data[PW_i+2] == 'D')) &&
             (in_data[PW_i+3] == '3') )
        {
          if ( testPMD3() != BAD ) /* yep, same tests apply */
	      {
            Rip_PMD3 ();
            Depack_PMD3 ();
		  
		  USE_MATCH
          }
        }

          /* POLKA Packer */
		  // UADE does not play some of the songs (e.g. Polka Brothers - The Prey - Ultimate Seduction 3.ppk)
        if ( ((in_data[PW_i+1] == 'W') &&
             (in_data[PW_i+2] == 'R') &&
             (in_data[PW_i+3] == '.')) ||
	     ((in_data[PW_i+1] == 'S') &&
             (in_data[PW_i+2] == 'U') &&
	     (in_data[PW_i+3] == 'X')))
        {
          if ( testPolka() != BAD )
          {
            Rip_Polka ();
            Depack_Polka ();
		  
		  USE_MATCH 
          }
        }

          /* PERFSONG Packer */
        if ( (in_data[PW_i+1] == 'E') &&
             (in_data[PW_i+2] == 'R') &&
             (in_data[PW_i+3] == 'F') &&
             (in_data[PW_i+4] == 'S') &&
             (in_data[PW_i+5] == 'O') &&
             (in_data[PW_i+6] == 'N') &&
             (in_data[PW_i+7] == 'G'))
        {
          if ( testPERFSONG() != BAD )
          {
            Rip_PERFSONG ();
            Depack_PERFSONG ();
		  
		  USE_MATCH
          }
        }
          /* PERFSNG2 Packer */
        if ( (in_data[PW_i+1] == 'E') &&
             (in_data[PW_i+2] == 'R') &&
             (in_data[PW_i+3] == 'F') &&
             (in_data[PW_i+4] == 'S') &&
             (in_data[PW_i+5] == 'N') &&
             (in_data[PW_i+6] == 'G') &&
             (in_data[PW_i+7] == '2'))
        {
          if ( testPERFSNG2() != BAD )
          {
            Rip_PERFSNG2 ();
            Depack_PERFSNG2 ();
		  
		  USE_MATCH
          }
        }
        break;

      case 'S':  /* 0x53 */
          /* STIM Slamtilt */
        if ( (in_data[PW_i+1] == 'T') &&
             (in_data[PW_i+2] == 'I') &&
             (in_data[PW_i+3] == 'M') )
        {
          if ( testSTIM() != BAD )
          {
            Rip_STIM ();
            Depack_STIM ();
		  
		  USE_MATCH
          }
        }

          /* SONG Fuchs Tracker / Sound Fx 1.* */
		  // UADE this is a keeper
        if ( (in_data[PW_i+1] == 'O') &&
             (in_data[PW_i+2] == 'N') &&
             (in_data[PW_i+3] == 'G') )
        {
          if ( testFuchsTracker() != BAD )
          {
            Rip_FuchsTracker ();
            Depack_FuchsTracker ();
		  
		  USE_MATCH
          }
        }
        break;

	  case 'T':

	  /* "TMK. Timetracker ?!? */
        if ( ( in_data[PW_i+1] == 'M' ) &&
             ( in_data[PW_i+2] == 'K' ) &&
             ( in_data[PW_i+3] == 0x01 ) )
        {
          if ( testTMK() != BAD )
          {
            Rip_TMK ();
            Depack_TMK ();
		  
		  USE_MATCH
          }
        }
        break;
		
      case 'U': /* "UNIC" */
        if ( ( in_data[PW_i+1] == 'N' ) &&
             ( in_data[PW_i+2] == 'I' ) &&
             ( in_data[PW_i+3] == 'C' ) )
        {
          /* Unic tracker v1 ? */
          if ( testUNIC_withID() != BAD )
          {
            Rip_UNIC_withID ();
            Depack_UNIC ();
		  
		  USE_MATCH 
          }
        }
        break;

      case 'W': /* 0x57 */
          /* "WN" Wanton Packer */
		  // this is a keeper
        if ( (in_data[PW_i+1] == 'N') &&
             (in_data[PW_i+2] == 0x00 ) )
        {
          if ( testWN() != BAD )
          {
            Rip_WN ();
            Depack_WN ();
		  
		  USE_MATCH
          }
        }
        break;

      case 0x60:
			// UADE keep "unused" branches in case there are dependencies 
	  
          /* promizer 1.8a ? */
        if ( (in_data[PW_i+1] == 0x38) &&
             (in_data[PW_i+2] == 0x60) &&
             (in_data[PW_i+3] == 0x00) &&
             (in_data[PW_i+4] == 0x00) &&
             (in_data[PW_i+5] == 0xa0) &&
             (in_data[PW_i+6] == 0x60) &&
             (in_data[PW_i+7] == 0x00) &&
             (in_data[PW_i+8] == 0x01) &&
             (in_data[PW_i+9] == 0x3e) &&
             (in_data[PW_i+10]== 0x60) &&
             (in_data[PW_i+11]== 0x00) &&
             (in_data[PW_i+12]== 0x01) &&
             (in_data[PW_i+13]== 0x0c) &&
             (in_data[PW_i+14]== 0x48) &&
             (in_data[PW_i+15]== 0xe7) )   /* gosh !, should be enough :) */
        {
          if ( testPMZ() != BAD )
          {
            Rip_PM18a ();
            Depack_PM18a ();
		  
		  CANCEL_SUPPORTED_FORMAT 
          }

          /* Promizer 1.0c */
          if ( testPM10c() != BAD )
          {
            Rip_PM10c ();
            Depack_PM10c ();
		  
		  CANCEL_SUPPORTED_FORMAT 
          }

          /* Promizer 1.0b */
		  // UADE keeper - old UADE doesn't support this
          if ( testPM10b() != BAD )
          {
            Rip_PM10b ();
            Depack_PM10b ();
		  USE_MATCH 
          }
        }

        break;

      default: /* do nothing ... save continuing :) */
        break;

    } /* end of switch */
  }
end:
  free ( in_data );

	UseConvertedResult = 0;
  
	// UADE
	if (cancel) {
		// keep those formats that are already handled (better) in UADE
		CONVERT = Save_Status = BAD;
		// fprintf (stdout, "using original '%s' input\n" , LastFormatName);
	} 
	else {
		if (Save_Status == GOOD) {
			
			if ( CONVERT == GOOD ) {
				fprintf (stdout, "Pro-Wizard v1.71 used to convert\n" );
				fprintf (stdout, "  %s to Protracker\n", LastFormatName);
				
				UseConvertedResult = 1;
			}
			else {
				// failed to detect format: the detection logic seesm to be quite flawed, 
				// e.g. "Richard Joseph" format files are "detected" to be 2 byte long
				// "Future Composer 1.3" files. These are actually saved as "0.FC13" 
				// before the conversion logic eventually fails.. (etc)

				LastFormatName = 0;	// whatever the logic thought it had detected was garbage
			}
		}		
	}

}
