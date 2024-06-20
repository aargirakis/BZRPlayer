/**
 * @ingroup file68_istream68_devel
 * @file    file68/istream68_curl.h
 * @author  benjamin gerard
 * @date    2003/08/08
 * @brief   @ref cURL stream header.
 *
 * $Id: istream68_curl.h 503 2005-06-24 08:52:56Z loke $
 *
 */

/* Copyright (C) 1998-2003 Benjamin Gerard */

#ifndef _ISTREAM68_CURL_H_
#define _ISTREAM68_CURL_H_

#include "file68/istream68.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @name cURL stream
 *  @ingroup file68_istream68_devel
 *
 *  @anchor cURL
 *
 *    @b cURL is a client-side URL transfer library. For more informations
 *    see <a href="http://curl.planetmirror.com/libcurl/">cURL website</a>.
 *
 *  @{
 */

/** Initialize curl engine.
 *
 *    The istream_curl_init() function initializes curl library.  It
 *    is called by the file68_init() function and everytime a new curl
 *    stream is created with the istream_curl_create() function.
 *
 *  @return error code
 *  @retval  0   success
 *  @retval  -1  failure
 */
int istream_curl_init(void);

/** Shutdown curl engine.
 *
 *    The istream_curl_shutdoen() function shutdown curl library. It
 *    is called by the file68_shutdown() function.
 */
void istream_curl_shutdown(void);

/** Creates an URL based stream using @ref cURL.
 *
 *  @param  url    URL
 *  @param  mode   bit-0: read access, bit-1: write access.
 *
 *  @return stream
 *  @retval 0 on error
 *
 *  @note     url is internally copied.
 *  @todo     Implement write mode.
 */
istream_t * istream_curl_create(const char * url, int mode);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif /* #define _ISTREAM68_CURL_H_ */
