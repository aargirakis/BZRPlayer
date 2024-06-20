/*
 *                         sc68 - CURL stream
 *         Copyright (C) 2001-2003 Benjamin Gerard <ben@sashipa.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <config68.h>

/* define this if you don't want CURL support. */
#ifdef ISTREAM_NO_CURL


#include <curl/curl.h>
#include <string.h>
#if defined (HAVE_USLEEP) || defined (HAVE_SLEEP)
# include <unistd.h>
#endif

#include "file68/istream68_curl.h"
#include "file68/istream68_def.h"
#include "file68/alloc68.h"
#include "file68/debugmsg68.h"

#ifndef ISF_VERBOSE
# ifdef _DEBUG
#  define ISF_VERBOSE 1
# else
#  define ISF_VERBOSE 0
# endif
#endif

/* Timeout in ms */
#define IS68_TIMEOUT        10000

#define IS68_CACHE_FIX      8
#define IS68_CACHE_SZ       (1<<IS68_CACHE_FIX)
#define IS68_CACHE_BLKS_FIX 7
#define IS68_CACHE_BLKS     (1<<IS68_CACHE_BLKS_FIX)

typedef struct {
  int top;
  int bytes;
  char buffer[IS68_CACHE_SZ];
} istream_cache_t;

/** istream curl structure. */
typedef struct {
  istream_t istream; /**< istream function. */

  CURLM * curm;       /**< CURL "multi" handle       */
  CURL  * curl;       /**< CURL "easy" handle.       */

  CURLcode code;      /**< CURL last "easy" operation code.  */
  CURLMcode mcode;    /**< CURL last "multi" operation code. */

  /*
  struct {
    fd_set r,w,e;
  } sets;
  */

  int mode:2;         /**< Open modes.                  */
  int has_hd_read:1;  /**< 1: read header has started.  */
  int has_hd_write:1; /**< 1: write header has started. */
  int has_read:1;     /**< 1: read data has started.    */
  int has_write:1;    /**< 1: write data has started.   */

  int pos;            /**< Current R/W position.        */
  int curl_pos;       /**< CURL current R/W position.   */
  int length;         /**< Known length (-1:unknown).   */

  /** Cached data. */
  istream_cache_t cache[IS68_CACHE_BLKS];

  /** Curl error message. */
  char error[CURL_ERROR_SIZE];

  /* MUST BE at the end of the structure because supplemental bytes will
   * be allocated to store curlname.
   */
  char name[1];       /**< url. */

} istream_curl_t;

static int init = 0;

static int mysleep(unsigned long ms)
{
#ifdef HAVE_USLEEP
  unsigned long us = ms * 1000u;
  usleep(us);
  return (int) ms;
#elif defined(_MSC_VER)
  Sleep(ms);
  return (int) ms;
#elif defined(HAVE_SLEEP)
  unsigned int sec = (ms + 999u) / 1000u;
  sleep(sec);
  return (int)(sec * 1000u);
#endif
  /* $$$ Ooops very bad thing */
  return ms;
}

static void isf_reset_cache_blocks(istream_curl_t *isf)
{
  int i;

  for (i=0; i<IS68_CACHE_BLKS; ++i) {
    istream_cache_t * c = isf->cache+i;
    c->top = -1;
    c->bytes = 0;
  }
}

static const char * isf_name(istream_t * istream)
{
  istream_curl_t * isf = (istream_curl_t *)istream;

  return (!isf || !isf->name[0])
    ? 0
    : isf->name;
}

/* $$$ TODO */
static size_t isf_read_cb(void *ptr, size_t size, size_t nmemb, void *stream)
{
  istream_curl_t * isf = (istream_curl_t *)stream;
  int bytes, org_bytes, pos;

  bytes = org_bytes = (int)(size * nmemb);
  pos = isf->curl_pos;

  isf->has_read = 1;
  if (!bytes) {
    return 0; /* Avoid 0 divide at ending return */
  }

  return (org_bytes - bytes) / size;
}

