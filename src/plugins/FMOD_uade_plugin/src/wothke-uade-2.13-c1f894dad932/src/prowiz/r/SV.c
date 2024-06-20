/* testSV() */
/* Rip_SV() */
/* Depack_SV() */


#include "globals.h"
#include "extern.h"


int16_t	 testSV ( void )
{
  /* test 1 */
  if ( PW_i < 1080 )
  {
    return BAD;
  }

  /* test 2 - ntk byte at 950 for this case. not 951 */
  PW_Start_Address = PW_i-1080;
  if ( (in_data[PW_Start_Address+950] != 0x7f) && (in_data[PW_Start_Address+950] != 0x00) )
  {
/*    printf ( "testSV() - test2 - start : %d\n", PW_Start_Address );*/
    return BAD;
  }

  /* test 3 - pattenlist len at 951 for this case*/
  if ( in_data[PW_Start_Address+951] > 0x7f )
  {
/*    printf ( "testSV() - test3 - start : %d\n", PW_Start_Address );*/
    return BAD;
  }

  /* test 4 */
  PW_WholeSampleSize = 0;
  for ( PW_k=0 ; PW_k<31 ; PW_k++ )
  {
    /* size */
    PW_j = (((in_data[PW_Start_Address+43+PW_k*30]*256)+in_data[PW_Start_Address+42+PW_k*30])*2);
    /* loop start */
    PW_m = (((in_data[PW_Start_Address+47+PW_k*30]*256)+in_data[PW_Start_Address+46+PW_k*30])*2);
    /* loop size */
    PW_n = (((in_data[PW_Start_Address+49+PW_k*30]*256)+in_data[PW_Start_Address+48+PW_k*30])*2);

    if ( test_smps(PW_j*2, PW_m, PW_n, in_data[PW_Start_Address+44+30*PW_k], in_data[PW_Start_Address+45+30*PW_k] ) == BAD )
    {
/*      printf ( "testSV() - test4 - start : %d\n", PW_Start_Address );*/
      return BAD; 
    }

    PW_WholeSampleSize += PW_j;
  }

  return GOOD;
}



void Rip_SV ( void )
{
  PW_l=0;
  for ( PW_k=0 ; PW_k<128 ; PW_k++ )
    if ( in_data[PW_Start_Address+952+PW_k] > PW_l )
      PW_l = in_data[PW_Start_Address+952+PW_k];
  PW_l += 1;
  OutputSize = (PW_l*1024) + 1084 + PW_WholeSampleSize;

  CONVERT = GOOD;
  Save_Rip ( "Software Visions DMF", SoftwareVisionsDMF );
  
  if ( Save_Status == GOOD )
    PW_i += 1;
}



/*
 *   SoftwareVisionsDMF.c   2023 (c) Sylvain Chipaux
 *
 * Converts MODs from the Software Vision catalogue
 * Example cases sent by Lachesis@Demozoo's discord
 *
 * Standard 4ch-MODs with the first 2108 bytes 2bytes-flipped
 * so ABCDEFGH become BADCFEHG
 *
*/
void Depack_SV ( void )
{
  uint8_t *Whatever;
  int32_t i;
  int32_t Where=PW_Start_Address;
  FILE *out;

  if ( Save_Status == BAD )
    return;

  sprintf ( Depacked_OutName , "%d.mod" , Cpt_Filename-1 );
  out = PW_fopen ( Depacked_OutName , "w+b" );

/*
  OutputSize contains already the output size.
  There's only the 2bytes flip to do at this point.
*/
  Whatever = (uint8_t *) malloc (2108);
  BZERO ( Whatever , 2108 );
  for (i=0;i<2108;i+=2)
  {
     Whatever[i] = in_data[Where+i+1];
     Whatever[i+1] = in_data[Where+i];
  }


  /* save that */
  fwrite ( Whatever , 2108 , 1 , out );
  fwrite ( &in_data[Where+2108] , OutputSize-2108 , 1 , out );


  /* crap */
  Crap ( "Software Visions DMF" , BAD , BAD , out );
  free (Whatever);
  fflush ( out );
  fclose ( out );

  printf ( "done\n" );
  return; /* useless ... but */
}

