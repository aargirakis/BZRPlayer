mysources=\
 mfp_io.c\
 mfpemul.c\
 mw_io.c\
 mwemul.c\
 paula_io.c\
 paulaemul.c\
 shifter_io.c\
 ym_io.c\
 ymemul.c

myheaders =\
 io68.h\
 mfp_io.h\
 mfpemul.h\
 mw_io.h\
 mwemul.h\
 paula_io.h\
 paulaemul.h\
 shifter_io.h\
 ym_fixed_vol.h\
 ym_io.h\
 ymemul.h\
 ymoutorg.h

lib_LTLIBRARIES = libio68.la
libio68_la_SOURCES = $(mysources) $(myheaders)
libio68_la_LDFLAGS = -version-info 2:0:0 -release @VERSION@

#EXTRA_DIST = io68.vcproj
