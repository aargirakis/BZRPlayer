#define SAMPRATE 44100
#define MIX_BUF_SAMPLES 2048
#define MIX_BUF_NUM 2
#define MAX_FN 256

#define WIN32_LEAN_AND_MEAN // for stripping windows.h include

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "winmmout.h"
#include "jxs.h"
#include "jaytrax.h"

WinmmFormat wf = {44100, 2, 16, 512, 8};

static char fileName[MAX_FN];
static JT1Player* jay;
static char isPlaying;

static BOOL getKeydownVK(DWORD* out) {
    DWORD iEvNum, dummy;
    INPUT_RECORD ir;
    static HANDLE hIn = NULL;
    
    if (!hIn) hIn = GetStdHandle(STD_INPUT_HANDLE);
    
    GetNumberOfConsoleInputEvents(hIn, &iEvNum);
    while (iEvNum--) {
        ReadConsoleInput(hIn, &ir, 1, &dummy);
        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
            if (out) *out = ir.Event.KeyEvent.wVirtualKeyCode;
            return TRUE;
        }
    }
    return FALSE;
}

static void pressAnyKey() {
    while (!getKeydownVK(NULL)) SleepEx(1,1);
}

static void clearScreen() {
   COORD coordScreen = {0, 0};
   DWORD cCharsWritten;
   CONSOLE_SCREEN_BUFFER_INFO csbi; 
   DWORD dwConSize;
   static HANDLE hOut = NULL;
   
   if (!hOut) hOut = GetStdHandle(STD_OUTPUT_HANDLE);
   if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;
   dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
   if (!FillConsoleOutputCharacter(hOut, (TCHAR) ' ', dwConSize, coordScreen, &cCharsWritten)) return;
   if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;
   if (!FillConsoleOutputAttribute(hOut, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten)) return;
   SetConsoleCursorPosition(hOut, coordScreen);
}

static void updateDisplay() {
    if (!jay || !jay->song) return;
    clearScreen();
    
    printf("Filename:  %s\n", &fileName[0]);
    printf("Songtitle: %s\n", &jay->subsong->name);
    printf("Subsong:   %02d/%02d\n", jay->subsongNr+1, jay->song->nrofsongs);
    printf("Interpol.: %s\n", &jay->itp->name);
    printf("\n");
    printf("Change subsong number with F1 and F2.\n");
    printf("Change interpolations with F3 and F4.\n");
    printf("Pause and resume playback with space.\n");
    printf("Exit with ESC.\n");
}

static void audioCB(LPSTR buf) {
    jaytrax_renderChunk(jay, (int16_t*)buf, wf.bufLength, wf.sampleRate);
}

//extracts filename from a path. Length includes \0
static void exFnameFromPath(char* dest, char* src, int max) {
    int i   = 0;
    int len = strlen(src);
    char* p = src+len;
    
    if (!len || !max) return;
    if (max > len) max = len;
    while (i<max && *p!='\\') {;p--;i++;}
    memcpy(dest, p+1, i);
}

int main(int argc, char* argv[]) {
    #define FAIL(x) {printf("%s\n", (x)); goto _ERR;}
    JT1Song* song;
    
    memset(&fileName[0], 34, MAX_FN);
    
    if (argc != 2) {
        printf("Usage: jaytrax.exe <module>\n");
        return (1);
    }
    
    exFnameFromPath(&fileName[0], argv[1], MAX_FN);
    jay = jaytrax_init();
    
    if (jxsfile_readSong(argv[1], &song)==0) {
        if (jaytrax_loadSong(jay, song)) {
            if (winmm_openMixer(&wf, &audioCB)) {
                updateDisplay();
                isPlaying = 1;
                for (;;) {
                    DWORD vk;
                    
                    if (getKeydownVK(&vk)) {
                        int subtune       = jay->subsongNr;
                        int subtune_total = jay->song->nrofsongs;
                        int interp        = jay->itp->id;
                        
                        winmm_enterCrit();
                        if (vk == VK_ESCAPE) {
                            winmm_leaveCrit();
                            winmm_closeMixer();
                            break;
                        } else if (vk == VK_F1) {
                            subtune = --subtune<0 ? subtune_total-1 : subtune;
                            jaytrax_changeSubsong(jay, subtune);
                        } else if (vk == VK_F2) {
                            subtune = (subtune+1) % subtune_total;
                            jaytrax_changeSubsong(jay, subtune);
                            isPlaying = 1;
                        } else if (vk == VK_F3) {
                            interp = --interp<0 ? INTERP_COUNT-1 : interp;
                            jaytrax_setInterpolation(jay, interp);
                        } else if (vk == VK_F4) {
                            interp = (interp+1) % INTERP_COUNT;
                            jaytrax_setInterpolation(jay, interp);
                        } else if (vk == VK_SPACE) {
                            isPlaying ? jaytrax_pauseSong(jay) : jaytrax_continueSong(jay);
                            isPlaying = !isPlaying;
                        }
                        winmm_leaveCrit();
                        updateDisplay();
                    }
                    SleepEx(1, 1);
                    // do other stuff
                }
            } else FAIL("Cannot open mixer.");
        } else FAIL("Invalid song.")
    } else FAIL("Cannot load file.");
    
    return 0;
    _ERR:
        pressAnyKey();
        return 1;
    #undef FAIL
}
