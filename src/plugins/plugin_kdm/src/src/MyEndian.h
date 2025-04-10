// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#ifndef MYENDIAN_H
#define MYENDIAN_H

// Convert high bytes and low bytes of MSW and LSW to 32-bit word.
// Used to read 32-bit words in little-endian order.
inline uint32_t readEndian(u_int8_t hihi, u_int8_t hilo, u_int8_t hi, u_int8_t lo)
{
  return(( (uint32_t)hihi << 24 ) + ( (uint32_t)hilo << 16 ) +
		 ( (uint32_t)hi << 8 ) + (uint32_t)lo );
}

#endif  // MYENDIAN_H
