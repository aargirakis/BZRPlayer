/*
 *                              sc68 - z stream
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
#include "istream68_z.h"

#ifdef HAVE_ZLIB_H

#include <../zlib.h>
#include <string.h>

#include "file68/istream68_z.h"
#include "file68/istream68_def.h"
#include "file68/alloc68.h"
#include "file68/debugmsg68.h"

/* defined in zutil.h */
#ifndef DEF_MEM_LEVEL
# define DEF_MEM_LEVEL 8
#endif

/* gzip flag byte (from gzio.c) */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text   */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present         */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present        */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present       */
#define RESERVED     0xE0 /* bits 5..7: reserved                   */

/** istream Z structure. */
typedef struct {
  istream_t istream;       /**< istream function.     */
  istream_t * is;          /**< Wrapped stream.       */
  unsigned int mode:2;     /**< Available open modes. */
  unsigned int is_err:1;   /**< Last is op failed.    */
  unsigned int is_eof:1;   /**< is EOF.               */
  unsigned int gzip:1;     /**< Gzip stream.          */
  unsigned int gz_name:1;  /**< Add filename.         */
  unsigned int level:4;    /**< Compression level.    */
  unsigned int strategy:2; /**< Compression startegy. */
  unsigned int inflate:1;  /**< Inflate inited.       */
  unsigned int deflate:1;  /**< Deflate inited.       */
  unsigned int hd_crc:17;  /**< Header CRC16 value.   */
  unsigned int c_crc32;    /**< Computed CRC32 value. */
  unsigned int gz_crc32;   /**< Read CRC32 value.     */
  unsigned int gz_len;     /**< Read length value.    */

  int pos;                 /**< Current R/W position. */
  int org_pos;             /**< Z stream start pos.   */
  int length;              /**< Known length.         */
  z_stream c_stream;       /**< Z compression stream. */
  Byte * write_in;         /**< Write buffer_in pos.  */
  Byte * read_out;         /**< Read buffer_out pos.  */
  Byte buffer_in[512];     /**< Input buffer.         */
  Byte buffer_out[512];    /**< Output buffer.        */
} istream_z_t;

/* $$$ FIXME:
   zlib uses a calloc function which cleans allocated memory buffers.
   Currently it seems to work without doing it but deeper checks
   should be performed.
   $$$ Currently it seems to work well.
*/
static voidpf isf_zcalloc(voidpf opaque, unsigned items, unsigned size)
{
  opaque = opaque;
  return SC68alloc(items*size);
}

void isf_zcfree (voidpf opaque, voidpf ptr)
{
  opaque = opaque;
  SC68free(ptr);
}

static const char * isf_name(istream_t * istream)
{
  istream_z_t * isf = (istream_z_t *)istream;

  return (!isf)
    ? 0
    : istream_filename(isf->is);
}

/* Fill input buffer.
 *
 * @return number of bytes read or -1
 */
static int isf_fill_input_buffer(istream_z_t * isf, const int max)
{
  int n;

  /* Try to fill whole buffer */
  n = istream_read(isf->is, isf->buffer_in, max);
  isf->is_eof = (n != max);
  isf->is_err = (n == -1);
  isf->c_stream.next_in = isf->buffer_in;
  isf->c_stream.avail_in = n; /* -1: generates error in isf_read_buffer() */
  return n;
}

/* Flush output buffer.
 *
 * @return number of bytes written or -1
 */
static int isf_flush_output_buffer(istream_z_t * isf)
{
  int n, w;

  w = n = isf->c_stream.next_out - isf->buffer_out; 
/*   debugmsg68(" FLUSH:%d",n); */
  if (n > 0) {
    w = istream_write(isf->is, isf->buffer_out, n);
    isf->is_eof = (w != n);
    isf->is_err = (w == -1);
    if (isf->is_err) {
/*       debugmsg68(" WRITE-ERROR"); */
      w = 0;
      n = -1;
    } else {
      int rem = n - w;

      if (rem) {
	/* Damned! ... Could be an error ... Trust istream & Pray */
	int i;
/* 	debugmsg68(" FLUSH-MISS:%d", rem); */
	for (i=0; i<rem; ++i) {
	  isf->buffer_out[i] = isf->buffer_out[i+w];
	}
      }
      n = w;
      w = rem;
    }
    isf->c_stream.next_out = isf->buffer_out + w;
  }
  isf->c_stream.avail_out = sizeof(isf->buffer_out) - w;
/*   debugmsg68("->%d ", n); */

  return n;
}

