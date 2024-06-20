#ifndef S_SNG_H__
#define S_SNG_H__

#include "kmsnddev.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	SNG_TYPE_SN76496,	/* TI SN76496 (TI TMS9919 VDP) */
	SNG_TYPE_GAMEGEAR	/* SEGA custom VDP */
};

KMIF_SOUND_DEVICE *SNGSoundAlloc(Uint32 sng_type);

#ifdef __cplusplus
}
#endif

#endif /* S_SNG_H__ */
