/*
 * libxmms_asap.c - ASAP plugin for XMMS2
 *
 * Copyright (C) 2021  Piotr Fusik
 *
 * This file is part of ASAP (Another Slight Atari Player),
 * see http://asap.sourceforge.net
 *
 * ASAP is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * ASAP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ASAP; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <xmms/xmms_xformplugin.h>
#include <xmms/xmms_log.h>

#include "asap.h"

static void set_str(xmms_xform_t *xform, const char *key, const char *val)
{
	if (val[0] != '\0')
		xmms_xform_metadata_set_str(xform, key, val);
}

static gboolean xmms_asap_init(xmms_xform_t *xform)
{
	const char *url = xmms_xform_get_url(xform);
	uint8_t module[ASAPInfo_MAX_MODULE_LENGTH];
	xmms_error_t error;
	int moduleLen = xmms_xform_read(xform, module, sizeof(module), &error);
	if (moduleLen == -1) {
		XMMS_DBG("Error reading %s", url);
		return FALSE;
	}

	ASAP *asap = ASAP_New();
	if (!ASAP_Load(asap, url, module, moduleLen)) {
		ASAP_Delete(asap);
		XMMS_DBG("Error decoding %s", url);
		return FALSE;
	}
	const ASAPInfo *info = ASAP_GetInfo(asap);
	int song = ASAPInfo_GetDefaultSong(info);
	int duration = ASAPInfo_GetDuration(info, song);
	if (!ASAP_PlaySong(asap, song, duration)) {
		ASAP_Delete(asap);
		return FALSE;
	}

	xmms_xform_private_data_set(xform, asap);
	xmms_xform_outdata_type_add(xform,
		XMMS_STREAM_TYPE_MIMETYPE, "audio/pcm",
		XMMS_STREAM_TYPE_FMT_FORMAT, XMMS_SAMPLE_FORMAT_S16,
		XMMS_STREAM_TYPE_FMT_CHANNELS, ASAPInfo_GetChannels(info),
		XMMS_STREAM_TYPE_FMT_SAMPLERATE, ASAP_SAMPLE_RATE,
		XMMS_STREAM_TYPE_END);
	xmms_xform_metadata_set_int(xform, XMMS_MEDIALIB_ENTRY_PROPERTY_DURATION, duration);
	set_str(xform, XMMS_MEDIALIB_ENTRY_PROPERTY_ARTIST, ASAPInfo_GetAuthor(info));
	set_str(xform, XMMS_MEDIALIB_ENTRY_PROPERTY_TITLE, ASAPInfo_GetTitle(info));
	set_str(xform, XMMS_MEDIALIB_ENTRY_PROPERTY_YEAR, ASAPInfo_GetDate(info));
	return TRUE;
}

static void xmms_asap_destroy(xmms_xform_t *xform)
{
	ASAP *asap = (ASAP *) xmms_xform_private_data_get(xform);
	ASAP_Delete(asap);
}

static gint xmms_asap_read(xmms_xform_t *xform, xmms_sample_t *buf, gint len, xmms_error_t *err)
{
	ASAP *asap = (ASAP *) xmms_xform_private_data_get(xform);
	return ASAP_Generate(asap, buf, len, G_BYTE_ORDER == G_LITTLE_ENDIAN ? ASAPSampleFormat_S16_L_E : ASAPSampleFormat_S16_B_E);
}

static gint64 xmms_asap_seek(xmms_xform_t *xform, gint64 samples, xmms_xform_seek_mode_t whence, xmms_error_t *err)
{
	g_return_val_if_fail(whence == XMMS_XFORM_SEEK_SET, -1);
	ASAP *asap = (ASAP *) xmms_xform_private_data_get(xform);
	return ASAP_SeekSample(asap, samples) ? samples : -1;
}

static gboolean xmms_asap_setup(xmms_xform_plugin_t *xform_plugin)
{
	xmms_xform_methods_t methods;
	XMMS_XFORM_METHODS_INIT(methods);
	methods.init = xmms_asap_init;
	methods.destroy = xmms_asap_destroy;
	methods.read = xmms_asap_read;
	methods.seek = xmms_asap_seek;

	xmms_xform_plugin_methods_set(xform_plugin, &methods);
	xmms_xform_plugin_indata_add(xform_plugin, XMMS_STREAM_TYPE_MIMETYPE, "application/x-sap", XMMS_STREAM_TYPE_END);
	xmms_magic_add("SAP file", "application/x-sap", "0 string SAP", NULL);
	xmms_magic_extension_add("application/x-sap", "*.sap");
	return true;
}

XMMS_XFORM_PLUGIN("asap",
	"ASAP decoder", ASAPInfo_VERSION,
	"Another Slight Atari Player decoder",
	xmms_asap_setup);
