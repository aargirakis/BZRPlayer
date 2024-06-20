bin_PROGRAMS = sourcer68
sourcer68_SOURCES = sourcer68.c sourcer68_doc.h
sourcer68_LDADD = \
 ../file68/libfile68.la\
 ../unice68/libunice68.la\
 ../desa68/libdesa68.la\
 ../emu68debug/libemu68dbg.la
sourcer68_CFLAGS =\
 -DEMU68DEBUG=1\
 -DSC68_SHARED_DATA_PATH='"$(pkgdatadir)"'\
 ${AM_CFLAGS}

#EXTRA_DIST = sourcer68.vcproj
