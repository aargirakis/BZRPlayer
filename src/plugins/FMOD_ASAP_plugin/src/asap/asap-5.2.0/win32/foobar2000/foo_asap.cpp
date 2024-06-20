/*
 * foo_asap.cpp - ASAP plugin for foobar2000
 *
 * Copyright (C) 2006-2021  Piotr Fusik
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

#define _WINSOCKAPI_ /* prevents compilation errors */
#include <windows.h>
#include <string.h>

#include "aatr-stdio.h"
#include "asap.h"
#include "info_dlg.h"
#include "settings_dlg.h"

#define UNICODE /* NOT for info_dlg.h */
#include "foobar2000/SDK/foobar2000.h"

#define BITS_PER_SAMPLE    16
#define BUFFERED_BLOCKS    1024

/* Configuration --------------------------------------------------------- */

static const GUID preferences_guid =
	{ 0xf7c0a763, 0x7c20, 0x4b64, { 0x92, 0xbf, 0x11, 0xe5, 0x5d, 0x8, 0xe5, 0x53 } };

static const GUID song_length_guid =
	{ 0x810e12f0, 0xa695, 0x42d2, { 0xab, 0xc0, 0x14, 0x1e, 0xe5, 0xf3, 0xb3, 0xb7 } };
static cfg_int song_length(song_length_guid, -1);

static const GUID silence_seconds_guid =
	{ 0x40170cb0, 0xc18c, 0x4f97, { 0xaa, 0x06, 0xdb, 0xe7, 0x45, 0xb0, 0x7e, 0x1d } };
static cfg_int silence_seconds(silence_seconds_guid, -1);

static const GUID play_loops_guid =
	{ 0xf08d12f8, 0x58d6, 0x49fc, { 0xae, 0xc5, 0xf4, 0xd0, 0x2f, 0xb2, 0x20, 0xaf } };
static cfg_bool play_loops(play_loops_guid, false);

static const GUID mute_mask_guid =
	{ 0x8bd94472, 0x82f1, 0x4e77, { 0x95, 0x78, 0xe9, 0x84, 0x75, 0xad, 0x17, 0x04 } };
static cfg_int mute_mask(mute_mask_guid, 0);

static const GUID playing_info_guid =
	{ 0x8a2d4509, 0x405e, 0x482e, { 0xa1, 0x30, 0x3f, 0x0c, 0xd6, 0x16, 0x98, 0x59 } };
static cfg_bool playing_info_cfg(playing_info_guid, false);

void onUpdatePlayingInfo()
{
	playing_info_cfg = playing_info != FALSE; /* avoid warning C4800 */
}


/* Decoding -------------------------------------------------------------- */

inline bool has_ext(const char *path, const char *ext)
{
	size_t len = strlen(path);
	return len >= 4 && _stricmp(path + len - 4, ext) == 0;
}

class input_asap
{
	service_ptr_t<file> m_file;
	char *url = nullptr;
	BYTE module[ASAPInfo_MAX_MODULE_LENGTH];
	int module_len;
	ASAP * const asap;

	int get_song_duration(int song, bool play) const
	{
		const ASAPInfo *info = ASAP_GetInfo(asap);
		int duration = ASAPInfo_GetDuration(info, song);
		if (duration < 0) {
			if (play)
				ASAP_DetectSilence(asap, silence_seconds);
			return 1000 * song_length;
		}
		if (play)
			ASAP_DetectSilence(asap, 0);
		if (play_loops && ASAPInfo_GetLoop(info, song))
			return 1000 * song_length;
		return duration;
	}

	static void meta_set(file_info &p_info, const char *p_name, const char *p_value)
	{
		if (p_value[0] != '\0')
			p_info.meta_set(p_name, p_value);
	}

	static const char *empty_if_null(const char *s)
	{
		return s != nullptr ? s : "";
	}

public:

	static bool g_is_our_content_type(const char *p_content_type)
	{
		return false;
	}

