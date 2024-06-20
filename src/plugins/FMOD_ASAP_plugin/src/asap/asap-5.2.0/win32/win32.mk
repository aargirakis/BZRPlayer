# MinGW for most ports
WIN32_CC = $(DO)i686-w64-mingw32-gcc $(WIN32_CARGS) $(filter-out %.h,$^)
WIN32_CXX = $(DO)i686-w64-mingw32-g++ $(WIN32_CARGS) -std=c++17 $(filter-out %.h %.hpp,$^)
WIN32_WINDRES = $(DO)i686-w64-mingw32-windres -o $@ $<
VLC_INCLUDE = ../vlc/include
VLC_LIB32 = "C:/Program Files (x86)/VideoLAN/VLC"
VLC_LIB64 = "C:/Program Files/VideoLAN/VLC"

# Microsoft compiler for foobar2000
FOOBAR2000_SDK_DIR = ../foobar2000_SDK
WIN32_CL = $(WIN32_CLDO)cl -GR- -GS- -wd4996 -DNDEBUG $(WIN32_CLARGS) $(filter-out %.h,$^)
WIN32_LINKOPT = -link -release
WIN32_MKLIB = $(DO)lib -nologo -ltcg -out:$@ $^

# MinGW x64
WIN64_CC = $(DO)x86_64-w64-mingw32-gcc $(WIN32_CARGS) $(filter-out %.h,$^)
WIN64_CXX = $(DO)x86_64-w64-mingw32-g++ $(WIN32_CARGS) -std=c++17 $(filter-out %.h %.hpp,$^)
WIN64_WINDRES = $(DO)x86_64-w64-mingw32-windres -o $@ $<

# Windows Installer XML
CANDLE = $(DO)candle -nologo -o $@
LIGHT = $(DO)light -nologo -o $@ -spdb

# Code signing
DO_SIGN = $(DO)signtool sign -d "ASAP - Another Slight Atari Player $(VERSION)" -n "Open Source Developer, Piotr Fusik" -tr http://time.certum.pl -fd sha256 -td sha256 $^ && touch $@

# no user-configurable paths below this line

ifndef DO
$(error Use "Makefile" instead of "win32.mk")
endif

comma = ,
WIN32_CARGS = -O2 -Wall -Wl,--nxcompat -o $@ $(if $(filter %.dll,$@),-shared -Wl$(comma)-subsystem$(comma)windows) $(INCLUDEOPTS) -static -s
WIN32_CLDO = $(DO)$(if $(filter-out %.obj,$@),mkdir -p win32/obj/$@ && )
WIN32_CLARGS = -nologo -O2 -GL -W3 $(if $(filter %.obj,$@),-c -Fo$@,-Fe$@ -Fowin32/obj/$@/) $(if $(filter %.dll,$@),-LD) $(INCLUDEOPTS)

mingw: $(addprefix win32/,asapconv.exe libasap.a asapscan.exe wasap.exe bass_asap.dll in_asap.dll xmp-asap.dll apokeysnd.dll ASAPShellEx.dll)
.PHONY: mingw

# asapconv

win32/asapconv.exe: $(call src,asapconv.c asap.[ch])
	$(WIN32_CC) -DHAVE_LIBMP3LAME -DHAVE_LIBMP3LAME_DLL
CLEAN += win32/asapconv.exe

win32/asapconv-static-lame.exe: $(call src,asapconv.c asap.[ch])
	$(WIN32_CC) -DHAVE_LIBMP3LAME -lmp3lame
CLEAN += win32/asapconv-static-lame.exe

win32/asapconv-no-lame.exe: $(call src,asapconv.c asap.[ch])
	$(WIN32_CC)
CLEAN += win32/asapconv-no-lame.exe

win32/msvc/asapconv.exe: $(call src,asapconv.c asap.[ch])
	$(WIN32_CL) -DHAVE_LIBMP3LAME -DHAVE_LIBMP3LAME_DLL
CLEAN += win32/msvc/asapconv.exe

win32/x64/asapconv.exe: $(call src,asapconv.c asap.[ch])
	$(WIN64_CC)
CLEAN += win32/x64/asapconv.exe

# lib

win32/libasap.a: win32/asap.o
	$(DO_AR)
CLEAN += win32/libasap.a

win32/asap.o: $(call src,asap.[ch])
	$(WIN32_CC) -c
CLEAN += win32/asap.o

win32/asap.lib: win32/asap.obj
	$(WIN32_MKLIB)
