/*
 Copyright (C) 2000-2005  Heikki Orsila
 Copyright (C) 2000-2005  Michael Doering

 This module is dual licensed under the GNU GPL and the Public Domain.
 Hence you may use _this_ module (not another code module) in any way you
 want in your projects.

 About security:

 This module tries to avoid any buffer overruns by not copying anything but
 hard coded strings (such as "FC13"). This doesn't
 copy any data from modules to program memory. Any memory writing with
 non-hard-coded data is an error by assumption. This module will only
 determine the format of a given module.

 Occasional memory reads over buffer ranges can occur, but they will of course
 be fixed when spotted :P The worst that can happen with reading over the
 buffer range is a core dump :)
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <uadeutils.h>
#include <amifilemagic.h>

#define FILEMAGIC_DEBUG 0

#if FILEMAGIC_DEBUG
#define amifiledebug(fmt, args...) do { fprintf(stderr, "%s:%d: %s: " fmt, __FILE__, __LINE__, __func__, ## args); } while(0)
#else
//#define amifiledebug(fmt, args...) 
#endif


enum {
  MOD_UNDEFINED = 0,
  MOD_SOUNDTRACKER25_NOISETRACKER10,
  MOD_NOISETRACKER12,
  MOD_NOISETRACKER20,
  MOD_STARTREKKER4,
  MOD_STARTREKKER8,
  MOD_AUDIOSCULPTURE4,
  MOD_AUDIOSCULPTURE8,
  MOD_PROTRACKER,
  MOD_FASTTRACKER,
  MOD_NOISETRACKER,
  MOD_PTK_COMPATIBLE,
  MOD_SOUNDTRACKER24
};


#define S15_HEADER_LENGTH 600
#define S31_HEADER_LENGTH 1084


static int chk_id_offset(unsigned char *buf, int bufsize,
			 const char *patterns[], int offset, char *pre);


/* check for 'pattern' in 'buf'.
   the 'pattern' must lie inside range [0, maxlen) in the buffer.
   returns true if pattern is at buf[offset], otherwrise false
 */
static int patterntest(const unsigned char *buf, const char *pattern,
		       int offset, int bytes, int maxlen)
{
  if ((offset + bytes) <= maxlen)
    return (memcmp(buf + offset, pattern, bytes) == 0) ? 1 : 0;
  return 0;
}





/* Calculate Module length:	Just need at max 1084	*/
/*				data in buf for a 	*/
/*				succesful calculation	*/
/* returns:				 		*/
/* 		 -1 for no mod				*/
/*		 1 for a mod with good length		*/
static size_t modlentest(unsigned char *buf, size_t bufsize, size_t filesize,
			 int header)
{
  int i;
  int no_of_instr;
  int smpl = 0;
  int plist;
  int maxpattern = 0;

  if (header > bufsize)
    return -1;			/* no mod */

  if (header == S15_HEADER_LENGTH)   {
    no_of_instr = 15;
    plist = header - 128;
  } else if (header == S31_HEADER_LENGTH) {
    no_of_instr = 31;
    plist = header - 4 - 128;
  } else {
    return -1;
  }

  for (i = 0; i < 128; i++) {
    if (buf[plist + i] > maxpattern)
      maxpattern = buf[plist + i];
  }

  if (maxpattern > 100)
    return -1;

  for (i = 0; i < no_of_instr; i++)
    smpl += 2 * read_be_u16(&buf[42 + i * 30]);	/* add sample length in bytes*/

  return header + (maxpattern + 1) * 1024 + smpl;
}


static void modparsing(unsigned char *buf, size_t bufsize, size_t header, int max_pattern, int pfx[], int pfxarg[])
{
  int offset;
  int i, j, fx;
  unsigned char fxarg;
  
  for (i = 0; i < max_pattern; i++) {
    for (j = 0; j < 256; j++) {
      offset = header + i * 1024 + j * 4;

      if ((offset + 4) > bufsize)
	return;

      fx = buf[offset + 2] & 0x0f;
      fxarg = buf[offset + 3];
      
      if (fx == 0) {
	if (fxarg != 0 )
	  pfx[fx] += 1;
	pfxarg[fx] = (pfxarg[fx] > fxarg) ? pfxarg[fx] : fxarg;

      } else if (1 <= fx && fx <= 13) {
	pfx[fx] +=1;
	pfxarg[fx] = (pfxarg[fx] > fxarg) ? pfxarg[fx] : fxarg;

      } else if (fx == 14) {
	pfx[((fxarg >> 4) & 0x0f) + 16] +=1;

      } else if (fx == 15) {
	if (fxarg > 0x1f)
	  pfx[14] +=1;
	else
	  pfx[15] +=1;
	pfxarg[15] = (pfxarg[15] > fxarg) ? pfxarg[15] : fxarg;
      }
    }
  }

}


