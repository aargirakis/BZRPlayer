
#ifndef __AUDIO_H__
#define __AUDIO_H__

void ws_audio_init(void);
void ws_audio_reset(void);
void ws_audio_done(void);
void ws_audio_update(short *buffer, int length);
void ws_audio_port_write(BYTE port,BYTE value);
BYTE ws_audio_port_read(BYTE port);
void ws_audio_process(void);
void ws_audio_sounddma(void);
int WaveAdrs;

#endif
