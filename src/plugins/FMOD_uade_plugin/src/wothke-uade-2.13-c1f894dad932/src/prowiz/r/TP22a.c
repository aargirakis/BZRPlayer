/* tests sone in ThePlayer40.c */
/* Rip_P22A() */
/* Depack_P22() */


#include "globals.h"
#include "extern.h"

// UADE; the "P22A" filemagic based check seems sufficiently robust to 
// avoid false positives..


void Rip_P22A ( void )
{
  /* PW_k is the number of sample */

  PW_l = ( (in_data[PW_Start_Address+16]*256*256*256) +
	   (in_data[PW_Start_Address+17]*256*256) +
	   (in_data[PW_Start_Address+18]*256) +
	   in_data[PW_Start_Address+19] );

  /* get whole sample size */
  /* starting from the highest addy and adding the sample size */
  PW_o = 0;
  for ( PW_j=0 ; PW_j<PW_k ; PW_j++ )
  {
    PW_m = ( (in_data[PW_Start_Address+20+PW_j*16]*256*256*256) +
	     (in_data[PW_Start_Address+21+PW_j*16]*256*256) +
	     (in_data[PW_Start_Address+22+PW_j*16]*256) +
	     in_data[PW_Start_Address+23+PW_j*16] );
    if ( PW_m > PW_o )
    {
      PW_o = PW_m;
      PW_n = ( (in_data[PW_Start_Address+24+PW_j*16]*256) +
	       in_data[PW_Start_Address+25+PW_j*16] );
    }
  }

  OutputSize = PW_l + PW_o + (PW_n*2) + 4;

  CONVERT = GOOD;
  Save_Rip ( "The Player 2.2A module", ThePlayer22a );
  
  if ( Save_Status == GOOD )
    PW_i += (OutputSize - 2);  /* 1 should do but call it "just to be sure" :) */
}



/*
 *   The_Player_2.2.c   2003 (c) Asle / ReDoX
 *
 * The Player 2.2a to Protracker.
 *
 * Update : 26 nov 2003
 *   - used htonl() so that use of addy is now portable on 68k archs
*/