static size_t isf_write_cb(void *ptr, size_t size, size_t nmemb, void *stream)
{
  istream_curl_t * isf = (istream_curl_t *)stream;

  int bytes, org_bytes, pos;
  int off, top, blk;

  bytes = org_bytes = (int)(size * nmemb);
  pos = isf->curl_pos;

  debugmsg68("istream_curl::write_cb(%s:%p,%d) pos=%x\n",
	     isf->name, isf->curm, bytes, pos);
  isf->has_write = 1;
  if (!bytes) {
    return 0; /* Avoid 0 divide at ending return */
  }

  off = pos & (IS68_CACHE_SZ-1);
  top = pos - off;
  blk = (pos>>IS68_CACHE_FIX) & (IS68_CACHE_BLKS-1);

  while (bytes > 0) {
    int n;

    if (!off) {
      /* Starting a new cache block */
      isf->cache[blk].top = top;
    } else if (isf->cache[blk].top != top) {
      /* Oops, got a problem here */
      break;
    }

    n = IS68_CACHE_SZ - off;
    if (n > bytes) {
      n = bytes;
    }

    memcpy(isf->cache[blk].buffer+off, ptr, n);
    ptr = (char *)ptr + n;
    isf->cache[blk].bytes = n+off;
    bytes -= n;
    off = 0;
    blk = (blk+1) & (IS68_CACHE_BLKS-1);
    top += IS68_CACHE_SZ;
  }
  org_bytes -= bytes;
  isf->curl_pos = pos + org_bytes;

  return org_bytes / size;
}

static int isf_passwd_cb(void *clientp,
			 const char *prompt, char *buffer, int buflen)
{
  /* Produce a CURLE_BAD_PASSWORD_ENTERED error. */
  return -1;
}


static int isf_prog_cb(void *clientp,
		       double dltotal,double dlnow,
		       double ultotal,double ulnow)
{
  return 0;
}

static int isf_debug_cb(CURL *handle, curl_infotype type,
			char *data, size_t size,
			void *userp)
{
#if 0  

  istream_curl_t * isf = (istream_curl_t *)userp;
  const char * typestr;

  switch (type) {
  case CURLINFO_TEXT:
    typestr = "TEXT";
    break;
  case CURLINFO_HEADER_IN:
    typestr = "HEADER_IN";
    break;
  case CURLINFO_HEADER_OUT:
    typestr = "HEADER_OUT";
    break;
  case CURLINFO_DATA_IN:
    typestr = "DATA_IN";
    data = 0;
    break;
  case CURLINFO_DATA_OUT:
    typestr = "DATA_OUT";
    data = 0;
    break;
  case CURLINFO_END:
    typestr = "DATA_END";
    data = 0;
    break;
  default:
    typestr = "UNKNOWN";
    data = 0;
    break;
  }

  debugmsg68("%s:%p:%p >%s\n", isf->name, isf->curm, handle, typestr);
  if (data) {
    int i;
    char tmp[256];
    int max = (size > sizeof(tmp)-1) ? sizeof(tmp)-1 : size;
    for (i=0; i<max; ++i) {
      int c = data[i] & 255;
      if (c==9) c = ' ';
      if (c < 32 || c > 127) break;
      tmp[i] = c;
    }
    tmp[i] = 0;
    if (i>0) {
      debugmsg68("[%s]\n", tmp);
    }
  }
#endif

  return 0;
}

static int match_header(int *v,
			char * ptr, const int bytes,
			const char *header, const int hdsize)
{
  if (bytes > hdsize+1 &&
      !memcmp(ptr,header,hdsize-1)) {
    int i, c;

    /* Find first digit */
    for (i=hdsize-1, c=0; i<bytes && (c=ptr[i], (c<'0' || c>'9')); ++i)
      ;
    /* Convert base 10 number */
    if (c>='0' || c<='9') {
      int len = c - '0';
      for (++i; i<bytes && (c=ptr[i], (c>='0' && c<='9')); ++i) {
	len = len * 10 + c - '0';
      }
      *v = len;
/*       debugmsg68("Match header %s-> %d\n", header, len); */
      return 1;
    }
  }
  return 0;
}

