/* testSLAM() */
/* Rip_SLAM() */
/* Depack_SLAM() */


/* First shot : 20070831 */

#include "globals.h"
#include "extern.h"


int16_t	 testSLAM ( void )
{
  PW_Start_Address = PW_i;

  /* address of sample sizes */
  PW_j = ((in_data[PW_Start_Address+40]*256*256*256)+
	  (in_data[PW_Start_Address+41]*256*256)+
	  (in_data[PW_Start_Address+42]*256)+
	  in_data[PW_Start_Address+43]);
  if ( PW_j < 406 )
  {
    return BAD;
  }

  /* size of file */
  PW_n = ((in_data[PW_Start_Address+20]*256*256*256)+
	  (in_data[PW_Start_Address+21]*256*256)+
	  (in_data[PW_Start_Address+22]*256)+
	  in_data[PW_Start_Address+23]) + 0x20;
  if ( PW_j < 406 )
  {
    return BAD;
  }
/************  PAS FAIT APRES ! */
  /* size of the pattern list */
  PW_k = ((in_data[PW_Start_Address+18]*256)+
	  in_data[PW_Start_Address+19]);
  if ( PW_k > 128 )
  {
    return BAD;
  }

  /* nbr of pattern saved */
  PW_k = ((in_data[PW_Start_Address+20]*256)+
	  in_data[PW_Start_Address+21]);
  if ( (PW_k > 64) || (PW_k == 0) )
  {
    return BAD;
  }

  /* pattern list */
  for ( PW_l=0 ; PW_l<128 ; PW_l++ )
  {
    if ( in_data[PW_Start_Address+22+PW_l] > PW_k )
    {
      return BAD;
    }
  }

  /* test sample sizes */
  PW_WholeSampleSize = 0;
  for ( PW_l=0 ; PW_l<31 ; PW_l++ )
  {
    /* addresse de la table */
    PW_o = PW_Start_Address+PW_j+PW_l*4;

    /* address du sample */
    PW_k = ((in_data[PW_o]*256*256*256)+
	    (in_data[PW_o+1]*256*256)+
	    (in_data[PW_o+2]*256)+
	    in_data[PW_o+3]);

    /* taille du smp */
    PW_m = ((in_data[PW_o+PW_k-PW_l*4]*256)+
	    in_data[PW_o+PW_k+1-PW_l*4])*2;

    PW_WholeSampleSize += PW_m;
  }

  if ( PW_WholeSampleSize <= 4 )
  {
    return BAD;
  }

  /* PW_WholeSampleSize is the size of the sample data */
  /* PW_j is the address of the sample desc */
  return GOOD;
}



void Rip_SLAM ( void )
{
  OutputSize = PW_WholeSampleSize + PW_j + 31*4 + 31*8;

  CONVERT = GOOD;
  Save_Rip ( "SLAM (Slamtilt) module", SLAM );
  
  if ( Save_Status == GOOD )
    PW_i += (OutputSize - 1);  /* 0 should do but call it "just to be sure" :) */
}


/*
 *   SLAM_Packer.c   1998 (c) Sylvain "Asle" Chipaux
 *
 * SLAM Packer to Protracker.
 ********************************************************
 * 13 april 1999 : Update
 *   - no more open() of input file ... so no more fread() !.
 *     It speeds-up the process quite a bit :).
 * 28 Nov 1999 : Update
 *   - Speed & Size optimizings
*/