/* Read data buffer_in data */
static int isf_read_buffer(istream_z_t * isf, void *buffer, int n)
{
  int bytes;

  if (!n) {
    return 0;
  }

  bytes = n;
  while (bytes > 0) {
    int n;

    n = isf->c_stream.avail_in;
    if (!n) {
      n = isf_fill_input_buffer(isf,bytes);
    }
    if (n == -1) {
      return -1;
    }
    if (!n) {
      break;
    }
    if (n > bytes) {
      n = bytes;
    }
    bytes -= n;
    memcpy(buffer, isf->c_stream.next_in, n);
    isf->c_stream.next_in += n;
    isf->c_stream.avail_in -= n;
    buffer = (char *)buffer + n;
  }
  return n - bytes;
}

static int isf_seek_buffer(istream_z_t * isf, int offset)
{
  Byte * next_in;
  int avail_in, n;

  if (!offset) {
    return 0;
  }

  next_in = isf->c_stream.next_in;
  avail_in = isf->c_stream.avail_in;
  n = offset;

  if (offset < 0) {
    if (-offset <= next_in - isf->buffer_in) {
      offset = 0;
    }
  } else if (offset <= avail_in) {
    offset = 0;
  }

  if (!offset) {
    isf->c_stream.next_in = next_in + n;
    isf->c_stream.avail_in = avail_in - n;
  } else {
    if (istream_seek(isf->is, offset) == -1) {
      isf->is_err = 1;
      return -1;
    }
    isf->c_stream.next_in = isf->buffer_in;
    isf->c_stream.avail_in = 0;
  }

  return 0;
}

static const Byte gz_magic[2] = {0x1f, 0x8b};

static int isf_write_gz_header(istream_z_t * isf)
{
  struct {
    Byte magic[2]; /* gz header                */
    Byte method;   /* method (Z_DEFLATED)      */
    Byte flags;    /* Xtra,name,comment...     */
    Byte info[6];  /* time, xflags and OS code */ 
  } header;
  int i;
  const char * name = isf->gz_name ? istream_filename(isf->is) : 0;

  header.magic[0] = gz_magic[0];
  header.magic[1] = gz_magic[1];
  header.method   = Z_DEFLATED;
  header.flags    = name ? ORIG_NAME : 0;
  for (i=0; i<sizeof(header.info); ++i) {
    header.info[i] = 0;
  }
  if (istream_write(isf->is, &header, sizeof(header)) != sizeof(header)) {
    return -1;
  }
  if (name) {
    const char * s1, * s2;
    int len;
    s1 = strrchr(name,'/');
    s2 = strrchr(name,'\\');
    name = (s1 > name) ? s1+1 : name;
    name = (s2 > name) ? s2+1 : name;
    len = strlen(name) + 1;
    if (istream_write(isf->is, name, len) != len) {
      return -1;
    }
  }
  return 0;
}