CLEAN += win32/asap.lib

win32/asap.obj: $(call src,asap.[ch])
	$(WIN32_CL)
CLEAN += win32/asap.obj

# SDL

win32/asap-sdl.exe: $(call src,asap-sdl.c asap.[ch])
	$(WIN32_CC) -lmingw32 -lSDLmain -lSDL
CLEAN += win32/asap-sdl.exe

# asapscan

win32/asapscan.exe: $(srcdir)asapscan.c asap-asapscan.h
	$(WIN32_CC)
CLEAN += win32/asapscan.exe

# sap2txt

win32/sap2txt.exe: $(srcdir)sap2txt.c
	$(WIN32_CC) -static -lz
CLEAN += win32/sap2txt.exe

# sap2ntsc

win32/sap2ntsc.exe: $(call src,sap2ntsc.c asap.h) # FIXME: also asap.c, but we #include it and not link
	$(WIN32_CC)
CLEAN += win32/sap2ntsc.exe

# WASAP

win32/wasap.exe: $(call src,win32/wasap/wasap.[ch] asap.[ch] astil.[ch] win32/info_dlg.[ch]) win32/wasap/wasap-res.o
	$(WIN32_CC) -Wl,-subsystem,windows -DWASAP -lcomctl32 -lcomdlg32 -lwinmm
CLEAN += win32/wasap.exe

win32/wasap/wasap-res.o: $(call src,win32/gui.rc asap.h win32/info_dlg.h win32/wasap/wasap.h win32/wasap/wasap.ico win32/wasap/play.ico win32/wasap/stop.ico)
	$(WIN32_WINDRES) -DWASAP
CLEAN += win32/wasap/wasap-res.o

win32/x64/wasap.exe: $(call src,win32/wasap/wasap.[ch] asap.[ch] astil.[ch] win32/info_dlg.[ch]) win32/x64/wasap-res.o
	$(WIN64_CC) -Wl,-subsystem,windows -DWASAP -lcomctl32 -lcomdlg32 -lwinmm
CLEAN += win32/x64/wasap.exe

win32/x64/wasap-res.o: $(call src,win32/gui.rc asap.h win32/info_dlg.h win32/wasap/wasap.h win32/wasap/wasap.ico win32/wasap/play.ico win32/wasap/stop.ico)
	$(WIN64_WINDRES) -DWASAP
CLEAN += win32/x64/wasap-res.o

# VLC

win32/libasap_plugin.dll: $(call src,vlc/libasap_plugin.c asap.[ch]) win32/libasap_plugin-res.o
	$(WIN32_CC:-static=-static-libgcc) -I$(VLC_INCLUDE) -L$(VLC_LIB32) -lvlccore
CLEAN += win32/libasap_plugin.dll

win32/libasap_plugin-res.o: $(call src,win32/gui.rc asap.h)
	$(WIN32_WINDRES) -DVLC
CLEAN += win32/libasap_plugin-res.o

win32/x64/libasap_plugin.dll: $(call src,vlc/libasap_plugin.c asap.[ch]) win32/x64/libasap_plugin-res.o
	$(WIN64_CC:-static=-static-libgcc) -I$(VLC_INCLUDE) -L$(VLC_LIB64) -lvlccore
CLEAN += win32/x64/libasap_plugin.dll

win32/x64/libasap_plugin-res.o: $(call src,win32/gui.rc asap.h)
	$(WIN64_WINDRES) -DVLC
CLEAN += win32/x64/libasap_plugin-res.o

# BASS

win32/bass_asap.dll: $(call src,win32/bass/bass_asap.c asap.[ch] win32/bass/bass-addon.h win32/bass/bass.h win32/bass/bass.lib) win32/bass/bass_asap-res.o
	$(WIN32_CC) -Wl,--kill-at -DBASS
CLEAN += win32/bass_asap.dll

win32/bass/bass_asap-res.o: $(call src,win32/gui.rc asap.h)
	$(WIN32_WINDRES) -DBASS
CLEAN += win32/bass/bass_asap-res.o

# foobar2000

FOOBAR2000_RUNTIME = win32/foobar2000/foobar2000_SDK.lib win32/foobar2000/pfc.lib $(FOOBAR2000_SDK_DIR)/foobar2000/shared/shared.lib

