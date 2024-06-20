#ifndef _AMIGADEPACKER_COMPAT_H_
#define _AMIGADEPACKER_COMPAT_H_

#include <stdint.h>
//#include "config.h"

static uint16_t read_be_u16(void *s)
{
  uint8_t *ptr = (uint8_t *) s;
  return ptr[1] + (ptr[0] << 8);
}

static uint32_t read_be_u32(void *s)
{
  uint8_t *ptr = (uint8_t *) s;
  return (ptr[0] << 24) + (ptr[1] << 16) + (ptr[2] << 8) + ptr[3];
}

static inline int16_t read_be_s16(void *s)
{
    return (int16_t) read_be_u16(s);
}

static inline int32_t read_be_s32(void *s)
{
    return (int32_t) read_be_u32(s);
}

#ifdef NO_MKSTEMP
int mkstemp(char *template);
#endif

#endif
