#ifndef	AUDIO_DEVICE_H
#define	AUDIO_DEVICE_H

/* Poor man's audio.device implementation.
 *
 * Copyright 2022, Juergen Wothke
 *
 *
 * This is only a "proof of concept" type implementation that
 * covers those features that were used in my test songs.
 */
#include <stdint.h>

/*
 * known limitation: impl expects a big-endian environment and the
 * respective "swap*()" functions would need to be made no-ops on
 * little-endian machines..
 */

// toggle the endianness of a respective value
uint16_t swap16(uint16_t in);
uint32_t swap32(uint32_t in);

void audiodevice_reset();
void audiodevice_open(uint32_t al_msg_addr);
void audiodevice_beginIO(uint32_t al_msg_addr, uint32_t al_dst_addr);
void audiodevice_abortIO(uint32_t al_msg_addr);


#endif	/* AUDIO_DEVICE_H */
