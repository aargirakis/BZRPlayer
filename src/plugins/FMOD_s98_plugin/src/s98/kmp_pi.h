#ifndef KMP_PI_H
#define KMP_PI_H
#include <windows.h>

/*
     KbMedia Player Plugin
*/

#define KMPMODULE_VERSION 100
#define KMP_GETMODULE     kmp_GetTestModule
#define SZ_KMP_GETMODULE "kmp_GetTestModule"

typedef void* HKMP;//'K'b'M'edia 'P'layer Plugin Handle

typedef struct {
    DWORD dwSamplesPerSec;
    DWORD dwChannels;
    DWORD dwBitsPerSample;
    DWORD dwLength;
    DWORD dwSeekable;

    DWORD dwUnitRender;
    DWORD dwReserved1;
    DWORD dwReserved2;
} SOUNDINFO;


#ifdef __cplusplus
extern "C" {
#endif

void *S98_OpenFromBuffer (const BYTE *buffer, DWORD dwSize, SOUNDINFO *pInfo);
void S98_Close (void *s98);
DWORD S98_Render (void *s98, BYTE *buffer, DWORD frames);
DWORD S98_SetPosition (void *s98, DWORD dwPos);

#ifdef __cplusplus
}
#endif

#endif
