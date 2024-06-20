/**
 * @ingroup file68_istream68_devel
 * @file    file68/istream68_z.h
 * @author  benjamin gerard
 * @date    2003/10/06
 * @brief   Z stream header.
 *
 * $Id: istream68_z.h 503 2005-06-24 08:52:56Z loke $
 */

/* Copyright (C) 1998-2003 Benjamin Gerard */

#ifndef _ISTREAM68_Z_H_
#define _ISTREAM68_Z_H_

#include "file68/istream68.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @name      Z stream
 *  @ingroup   file68_istream68_devel
 *
 *  @anchor zlib
 *
 *    Implements istream_t for Z library.
 *    Z is a famous compression/decompression library. For more informations
 *    see <a href="http://www.gzip.org">gzip website</a>.
 *
 *  @todo      deflate (compression) mode.
 *  @{
 */

/** gzip options. */
typedef struct {
  unsigned int gzip:1;     /**< Read/Write gzip file format.                */
  unsigned int level:4;    /**< Compression level [0..9] or -1 for default. */
  unsigned int strategy:3; /**< Compression strategy (0 is default).        */
  unsigned int name:1;     /**< Include original name to gzip.              */
} istream_z_option_t;

/** Default gzip option. */
extern const istream_z_option_t istream_z_default_option;

/** Create a @ref zlib "Z" stream.
 *
 *  @param  is     Stream to compress/decompress.
 *  @param  mode   bit-0: read access, bit-1: write access.
 *  @param  opt    gzip options.
 *
 *  @return stream
 *  @retval 0 on error
 */
istream_t * istream_z_create(istream_t * is, int mode,
			     const istream_z_option_t opt);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif /* #define _ISTREAM68_Z_H_ */
