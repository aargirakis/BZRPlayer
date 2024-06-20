#AM_CXXFLAGS = -I$(srcdir)/../libtools -I$(srcdir)/../libcpcfs
ACLOCAL_AMFLAGS = -I m4
lib_LTLIBRARIES = libstsound.la
libstsound_la_SOURCES = ./YmMusic.cpp ./Ymload.cpp ./YmUserInterface.cpp ./Ym2149Ex.cpp ./digidrum.cpp ./LZH/LzhLib.cpp ./digidrum.h ./StSoundLibrary.h ./YmProfiles.h ./YmTypes.h ./Ymload.h ./YmMusic.h ./Ym2149Ex.h ./LZH/LZH.H ./YmFilters.h ./YmFilters.cpp
libstsound_la_LDFLAGS = -no-undefined -version-info 2:0:0

libstsound_la_CPPFLAGS = -O2 -s

if YM_INTEGER_ONLY
    libstsound_la_CPPFLAGS += -DYM_INTEGER_ONLY
endif

if YM_SIMPLIFIED_FILTER
    libstsound_la_CPPFLAGS += -DYM_SIMPLIFIED_FILTER
endif

include_HEADERS = ./digidrum.h ./StSoundLibrary.h ./YmProfiles.h ./YmTypes.h ./Ymload.h ./YmMusic.h ./Ym2149Ex.h ./LZH/LZH.H ./YmFilters.h