	static bool g_is_our_path(const char *p_path, const char *p_extension)
	{
		return ASAPInfo_IsOurFile(p_path) != 0;
	}

	static GUID g_get_guid()
	{
		static const GUID guid =
			{ 0xe8790443, 0x3a6b, 0x47d9, { 0x80, 0x4c, 0x1, 0x58, 0x93, 0xbf, 0xfe, 0x96 } };
		return guid;
	}

	static const char *g_get_name()
	{
		return "ASAP";
	}

	static GUID g_get_preferences_guid()
	{
		return preferences_guid;
	}

	static bool g_is_low_merit()
	{
		return false;
	}

	input_asap() : asap(ASAP_New())
	{
	}

	~input_asap()
	{
		free(url);
		ASAP_Delete(asap);
	}

	void open(service_ptr_t<file> p_filehint, const char *p_path, t_input_open_reason p_reason, abort_callback &p_abort)
	{
		switch (p_reason) {
		case input_open_info_write:
			if (!has_ext(p_path, ".sap"))
				throw exception_io_unsupported_format();
			/* FALLTHROUGH */
		case input_open_decode:
			free(url);
			url = strdup(p_path);
			break;
		default:
			break;
		}
		if (p_filehint.is_empty())
			filesystem::g_open(p_filehint, p_path, filesystem::open_mode_read, p_abort);
		m_file = p_filehint;
		module_len = static_cast<int>(m_file->read(module, sizeof(module), p_abort));
		if (!ASAP_Load(asap, p_path, module, module_len))
			throw exception_io_unsupported_format();
	}

	t_uint32 get_subsong_count() const
	{
		return ASAPInfo_GetSongs(ASAP_GetInfo(asap));
	}

	t_uint32 get_subsong(t_uint32 p_index) const
	{
		return p_index;
	}

	void get_info(t_uint32 p_subsong, file_info &p_info, abort_callback &p_abort) const
	{
		int duration = get_song_duration(p_subsong, false);
		if (duration >= 0)
			p_info.set_length(duration / 1000.0);
		const ASAPInfo *info = ASAP_GetInfo(asap);
		p_info.info_set_int("channels", ASAPInfo_GetChannels(info));
		p_info.info_set_int("subsongs", ASAPInfo_GetSongs(info));
		meta_set(p_info, "composer", ASAPInfo_GetAuthor(info));
		meta_set(p_info, "title", ASAPInfo_GetTitle(info));
		meta_set(p_info, "date", ASAPInfo_GetDate(info));
	}

	t_filestats get_file_stats(abort_callback &p_abort) const
	{
		return m_file->get_stats(p_abort);
	}

	void decode_initialize(t_uint32 p_subsong, unsigned p_flags, abort_callback &p_abort) const
	{
		int duration = get_song_duration(p_subsong, true);
		if (!ASAP_PlaySong(asap, p_subsong, duration))
			throw exception_io_unsupported_format();
		const char *filename = url;
		if (foobar2000_io::_extract_native_path_ptr(filename))
			setPlayingSong(filename, p_subsong);
	}

	bool decode_run(audio_chunk &p_chunk, abort_callback &p_abort) const
	{
		ASAP_MutePokeyChannels(asap, mute_mask);

		int channels = ASAPInfo_GetChannels(ASAP_GetInfo(asap));
		int buffered_bytes = BUFFERED_BLOCKS * channels * (BITS_PER_SAMPLE / 8);
		BYTE buffer[BUFFERED_BLOCKS * 2 * (BITS_PER_SAMPLE / 8)];

		buffered_bytes = ASAP_Generate(asap, buffer, buffered_bytes,
			BITS_PER_SAMPLE == 8 ? ASAPSampleFormat_U8 : ASAPSampleFormat_S16_L_E);
		if (buffered_bytes == 0)
			return false;
		p_chunk.set_data_fixedpoint(buffer, buffered_bytes, ASAP_SAMPLE_RATE,
			channels, BITS_PER_SAMPLE,
			channels == 2 ? audio_chunk::channel_config_stereo : audio_chunk::channel_config_mono);
		return true;
	}

