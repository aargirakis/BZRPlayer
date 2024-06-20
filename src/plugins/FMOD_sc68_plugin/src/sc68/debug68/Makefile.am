mysources=\
 SC68debug_breakpoint.c\
 SC68debug_comline.c\
 SC68debug_debugcom.c\
 SC68debug_eval.c\
 SC68debug_io.c\
 sc68debug.c

myheaders=\
 SC68debug_breakpoint.h\
 SC68debug_comline.h\
 SC68debug_debugcom.h\
 SC68debug_eval.h\
 SC68debug_io.h

bin_PROGRAMS = debug68

debug68_SOURCES = $(mysources) $(myheaders)
debug68_LDADD = \
 ../file68/libfile68.la\
 ../unice68/libunice68.la\
 ../desa68/libdesa68.la\
 ../emu68debug/libemu68dbg.la
debug68_CFLAGS = -DEMU68DEBUG=1 ${AM_CFLAGS}
debug68_LDFLAGS = @READLINE_LIB@ $(AM_LDFLAGS)

#EXTRA_DIST = debug68.vcproj