static int isf_read_gz_header(istream_z_t * isf)
{
  int len;
  struct {
    Byte magic[2]; /* gz header                */
    Byte method;   /* method (Z_DEFLATED)      */
    Byte flags;    /* Xtra,name,comment...     */
    Byte info[6];  /* time, xflags and OS code */ 
  } header;

  /* Read gzip header. */
  len = isf_read_buffer(isf, &header, sizeof(header));
  if (len != sizeof(header)) {
    goto error;
  }
  /* Preliminary tests. */
  if (0
      || gz_magic[0] != header.magic[0]
      || gz_magic[1] != header.magic[1]
      || header.method != Z_DEFLATED
      || (header.flags & RESERVED) != 0) {
    goto error;
  }
  /* Skip the extra field. */
  if ((header.flags & EXTRA_FIELD) != 0) {
    unsigned char xtraLen[2];
    int xlen = isf_read_buffer(isf,&xtraLen,2);
    if (xlen == -1) {
      goto error;
    }
    len += xlen;
    if (xlen != 2) {
      goto error;
    }
    xlen = xtraLen[0] + (xtraLen[1]<<8);
    if (isf_seek_buffer(isf,xlen) == -1) {
      goto error;
    }
    len += xlen;
  }

  /* Skip the original file name & .gz file comment. */
  while (header.flags & (ORIG_NAME|COMMENT)) {
    char c;
    int e;
    if (header.flags & ORIG_NAME) {
      header.flags &= ~ORIG_NAME;
    } else {
      header.flags &= ~COMMENT;
    }
    while (e=isf_read_buffer(isf, &c, 1), e == 1) {
      ++len;
      if (!c) break;
    }
    if (e != 1) {
      goto error;
    }
  }

  /* Skip the header crc */
  if (header.flags & HEAD_CRC) {
    unsigned char crc[2];
    int e = isf_read_buffer(isf,crc,2);
    if (e == -1) {
      goto error;
    }
    len += e;
    if (e != 2) {
      goto error;
    }
    /* Add 0x10000 so that the value could not be 0. */
    isf->hd_crc = crc[0] | (crc[1]<<8) | (1<<16);
  }
  return 0;

 error:
  /* Try to seek back to starting position. */
  isf_seek_buffer(isf,-len);
  return -1;
}

/* Inflate as much as possible.
 * returns number of byte available in out buffer
 */
static int isf_inflate_buffer(istream_z_t * isf)
{
  int err, n;
  err = 0;
  isf->c_stream.avail_out = sizeof(isf->buffer_out);
  isf->c_stream.next_out = isf->read_out = isf->buffer_out;

  while (isf->c_stream.avail_out) {

/*     debugmsg68("INFLATE: (%d,%d)", */
/* 	       isf->c_stream.avail_in, isf->c_stream.avail_out); */
    if (!isf->c_stream.avail_in) {
      err = isf_fill_input_buffer(isf,sizeof(isf->buffer_in));
/*       debugmsg68(" fill_in(%d) ",err); */
      if (err <= 0) {
	break;
      }
    }
/*     debugmsg68(" (%d,%d), inflate->", */
/* 	       isf->c_stream.avail_in, isf->c_stream.avail_out); */
    err = inflate(&isf->c_stream, Z_NO_FLUSH);
/*     debugmsg68("(%d,%d)", isf->c_stream.avail_in, isf->c_stream.avail_out); */

    if (err == Z_STREAM_END) {
/*       debugmsg68(" Z_STREAM_END\n"); */
      if (isf->gzip) {
	unsigned char data[8];

	err = isf_read_buffer(isf, data, 8);
	if (err >= 4) {
	  isf->gz_crc32 = data[0]
	    | (data[1]<<8)
	    | (data[2]<<16)
	    | (data[3]<<24);
	}
	if (err >= 8) {
	  isf->gz_len = data[4]
	    | (data[5]<<8)
	    | (data[6]<<16)
	    | (data[7]<<24);
	}
	debugmsg68("Total In  : %8d\n"
		   "Total Out : %8d\n"
		   "Crc32     : %08X\n"
		   "Gzip-Size : %8d\n",
		   isf->c_stream.total_in,
		   isf->c_stream.total_out,
		   isf->gz_crc32,
		   isf->gz_len);
      }
      err = 0;
      break;
    } else if (err == Z_OK) {
/*       debugmsg68(" Z_STREAM_OK\n"); */
      err = 0;
    } else {
      debugmsg68("Z_ERROR:[%s]\n", isf->c_stream.msg);
      break;
    }

  }
  
  n = isf->c_stream.next_out - isf->buffer_out;
  if (isf->gzip) {
    isf->c_crc32 = crc32(isf->c_crc32, isf->buffer_out, n);
/*     debugmsg68("CRC32:%08X\n",isf->c_crc32); */
  }
  
  return err
    ? -1
    : n;
}

