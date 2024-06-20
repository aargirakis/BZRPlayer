/* Copyright (c) 2013-2016 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <mgba/core/core.h>

#include <mgba/core/log.h>
#include <mgba/core/serialize.h>
#include <mgba-util/vfs.h>

#ifdef M_CORE_GB
#include <mgba/gb/core.h>
// TODO: Fix layering violation
#include <mgba/internal/gb/gb.h>
#endif
#ifdef M_CORE_GBA
#include <mgba/gba/core.h>
#include <mgba/internal/gba/gba.h>
#endif

static const struct mCoreFilter {
	bool (*filter)(struct VFile*);
	struct mCore* (*open)(void);
	enum mPlatform platform;
} _filters[] = {
#ifdef M_CORE_GBA
	{ GBAIsROM, GBACoreCreate, PLATFORM_GBA },
#endif
#ifdef M_CORE_GB
	{ GBIsROM, GBCoreCreate, PLATFORM_GB },
#endif
	{ 0, 0, PLATFORM_NONE }
};

struct mCore* mCoreFindVF(struct VFile* vf) {
	if (!vf) {
		return NULL;
	}
	struct mCoreFilter* filter;
	for (filter = &_filters[0]; filter->filter; ++filter) {
		if (filter->filter(vf)) {
			break;
		}
	}
	if (filter->open) {
		return filter->open();
	}
	return NULL;
}

enum mPlatform mCoreIsCompatible(struct VFile* vf) {
	if (!vf) {
		return false;
	}
	struct mCoreFilter* filter;
	for (filter = &_filters[0]; filter->filter; ++filter) {
		if (filter->filter(vf)) {
			return filter->platform;
		}
	}
	return PLATFORM_NONE;
}

#if !defined(MINIMAL_CORE) || MINIMAL_CORE < 2
#include <mgba-util/png-io.h>

#ifdef PSP2
#include <psp2/photoexport.h>
#endif

struct mCore* mCoreFind(const char* path) {
	struct VDir* archive = VDirOpenArchive(path);
	struct mCore* core = NULL;
	if (archive) {
		struct VDirEntry* dirent = archive->listNext(archive);
		while (dirent) {
			struct VFile* vf = archive->openFile(archive, dirent->name(dirent), O_RDONLY);
			if (!vf) {
				dirent = archive->listNext(archive);
				continue;
			}
			core = mCoreFindVF(vf);
			vf->close(vf);
			if (core) {
				break;
			}
			dirent = archive->listNext(archive);
		}
		archive->close(archive);
	} else {
		struct VFile* vf = VFileOpen(path, O_RDONLY);
		if (!vf) {
			return NULL;
		}
		core = mCoreFindVF(vf);
		vf->close(vf);
	}
	if (core) {
		return core;
	}
	return NULL;
}

bool mCoreLoadFile(struct mCore* core, const char* path) {
	struct VFile* rom = mDirectorySetOpenPath(&core->dirs, path, core->isROM);
	if (!rom) {
		return false;
	}

	bool ret = core->loadROM(core, rom);
	if (!ret) {
		rom->close(rom);
	}
	return ret;
}

bool mCoreAutoloadSave(struct mCore* core) {
	return core->loadSave(core, mDirectorySetOpenSuffix(&core->dirs, core->dirs.save, ".sav", O_CREAT | O_RDWR));
}

bool mCoreAutoloadPatch(struct mCore* core) {
	return core->loadPatch(core, mDirectorySetOpenSuffix(&core->dirs, core->dirs.patch, ".ups", O_RDONLY)) ||
	       core->loadPatch(core, mDirectorySetOpenSuffix(&core->dirs, core->dirs.patch, ".ips", O_RDONLY)) ||
	       core->loadPatch(core, mDirectorySetOpenSuffix(&core->dirs, core->dirs.patch, ".bps", O_RDONLY));
}

bool mCoreSaveState(struct mCore* core, int slot, int flags) {
	struct VFile* vf = mCoreGetState(core, slot, true);
	if (!vf) {
		return false;
	}
	bool success = mCoreSaveStateNamed(core, vf, flags);
	vf->close(vf);
	if (success) {
		mLOG(STATUS, INFO, "State %i saved", slot);
	} else {
		mLOG(STATUS, INFO, "State %i failed to save", slot);
	}

	return success;
}

bool mCoreLoadState(struct mCore* core, int slot, int flags) {
	struct VFile* vf = mCoreGetState(core, slot, false);
	if (!vf) {
		return false;
	}
	bool success = mCoreLoadStateNamed(core, vf, flags);
	vf->close(vf);
	if (success) {
		mLOG(STATUS, INFO, "State %i loaded", slot);
	} else {
		mLOG(STATUS, INFO, "State %i failed to loaded", slot);
	}

	return success;
}

struct VFile* mCoreGetState(struct mCore* core, int slot, bool write) {
	char name[PATH_MAX];
	snprintf(name, sizeof(name), "%s.ss%i", core->dirs.baseName, slot);
	return core->dirs.state->openFile(core->dirs.state, name, write ? (O_CREAT | O_TRUNC | O_RDWR) : O_RDONLY);
}

void mCoreDeleteState(struct mCore* core, int slot) {
	char name[PATH_MAX];
	snprintf(name, sizeof(name), "%s.ss%i", core->dirs.baseName, slot);
	core->dirs.state->deleteFile(core->dirs.state, name);
}

void mCoreTakeScreenshot(struct mCore* core) {
#ifdef USE_PNG
	size_t stride;
	const void* pixels = 0;
	unsigned width, height;
	core->desiredVideoDimensions(core, &width, &height);
	struct VFile* vf;
#ifndef PSP2
	vf = VDirFindNextAvailable(core->dirs.screenshot, core->dirs.baseName, "-", ".png", O_CREAT | O_TRUNC | O_WRONLY);
#else
	vf = VFileMemChunk(0, 0);
#endif
	bool success = false;
	if (vf) {
		core->getPixels(core, &pixels, &stride);
		png_structp png = PNGWriteOpen(vf);
		png_infop info = PNGWriteHeader(png, width, height);
		success = PNGWritePixels(png, width, height, stride, pixels);
		PNGWriteClose(png, info);
#ifdef PSP2
		void* data = vf->map(vf, 0, 0);
		PhotoExportParam param = {
			0,
			NULL,
			NULL,
			NULL,
			{ 0 }
		};
		scePhotoExportFromData(data, vf->size(vf), &param, NULL, NULL, NULL, NULL, 0);
#endif
		vf->close(vf);
	}
	if (success) {
		mLOG(STATUS, INFO, "Screenshot saved");
		return;
	}
#else
	UNUSED(core);
#endif
	mLOG(STATUS, WARN, "Failed to take screenshot");
}
#endif

void mCoreInitConfig(struct mCore* core, const char* port) {
	mCoreConfigInit(&core->config, port);
}

void mCoreLoadConfig(struct mCore* core) {
#ifndef MINIMAL_CORE
	mCoreConfigLoad(&core->config);
#endif
	mCoreLoadForeignConfig(core, &core->config);
}

void mCoreLoadForeignConfig(struct mCore* core, const struct mCoreConfig* config) {
	mCoreConfigMap(config, &core->opts);
#ifndef MINIMAL_CORE
	mDirectorySetMapOptions(&core->dirs, &core->opts);
#endif
	if (core->opts.audioBuffers) {
		core->setAudioBufferSize(core, core->opts.audioBuffers);
	}
	core->loadConfig(core, config);
}

void mCoreSetRTC(struct mCore* core, struct mRTCSource* rtc) {
	core->rtc.custom = rtc;
	core->rtc.override = RTC_CUSTOM_START;
}
