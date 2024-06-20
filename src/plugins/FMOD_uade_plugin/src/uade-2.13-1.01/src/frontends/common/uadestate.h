#ifndef _UADE_STATE_H_
#define _UADE_STATE_H_

#include <sys/types.h>
#include <unistd.h>

#include <eagleplayer.h>
#include <effects.h>
#ifndef EMSCRIPTEN
#include <uadeipc.h>
#endif

struct uade_state {
	/* Per song members */
	struct uade_config config;
	struct uade_song *song;
	struct uade_effect effects;
	struct eagleplayer *ep;

	/* Permanent members */
	int validconfig;
	struct eagleplayerstore *playerstore;
#ifndef EMSCRIPTEN
	struct uade_ipc ipc;
#endif
	pid_t pid;
};

#endif