static int mod32check(unsigned char *buf, size_t bufsize, size_t realfilesize,
		      int verbose)
{
  /* mod patterns at file offset 0x438 */
  char *mod_patterns[] = { "M.K.", ".M.K", NULL};
  /* startrekker patterns at file offset 0x438 */
  char *startrekker_patterns[] = { "FLT4", "FLT8", "EXO4", "EXO8", NULL};

  int max_pattern = 0;
  int i, j, t, ret;
  int pfx[32];
  int pfxarg[32];

  /* instrument var */
  int vol, slen, srep, sreplen;

  int has_slen_sreplen_zero = 0; /* sreplen empty of non looping instrument */
  int no_slen_sreplen_zero = 0; /* sreplen */

  int has_slen_sreplen_one = 0;
  int no_slen_sreplen_one = 0;

  int no_slen_has_volume = 0;
  int finetune_used = 0;

  size_t calculated_size;

  /* returns:	 0 for undefined                            */
  /* 		 1 for a Soundtracker2.5/Noisetracker 1.0   */
  /*		 2 for a Noisetracker 1.2		    */
  /*		 3 for a Noisetracker 2.0		    */
  /*		 4 for a Startrekker 4ch		    */
  /*		 5 for a Startrekker 8ch		    */
  /*		 6 for Audiosculpture 4 ch/fm		    */
  /*		 7 for Audiosculpture 8 ch/fm		    */
  /*		 8 for a Protracker 			    */
  /*		 9 for a Fasttracker			    */
  /*		 10 for a Noisetracker (M&K!)		    */
  /*		 11 for a PTK Compatible		    */
  /*		 12 for a Soundtracker 31instr. with repl in bytes	    */

  /* Special cases first */
  if (patterntest(buf, "M&K!", (S31_HEADER_LENGTH - 4), 4, bufsize))
    return MOD_NOISETRACKER;	/* Noisetracker (M&K!) */
  
  if (patterntest(buf, "M!K!", (S31_HEADER_LENGTH - 4), 4, bufsize))
    return MOD_PROTRACKER;		/* Protracker (100 patterns) */

  if (patterntest(buf, "N.T.", (S31_HEADER_LENGTH - 4), 4, bufsize))
    return MOD_NOISETRACKER20;		/* Noisetracker2.x */

  for (i = 0; startrekker_patterns[i]; i++) {
    if (patterntest(buf, startrekker_patterns[i], (S31_HEADER_LENGTH - 4), 4, bufsize)) {
      t = 0;
      for (j = 0; j < 30 * 0x1e; j = j + 0x1e) {
	if (buf[0x2a + j] == 0 && buf[0x2b + j] == 0 && buf[0x2d + j] != 0) {
	  t = t + 1;		/* no of AM instr. */
	}
      }
      if (t > 0) {
	if (buf[0x43b] == '4'){
	  ret = MOD_AUDIOSCULPTURE4;	/* Startrekker 4 AM / ADSC */
	} else { 		
	  ret = MOD_AUDIOSCULPTURE8;	/* Startrekker 8 AM / ADSC */	
	}
      } else {
	if (buf[0x43b] == '4'){
	  ret = MOD_STARTREKKER4;	/* Startrekker 4ch */
	} else { 		
	  ret = MOD_STARTREKKER8;	/* Startrekker 8ch */	
	}
      }
      return ret;
    }
  }

  calculated_size = modlentest(buf, bufsize, realfilesize, S31_HEADER_LENGTH);

  if (calculated_size == -1)
  return MOD_UNDEFINED;


  for (i = 0; mod_patterns[i]; i++) {
    if (patterntest(buf, mod_patterns[i], S31_HEADER_LENGTH - 4, 4, bufsize)) {
      /* seems to be a generic M.K. MOD                              */
      /* only spam filesize message when it's a tracker module */

    if (calculated_size != realfilesize) {
      fprintf(stderr, "uade: file size is %zd but calculated size for a mod file is %zd.\n", realfilesize, calculated_size); 
    }

    if (calculated_size > realfilesize) {
        fprintf(stderr, "uade: file is truncated and won't get played.\n");
      return MOD_UNDEFINED;
    }

    if (calculated_size < realfilesize) {
        fprintf(stderr, "uade: file has trailing garbage behind the actual module data. Please fix it.\n");
    }

    /* parse instruments */
    for (i = 0; i < 31; i++) {
      vol = buf[45 + i * 30];
      slen = ((buf[42 + i * 30] << 8) + buf[43 + i * 30]) * 2;
      srep = ((buf[46 + i * 30] << 8) + buf[47 + i * 30]) *2;
      sreplen = ((buf[48 + i * 30] << 8) + buf[49 + i * 30]) * 2;
      /* fprintf (stderr, "%d, slen: %d, %d (srep %d, sreplen %d), vol: %d\n",i, slen, srep+sreplen,srep, sreplen, vol); */

      if (vol > 64)
        return MOD_UNDEFINED;

      if (buf[44 + i * 30] != 0) {
        if (buf[44+i*30] > 15) {
  	  return MOD_UNDEFINED;
        } else {
	  finetune_used++;
        }
      }

      if (slen > 0 && (srep + sreplen) > slen) {
        /* Old Noisetracker /Soundtracker with repeat offset in bytes */
        return MOD_SOUNDTRACKER24;
      }

      if (srep == 0) {
        if (slen > 0) {
	  if (sreplen == 2){
	    has_slen_sreplen_one++;
	  }
	  if (sreplen == 0){
	    has_slen_sreplen_zero++;
	  }
        } else {
	  if (sreplen > 0){
	    no_slen_sreplen_one++;
	  } else {
	    no_slen_sreplen_zero++;
	  }
	  if (vol > 0)
	    no_slen_has_volume++;
        }
       }
     }

      for (i = 0; i < 128; i++) {
	if (buf[1080 - 130 + 2 + i] > max_pattern)
	  max_pattern = buf[1080 - 130 + 2 + i];
      }
      
      if (max_pattern > 100) {
	/* pattern number can only be  0 <-> 100 for mod*/
	return MOD_UNDEFINED;
      }

      memset (pfx, 0, sizeof (pfx));
      memset (pfxarg, 0, sizeof (pfxarg));
      modparsing(buf, bufsize, S31_HEADER_LENGTH-4, max_pattern, pfx, pfxarg);

      /* and now for let's see if we can spot the mod */

      /* FX used:					  		     */
      /* DOC Soundtracker 2.x(2.5):	0,1,2(3,4)	    a,b,c,d,e,f	     */
      /* Noisetracker 1.x:		0,1,2,3,4	    a,b,c,d,e,f      */
      /* Noisetracker 2.x:		0,1,2,3,4           a,b,c,d,e,f      */
      /* Protracker:			0,1,2,3,4,5,6,7   9,a,b,c,d,e,f	+e## */
      /* PC tracker:			0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f +e## */

      for (j = 17; j <= 31; j++) {
	if (pfx[j] != 0 || finetune_used >0) /* Extended fx used */ {
	  if (buf[0x3b7] != 0x7f && buf[0x3b7] != 0x78) {
	    return MOD_FASTTRACKER; /* Definetely Fasttracker*/
	  } else {
	    return MOD_PROTRACKER; /* Protracker*/
	  }
	}
      }

      if ((buf[0x3b7] == 0x7f) && 
	  (has_slen_sreplen_zero <= has_slen_sreplen_one) &&
	  (no_slen_sreplen_zero <=no_slen_sreplen_one))
	return MOD_PROTRACKER; /* Protracker */

      if (buf[0x3b7] >0x7f)
	return MOD_PTK_COMPATIBLE; /* Protracker compatible */

      if ((buf[0x3b7] == 0) && 
	  (has_slen_sreplen_zero >  has_slen_sreplen_one) &&
	  (no_slen_sreplen_zero > no_slen_sreplen_one)){
	if (pfx[0x10] == 0) {
	  /* probl. Fastracker or Protracker compatible */
	  return MOD_PTK_COMPATIBLE;
	}
	  /* FIXME: Investigate
	  else {
	  return MOD_PROTRACKER; // probl. Protracker
	  } */
      }
	    
      if (pfx[0x05] != 0 || pfx[0x06] != 0 || pfx[0x07] != 0 ||
	  pfx[0x09] != 0) {
	/* Protracker compatible */
	return MOD_PTK_COMPATIBLE;
      }

      if ((buf[0x3b7] >0 && buf[0x3b7] <= buf[0x3b6]) && 
	  (has_slen_sreplen_zero <= has_slen_sreplen_one) &&
	  (no_slen_sreplen_zero == 1) &&
	  (no_slen_sreplen_zero <= no_slen_sreplen_one))    
	return MOD_NOISETRACKER12; // Noisetracker 1.2

      if ((buf[0x3b7] <0x80) && 
	  (has_slen_sreplen_zero <= has_slen_sreplen_one) &&
	  (no_slen_sreplen_zero <=no_slen_sreplen_one))    
	return MOD_NOISETRACKER20; // Noisetracker 2.x

      if ((buf[0x3b7] <0x80) && 
	  (pfx[0x0e] ==0) &&
	  (has_slen_sreplen_zero <= has_slen_sreplen_one) &&
	  (no_slen_sreplen_zero >=no_slen_sreplen_one))    
	return MOD_SOUNDTRACKER25_NOISETRACKER10; // Noisetracker 1.x

      return MOD_PTK_COMPATIBLE; // Protracker compatible
    }
  }

  return MOD_UNDEFINED;
}


