/* testDelta2() */
/* Rip_Delta2() */

#include "globals.h"
#include "extern.h"

int i,TracksSizes[4],BlocksSizes,InstTableLength[128];

int16_t	 testDM2 ( void )
{  
  /* test 1 */
  PW_Start_Address = PW_i-4;

  if (PW_Start_Address + 0xfda > PW_in_size)
  {
    return BAD;
  }
  
  
  PW_m = PW_Start_Address + 0xbc6;
  
  if ((in_data[PW_m] != '.') || (in_data[PW_m+1] != 'F') || (in_data[PW_m+2] != 'N') || (in_data[PW_m+3] != 'L'))
  {
    /*printf ("#1:no FNL tag at 0xbc6\n");*/
    return BAD;
  }
  
  PW_m = PW_Start_Address + 0xfca;

  TracksSizes[0]=(in_data[PW_m+2]*256)+(in_data[PW_m+3]);	/* get 4 tracks sizes */
  TracksSizes[1]=(in_data[PW_m+6]*256)+(in_data[PW_m+7]);
  TracksSizes[2]=(in_data[PW_m+10]*256)+(in_data[PW_m+11]);
  TracksSizes[3]=(in_data[PW_m+14]*256)+(in_data[PW_m+15]);

  PW_m += (TracksSizes[0] + TracksSizes[1] + TracksSizes[2] + TracksSizes[3] + 16);
  if (PW_m + 4> PW_in_size)
  {
    printf ("#2:out of range\n");
    return BAD;
  }

/*
?	l	1	block part length
?	b	?*64	block
			0	b	1	note
			1	b	1	instrument
			2	b	1	effect number
			3	b	1	effect data
*/

  PW_n = (in_data[PW_m+2]*256)+(in_data[PW_m+3]);
  PW_m += (PW_n+4);
  if (PW_m > PW_in_size)
  {
    printf ("#3:out of range\n");
    return BAD;
  }
/*
?	w	127	instruments offset table
*/
  if (PW_m + 256 > PW_in_size)
  {
    printf ("#4:out of range\n");
    return BAD;
  }
  PW_o=0;
/*
?	w	1	instruments info table length
*/
printf ("at %x\n",PW_m);
  for (PW_j=0; PW_j<128; PW_j+=2)
  { /* highest sample refs */
    PW_k = (in_data[PW_m+PW_j]*256)+(in_data[PW_m+PW_j+1]);
    if (PW_k > PW_o)PW_o = PW_k;
  }
  printf ("highest sample ref : %x\n",PW_o);
  PW_m += 256;
  
/*
?	b	?*88	instruments info table

			0	w	1	len
			2	w	1	repeat
			4	w	1	replen
			6	b	15	volume
			15	b	15	vibrator
			24	w	1	bendrate ?
			26	b	1	instrument mode : synth/sample
			27	b	1	sample number
			28	b	48	waveform table
*/
  if (PW_m + PW_o > PW_in_size)
  {
    printf ("#5:out of range\n");
    return BAD;
  } 
 /* highest waveform ref - starts at 0 */
  PW_n = 0;
  for (PW_j=0; PW_j<PW_o; PW_j+=0x58)
  {
/*    printf ("@%x\n",PW_m+PW_j);*/
    for (PW_l=0;PW_l<0x30;PW_l++)
    {
/*      printf ("%02x-",in_data[PW_m+PW_j+PW_l+0x28]);fflush (stdout);*/
      if (in_data[PW_m+PW_j+PW_l+0x28]==0xFF)break;
      if (in_data[PW_m+PW_j+PW_l+0x28]>PW_n)PW_n = in_data[PW_m+PW_j+PW_l+0x28];
    }
/*    printf ("\n%02x\n",PW_n);fflush(stdout);*/
  }
printf ("highest waveform ref : %d\n",PW_n);
  PW_n += 1;
  PW_m += PW_o;
/*
?	b	128*256	synth waveform table	
*/
/* each waveform is 256 bytes long */
  PW_m += (PW_n*256);
  if (PW_m+(16*4)+(8*4)+4 > PW_in_size)
  {
    printf ("#6:out of range (PW_n:%d)(PW_m:%d)\n",PW_n,PW_m);
    return BAD;
  }
  PW_m += 4; /* unknown missed 4 bytes in the description */
/*
?	l	16	sample len
*/
  PW_m += (16*4);

/*
?	l	8	sample offset
*/
  PW_k = 0;
  printf ("addy of sample offsets : %x\n",PW_m);
  for (PW_j=0;PW_j<32;PW_j+=4)
  { /* sample offsets follow, if any */
    PW_l = (in_data[PW_m+PW_j+1]*256*256)+(in_data[PW_m+PW_j+2]*256)+in_data[PW_m+PW_j+3];
    printf ("%x - ",PW_l);
    if (PW_l > PW_k)PW_k = PW_l;
  }
  printf ("\n");
/* check if last sample has a size that is not 0 */
  PW_n = (in_data[PW_m-0x23]*256*256)+(in_data[PW_m-0x22]*256)+in_data[PW_m-0x21];

  PW_m += (8*4);
  PW_m += PW_k;
  
  printf ("last sample size set (%d)\n",PW_n);
  PW_m += PW_n;
  


  /* PW_m is now at the end of the DM2 */

  return GOOD;
}

void Rip_DM2 ( void )
{
  OutputSize = (PW_m - PW_Start_Address);

  CONVERT = BAD;
  Save_Rip ( "Delta Music 2 module", DM2 );

  if ( Save_Status == GOOD )
    PW_i += 1;
}
