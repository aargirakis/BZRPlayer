/* testChrysalis() */
/* Rip_Chrysalis() */
/* Depack_Chrysalis() */

#include "globals.h"
#include "extern.h"


int16_t	 testChrysalis ( void ) [] /* TODO */
{
  /* test 1 */
  if ( PW_i < 1080 )
  {
    /*printf ( "#1 (PW_i:%ld)\n" , PW_i );*/
    return BAD;
  }

  /* test 2 */
  PW_Start_Address = PW_i-1080;
  PW_WholeSampleSize = 0;
  for ( PW_k=0 ; PW_k<31 ; PW_k++ )
  {
    /* size */
    PW_j = (((in_data[PW_Start_Address+42+PW_k*30]*256)+in_data[PW_Start_Address+43+PW_k*30])*2);
    /* loop start */
    PW_m = (((in_data[PW_Start_Address+46+PW_k*30]*256)+in_data[PW_Start_Address+47+PW_k*30])*2);
    /* loop size */
    PW_n = (((in_data[PW_Start_Address+48+PW_k*30]*256)+in_data[PW_Start_Address+49+PW_k*30])*2);

    if ( test_smps(PW_j*2, PW_m, PW_n, in_data[PW_Start_Address+45+30*PW_k], in_data[PW_Start_Address+44+30*PW_k] ) == BAD )
    {
/*      printf ( "[devils.c]test2 : %d\n", PW_Start_Address );*/
      return BAD; 
    }

    PW_WholeSampleSize += PW_j;
  }

  /* test 3 if pattern data isn't out of file */
  PW_j = (in_data[PW_Start_Address+1084]*256)+in_data[PW_Start_Address+1084 + 1];
  if ( (PW_Start_Address + PW_j + 2 + 1084) > PW_in_size )
  {
/*    printf ( "[devils.c]test3 : %d\n", PW_Start_Address );*/
    return BAD;
  }
  /* test 3a - there's at least one pattern of data ? */
  if (PW_j <192)
  {
/*    printf ( "[devils.c]test3a : %d\n", PW_Start_Address );*/
    return BAD;
  }

  /* test 4 if reference table isn't out of file */
  PW_k = (in_data[PW_Start_Address+1084 + 2 + PW_j]*256)+in_data[PW_Start_Address+1084 + 2 + PW_j +1];
  if ( (PW_Start_Address + PW_j + PW_k + 2 + 1084 + 2) > PW_in_size )
  {
/*    printf ( "[devils.c]test4 : %d\n", PW_Start_Address );*/
    return BAD;
  }
/*
printf ("PW_j:%4x\n",PW_j);
printf ("PW_k:%4x\n",PW_k);
printf ("PW_WholeSampleSize:%d\n",PW_WholeSampleSize);
*/
  return GOOD;
}



void Rip_Chrysalis ( void ) /* TODO */
{
  /* PW_j is the pattern data size */
  /* PW_k is the reference table size */
  /* PW_WholeSampleSize is the whole sample size */

  OutputSize = 1084 + 2 + PW_j + 2 + PW_k + PW_WholeSampleSize;

  CONVERT = GOOD;
  Save_Rip ( "   Devils' replay   ", Devils );
  
  if ( Save_Status == GOOD )
    PW_i += 2;
}



/*
 *   Chrysalis Replay.c   2021-02 (c) Asle
 *
 * Converts packed MODs back to PTK MODs
 *
 * Source material is found in the following demo
 * - Alarm intro by Crysalis
 
.w number of sample header [A]
---8 bytes per sample desc
.w size /2
.b fine ?
.b vol [0-40h]
.w loop start (/2 ?)
.w loop end /2
---


pattern data:
192 bytes per pattern
3 bytes per row:
 0 byte : filter
 1 short : address to read in reference table

Reference table:
contains rows of output unpacked notes - so up to 16 bytes.
depending of the filter, above, the blocks of word/short are stored
filter : abcd-efgh 
output: in 16 bytes
B1 B2 B3 B4 B5 B6 B7 B8 B8 B10 B11 B12 B13 B14 B15 B16
|  |  |  |  |  |  |  |  |  |   |   |   |   |   |   |
 \/    \/    \/    \/    \/     \ /     \ /     \ / 
  a     b     c     d     e      f       g       h
if the filter bit is set, the corresponding output is stored in the reference table.

*/