/* Deflate as much as possible.
 * @return  Number of byte 
 */
static int isf_deflate_buffer(istream_z_t * isf, int finish)
{
  int err = 0, mia, zeof;
  const int z_flush_mode = finish ? Z_FINISH : Z_NO_FLUSH;

  /* fake zeof to avoid flushing */
  zeof = (z_flush_mode != Z_FINISH);

  isf->c_stream.next_in = isf->buffer_in;
  isf->c_stream.avail_in = isf->write_in - isf->buffer_in;

  /* Loop while there is data to compress ... */
  while (!zeof || isf->c_stream.avail_in) {
/*     debugmsg68("%sDEFLATE: (%d:%d/%d:%d) ", */
/* 	       finish ? "F-" : "", */
/* 	       isf->c_stream.next_in - isf->buffer_in, */
/* 	       isf->c_stream.avail_in, */
/* 	       isf->c_stream.next_out - isf->buffer_out, */
/* 	       isf->c_stream.avail_out); */

    /* Flush output buffer and win a fresh one. */
    err = isf_flush_output_buffer(isf);
    if (err == -1) {
/*       debugmsg68("\n"); */
      return -1;
    }

    /* Do the deflate thing */
/*     debugmsg68("(%d:%d/%d:%d), deflate->", */
/* 	       isf->c_stream.next_in - isf->buffer_in, */
/* 	       isf->c_stream.avail_in, */
/* 	       isf->c_stream.next_out - isf->buffer_out, */
/* 	       isf->c_stream.avail_out); */
    {
      Byte * start = isf->c_stream.next_in;
      int len = isf->c_stream.avail_in;
      err = deflate(&isf->c_stream, z_flush_mode);
      if (isf->gzip) {
	isf->c_crc32 = crc32(isf->c_crc32, start, len-isf->c_stream.avail_in);
      }
    }
/*     debugmsg68("(%d:%d/%d:%d) ", */
/* 	       isf->c_stream.next_in - isf->buffer_in, */
/* 	       isf->c_stream.avail_in, */
/* 	       isf->c_stream.next_out - isf->buffer_out, */
/* 	       isf->c_stream.avail_out); */

    if (err == Z_STREAM_END) {
      /* Zlib tell us this is the end my friend ?? */
/*       debugmsg68("Z_STREAM_END!! "); */
      err = isf_flush_output_buffer(isf) == -1;
      zeof = 1;
      break;
    }
    if (err != Z_OK) {
      debugmsg68("Z_ERROR(%d:[%s])\n", err, isf->c_stream.msg);
      err = -1;
      break;
    } else {
      err = 0;
/*       debugmsg68("Z_OK "); */
    }
  }

  /* Are there "Missing In Action" bytes in the input buffer ? */
  mia = isf->c_stream.avail_in;
/*   debugmsg68("MIA:%d\n", mia); */

  isf->write_in = isf->buffer_in + mia;
  if (mia > 0) {
    Byte * src = isf->c_stream.next_in;
    int i;
    for (i=0; i<mia; ++i) {
      isf->buffer_in[i] = src[i];
    }
  }

  return err
    ? -1
    : sizeof(isf->buffer_in) - mia;
}


static int isf_close(istream_t * istream)
{
  istream_z_t * isf = (istream_z_t *)istream;
  int err = -1;

  debugmsg68("istream_z:close[%s] ", istream_filename(istream));

  if (isf) {
    err = isf->is_err;

    if (isf->deflate) {
      debugmsg68("DEFLATED ");
      isf->deflate = 0;
      if (!err) {
	err = isf_deflate_buffer(isf, 1) == -1;
      }
      debugmsg68("in:%d out:%d ",
		 isf->c_stream.total_in,isf->c_stream.total_out);
      if (isf->gzip) {
	debugmsg68("crc:%08X ",isf->c_crc32);
      }
      if (!err) {
	int i, c, t;
	unsigned char hd[8];
	for (i=0, c=isf->c_crc32, t=isf->c_stream.total_in;
	     i<4;
	     ++i, c>>=8, t>>=8) {
	  hd[0+i] = (unsigned char)c;
	  hd[4+i] = (unsigned char)t;
	}
	err = istream_write(isf->is, hd, 8) != 8;
      }
      deflateEnd(&isf->c_stream);
    }

    if (isf->inflate) {
      debugmsg68("INFLATED ");
      if (isf->gzip) {
	debugmsg68("crc:%08X/%08X ",isf->c_crc32,isf->gz_crc32);
      }
      inflateEnd(&isf->c_stream);
      isf->inflate = 0;
    }
  }
  debugmsg68(" : %s\n", err?"FAILED":"SUCCESS");

  return err;
}

