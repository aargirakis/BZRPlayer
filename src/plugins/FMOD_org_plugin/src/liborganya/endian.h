/*
Copyright (C) 2004 Hollis Blanchard
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __ENDIAN_HPP__
#define __ENDIAN_HPP__

#include <inttypes.h>

static inline uint16_t __swap16(uint16_t val)
{
    return  (((uint16_t)(val) & (uint16_t)0x00ffU) << 8) |
            (((uint16_t)(val) & (uint16_t)0xff00U) >> 8);
}

static inline uint32_t __swap32(uint32_t val)
{
    return  (((uint32_t)(val) & (uint32_t)0xff000000U) >> 24) |
            (((uint32_t)(val) & (uint32_t)0x00ff0000U) >> 8) |
            (((uint32_t)(val) & (uint32_t)0x0000ff00U) << 8) |
            (((uint32_t)(val) & (uint32_t)0x000000ffU) << 24);
}

#ifdef WORDS_BIGENDIAN

static inline uint16_t htol16(uint16_t val) { return __swap16(val); }
static inline uint32_t htol32(uint32_t val) { return __swap32(val); }

static inline uint16_t ltoh16(uint16_t val) { return __swap16(val); }
static inline uint32_t ltoh32(uint32_t val) { return __swap32(val); }

static inline uint16_t htob16(uint16_t val) { return val; }
static inline uint32_t htob32(uint32_t val) { return val; }

static inline uint16_t btoh16(uint16_t val) { return val; }
static inline uint32_t btoh32(uint32_t val) { return val; }

#else // WORDS_BIGENDIAN

/** convert a 16bit value from host to little-endian format */
static inline uint16_t htol16(uint16_t val) { return val; }
/** convert a 32bit value from host to little-endian format */
static inline uint32_t htol32(uint32_t val) { return val; }

/** convert a 16bit value from little-endian to host format */
static inline uint16_t ltoh16(uint16_t val) { return val; }
/** convert a 32bit value from little-endian to host format */
static inline uint32_t ltoh32(uint32_t val) { return val; }

/** convert a 16bit value from host to big-endian format */
static inline uint16_t htob16(uint16_t val) { return __swap16(val); }
/** convert a 32bit value from host to big-endian format */
static inline uint32_t htob32(uint32_t val) { return __swap32(val); }

/** convert a 16bit value from big-endian to host format */
static inline uint16_t btoh16(uint16_t val) { return __swap16(val); }
/** convert a 32bit value from big-endian to host format */
static inline uint32_t btoh32(uint32_t val) { return __swap32(val); }

#endif // WORDS_BIGENDIAN

#endif // __ENDIAN_HPP__
