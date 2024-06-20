#ifndef WINMMOUT_H
#define WINMMOUT_H

#include <windows.h>
#include <stdint.h>

typedef void (WinmmCallBack)(LPSTR buf);

typedef struct WinmmFormat WinmmFormat;
struct WinmmFormat {
    int sampleRate;
    int chanNr;
    int bits;
    int bufLength;
    int bufNr;
};

void winmm_enterCrit();
void winmm_leaveCrit();
void winmm_closeMixer();
BOOL winmm_openMixer(WinmmFormat* wf, WinmmCallBack* cb);

#endif
