/* testWN() */
/* Rip_WN() */
/* Depack_WN() */

#include "globals.h"
#include "extern.h"

static unsigned int wanto[] = {	// UADE
	0x1e8d72eb,
	0x3449fad5,
	0x37c05d4e,
	0x50166c71,
	0x5533fd04,
	0x801148c6,
	0x99ea0fe1,
	0x9de85d00,
	0xa685d4fe,
	0xd51c96d0,
	0xe02df2ff,
	0
};

int16_t	 testWN ( void )
{
  // UADE: failsave needed to avoid false positives
  if(!checkcrc(wanto)) return BAD;	// UADE
  
  /* test 1 */
  if ( PW_i < 1080 )
  {
    return BAD;
  }

  /* test 2 */
  PW_Start_Address = PW_i-1080;
  if ( in_data[PW_Start_Address+951] != 0x7f )
  {
    return BAD;
  }

  /* test 3 */
  if ( in_data[PW_Start_Address+950] > 0x7f )
  {
    return BAD;
  }

  return GOOD;
}



void Rip_WN ( void )
{
  PW_WholeSampleSize = 0;
  for ( PW_k=0 ; PW_k<31 ; PW_k++ )
    PW_WholeSampleSize += (((in_data[PW_Start_Address+42+30*PW_k]*256)+in_data[PW_Start_Address+43+30*PW_k])*2);
  PW_j = in_data[PW_Start_Address+1083];
  OutputSize = PW_WholeSampleSize + (PW_j*1024) + 1084;

  CONVERT = GOOD;
  Save_Rip ( "Wanton Packer module", Wanton_packer );
  
  if ( Save_Status == GOOD )
    PW_i += (OutputSize - 1082);  /* 1081 should do but call it "just to be sure" :) */
}



/*
 *   Wanton_Packer.c   1997 (c) Asle / ReDoX
 *
 * Converts MODs converted with Wanton packer
 ********************************************************
 * 13 april 1999 : Update
 *   - no more open() of input file ... so no more fread() !.
 *     It speeds-up the process quite a bit :).
 * 28 nov 1999 : Update
 *   - small size optimizing.
*/

void Depack_WN ( void )
{
  uint8_t poss[37][2];
  uint8_t *Whatever;
  int32_t	 WholeSampleSize=0;
  int32_t	 i=0,j=0;
  int32_t	 Where=PW_Start_Address;   /* main pointer to prevent fread() */
  FILE *out;

  fillPTKtable(poss);

  if ( Save_Status == BAD )
    return;

  sprintf ( Depacked_OutName , "%d.mod" , Cpt_Filename-1 );
  out = PW_fopen ( Depacked_OutName , "w+b" );

  /* read header */
  fwrite ( &in_data[Where] , 950 , 1 , out );

  /* get whole sample size */
  for ( i=0 ; i<31 ; i++ )
    WholeSampleSize += (((in_data[Where+42+i*30]*256)+in_data[Where+43+i*30])*2);
/*  printf ( "Whole sample size : %ld\n" , WholeSampleSize );*/

  /* read size of pattern list */
  Where += 950;
  fwrite ( &in_data[Where] , 1 , 1 , out );
  Where += 1;

  i = Where;
  fwrite ( &in_data[Where] , 129 , 1 , out );
  Where += 129;

  /* write ptk's ID */
  Whatever = (uint8_t *) malloc (5);
  Whatever[0] = 'M';
  Whatever[1] = '.';
  Whatever[2] = 'K';
  Whatever[3] = '.';
  fwrite ( Whatever , 4 , 1 , out );

  /* get highest pattern number */
  Whatever[4] = 0x00;
  for ( i=0 ; i<128 ; i++ )
  {
    if ( in_data[PW_Start_Address + 952 + i] > Whatever[4] )
      Whatever[4] = in_data[PW_Start_Address + 952 + i];
  }
/*  printf ( "Max : %d\n" , Whatever[4] );*/

  /* pattern data */
  Where = PW_Start_Address + 1084;
  for ( i=0 ; i<=Whatever[4] ; i++ )
  {
    for ( j=0 ; j<256 ; j++ )
    {
      Whatever[0] = in_data[Where+1] & 0xf0;
      Whatever[0] |= poss[(in_data[Where]/2)][0];
      Whatever[1] = poss[(in_data[Where]/2)][1];
      Whatever[2] = (in_data[Where+1]<<4)&0xf0;
      Whatever[2] |= in_data[Where+2];
      Whatever[3] = in_data[Where+3];

      fwrite ( Whatever , 4 , 1 , out );
      Where += 4;
    }
/*    printf ( "+" );*/
  }
/*  printf ( "\n" );*/
  free ( Whatever );

  /* sample data */
  fwrite ( &in_data[Where] , WholeSampleSize , 1 , out );

  /* crap */
  Crap ( "  Wanton Packer   " , BAD , BAD , out );

  fflush ( out );
  fclose ( out );

  printf ( "done\n" );
  return; /* useless ... but */
}
