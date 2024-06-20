/* testS3M() */
/* Rip_S3M() */


#include "globals.h"
#include "extern.h"


int16_t	 testS3M ( void )
{
  /* test #1 */
  PW_Start_Address = PW_i-44;
  if ( (PW_Start_Address + 0x60) > PW_in_size)
  {
    /*printf ( "#1 Start:%ld\n" , PW_Start_Address );*/
    return BAD;
  }
  /* must be 0x1a10 */
  if ( (in_data[PW_Start_Address + 28] != 0x1A) || (in_data[PW_Start_Address + 29] != 0x10) )
  {
    /*printf ( "#2 Start:%ld\n" , PW_Start_Address );*/
    return BAD;
  }
  /* must be 0x0100 or 0x0200 */
  if ( ((in_data[PW_Start_Address + 42] != 0x01) && (in_data[PW_Start_Address + 42] != 0x02)) || (in_data[PW_Start_Address + 43] != 0x00) )
  {
    /*printf ( "#3 Start:%ld\n" , PW_Start_Address );*/
    return BAD;
  }

  /* get patternlist size */
  PW_j = ( (in_data[PW_Start_Address+33]*256)+
	   in_data[PW_Start_Address+32] );
/* test uneven patternlist size */  
/*if ( ((PW_j/2)*2) != PW_j )*/
/*  {*/
    /*printf ( "#4 Start:%ld\n" , PW_Start_Address );*/
    /* unreal musics with uneven patterlist size ?!? */
    /*return BAD;*/
/*  }*/

  /* get number of instruments */
  PW_k = ( (in_data[PW_Start_Address+35]*256)+
	   in_data[PW_Start_Address+34] );
  /* get number of pattern stored */
  PW_l = ( (in_data[PW_Start_Address+37]*256)+
	   in_data[PW_Start_Address+36] );

  /* PW_j is the patternlist size */
  /* PW_k is the number of instruments */
  /* PW_l is the number of pattern stored */
  return GOOD;
}


/*
 * S3M ripper
 * 20100207 - Sylvain "Asle" Chipaux
 *
 * pfiew - finaly completed. some "features" not documented :(
*/
void Rip_S3M ( void )
{
  int32_t	 currentptr;
  int32_t	 max = 0;
  /*int32_t	 lastnonzerosize = 0;*/
  /*int32_t	 gap;*/
  /*int32_t whole_inst_size=0;*/
  int32_t whole_head_size=0;

  /* read the pattern adresses and get sizes */
  currentptr = PW_Start_Address + 0x60 + PW_j + (PW_k*2);
  max = 0;
  for ( PW_o=0 ; PW_o<PW_l ; PW_o++ )
  {
    int32_t	 tmp_addy=0;
    tmp_addy = ((in_data[currentptr+(PW_o*2)+1]*256) + in_data[currentptr+(PW_o*2)])*16;
    /* + PW_Start_Address;*/
    if (tmp_addy == 0)
      continue;
printf ("[PAT]][%02d][@%02X-%02X][addy:%04X]\n",PW_o,in_data[currentptr+(PW_o*2)+1], in_data[currentptr+(PW_o*2)], tmp_addy);
    if ( tmp_addy > max )
    {
      max = tmp_addy;
      whole_head_size = max + ((in_data[tmp_addy+1+ PW_Start_Address]*256)+in_data[tmp_addy+ PW_Start_Address]);
    }
    /*printf ("[%ld] tmp_addy : %lx,(%lx)\n",PW_o,tmp_addy,(in_data[tmp_addy+1]*256)+in_data[tmp_addy]);*/
  }
  while (((whole_head_size/16)*16) != whole_head_size)
  {
    whole_head_size += 1;
  }


  /* read the instruments adresses and get sizes */
  /* they should be _before_ the pattern data, handled above */
  currentptr = PW_Start_Address + 0x60 + PW_j;
  max=0;
  for ( PW_o=0 ; PW_o<PW_k ; PW_o++ )
  {
    int32_t tmp_addy=0, tmp_addy2=0;
/*    int32_t	 tmp = 0;*/
    tmp_addy = ((in_data[currentptr+(PW_o*2)+1]*256) + in_data[currentptr+(PW_o*2)])*16;
    /* + PW_Start_Address;*/
    if (tmp_addy == 0)
      continue;
printf ("[SMP]][%02d][@%02X-%02X][addy:%04X]",PW_o,in_data[currentptr+(PW_o*2)+1], in_data[currentptr+(PW_o*2)], tmp_addy);
    if (in_data[tmp_addy+ PW_Start_Address] == 1)
    {
      /* get sample size */
      int32_t t = ((in_data[tmp_addy+18+ PW_Start_Address]*256*256)+(in_data[tmp_addy+17+ PW_Start_Address]*256)+in_data[tmp_addy+16+ PW_Start_Address]);

      /* get address of sample data in file */
      tmp_addy2 = ((in_data[tmp_addy+15+ PW_Start_Address]*256)+in_data[tmp_addy+14+ PW_Start_Address]+(in_data[tmp_addy+13+ PW_Start_Address]*256*256))*16;
      printf ("[pos:%05X]",tmp_addy2);
      if ((tmp_addy2 > max) && (t!=0))
      {
        if ((in_data[tmp_addy+31]&0x04)==0x04) /* 16 bit ? */
          t *= 2;
        if ((in_data[tmp_addy+31]&0x02)==0x02) /* stereo ? */
          t *= 2;
        printf ("[pos:%05X]",t);
        max = tmp_addy2;
        whole_head_size = max + t;
      }
    }
    else
      printf ("[pos:-][siz:-]");
printf ("\n");
  }

  OutputSize = whole_head_size;
  printf ("\nwhole_head_size : %d\n"
         "PW_j : %x\n"
         "PW_k*2 : %x\n"
         "PW_l*2 : %x\n"
         ,whole_head_size, PW_j, (PW_k*2), (PW_l*2));

  CONVERT = BAD;
  Save_Rip ( "ScreamTracker 3 module", S3M );
  
  if ( Save_Status == GOOD )
    PW_i += 1;
}
