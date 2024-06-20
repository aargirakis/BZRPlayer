mysources=\
 ../emu68/cc68.c \
 ../emu68/emu68.c \
 ../emu68/error68.c \
 ../emu68/getea68.c \
 ../emu68/inst68.c \
 ../emu68/ioplug68.c \
 ../emu68/line0_68.c \
 ../emu68/line1_68.c \
 ../emu68/line2_68.c \
 ../emu68/line3_68.c \
 ../emu68/line4_68.c \
 ../emu68/line5_68.c \
 ../emu68/line6_68.c \
 ../emu68/line7_68.c \
 ../emu68/line8_68.c \
 ../emu68/line9_68.c \
 ../emu68/lineA_68.c \
 ../emu68/lineB_68.c \
 ../emu68/lineC_68.c \
 ../emu68/lineD_68.c \
 ../emu68/lineE_68.c \
 ../emu68/lineF_68.c \
 ../emu68/mem68.c \
 ../emu68/table68.c

lib_LTLIBRARIES = libemu68dbg.la
libemu68dbg_la_SOURCES = ${mysources}
libemu68dbg_la_LDFLAGS = -version-info 2:0:0 -release @VERSION@
libemu68dbg_la_CFLAGS = -DEMU68DEBUG=1 $(AM_CFLAGS)