static int isf_open(istream_t * istream)
{
  istream_z_t * isf = (istream_z_t *)istream;
  int err;

  if (!isf || !isf->is || isf->inflate || isf->deflate) {
    return -1;
  }

/*   debugmsg68("istream_z::open [%s]\n",istream_filename(istream)); */

  memset(&isf->c_stream, 0 , sizeof(isf->c_stream));
  isf->c_stream.zalloc = isf_zcalloc;
  isf->c_stream.zfree = isf_zcfree;
  isf->c_crc32 = crc32(0, 0, 0);
  isf->pos = 0;
  isf->length = -1;
  isf->write_in = isf->c_stream.next_in = isf->buffer_in;
  isf->read_out = isf->c_stream.next_out = isf->buffer_out;

  err = 0;
  switch (isf->mode) {

  case ISTREAM_OPEN_READ:
/*     debugmsg68(" -> READ"); */
    if (isf->gzip) {
/*       debugmsg68(" GZIP"); */
      err = isf_read_gz_header(isf)
	|| inflateInit2(&isf->c_stream, -MAX_WBITS) != Z_OK;
    } else {
/*       debugmsg68(" INFLATE"); */
      err = inflateInit(&isf->c_stream) != Z_OK;
    }
    isf->inflate = !err;
/*     debugmsg68(" : %s\n", err ? "FAILED":"SUCCESS"); */

    break;

  case ISTREAM_OPEN_WRITE: {
    int level = isf->level;
/*     debugmsg68(" -> WRITE"); */
/*     debugmsg68(" lvl:%d st:%d", level, isf->strategy); */
    if (level > Z_BEST_COMPRESSION) {
      level = Z_DEFAULT_COMPRESSION;
    }
    if (isf->gzip) {
/*       debugmsg68(" GZIP"); */
      err = Z_OK != deflateInit2(&isf->c_stream, level, Z_DEFLATED,
				 -MAX_WBITS, DEF_MEM_LEVEL, isf->strategy)
	|| isf_write_gz_header(isf);
    } else {
/*       debugmsg68(" DEFLATE"); */
      err = Z_OK != deflateInit2(&isf->c_stream, level, Z_DEFLATED,
				 MAX_WBITS, DEF_MEM_LEVEL, isf->strategy);
    }
    isf->deflate = !err;
    isf->length = 0;
/*     debugmsg68(" : %s\n", err ? "FAILED":"SUCCESS"); */
  } break;
  default:
/*     debugmsg68(" -> INVALID MODE ?? (%d)\n", isf->mode); */
    err = -1;
  }

  if (err) {
    isf_close(istream);
    err = -1;
  }
  return err;
}


static int isf_read(istream_t * istream, void * data, int n)
{
  istream_z_t * isf = (istream_z_t *)istream;
  int bytes;

  if (!isf || !isf->inflate) {
    return -1;
  } else if (!n) {
    return 0;
  }

/*   debugmsg68("READ:%d\n",n); */
  for (bytes=n; bytes>0;) {
    int n;

    /* Get number of bytes in output buffer. */
    n = isf->c_stream.next_out - isf->read_out;

    if (!n) {
      n = isf_inflate_buffer(isf);
      if (n == -1) {
	return -1;
      }
      if (!n) {
	break;
      }
    }
    if (n > bytes) {
      n = bytes;
    }
    memcpy (data, isf->read_out, n);
/*     debugmsg68("COPY:%d\n",n); */
    data = (char *) data + n;
    isf->read_out += n;
    isf->pos += n;
    bytes -= n;
  }
/*   debugmsg68("READ -> %d\n",n-bytes); */
  return n - bytes;
}