	void decode_seek(double p_seconds, abort_callback &p_abort) const
	{
		ASAP_Seek(asap, static_cast<int>(p_seconds * 1000));
	}

	bool decode_can_seek() const
	{
		return true;
	}

	bool decode_get_dynamic_info(file_info &p_out, double &p_timestamp_delta) const
	{
		return false;
	}

	bool decode_get_dynamic_info_track(file_info &p_out, double &p_timestamp_delta) const
	{
		return false;
	}

	void decode_on_idle(abort_callback &p_abort) const
	{
		m_file->on_idle(p_abort);
	}

	void retag_set_info(t_uint32 p_subsong, const file_info &p_info, abort_callback &p_abort) const
	{
		ASAPInfo *info = const_cast<ASAPInfo *>(ASAP_GetInfo(asap));
		ASAPInfo_SetAuthor(info, empty_if_null(p_info.meta_get("composer", 0)));
		ASAPInfo_SetTitle(info, empty_if_null(p_info.meta_get("title", 0)));
		ASAPInfo_SetDate(info, empty_if_null(p_info.meta_get("date", 0)));
	}

	void retag_commit(abort_callback &p_abort)
	{
		ASAPWriter *writer = ASAPWriter_New();
		if (writer == nullptr)
			throw exception_io_data();
		BYTE output[ASAPInfo_MAX_MODULE_LENGTH];
		ASAPWriter_SetOutput(writer, output, 0, sizeof(output));
		int output_len = ASAPWriter_Write(writer, url, ASAP_GetInfo(asap), module, module_len, FALSE);
		ASAPWriter_Delete(writer);
		if (output_len < 0)
			throw exception_io_unsupported_format();

		m_file.release();
		filesystem::g_open(m_file, url, filesystem::open_mode_write_new, p_abort);
		m_file->write(output, output_len, p_abort);
	}

	void set_logger(event_logger::ptr ptr) const
	{
	}

	typedef input_decoder interface_decoder_t;
	typedef input_info_reader interface_info_reader_t;
	typedef input_info_writer interface_info_writer_t;
};

static input_factory_t<input_asap> g_input_asap_factory;


/* Configuration --------------------------------------------------------- */

static preferences_page_callback::ptr g_callback;

static INT_PTR CALLBACK settings_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		settingsDialogSet(hDlg, song_length, silence_seconds, play_loops, mute_mask);
		return TRUE;
	case WM_COMMAND:
		switch (wParam) {
		case MAKEWPARAM(IDC_UNLIMITED, BN_CLICKED):
			enableTimeInput(hDlg, FALSE);
			g_callback->on_state_changed();
			return TRUE;
		case MAKEWPARAM(IDC_LIMITED, BN_CLICKED):
			enableTimeInput(hDlg, TRUE);
			setFocusAndSelect(hDlg, IDC_MINUTES);
			g_callback->on_state_changed();
			return TRUE;
		case MAKEWPARAM(IDC_SILENCE, BN_CLICKED):
		{
			BOOL enabled = (IsDlgButtonChecked(hDlg, IDC_SILENCE) == BST_CHECKED);
			EnableWindow(GetDlgItem(hDlg, IDC_SILSECONDS), enabled);
			if (enabled)
				setFocusAndSelect(hDlg, IDC_SILSECONDS);
			g_callback->on_state_changed();
			return TRUE;
		}
		case MAKEWPARAM(IDC_MINUTES, EN_CHANGE):
		case MAKEWPARAM(IDC_SECONDS, EN_CHANGE):
		case MAKEWPARAM(IDC_SILSECONDS, EN_CHANGE):
		case MAKEWPARAM(IDC_LOOPS, BN_CLICKED):
		case MAKEWPARAM(IDC_NOLOOPS, BN_CLICKED):
		case MAKEWPARAM(IDC_MUTE1, BN_CLICKED):
		case MAKEWPARAM(IDC_MUTE1 + 1, BN_CLICKED):
		case MAKEWPARAM(IDC_MUTE1 + 2, BN_CLICKED):
		case MAKEWPARAM(IDC_MUTE1 + 3, BN_CLICKED):
		case MAKEWPARAM(IDC_MUTE1 + 4, BN_CLICKED):
		case MAKEWPARAM(IDC_MUTE1 + 5, BN_CLICKED):
		case MAKEWPARAM(IDC_MUTE1 + 6, BN_CLICKED):
		case MAKEWPARAM(IDC_MUTE1 + 7, BN_CLICKED):
			g_callback->on_state_changed();
			return TRUE;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return FALSE;
}