static size_t isf_header_cb(void  *ptr, size_t  size,
			    size_t  nmemb,  void  *stream)
{
  istream_curl_t * isf = (istream_curl_t *)stream;

  static const char ftp_size[] = "213";
/*   static const char ftp_complete[] = "226"; */

  static const char content_length[] = "Content-Length:";
/*   static const char content_type[] = "Content-Type:"; */

  int len;
  int bytes = nmemb*size;

  isf->has_hd_write = 1;

/*   debugmsg68("istream_curl::header_cb(%s:%p, %d)\n", */
/*  	     isf->name, isf->curm, size*nmemb); */

  if (match_header(&len, ptr, bytes, content_length, sizeof(content_length))
      || match_header(&len, ptr, bytes, ftp_size, sizeof(ftp_size))) {
    isf->length = len;
/*     debugmsg68("%s::Set length to %d\n", isf->name, len); */
  }

  /* $$$ TODO : Add transfert complete checks */

  return nmemb;
}

static int isf_open(istream_t * istream)
{
  istream_curl_t * isf = (istream_curl_t *)istream;
  const int verbose = ISF_VERBOSE;
  CURLcode code = 0;
  CURLMcode mcode = 0;
  int err;

  if (!isf || !isf->name[0] || isf->curm || isf->curl) {
    return -1;
  }

/*   SC68os_pdebug("istream_curl::open(%s,%c%c)\n",isf->name, */
/* 		(isf->mode & ISTREAM_OPEN_READ)?'R':'r', */
/* 		(isf->mode & ISTREAM_OPEN_WRITE)?'W':'w'); */

  /* Reset info. */
  isf->has_read = isf->has_write = isf->has_hd_read = isf->has_hd_write = 0;
  isf->curl_pos = isf->pos = 0;
  isf->length = -1;
  isf->error[0] = 0;
  isf->code = 0;
  isf->mcode = 0;

  /*
  FD_ZERO(&isf->sets.r);
  FD_ZERO(&isf->sets.w);
  FD_ZERO(&isf->sets.e);
  */

  /* Currently accapts only read only open mode. */
  if (isf->mode != ISTREAM_OPEN_READ) {
    strcpy(isf->error, "Invalid open mode");
    return -1;
  }

  /* Create curl handle */
  isf->curl = curl_easy_init();
  isf->curm = curl_multi_init();
/*   debugmsg68("%s:%p:%p:open()\n",isf->name, isf->curm, isf->curl); */
  if (!isf->curl || !isf->curm) {
    goto error;
  }

  /* URL, most important thing to setup. */
  err = 0;
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_URL,isf->name)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_ERRORBUFFER,isf->error)));
     /* Set curl internal buffer (just an hint) */
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_BUFFERSIZE,1024)));
     
     /* Debug options. */
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_VERBOSE,verbose)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_DEBUGFUNCTION,isf_debug_cb)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_DEBUGDATA,isf)));
     /* Our read/write/progress callback. */
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_WRITEFUNCTION,isf_write_cb)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_WRITEDATA,isf)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_READFUNCTION,isf_read_cb)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_READDATA,isf)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_PROGRESSFUNCTION,isf_prog_cb)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_PROGRESSDATA,isf)));
     
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_HEADERFUNCTION,isf_header_cb)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_HEADERDATA,isf)));
     
     /* Password callback (currently raises an error) */
  /* $$$ OBSOLETE */
  /*
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_PASSWDFUNCTION,isf_passwd_cb)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_PASSWDDATA,isf)));
  */ 
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_NETRC,CURL_NETRC_OPTIONAL)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_FOLLOWLOCATION,1)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_UNRESTRICTED_AUTH,0)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_TRANSFERTEXT,0)));
     
     // $$$ FIXME
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_INFILESIZE,0)));
  err = err || (CURLE_OK != (code=curl_easy_setopt(isf->curl,CURLOPT_UPLOAD,0)));

  if (err) {
    goto error;
  }

  isf_reset_cache_blocks(isf);
  mcode = curl_multi_add_handle(isf->curm, isf->curl);
  if (mcode != CURLM_OK && mcode != CURLM_CALL_MULTI_PERFORM) {
    goto error;
  } else {
    const int re_timeout = IS68_TIMEOUT;
    int timeout = re_timeout;

    do {
      int n;
      mcode = curl_multi_perform(isf->curm, &n);
/*       debugmsg68("READ_HEADER: MCODE=[%s]\n", */
/* 		 mcode == CURLM_OK ? "OK" : */
/* 		 (mcode == CURLM_CALL_MULTI_PERFORM) ? "MULTI" : "OTHER"); */

      if (mcode == CURLM_OK) {
	timeout -= mysleep(200);
	if (timeout < 0) {
	  debugmsg68("READ_HEADER: TIME-OUT\n");
	  break;
	}
      } else if (mcode != CURLM_CALL_MULTI_PERFORM) {
	goto error;
      } else {
	timeout = re_timeout;
      }
    } while (!(isf->has_read | isf->has_write));
    if ( ! (isf->has_hd_write | isf->has_read | isf->has_write) ) {
      goto error;
    }
  }
  isf->code = code;
  isf->mcode = mcode;
  return 0;
  
 error:
  isf->code = code;
  isf->mcode = mcode;
  if (isf->curm) {
    if (isf->curl) {
      curl_multi_remove_handle(isf->curm, isf->curl);
    }
    curl_multi_cleanup(isf->curm);
    isf->curm = 0;
  }
  if (isf->curl) {
    curl_easy_cleanup(isf->curl);
    isf->curl = 0;
  }
  
  return -1;
}