static int isf_write(istream_t * istream, const void * data, int n)
{
  istream_z_t * isf = (istream_z_t *)istream;
  int bytes;

  if (!isf || !isf->deflate) {
    return -1;
  } else if (!n) {
    return 0;
  }

/*   debugmsg68("WRITE:%d\n",n); */

  for (bytes=n; bytes>0;) {
    int n;

    /* Get number of byte free in the input buffer. */
    n = isf->buffer_in + sizeof(isf->buffer_in) - isf->write_in;
/*     debugmsg68("bytes:%d, free_in:%d\n", bytes, n); */
    if (!n) {
      /* No more bytes, it is time to compress and write some. */
      n = isf_deflate_buffer(isf,0);
      if (n == -1) {
	return -1;
      }
      if (!n) {
	break;
      }
    }
    if (n > bytes) {
      n = bytes;
    }
/*     debugmsg68("COPY:%d\n",n); */
    memcpy (isf->write_in, data, n);
    data = (char *) data + n;
    isf->write_in += n;
    isf->pos += n;
    bytes -= n;
  }

/*   debugmsg68("WRITE -> %d\n",n-bytes); */
  return n - bytes;
}

/* We could have store the length value at opening, but this way it handles
 * dynamic changes of curl size.
 */
static int isf_length(istream_t * istream)
{
  istream_z_t * isf = (istream_z_t *)istream;

  return (!isf)
    ? -1
    : isf->length;
}

static int isf_tell(istream_t * istream)
{
  istream_z_t * isf = (istream_z_t *)istream;
  return (!isf)
    ? -1
    : isf->pos;
}

static int isf_seek(istream_t * istream, int offset)
{
  istream_z_t * isf = (istream_z_t *)istream;
  int pos;

  if (!isf ) {
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
/*   istream_z_t * isf = (istream_z_t *)istream; */
  SC68free(istream);
}

static const istream_t istream_z = {
  isf_name,
  isf_open, isf_close,
  isf_read, isf_write,
  isf_length, isf_tell, isf_seek, isf_seek,
  isf_destroy
};

/** Default gzip option. */
const istream_z_option_t istream_z_default_option = {
  1,   /* gzip      */
  3,   /* level     */
  0,   /* strategy  */
  1    /* name      */
};

istream_t * istream_z_create(istream_t * is, int mode,
			     const istream_z_option_t opt)
{
  istream_z_t * isf;

  if (!is) {
    return 0;
  }
  
  if (!(1&(mode^(mode>>1)))) {
    return 0;
  }
 
  isf = SC68alloc(sizeof(istream_z_t));
  if (!isf) {
    return 0;
  }

  /* Copy istream functions. */
  memset(isf,0,sizeof(*isf));
  memcpy(&isf->istream, &istream_z, sizeof(istream_z));
  /* Setup */
  isf->is = is;
  isf->mode = mode & ISTREAM_OPEN_MASK;
  isf->length = -1;
  isf->org_pos = istream_tell(is);
  /* Setup gzip option. */
  isf->gzip = opt.gzip;
  isf->level = opt.level;
  isf->strategy = opt.strategy;
  isf->gz_name = opt.name;

/*   debugmsg68("istream_z::[%s],%c gz:%c level:%d name:%c\n", */
/* 	     istream_filename(isf->is), (mode&1)?'R':'W', */
/* 	     isf->gzip?'Y':'N', isf->level, isf->gz_name?'Y':'N'); */

  return &isf->istream;
}

#else /* #ifndef HAVE_ZLIB_H */

/* istream Z must not be include in this package. Anyway the creation
 * function still exists but it always returns an error.
 */
//commented out by blazer
//typedef struct _istream_t istream_t;

istream_t * istream_z_create(istream_t * is, int mode,
			     const istream_z_option_t opt)
{
  return 0;
}

#endif /* #ifndef ISTREAM_NO_CURL */