win32/foo_asap.dll: $(call src,win32/foobar2000/foo_asap.cpp asap.[ch] astil.[ch] aatr-stdio.[ch] aatr.h win32/info_dlg.[ch] win32/settings_dlg.[ch]) win32/foobar2000/foo_asap.res $(FOOBAR2000_RUNTIME)
	$(WIN32_CL) -DFOOBAR2000 -DWIN32 -EHsc -I$(FOOBAR2000_SDK_DIR) comctl32.lib comdlg32.lib ole32.lib shell32.lib shlwapi.lib user32.lib $(WIN32_LINKOPT)
CLEAN += win32/foo_asap.dll win32/foo_asap.exp win32/foo_asap.lib

win32/foobar2000/foobar2000_SDK.lib: $(patsubst %,win32/foobar2000/%.obj,component_client abort_callback audio_chunk audio_chunk_channel_config \
	cfg_var console file_info file_info_impl file_info_merge filesystem filesystem_helper foosort guids input \
	main_thread_callback metadb_handle metadb_handle_list playable_location playlist preferences_page replaygain_info service titleformat utility)
	$(WIN32_MKLIB)
CLEAN += win32/foobar2000/foobar2000_SDK.lib

win32/foobar2000/component_client.obj: $(FOOBAR2000_SDK_DIR)/foobar2000/foobar2000_component_client/component_client.cpp
	$(WIN32_CL) -DWIN32 -DUNICODE -EHsc

win32/foobar2000/%.obj: $(FOOBAR2000_SDK_DIR)/foobar2000/SDK/%.cpp
	$(WIN32_CL) -DWIN32 -DUNICODE -EHsc -D_WIN32_IE=0x550 -I$(FOOBAR2000_SDK_DIR)