static int isf_close(istream_t * istream)
{
  istream_curl_t * isf = (istream_curl_t *)istream;
  int err = -1;

  if (isf) {
/*     debugmsg68("istream_curl::%s:%p:%p:close()\n", */
/* 	       isf->name, isf->curm, isf->curl); */
    if (isf->curm) {
      err = 0;
      if (isf->curl) {
	err = -!!curl_multi_remove_handle(isf->curm, isf->curl);
	curl_easy_cleanup(isf->curl);
	isf->curl = 0;
      }
      err |= -!!curl_multi_cleanup(isf->curm);
      isf->curm = 0;
    }
  }
  return err;
}

static int isf_need_more(istream_curl_t * isf)
{
  int n=0, pos;

  pos = isf->curl_pos;
  isf->mcode = curl_multi_perform(isf->curm, &n);
  n = isf->curl_pos - pos;
  if (n) {
/*     debugmsg68("%s:%p:GETS MORE\n" */
/* 	       "-> %d (code=%d)\n", */
/* 	       isf->name, isf->curm, */
/* 	       n, isf->mcode); */
  } else {
    switch (isf->mcode) {
    case CURLM_OK:
      break;
    case CURLM_CALL_MULTI_PERFORM:
    default:
/*       debugmsg68("%s:%p:GETS MORE DETECTS EOF\n" */
/* 		 "-> pos:%d length:%d (code=%d)\n", */
/* 		 isf->name, isf->curm, */
/* 		 isf->curl_pos, isf->length, isf->mcode); */
      n = -1;
      break;
    }
  }
  return n;
}


static int isf_read(istream_t * istream, void * data, int n)
{
  istream_curl_t * isf = (istream_curl_t *)istream;
  int pos, off, top, blk, bytes;
  const int re_timeout = IS68_TIMEOUT;
  int timeout = re_timeout;

/*   SC68os_pdebug("%s:%p::read: %d bytes -> %p\n", */
/* 		isf->name, isf->curm, n, data); */

  if (!isf || !isf->curl) {
    return -1;
  } else if (!n) {
    return 0;
  }

  pos = isf->pos;
  bytes = n;

  while (bytes > 0) {
    char * src = 0;
    int n = -1;


    if (isf->length != -1 && pos >= isf->length) {
/*       debugmsg68("%s:%p, EOF:\n" */
/* 		 "->pos:%p/%p\n", */
/* 		 isf->name, isf->curm, */
/* 		 pos, isf->length); */
      break;
    } else if (timeout < 0) {
      debugmsg68("%s:%p, TIME-OUT :\n"
		 "->pos:%d/%d (%d ms)\n",
		 isf->name, isf->curm,
		   pos, isf->length, re_timeout);
      break;
    }

    off = pos & (IS68_CACHE_SZ-1);
    top = pos - off;
    blk = (pos>>IS68_CACHE_FIX) & (IS68_CACHE_BLKS-1);

    if (top == isf->cache[blk].top) {

/*       debugmsg68("%s:%p, read block:\n" */
/* 		 "->pos:%x top:%x blk:%x off:%x bytes:%d cache:[%x:%d]\n", */
/* 	       isf->name, isf->curm, */
/* 	       pos, top, blk, off, bytes, */
/* 	       isf->cache[blk].top, isf->cache[blk].bytes); */

      src = isf->cache[blk].buffer + off;
      n = isf->cache[blk].bytes - off;
      if (n > bytes) {
	n = bytes;
      }
    } else {
/*       debugmsg68("NOT MY TOP\n"); */
    }

    if (n <= 0) {
      if (pos < isf->curl_pos) {
	debugmsg68("%s:%p: Can not seek backward from %d to %d\n",
		   isf->name, isf->curm, isf->curl_pos, pos);
	break;
      }

      n=isf_need_more(isf);
      if (!n) {
	/* $$$ FIXME : should use fd_set here ! */
	timeout -= mysleep(100);
      } else if (n == -1) {
	break;
      } else {
	timeout = re_timeout;
      }
    } else {
      memcpy(data, src, n);
      data = (char *)data + n;
      pos += n;
      bytes -= n;
    }
  }
  isf->pos = pos;

  if (isf->mcode != CURLM_OK && isf->mcode != CURLM_CALL_MULTI_PERFORM) {
    debugmsg68("%s:%p:read-error:[%s]\n",
	       isf->name, isf->curm, isf->error);
    return -1;
  }
/*   debugmsg68("%s:%p:read -> %d\n", isf->name, isf->curm, n-bytes); */
  return n - bytes;
}