int mod15check(unsigned char *buf, size_t bufsize, size_t realfilesize)
/* pattern parsing based on Sylvain 'Asle' Chipaux'	*/
/* Modinfo-V2						*/
/*							*/
/* returns:	 0 for an undefined mod 		*/
/* 		 1 for a DOC Soundtracker mod		*/
/*		 2 for a Ultimate ST mod		*/
/*		 3 for a Mastersoundtracker		*/
/*		 4 for a SoundtrackerV2.0 -V4.0		*/
{
  int i = 0, j = 0;
  int slen = 0;
  int srep = 0;
  int sreplen = 0;
  int vol = 0;

  int noof_slen_zero_sreplen_zero = 0;
  int noof_slen_zero_vol_zero = 0;
  int srep_bigger_slen = 0;
  int srep_bigger_ffff = 0;
  int st_xy = 0;
  
  int max_pattern = 1;
  int pfx[32];
  int pfxarg[32];

  size_t calculated_size;

  /* sanity checks */
  if (bufsize < 0x1f3)
    return 0;			/* file too small */

  if (bufsize < 2648+4 || realfilesize <2648+4) /* size 1 pattern + 1x 4 bytes Instrument :) */
    return 0;

  calculated_size = modlentest(buf, bufsize, realfilesize, S15_HEADER_LENGTH);
  if (calculated_size == -1)
    return 0; /* modlentest failed */

  if (calculated_size != realfilesize) {
      return 0 ;
    }

  if (calculated_size > realfilesize) {
      fprintf(stderr, "uade: file is truncated and won't get played.\n");
      return 0 ;
    }



  /* check for 15 instruments */
  if (buf[0x1d6] != 0x00 && buf[0x1d6] < 0x81 && buf[0x1f3] !=1) {
    for (i = 0; i < 128; i++) {	/* pattern list table: 128 posbl. entries */
      max_pattern=(buf[600 - 130 + 2 + i] > max_pattern) ? buf[600 - 130 + 2 + i] : max_pattern;
    }
    if (max_pattern > 63)
      return 0;   /* pattern number can only be  0 <-> 63 for mod15 */
  } else {
    return 0;
  }

  /* parse instruments */
  for (i = 0; i < 15; i++) {
    vol = buf[45 + i * 30];
    slen = ((buf[42 + i * 30] << 8) + buf[43 + i * 30]) * 2;
    srep = ((buf[46 + i * 30] << 8) + buf[47 + i * 30]);
    sreplen = ((buf[48 + i * 30] << 8) + buf[49 + i * 30]) * 2;
    /* fprintf (stderr, "%d, slen: %d, %d (srep %d, sreplen %d), vol: %d\n",i, slen, srep+sreplen,srep, sreplen, vol); */

    if (vol > 64 && buf[44+i*30] != 0) return 0; /* vol and finetune */

    if (slen == 0) {

      if (vol == 0)
	noof_slen_zero_vol_zero++;

      if (sreplen == 0 )
	noof_slen_zero_sreplen_zero++;

    } else {
      if ((srep+sreplen) > slen)
	srep_bigger_slen++;
    }
       	
    /* slen < 9999 */
    slen = (buf[42 + i * 30] << 8) + buf[43 + i * 30];
    if (slen <= 9999) {
      /* repeat offset + repeat size*2 < word size */
      srep = ((buf[48 + i * 30] << 8) + buf[49 + i * 30]) * 2 +
	((buf[46 + i * 30] << 8) + buf[47 + i * 30]);
      if (srep > 0xffff) srep_bigger_ffff++;
    }

    if  (buf[25+i*30] ==':' && buf [22+i*30] == '-' &&
	 ((buf[20+i*30] =='S' && buf [21+i*30] == 'T') ||
	  (buf[20+i*30] =='s' && buf [21+i*30] == 't'))) st_xy++;
  }

  /* parse pattern data -> fill pfx[] with number of times fx being used*/
  memset (pfx, 0, sizeof (pfx));
  memset (pfxarg, 0, sizeof (pfxarg));

  modparsing(buf, bufsize, S15_HEADER_LENGTH, max_pattern, pfx, pfxarg);

  /* and now for let's see if we can spot the mod */

/* FX used:					  */
/* Ultimate ST:			0,1,2		  */
/* MasterSoundtracker:		0,1,2,    c,  e,f */
/* DOC-Soundtracker V2.2:	0,1,2,a,b,c,d,e,f */
/* Soundtracker I-VI		0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f*/


  /* Check for fx used between 0x3 <-> 0xb for some weird ST II-IV mods */ 
  for (j = 0x5; j < 0xa; j++) {
    if (pfx[j] != 0)
      return 4; /* ST II-IV */
  }

  for (j = 0x0c; j < 0x11; j++) {
    if (pfx[j] != 0) {

      if (pfx[0x0d] != 0 && pfxarg[0x0d] != 0)
	return 4; /* ST II-IV */

      if (pfx[0x0b] != 0 || pfx[0x0d] != 0 || pfx[0x0a]!= 0 ) {
	return 1;	/* DOC ST */
      } else {
	if (pfxarg[1] > 0xe || pfxarg[2] > 0xe)
	  return 1;	/* DOC ST */

	return 3;	/* Master ST */
      }
    }
  }

  /* pitchbend out of range ? */
  if ((pfxarg[1] > 0 && pfxarg[1] <0x1f) ||
      (pfxarg[2] > 0 && pfxarg [2] <0x1f) ||
      pfx [0] >2) return 1; // ST style Arpeggio, Pitchbends ???
  
  if (pfx[1] > 0 || pfx[2] > 0)
    return 2; /* nope UST like fx */

  /* the rest of the files has no fx. so check instruments */
  if (st_xy!=0 && noof_slen_zero_vol_zero == 0 &&
      noof_slen_zero_sreplen_zero == 0 && buf[0x1d7] == 120) {
    return 3;
  }

  /* no fx, no loops... let's simply guess :)*/
  if (srep_bigger_slen == 0 && srep_bigger_ffff == 0 &&
      ((st_xy != 0 && buf[0x1d7] != 120 ) || st_xy==0))
    return 2;

  return 3; /* anything is played as normal soundtracker */
}


/* We are currently stupid and check only for a few magic IDs at the offsets
 * chk_id_offset returns 1 on success and sets the right prefix/extension
 * in pre
 * TODO: more and less easy check for the rest of the 52 trackerclones
 */
static int chk_id_offset(unsigned char *buf, int bufsize,
			 const char *patterns[], int offset, char *pre)
{
  int i;
  for (i = 0; patterns[i]; i = i + 2) {
    if (patterntest(buf, patterns[i], offset, strlen(patterns[i]), bufsize)) {
      /* match found */
      strcpy(pre, patterns[i + 1]);
      return 1;
    }
  }
  return 0;
}