void Depack_Chrysalis ( void )
{
  uint8_t *Whatever;
  uint8_t nbrpattern=0x00; /* number of pattern */
  uint8_t *referencetable;
  uint8_t *patternsource;

  int32_t i=0,j=0,k=0, l=0;
  int32_t wholesamplesize=0;
  int32_t patternsourcesize=0;
  int32_t referencetablesize=0;
  int32_t Where;
  FILE *out;

  if ( Save_Status == BAD )
    return;

  sprintf ( Depacked_OutName , "%d.mod" , Cpt_Filename-1 );
  out = PW_fopen ( Depacked_OutName , "w+b" );

  /* set the input pointer to the beginning */
  Where = PW_Start_Address;

  /* write header as it is identical */
  fwrite (&in_data[Where], 1084, 1, out);fflush(out);
printf ("\n[devils.c] Header written\n");fflush(stdout);

  /* get full sample data size */
  for ( i=0 ; i<31 ; i++ )
  {
    wholesamplesize += (((in_data[Where+42+30*i]*256)+in_data[Where+42+30*i+1])*2);
  }
  Where += 952; /* go to patternlist */
  /* fetch the highest pattern number */
  for ( i=0 ; i<128 ; i++ )
  {
    if (in_data[Where+i] > nbrpattern)
      nbrpattern = in_data[Where+i];
  }
  nbrpattern += 0x01; /* because first pattern is 0 */
printf ("[devils.c] nbrpattern : %d\n",nbrpattern);fflush(stdout);

  /* prepare pattern output */
  Whatever = (uint8_t *) malloc (1024*nbrpattern);
  if (!Whatever){printf ("[devils.c]memalloc error\n");return;}
  BZERO (Whatever, 1024*nbrpattern);
printf ("[devils.c] allocated : %d\n",1024*nbrpattern);fflush(stdout);

  /* fetch input pattern data size */
  Where += 132; /*bypass pattern list and M.K.*/
  patternsourcesize = (in_data[Where]*256) + in_data[Where+1];
printf ("[devils.c] patternsourcesize: %d (%02x)\n",patternsourcesize,patternsourcesize);
  Where += 2;
  /* grab pattern data and put that somewhere convenient */
  patternsource = (uint8_t *) malloc (patternsourcesize);
  if (!patternsource){printf ("[devils.c]memalloc error\n");return;}
  BZERO (patternsource, patternsourcesize);
  for (i=0;i<patternsourcesize;i+=1)
    patternsource[i] = in_data[Where+i];

  /* fetch reference table size */
  Where += patternsourcesize;
  referencetablesize = (in_data[Where]*256)+in_data[Where+1];
printf ("[devils.c] referencetablesize: %d (%02x)\n",referencetablesize,referencetablesize);
  Where += 2;
  /* grab reference table and put that somewhere convenient */
  referencetable = (uint8_t *) malloc (referencetablesize);
  if (!referencetable){printf ("[devils.c]memalloc error\n");return;}
  BZERO (referencetable,referencetablesize);
  for (i=0;i<referencetablesize;i+=1)
    referencetable[i] = in_data[Where+i];

  /* go point on the sample data now */
  Where += referencetablesize;


  /*
  at this point, we need to recreate the output pattern data
  patternsource : source pattern data
  referencetable : actual notes
  Whatever : output pattern data to be filled up
  nbrpattern : number of pattern stored
  */

  /* lets built this output pattern data */
  /* but it must be for a total that is multiple of 3 only - it is stored as an even block */
  l = patternsourcesize;
  if (((l/3)*3) != l)
    l-=1;
  if (((l/3)*3) != l)
    l-=1;
  for ( i=0 ; i<l ; i+=3 )
  {
    k = 0;
    
    if ((((i/3)*16)+15)>(1024*nbrpattern))
    {
/*      printf ("[devils.c] overflow at i : %d (%d > %d)\n",i,((i/3)*16)+15, 1024*nbrpattern);fflush(stdout);*/
      break;
	}
     if (j > referencetablesize)
    {
/*      printf ("[devils.c] overflow at i : %d (%d > %d)\n",i,j, referencetablesize);fflush(stdout);*/
      break;
	}
   
    /* no note */
    if ( patternsource[i] == 0x00)
      continue;

    /* calc ref */
    j = (patternsource[i+1]*256)+patternsource[i+2];

    /* got through all possible value of filter */
    if ( (patternsource[i] & 0x80) == 0x80)
    {
      Whatever[(i/3)*16 + 0]  = referencetable[j+0];
      Whatever[(i/3)*16 + 1]  = referencetable[j+1];
      k += 2;
    }
    if ( (patternsource[i] & 0x40) == 0x40)
    {
      Whatever[(i/3)*16 + 2]  = referencetable[j+0+k];
      Whatever[(i/3)*16 + 3]  = referencetable[j+1+k];
      k += 2;
    }
    if ( (patternsource[i] & 0x20) == 0x20)
    {
      Whatever[(i/3)*16 + 4]  = referencetable[j+0+k];
      Whatever[(i/3)*16 + 5]  = referencetable[j+1+k];
      k += 2;
    }
    if ( (patternsource[i] & 0x10) == 0x10)
    {
      Whatever[(i/3)*16 + 6]  = referencetable[j+0+k];
      Whatever[(i/3)*16 + 7]  = referencetable[j+1+k];
      k += 2;
    }
    if ( (patternsource[i] & 0x08) == 0x08)
    {
      Whatever[(i/3)*16 + 8]  = referencetable[j+0+k];
      Whatever[(i/3)*16 + 9]  = referencetable[j+1+k];
      k += 2;
    }
    if ( (patternsource[i] & 0x04) == 0x04)
    {
      Whatever[(i/3)*16 + 10] = referencetable[j+0+k];
      Whatever[(i/3)*16 + 11] = referencetable[j+1+k];
      k += 2;
    }
    if ( (patternsource[i] & 0x02) == 0x02)
    {
      Whatever[(i/3)*16 + 12] = referencetable[j+0+k];
      Whatever[(i/3)*16 + 13] = referencetable[j+1+k];
      k += 2;
    }
    if ( (patternsource[i] & 0x01) == 0x01)
    {
      Whatever[(i/3)*16 + 14] = referencetable[j+0+k];
      Whatever[(i/3)*16 + 15] = referencetable[j+1+k];
      /*k += 2;*/
    }
/*    printf ("%d/%d\n",i,patternsourcesize);fflush(stdout);*/
  }
  free ( referencetable );
  free ( patternsource );

  /* OK, let write the remaining stuff */
  /* pattern data */
  fwrite ( Whatever, 1024*nbrpattern, 1, out );fflush(out);
printf ("[devils.c] pattern data written (1024x%d))\n",nbrpattern);fflush(stdout);
  free ( Whatever );

  /* smp data */
  fwrite ( &in_data[Where] , wholesamplesize , 1 , out );fflush(out);
printf ("[devils.c] sample data written (%d))\n",wholesamplesize);fflush(stdout);

/*  Crap ( "  Devils Replay   " , BAD , BAD , out );*/

  fflush ( out );
  fclose ( out );

  printf ( "done\n" );
  return; /* useless ... but */
}