void Depack_P22 ( void )
{
  uint8_t c1,c2,c3,c4;
  uint8_t *Whatever;
  uint8_t PatPos = 0x00;
  uint8_t Nbr_Sample = 0x00;
  uint8_t poss[37][2];
  uint8_t sample,note,Note[2];
  uint8_t Pattern_Data[128][1024];
  int16_t	 Pattern_Addresses[128];
  int32_t	 Track_Data_Address = 0;
  int32_t	 Track_Table_Address = 0;
  int32_t	 Sample_Data_Address = 0;
  int32_t	 WholeSampleSize = 0;
  int32_t	 SampleAddress[31];
  int32_t	 SampleSize[31];
  int32_t	 i=0,j,k,l;
  int32_t	 voice[4];
  int32_t	 Where = PW_Start_Address;
  FILE *out;/*,*debug;*/

  if ( Save_Status == BAD )
    return;

  BZERO ( Pattern_Addresses , 128*2 );
  BZERO ( Pattern_Data , 128*1024 );
  BZERO ( SampleAddress , 31*4 );
  BZERO ( SampleSize , 31*4 );

  fillPTKtable(poss);

  sprintf ( Depacked_OutName , "%d.mod" , Cpt_Filename-1 );
  out = PW_fopen ( Depacked_OutName , "w+b" );
  /*debug = fopen ( "debug", "w+b" );*/

  /* read check ID */
  Where += 4;

  /* bypass Real number of pattern */
  Where += 1;

  /* read number of pattern in pattern list */
  PatPos = (in_data[Where++]/2) - 1;

  /* read number of samples */
  Nbr_Sample = in_data[Where++];

  /* bypass empty byte */
  Where += 1;


/**********/

  /* read track data address */
  Track_Data_Address = (in_data[Where]*256*256*256)+
                       (in_data[Where+1]*256*256)+
                       (in_data[Where+2]*256)+
                        in_data[Where+3];
  Where += 4;

  /* read track table address */
  Track_Table_Address = (in_data[Where]*256*256*256)+
                        (in_data[Where+1]*256*256)+
                        (in_data[Where+2]*256)+
                         in_data[Where+3];
  Where += 4;

  /* read sample data address */
  Sample_Data_Address = (in_data[Where]*256*256*256)+
                        (in_data[Where+1]*256*256)+
                        (in_data[Where+2]*256)+
                         in_data[Where+3];
  Where += 4;


  /* write title */
  Whatever = (uint8_t *) malloc ( 1024 );
  BZERO ( Whatever , 1024 );
  fwrite ( Whatever , 20 , 1 , out );

  /* sample headers stuff */
  for ( i=0 ; i<Nbr_Sample ; i++ )
  {
    /* read sample data address */
    j = (in_data[Where]*256*256*256)+
        (in_data[Where+1]*256*256)+
        (in_data[Where+2]*256)+
         in_data[Where+3];
    SampleAddress[i] = j;

    /* write sample name */
    fwrite ( Whatever , 22 , 1 , out );

    /* read sample size */
    SampleSize[i] = ((in_data[Where+4]*256)+in_data[Where+5])*2;
    WholeSampleSize += SampleSize[i];

    /* loop start */
    k = (in_data[Where+6]*256*256*256)+
        (in_data[Where+7]*256*256)+
        (in_data[Where+8]*256)+
         in_data[Where+9];

    /* writing now */
    fwrite ( &in_data[Where+4] , 2 , 1 , out );
    c1 = ((in_data[Where+12]*256)+in_data[Where+13])/74;
    fwrite ( &c1 , 1 , 1 , out );
    fwrite ( &in_data[Where+15] , 1 , 1 , out );
    k -= j;
    k /= 2;
    /* use of htonl() suggested by Xigh !.*/
    l = htonl(k);
    c1 = *((uint8_t *)&l+2);
    c2 = *((uint8_t *)&l+3);
    fwrite ( &c1 , 1 , 1 , out );
    fwrite ( &c2 , 1 , 1 , out );
    fwrite ( &in_data[Where+10] , 2 , 1 , out );

    Where += 16;
  }

  /* go up to 31 samples */
  Whatever[29] = 0x01;
  while ( i != 31 )
  {
    fwrite ( Whatever , 30 , 1 , out );
    i += 1;
  }

  /* write size of pattern list */
  fwrite ( &PatPos , 1 , 1 , out );

  /* write noisetracker byte */
  c1 = 0x7f;
  fwrite ( &c1 , 1 , 1 , out );

  /* place file pointer at the pattern list address ... should be */
  /* useless, but then ... */
  Where = PW_Start_Address + Track_Table_Address + 4;

  /* create and write pattern list .. no optimization ! */
  /* I'll optimize when I'll feel in the mood */
  for ( c1=0x00 ; c1<PatPos ; c1++ )
  {
    fwrite ( &c1 , 1 , 1 , out );
  }
  c2 = 0x00;
  while ( c1<128 )
  {
    fwrite ( &c2 , 1 , 1 , out );
    c1 += 0x01;
  }

  /* write ptk's ID */
  Whatever[0] = 'M';
  Whatever[1] = '.';
  Whatever[2] = 'K';
  Whatever[3] = '.';
  fwrite ( Whatever , 4 , 1 , out );

  /* reading all the track addresses .. which seem to be pattern addys ... */
  for ( i=0 ; i<PatPos ; i++ )
  {
    Pattern_Addresses[i] = (in_data[Where]*256) + in_data[Where+1] + Track_Data_Address+4;
    Where += 2;
  }


  /* rewrite the track data */
  /*printf ( "sorting and depacking tracks data ... " );*/
  for ( i=0 ; i<PatPos ; i++ )
  {
    /*fprintf (debug,"---------------------\nPattern %ld\n",i);*/
    Where = PW_Start_Address + Pattern_Addresses[i];
    voice[0] = voice[1] = voice[2] = voice[3] = 0;
    for ( k=0 ; k<64 ; k++ )
    {
      for ( j=0; j<4 ; j++ )
      {
	if ( voice[j] > k )
	  continue;
	
	c1 = in_data[Where++];
	c2 = in_data[Where++];
	c3 = in_data[Where++];
	c4 = in_data[Where++];

	/*fprintf (debug,"[%2ld][%2d][%4x] - %2x,%2x,%2x,%2x -> ",j,voice[j],Where-4,c1,c2,c3,c4);*/

        sample = ((c1<<4)&0x10) | ((c2>>4)&0x0f);
	BZERO ( Note , 2 );
	note = c1 & 0x7f;
	Note[0] = poss[(note/2)][0];
	Note[1] = poss[(note/2)][1];
	switch ( c2&0x0f )
	{
	  case 0x08:
	    c2 -= 0x08;
	    break;
	  case 0x05:
	  case 0x06:
	  case 0x0A:
	    c3 = (c3 > 0x7f) ? ((0x100-c3)<<4) : c3;
	    break;
	  default:
	    break;
	}
	Pattern_Data[i][voice[j]*16+j*4]   = (sample&0xf0) | (Note[0]&0x0f);
	Pattern_Data[i][voice[j]*16+j*4+1] = Note[1];
	Pattern_Data[i][voice[j]*16+j*4+2] = c2;
	Pattern_Data[i][voice[j]*16+j*4+3] = c3;
	
	/*fprintf ( debug, "%2x,%2x,%2x,%2x",Pattern_Data[i][voice[j]*16+j*4],Pattern_Data[i][voice[j]*16+j*4+1],Pattern_Data[i][voice[j]*16+j*4+2],Pattern_Data[i][voice[j]*16+j*4+3]);*/

        if ( (c4 > 0x00) && (c4 <0x80) )
	{
	  voice[j] += c4;
	  /*fprintf ( debug, "  <-- %d empty lines",c4 );*/
	}
	/*fprintf ( debug, "\n" );*/
        voice[j] += 1; 
      } /* end of case 0x80 for first byte */
    }
  }
  /*  printf ( "ok\n" );*/



  /* write pattern data */
  /*printf ( "writing pattern data ... " );*/
  /*fflush ( stdout );*/
  for ( i=0 ; i<PatPos ; i++ )
  {
    fwrite ( Pattern_Data[i] , 1024 , 1 , out );
  }
  free ( Whatever );
  /*printf ( "ok\n" );*/


  /* read and write sample data */
  /*printf ( "writing sample data ... " );*/
  for ( i=0 ; i<Nbr_Sample ; i++ )
  {
    Where = PW_Start_Address + SampleAddress[i]+Sample_Data_Address;
    fwrite ( &in_data[Where] , SampleSize[i] , 1 , out );
  }
  /*printf ( "ok\n" );*/

  Crap ( " The Player 2.2A  " , BAD , BAD , out );

  fflush ( out );
  fclose ( out );
  /*  fclose ( debug );*/

  printf ( "done\n" );
  return; /* useless ... but */
}