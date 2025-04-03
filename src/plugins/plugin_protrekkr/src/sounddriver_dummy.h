#ifndef _SOUNDDRIVER_DUMMY_H_
#define _SOUNDDRIVER_DUMMY_H_

#include <cstdint>

#define STDCALL
#define FALSE 0
#define TRUE 1

typedef uint32_t Uint32;
typedef uint8_t Uint8;
typedef int32_t int32;
typedef int8_t int8;

static constexpr int AUDIO_Latency = 0;

inline void AUDIO_ResetTimer(void)
{
}

inline int AUDIO_GetSamples(void) { return 0; }

#endif