class preferences_page_instance_asap : public preferences_page_instance
{
	const HWND m_parent;
	const HWND m_hWnd;

	int get_time_input() const
	{
		HWND hDlg = m_hWnd;
		if (IsDlgButtonChecked(hDlg, IDC_UNLIMITED) == BST_CHECKED)
			return -1;
		UINT minutes = GetDlgItemInt(hDlg, IDC_MINUTES, NULL, FALSE);
		UINT seconds = GetDlgItemInt(hDlg, IDC_SECONDS, NULL, FALSE);
		return static_cast<int>(60 * minutes + seconds);
	}

	int get_silence_input() const
	{
		HWND hDlg = m_hWnd;
		if (IsDlgButtonChecked(hDlg, IDC_SILENCE) != BST_CHECKED)
			return -1;
		return GetDlgItemInt(hDlg, IDC_SILSECONDS, NULL, FALSE);
	}

	bool get_loops_input() const
	{
		return IsDlgButtonChecked(m_hWnd, IDC_LOOPS) == BST_CHECKED;
	}

	int get_mute_input() const
	{
		HWND hDlg = m_hWnd;
		int mask = 0;
		for (int i = 0; i < 8; i++)
			if (IsDlgButtonChecked(hDlg, IDC_MUTE1 + i) == BST_CHECKED)
				mask |= 1 << i;
		return mask;
	}

public:

	preferences_page_instance_asap(HWND parent) : m_parent(parent),
		m_hWnd(CreateDialog(core_api::get_my_instance(), MAKEINTRESOURCE(IDD_SETTINGS), parent, ::settings_dialog_proc))
	{
	}

	t_uint32 get_state() override
	{
		if (song_length != get_time_input()
		 || silence_seconds != get_silence_input()
		 || play_loops != get_loops_input())
			return preferences_state::changed /* | preferences_state::needs_restart_playback */ | preferences_state::resettable;
		if (mute_mask != get_mute_input())
			return preferences_state::changed | preferences_state::resettable;
		return preferences_state::resettable;
	}

	HWND get_wnd() override
	{
		return m_hWnd;
	}

	void apply() override
	{
		song_length = get_time_input();
		silence_seconds = get_silence_input();
		play_loops = get_loops_input();
		mute_mask = get_mute_input();
		g_callback->on_state_changed();
	}

	void reset() override
	{
		settingsDialogSet(m_hWnd, -1, -1, FALSE, 0);
		g_callback->on_state_changed();
	}
};

class preferences_page_asap : public preferences_page_v3
{
public:

	preferences_page_instance::ptr instantiate(HWND parent, preferences_page_callback::ptr callback) override
	{
		g_callback = callback;
		return new service_impl_t<preferences_page_instance_asap>(parent);
	}

	const char *get_name() override
	{
		return "ASAP";
	}

	GUID get_guid() override
	{
		return preferences_guid;
	}

	GUID get_parent_guid() override
	{
		return guid_input;
	}

	bool get_help_url(pfc::string_base &p_out) override
	{
		p_out = "http://asap.sourceforge.net/";
		return true;
	}
};

static service_factory_single_t<preferences_page_asap> g_preferences_page_asap_factory;


