/*
 * 20220410 - THX adapted for Pretracker PRT

  supports version 19, 1a & 1b (4th byte)
  4 DWORD follow which are pointers. they should be from lower to higher.
  the last one points on waveforms - up to 24 stored
  - waveforms text names separated by 00h
  - an additional 00h
  - and the waveforms which are 42 bytes each - if set
*/
/* testPRT() */
/* Rip_PRT() */



#include "globals.h"
#include "extern.h"


int16_t	 testPRT ( void )
{
  int16_t v1, v2, v3, v4;
  PW_Start_Address = PW_i;

  /* file size at least 20 bytes */
  if ( (PW_in_size - PW_Start_Address) < 20 )
  {
    return BAD;
  }

  /* get first 4 dword to to compare */
  v1 = ( ( in_data[PW_Start_Address+6]*256) + in_data[PW_Start_Address+7] );
  v2 = ( ( in_data[PW_Start_Address+10]*256) + in_data[PW_Start_Address+11] );
  v3 = ( ( in_data[PW_Start_Address+14]*256) + in_data[PW_Start_Address+15] );
  /* pointer on waeforms */
  v4 = ( ( in_data[PW_Start_Address+18]*256) + in_data[PW_Start_Address+19] );

  /* those 4 dword should go from smallest to highest - and not null*/
  if ((v1 == 0)||(v2 == 0)||(v3 == 0)||(v4 == 0))return BAD;
  if ((v4<=v3)||(v4<=v2)||(v4<=v1)||(v3<=v2)||(v3<=v1)||(v2<=v1))return BAD;

  /* test in-size again */
  PW_m=v4;
  if ( PW_Start_Address+PW_m > PW_in_size )
  {
/*    printf ( "#2 (start:%d) (1st waveform addy:%d)\n" , PW_Start_Address , PW_m);*/
    return BAD;
  }

  /* test nbr of waveform */
  if ( in_data[PW_Start_Address+0x41] == 0x00 )
  {
/*    printf ( "#3 (start:%d) (nbr waveforms null)\n" , PW_Start_Address);*/
    return BAD;
  }


  /*PW_m points on waveforms */
  return GOOD;
}



void Rip_PRT ( void )
{
  /* PW_m is the address of waveforms */

  uint32_t	 Where = PW_Start_Address+PW_m;
  uint8_t WaveformCPT = 0x00, i=0x00, version=0x00;
  
  OutputSize = PW_m;
  WaveformCPT = in_data[PW_Start_Address+0x41];

  for (i=0; i<24;i++)
  {
    while (in_data[Where] != 0x00)
    { 
      printf ("%c",in_data[Where]);
      Where += 1;
    }
    printf ("\n");
    Where += 1;
  }
printf ("WaveformCPT:%d\n",WaveformCPT);
version = in_data[PW_Start_Address+3];
printf ("version:%Xh\n",version);
  Where += (WaveformCPT*42);

  OutputSize = Where - PW_Start_Address;
  if (((OutputSize/2)*2)!=OutputSize)
  {
    OutputSize+=1;
    printf ("(!) +1 to size because it would be uneven otherwise\n");
  }

  CONVERT = BAD;

  Save_Rip ( "Preracker", Pretracker );
  
  
  if ( Save_Status == GOOD )
    PW_i += 1; /* after PRT tag */
}

