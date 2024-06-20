#include "tn_sc68.h"
#include "TN_SC68.bin_rev.h"
#include <stdlib.h>
#include <string.h>

const char verstag[] = VERSTAG;

struct ExecIFace *IExec;
struct Interface *INewlib;

static void *sc68_alloc (unsigned int size);
static void sc68_free (void *mem);
static int32 sc68_open (struct TNDecHandle *handle, char *openme);
static void sc68_close (struct TNDecHandle *handle);
static int32 sc68_process (struct TNDecHandle *handle, void *buf, uint32 len);
static void sc68_seek (struct TNDecHandle *handle, uint32 seconds);

sc68iface_t isc68 = {
	sc68_open,
	sc68_close,
	sc68_process,
	sc68_seek
};

sc68iface_t *_start (char *argstr, int32 arglen, struct ExecBase *SysBase) {
	IExec = (struct ExecIFace *)SysBase->MainInterface;
	return &isc68;
}

#if 0
static void *sc68_alloc (unsigned int size) {
	return malloc(size);
}

static void sc68_free (void *mem) {
	free(mem);
}
#endif

static int32 sc68_open (struct TNDecHandle *handle, char *openme) {
	api68_init_t init68;
	int code;

	INewlib = handle->newlib;

	memset(&init68, 0, sizeof(init68));
	init68.alloc = malloc;
	init68.free = free;
	handle->sc68 = api68_init(&init68);
	if (!handle->sc68) return FALSE;

	if (api68_load_file(handle->sc68, openme)) return FALSE;

	api68_play(handle->sc68, -1, -1);

	handle->code = api68_process(handle->sc68, NULL, 0);
	if (handle->code & API68_END) return FALSE;

	if (api68_music_info(handle->sc68, &handle->info, -1, NULL)) return FALSE;

	return TRUE;
}

static void sc68_close (struct TNDecHandle *handle) {
	api68_shutdown(handle->sc68);
	handle->sc68 = NULL;
	INewlib = NULL;
}

static int32 sc68_process (struct TNDecHandle *handle, void *buf, uint32 len) {
	if (handle->code & API68_END) return 0;
	handle->code = api68_process(handle->sc68, buf, len);
	return len;
}

static void sc68_seek (struct TNDecHandle *handle, uint32 seconds) {
	api68_seek(handle->sc68, seconds * 1000, NULL);
}