/* File types ------------------------------------------------------------ */

class input_file_type_asap : public service_impl_single_t<input_file_type>
{
	static constexpr const char *names_and_masks[][2] = {
		{ "Slight Atari Player", "*.SAP" },
		{ "Chaos Music Composer", "*.CMC;*.CM3;*.CMR;*.CMS;*.DMC" },
		{ "Delta Music Composer", "*.DLT" },
		{ "Music ProTracker", "*.MPT;*.MPD" },
		{ "Raster Music Tracker", "*.RMT" },
		{ "Theta Music Composer 1.x", "*.TMC;*.TM8" },
		{ "Theta Music Composer 2.x", "*.TM2" },
		{ "Future Composer", "*.FC" }
	};

public:

	unsigned get_count() override
	{
		return ARRAYSIZE(names_and_masks);
	}

	bool get_name(unsigned idx, pfc::string_base &out) override
	{
		if (idx < ARRAYSIZE(names_and_masks)) {
			out = names_and_masks[idx][0];
			return true;
		}
		return false;
	}

	bool get_mask(unsigned idx, pfc::string_base &out) override
	{
		if (idx < ARRAYSIZE(names_and_masks)) {
			out = names_and_masks[idx][1];
			return true;
		}
		return false;
	}

	bool is_associatable(unsigned idx) override
	{
		return true;
	}
};

static service_factory_single_t<input_file_type_asap> g_input_file_type_asap_factory;


/* Info window ----------------------------------------------------------- */

class info_menu : public mainmenu_commands
{
	t_uint32 get_command_count() override
	{
		return 1;
	}

	GUID get_command(t_uint32 p_index) override
	{
		static const GUID guid = { 0x15a24bcd, 0x176b, 0x4e15, { 0xbc, 0xb1, 0x24, 0xfd, 0x92, 0x67, 0xaf, 0x8f } };
		return guid;
	}

	void get_name(t_uint32 p_index, pfc::string_base &p_out) override
	{
		p_out = "ASAP info";
	}

	bool get_description(t_uint32 p_index, pfc::string_base &p_out) override
	{
		p_out = "Activates the ASAP File Information window.";
		return true;
	}

	GUID get_parent() override
	{
		return mainmenu_groups::view;
	}

	bool get_display(t_uint32 p_index, pfc::string_base &p_text, t_uint32 &p_flags) override
	{
		p_flags = 0;
		get_name(p_index, p_text);
		return true;
	}

	void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback) override
	{
		if (p_index == 0) {
			playing_info = playing_info_cfg;
			const char *filename = nullptr;
			int song = -1;
			metadb_handle_ptr item;
			if (static_api_ptr_t<playlist_manager>()->activeplaylist_get_focus_item_handle(item) && item.is_valid()) {
				const char *url = item->get_path();
				if (foobar2000_io::_extract_native_path_ptr(url)) {
					filename = url;
					song = item->get_subsong_index();
				}
			}
			showInfoDialog(core_api::get_my_instance(), core_api::get_main_window(), filename, song);
		}
	}
};

static mainmenu_commands_factory_t<info_menu> g_info_menu_factory;


/* ATR filesystem -------------------------------------------------------- */

class file_atr : public file_readonly
{
	AATRFileStream *stream = nullptr;

public:

	file_atr(const char *atr_filename, const char *inside_filename)
	{
		AATR *disk = AATRStdio_New(atr_filename);
		if (disk == nullptr)
			throw exception_io_data();
		AATRDirectory *directory = AATRDirectory_New();
		if (directory == nullptr) {
			AATRStdio_Delete(disk);
			throw exception_out_of_resources();
		}
		AATRDirectory_OpenRoot(directory, disk);
		if (!AATRDirectory_FindEntryRecursively(directory, inside_filename) || AATRDirectory_IsEntryDirectory(directory)) {
			AATRDirectory_Delete(directory);
			AATRStdio_Delete(disk);
			throw exception_io_not_found();
		}
		stream = AATRFileStream_New();
		if (stream == nullptr) {
			AATRDirectory_Delete(directory);
			AATRStdio_Delete(disk);
			throw exception_out_of_resources();
		}
		AATRFileStream_Open(stream, directory);
		AATRDirectory_Delete(directory);
	}

