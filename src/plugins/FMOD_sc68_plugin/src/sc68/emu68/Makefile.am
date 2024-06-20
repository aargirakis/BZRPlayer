mysources=\
 cc68.c \
 emu68.c \
 error68.c \
 getea68.c \
 inst68.c \
 ioplug68.c \
 line0_68.c \
 line1_68.c \
 line2_68.c \
 line3_68.c \
 line4_68.c \
 line5_68.c \
 line6_68.c \
 line7_68.c \
 line8_68.c \
 line9_68.c \
 lineA_68.c \
 lineB_68.c \
 lineC_68.c \
 lineD_68.c \
 lineE_68.c \
 lineF_68.c \
 mem68.c \
 table68.c

myheaders=\
 cc68.h\
 emu68.h\
 error68.h\
 excep68.h\
 getea68.h\
 inst68.h\
 ioplug68.h\
 macro68.h\
 mem68.h\
 srdef68.h\
 struct68.h\
 type68.h

# emu68debug.vcproj
# emu68.vcproj

myextra=\
 line0.inc\
 line1.inc\
 line2.inc\
 line3.inc\
 line4.inc\
 line5.inc\
 line6.inc\
 line7.inc\
 line8.inc\
 line9.inc\
 lineA.inc\
 lineB.inc\
 lineC.inc\
 lineD.inc\
 lineE.inc\
 lineF.inc\
 table.inc

lib_LTLIBRARIES = libemu68.la
libemu68_la_SOURCES = ${mysources} ${myheaders}
libemu68_la_LDFLAGS = -version-info 2:0:0 -release @VERSION@

EXTRA_DIST = $(myextra)