void Depack_SLAM ( void )
{
  uint8_t *Whatever;
  uint8_t c1=0x00,c2=0x00,c3=0x00,c4=0x00;
  uint8_t poss[36][2];
  uint8_t Max=0x00;
  uint8_t Note,Smp,Fx,FxVal;
  int16_t	 TracksAdd[4];
  int32_t	 i=0,j=0,k=0;
  int32_t	 WholeSampleSize=0;
  int32_t	 SmpDescAdd=0;
  int32_t	 PatAdds[64];
  int32_t	 SmpDataAdds[31];
  int32_t	 SmpSizes[31];
  int32_t	 Where=PW_Start_Address;   /* main pointer to prevent fread() */
  FILE *out;

  if ( Save_Status == BAD )
    return;

  fillPTKtable(poss);

  BZERO ( PatAdds , 64*4 );
  BZERO ( SmpDataAdds , 31*4 );
  BZERO ( SmpSizes , 31*4 );

  sprintf ( Depacked_OutName , "%d.mod" , Cpt_Filename-1 );
  out = PW_fopen ( Depacked_OutName , "w+b" );

  /* write title */
  Whatever = (uint8_t *) malloc (1024);
  BZERO ( Whatever , 1024 );
  fwrite ( Whatever , 20 , 1 , out );

  /* bypass ID */
  Where += 4;

  /* read $ of sample description */
  SmpDescAdd = (in_data[Where]*256*256*256)+
               (in_data[Where+1]*256*256)+
               (in_data[Where+2]*256)+
                in_data[Where+3];
  /* "Where" isn't "+=4" coz it's assigned below */
  /*printf ( "SmpDescAdd : %ld\n" , SmpDescAdd );*/

  /* convert and write header */
  for ( i=0 ; i<31 ; i++ )
  {
    Where = PW_Start_Address + SmpDescAdd + i*4;
    SmpDataAdds[i]=(in_data[Where]*256*256*256)+
                   (in_data[Where+1]*256*256)+
                   (in_data[Where+2]*256)+
                    in_data[Where+3];
    SmpDataAdds[i] += SmpDescAdd;
    Where = PW_Start_Address + SmpDataAdds[i];
    SmpDataAdds[i] += 8;

    /* write sample name */
    fwrite ( Whatever , 22 , 1 , out );

    /* sample size */
    SmpSizes[i] = (((in_data[Where]*256)+in_data[Where+1])*2);
    WholeSampleSize += (((in_data[Where]*256)+in_data[Where+1])*2);
    /* size,fine,vol,loops */
    fwrite ( &in_data[Where] , 8 , 1 , out );

    /* no "Where += 8" coz it's reassigned inside and after loop */
  }

  /* size of the pattern list */
  Where = PW_Start_Address + 19;
  fwrite ( &in_data[Where++] , 1 , 1 , out );
  Whatever[0] = 0x7f;
  fwrite ( Whatever , 1 , 1 , out );

  /* pattern table */
  Where += 1;
  Max = in_data[Where++];
  fwrite ( &in_data[Where] , 128 , 1 , out );
  Where += 128;

  /*printf ( "number of pattern : %d\n" , Max );*/

  /* write Protracker's ID */
  Whatever[0] = 'M';
  Whatever[1] = '.';
  Whatever[2] = 'K';
  Whatever[3] = '.';
  fwrite ( Whatever , 4 , 1 , out );

  /* read pattern addresses */
  for ( i=0 ; i<64 ; i++ )
  {
    PatAdds[i] = (in_data[Where]*256*256*256)+
                 (in_data[Where+1]*256*256)+
                 (in_data[Where+2]*256)+
                  in_data[Where+3];
    PatAdds[i] += 0x0c;
    Where += 4;
  }

  /* pattern data */
  for ( i=0 ; i<Max ; i++ )
  {
    Where = PW_Start_Address + PatAdds[i];
    for ( k=0 ; k<4 ; k++ )
    {
      TracksAdd[k] = (in_data[Where]*256)+in_data[Where+1];
      Where += 2;
    }

    BZERO ( Whatever , 1024 );
    for ( k=0 ; k<4 ; k++ )
    {
      Where = PW_Start_Address + PatAdds[i]+TracksAdd[k];
      for ( j=0 ; j<64 ; j++ )
      {
        c1 = in_data[Where++];
	if ( (c1&0x80) == 0x80 )
	{
	  j += (c1&0x7F);
	  continue;
	}
        c2 = in_data[Where++];
        c3 = in_data[Where++];

	Smp  = c1&0x1F;
	Note = c2&0x3F;
	Fx   = ((c1>>5)&0x03);
        c4   = ((c2>>4)&0x0C);
        Fx   |= c4;
	FxVal = c3;

	Whatever[j*16+k*4] = (Smp & 0xf0);

        if ( Note != 0 )
        {
          Whatever[j*16+k*4] |= poss[Note-1][0];
          Whatever[j*16+k*4+1] = poss[Note-1][1];
        }

	Whatever[j*16+k*4+2] = ((Smp<<4)&0xf0);
	Whatever[j*16+k*4+2] |= Fx;
	Whatever[j*16+k*4+3] = FxVal;
      }
    }
    fwrite ( Whatever , 1024 , 1 , out );
/*    printf ( "pattern %ld written\n" , i );*/
  }
  free ( Whatever );

  /* sample data */
  for ( i=0 ; i<31 ; i++ )
  {
    Where = PW_Start_Address + SmpDataAdds[i];
    fwrite ( &in_data[Where] , SmpSizes[i] , 1 , out );
  }


  Crap ( " SLAM (Slamtilt)  " , BAD , BAD , out );

  fflush ( out );
  fclose ( out );

  printf ( "done\n" );
  return; /* useless ... but */
}