/* $$$ TODO */
static int isf_write(istream_t * istream, const void * data, int n)
{
  istream_curl_t * isf = (istream_curl_t *)istream;

  return (!isf || !isf->curl)
    ? -1
    : 0;
}


/* We could have store the length value at opening, but this way it handles
 * dynamic changes of curl size.
 */
static int isf_length(istream_t * istream)
{
  istream_curl_t * isf = (istream_curl_t *)istream;

  return (!isf || !isf->curl)
    ? -1
    : isf->length;
}

static int isf_tell(istream_t * istream)
{
  istream_curl_t * isf = (istream_curl_t *)istream;
  return (!isf || !isf->curl)
    ? -1
    : isf->pos;
}

static int isf_seek(istream_t * istream, int offset)
{
  istream_curl_t * isf = (istream_curl_t *)istream;
  int pos;

  if (!isf || !isf->curl) {
    return -1;
  }

  pos = isf->pos + offset;
  if (pos < 0 || (isf->length != -1 && pos > isf->length)) {
    return -1;
  }
  isf->pos = pos;
  return 0;
}

static void isf_destroy(istream_t * istream)
{
  SC68free(istream);
}

static const istream_t istream_curl = {
  isf_name,
  isf_open, isf_close,
  isf_read, isf_write,
  isf_length, isf_tell, isf_seek, isf_seek,
  isf_destroy
};

int istream_curl_init(void)
{
  if (!init) {
    CURLcode code;

    code = curl_global_init(CURL_GLOBAL_ALL);
    init = code ? -1 : 1;
  }
  return -(init != 1);
}

void istream_curl_shutdown(void)
{
  if (init == 1) {
    curl_global_cleanup();
    init = 0;
  }
}

istream_t * istream_curl_create(const char * fname, int mode)
{
  
  istream_curl_t *isf;
  int len;

  if (istream_curl_init()) {
    return 0;
  }

  if (!fname || !fname[0]) {
    return 0;
  }

  /* Don't need 0, because 1 byte already allocated in the
   * istream_curl_t::fname.
   */
  len = strlen(fname);
  isf = SC68alloc(sizeof(istream_curl_t) + len);
  if (!isf) {
    return 0;
  }

  /* Copy istream functions. */
  memset(isf,0,sizeof(*isf));
  memcpy(&isf->istream, &istream_curl, sizeof(istream_curl));
  /* Clean curl handle. */
  isf->mode = mode & (ISTREAM_OPEN_READ|ISTREAM_OPEN_WRITE);
  
  strcpy(isf->name, fname);
  return &isf->istream;
}

#else /* #ifndef ISTREAM_NO_CURL */

/* istream curl must not be include in this package. Anyway the creation
 * still exist but it always returns error.
 */

#include "file68/istream68_curl.h"
#include "file68/istream68_def.h"

istream_t * istream_curl_create(const char * fname, int mode)
{
  return 0;
}

int istream_curl_init(void)
{
  return 0;
}

void istream_curl_shutdown(void)
{
}

#endif /* #ifndef ISTREAM_NO_CURL */

