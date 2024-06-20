#ifndef	AUDIO_DEVICE_H
#define	AUDIO_DEVICE_H

/* Poor man's audio.device implementation.
 *
 * Copyright 2022, Juergen Wothke
 *
 * 
 * This is only a "proof of concept" type implementation that
 * covers those features used by the MaxTrax player.
 *
 */ 
#include <stdint.h>

/*
 * known limitation: impl expects a big-endian environment and the
 * respective *SwapEndian() functions would need to be made no-ops on 
 * little-endian machines..
 */
uint16_t wordSwapEndian(uint16_t in);
uint32_t intSwapEndian(uint32_t in);

void audiodevice_reset();
void audiodevice_open(uint32_t al_msgAddr);
void audiodevice_beginIO(uint32_t al_msgAddr);
void audiodevice_abortIO(uint32_t al_msgAddr);


#endif	/* AUDIO_DEVICE_H */