	~file_atr()
	{
		if (stream != nullptr) {
			AATR *disk = const_cast<AATR *>(AATRFileStream_GetDisk(stream));
			AATRFileStream_Delete(stream);
			AATRStdio_Delete(disk);
		}
	}

	t_size read(void *p_buffer, t_size p_bytes, abort_callback &p_abort) override
	{
		int length = p_bytes < INT_MAX ? static_cast<int>(p_bytes) : INT_MAX;
		int result = AATRFileStream_Read(stream, static_cast<byte *>(p_buffer), 0, length);
		if (result < 0)
			throw exception_io_data();
		return result;
	}

	t_filesize get_size(abort_callback &p_abort) override
	{
		return AATRFileStream_GetLength(stream);
	}

	t_filesize get_position(abort_callback &p_abort) override
	{
		return AATRFileStream_GetPosition(stream);
	}

	void seek(t_filesize p_position, abort_callback &p_abort) override
	{
		int position = static_cast<int>(p_position);
		if (position != p_position || !AATRFileStream_SetPosition(stream, position))
			throw exception_io_seek_out_of_range();
	}

	bool can_seek() override
	{
		return true;
	}

	bool get_content_type(pfc::string_base &p_out) override
	{
		return false;
	}

	void reopen(abort_callback &p_abort) override
	{
		AATRFileStream_SetPosition(stream, 0);
	}

	bool is_remote() override
	{
		return false;
	}
};

class archive_atr : public archive_impl
{
public:

	bool supports_content_types() override
	{
		return false;
	}

	const char *get_archive_type() override
	{
		return "atr";
	}

	t_filestats get_stats_in_archive(const char *p_archive, const char *p_file, abort_callback &p_abort) override
	{
		service_impl_single_t<file_atr> f(p_archive, p_file);
		t_filestats stats = { f.get_size(abort_callback_dummy()), filetimestamp_invalid };
		return stats;
	}

	void open_archive(service_ptr_t<file> &p_out, const char *archive, const char *file, abort_callback &p_abort) override
	{
		p_out = new service_impl_t<file_atr>(archive, file);
	}

	void archive_list(const char *path, const service_ptr_t<file> &p_reader, archive_callback &p_out, bool p_want_readers) override
	{
		if (!_extract_native_path_ptr(path))
			throw exception_io_data();
		AATR *disk = AATRStdio_New(path);
		if (disk == nullptr)
			throw exception_io_data();
		AATRRecursiveLister *lister = AATRRecursiveLister_New();
		if (lister == nullptr) {
			AATRStdio_Delete(disk);
			throw exception_out_of_resources();
		}
		AATRRecursiveLister_Open(lister, disk);
		pfc::string8_fastalloc url;
		for (;;) {
			const char *fn = AATRRecursiveLister_NextFile(lister);
			if (fn == nullptr)
				break;
			t_filestats stats = { filesize_invalid, filetimestamp_invalid };
			service_ptr_t<file> p_file;
			if (p_want_readers) {
				p_file = new service_impl_t<file_atr>(path, fn);
			}
			archive_impl::g_make_unpack_path(url, path, fn, "atr");
			if (!p_out.on_entry(this, url, stats, p_file))
				break;
		}
		AATRRecursiveLister_Delete(lister);
		AATRStdio_Delete(disk);
	}

	bool is_our_archive(const char *path) override
	{
		return has_ext(path, ".atr");
	}
};

static archive_factory_t<archive_atr> g_archive_atr_factory;

DECLARE_COMPONENT_VERSION("ASAP", ASAPInfo_VERSION, ASAPInfo_CREDITS "\n" ASAPInfo_COPYRIGHT);