CLEAN += win32/foobar2000/*.obj

win32/foobar2000/pfc.lib: $(patsubst %,win32/foobar2000/%.obj,audio_math audio_sample bit_array bsearch \
	guid other pathUtils sort stringNew string_base string_conv threads timers utf8 win-objects)
	$(WIN32_MKLIB)
CLEAN += win32/foobar2000/pfc.lib

win32/foobar2000/%.obj: $(FOOBAR2000_SDK_DIR)/pfc/%.cpp
	$(WIN32_CL) -DWIN32 -DUNICODE -D_UNICODE -EHsc

win32/foobar2000/foo_asap.res: $(call src,win32/gui.rc asap.h win32/settings_dlg.h)
	$(WIN32_WINDRES) -DFOOBAR2000
CLEAN += win32/foobar2000/foo_asap.res

# Winamp

win32/in_asap.dll: $(call src,win32/winamp/in_asap.c asap.[ch] astil.[ch] aatr-stdio.[ch] aatr.h win32/info_dlg.[ch] win32/settings_dlg.[ch] win32/winamp/in2.h win32/winamp/out.h win32/winamp/ipc_pe.h win32/winamp/wa_ipc.h) win32/winamp/in_asap-res.o
	$(WIN32_CC) -DWINAMP -lcomctl32 -lcomdlg32
CLEAN += win32/in_asap.dll

win32/winamp/in_asap-res.o: $(call src,win32/gui.rc asap.h win32/info_dlg.h win32/settings_dlg.h)
	$(WIN32_WINDRES) -DWINAMP
CLEAN += win32/winamp/in_asap-res.o

# XMPlay

win32/xmp-asap.dll: $(call src,win32/xmplay/xmp-asap.c asap.[ch] astil.[ch] win32/info_dlg.[ch] win32/settings_dlg.[ch] win32/xmplay/xmpin.h win32/xmplay/xmpfunc.h) win32/xmplay/xmp-asap-res.o
	$(WIN32_CC) -Wl,--kill-at -DXMPLAY -lcomctl32 -lcomdlg32
CLEAN += win32/xmp-asap.dll

win32/xmplay/xmp-asap-res.o: $(call src,win32/gui.rc asap.h win32/info_dlg.h win32/settings_dlg.h)
	$(WIN32_WINDRES) -DXMPLAY
CLEAN += win32/xmplay/xmp-asap-res.o

# Raster Music Tracker

win32/apokeysnd.dll: $(call src,win32/rmt/apokeysnd_dll.c) win32/rmt/pokey.c win32/rmt/pokey.h win32/rmt/apokeysnd-res.o
	$(WIN32_CC) --std=c99 -DAPOKEYSND
CLEAN += win32/apokeysnd.dll

win32/rmt/pokey.h: $(srcdir)pokey.ci | win32/rmt/pokey.c

win32/rmt/pokey.c: $(srcdir)pokey.ci
	$(CITO) -D APOKEYSND
CLEAN += win32/rmt/pokey.c win32/rmt/pokey.h

win32/rmt/apokeysnd-res.o: $(call src,win32/gui.rc asap.h)
	$(WIN32_WINDRES) -DAPOKEYSND
CLEAN += win32/rmt/apokeysnd-res.o

# ASAPShellEx

win32/ASAPShellEx.dll: $(srcdir)win32/shellex/ASAPShellEx.cpp win32/shellex/asap-infowriter.cpp win32/shellex/asap-infowriter.hpp win32/shellex/ASAPShellEx-res.o
	$(WIN32_CXX) -Wl,--kill-at -lole32 -loleaut32 -lshlwapi -luuid
CLEAN += win32/ASAPShellEx.dll

win32/shellex/ASAPShellEx-res.o: $(call src,win32/gui.rc asap.h)
	$(WIN32_WINDRES) -DSHELLEX
CLEAN += win32/shellex/ASAPShellEx-res.o

win32/x64/ASAPShellEx.dll: $(srcdir)win32/shellex/ASAPShellEx.cpp win32/shellex/asap-infowriter.cpp win32/shellex/asap-infowriter.hpp win32/x64/ASAPShellEx-res.o
	$(WIN64_CXX) -Wl,--kill-at -lole32 -loleaut32 -lshlwapi -luuid
CLEAN += win32/x64/ASAPShellEx.dll

win32/x64/ASAPShellEx-res.o: $(call src,win32/gui.rc asap.h)
	$(WIN64_WINDRES) -DSHELLEX
CLEAN += win32/x64/ASAPShellEx-res.o

win32/shellex/asap-infowriter.hpp: $(call src,asapinfo.ci asap6502.ci asapwriter.ci flashpack.ci) $(ASM6502_OBX) | win32/shellex/asap-infowriter.cpp

win32/shellex/asap-infowriter.cpp: $(call src,asapinfo.ci asap6502.ci asapwriter.ci flashpack.ci) $(ASM6502_OBX)
	$(CITO)
CLEAN += win32/shellex/asap-infowriter.cpp win32/shellex/asap-infowriter.hpp

# setups

win32/setup: release/asap-$(VERSION)-win32.msi
.PHONY: win32/setup

release/asap-$(VERSION)-win32.msi: win32/setup/asap.wixobj \
	$(call src,win32/wasap/wasap.ico win32/setup/license.rtf win32/setup/asap-banner.jpg win32/setup/asap-dialog.jpg win32/diff-sap.js win32/shellex/ASAPShellEx.propdesc) \
	$(addprefix win32/,asapconv.exe sap2txt.exe wasap.exe in_asap.dll xmp-asap.dll bass_asap.dll apokeysnd.dll ASAPShellEx.dll foo_asap.dll libasap_plugin.dll signed)
	$(LIGHT) -ext WixUIExtension -sice:ICE69 -b win32 -b release -b $(srcdir)win32/setup -b $(srcdir)win32 $<

win32/setup/asap.wixobj: $(srcdir)win32/setup/asap.wxs release/release.mk
	$(CANDLE) -dVERSION=$(VERSION) $<
CLEAN += win32/setup/asap.wixobj

release/asap-$(VERSION)-win64.msi: win32/x64/asap.wixobj \
	$(call src,win32/wasap/wasap.ico win32/setup/license.rtf win32/setup/asap-banner.jpg win32/setup/asap-dialog.jpg win32/shellex/ASAPShellEx.propdesc) \
	win32/x64/ASAPShellEx.dll win32/x64/libasap_plugin.dll win32/signed
	$(LIGHT) -ext WixUIExtension -sice:ICE69 -b win32 -b $(srcdir)/win32/setup -b $(srcdir)win32 $<

win32/x64/asap.wixobj: $(srcdir)win32/setup/asap.wxs release/release.mk
	$(CANDLE) -arch x64 -dVERSION=$(VERSION) $<
CLEAN += win32/x64/asap.wixobj

win32/signed: $(addprefix win32/,asapconv.exe sap2txt.exe wasap.exe in_asap.dll xmp-asap.dll bass_asap.dll apokeysnd.dll ASAPShellEx.dll foo_asap.dll libasap_plugin.dll x64/ASAPShellEx.dll x64/libasap_plugin.dll)
	$(DO_SIGN)
CLEAN += win32/signed

release/signed-msi: release/asap-$(VERSION)-win32.msi release/asap-$(VERSION)-win64.msi
	$(DO_SIGN)
CLEAN += release/signed-msi
