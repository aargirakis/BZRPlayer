#pragma once

#include "types.h"

extern int SampleRate;
extern BYTE *ROM;
extern int ROMSize;
extern int ROMBank;

int Get_FirstSong(void);
void Init_WSR(void);
void Reset_WSR(int SongNo);
void Close_WSR(void);
void Update_WSR(int Cycles, int Length);
void Update_SampleData(void);

void ws_timer_reset(void);
void ws_timer_count(int Cycles);
void ws_timer_set(int no, int timer);
void ws_timer_update(void);
int ws_timer_min(int Cycles);
