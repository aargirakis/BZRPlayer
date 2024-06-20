dnl# -*- sh -*-
dnl# the "configure" script is made from this by running GNU "autoconf"
dnl#
dnl# Configuration script for sc68
dnl#
dnl# by Benjamin Gerard <ben@sashipa.com>

AC_INIT(sc68,2.2.1,bug@sashipa.com)
AC_CONFIG_SRCDIR([api68/api68.c])
AM_INIT_AUTOMAKE([gnu 1.7 check-news no-define dist-bzip2])
AC_CONFIG_HEADERS(config_config68.h)

AC_ARG_ENABLE(   debug,       [  --enable-debug          Enable debug facilities [[default=no]]], enable_debug="${enableval}", enable_debug="no")
AC_ARG_ENABLE(    file,       [  --enable-file           Enable FILE stream [[default=yes]]],enable_file="${enableval}",enable_file="yes")
AC_ARG_ENABLE(      fd,       [  --enable-fd             Enable file descriptor stream [[default=yes]]],enable_fd="${enableval}",enable_fd="yes")
AC_ARG_ENABLE(     mem,       [  --enable-mem            Enable memory stream [[default=yes]]],enable_mem="${enableval}",enable_mem="yes")
AC_ARG_ENABLE(readline,       [  --enable-readline       Enable readline support [[default=auto]]],enable_readline="${enableval}",enable_readline="yes")
AC_ARG_ENABLE(    zlib,       [  --enable-zlib           Enable zlib support [[default=auto]]],enable_zlib="${enableval}",enable_zlib="yes")
AC_ARG_ENABLE(     doc,       [  --enable-doc            Enable doc generation [[default=no]]],enable_doc="${enableval}",enable_doc="no")

# ######################################################################
# ###
# ##  Configuring package version
# #

VERSION_NUM=221
PACKAGE_URL="http://sashipa.ben.free.fr/sc68"
SC68URL="${PACKAGE_URL}"

AC_SUBST(PACKAGE)
AC_SUBST(PACKAGE_BUGREPORT)
AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE_STRING)
AC_SUBST(PACKAGE_TARNAME)
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(VERSION)
AC_SUBST(VERSION_NUM)
AC_SUBST(PACKAGE_URL)
AC_SUBST(SC68URL)

# ######################################################################
# ###
# ##  Configuring options
# #

DEBUG="0";           test "x${enable_debug}" = "xyes" && DEBUG="1"
ISTREAM_NO_FILE="0"; test "x${enable_file}" != "xyes" && ISTREAM_NO_FILE="1"
ISTREAM_NO_FD="0";   test "x${enable_fd}"   != "xyes" && ISTREAM_NO_FD="1"
ISTREAM_NO_MEM="0";  test "x${enable_mem}"  != "xyes" && ISTREAM_NO_MEM="1"

AC_SUBST(DEBUG)
AC_SUBST(ISTREAM_NO_FILE)
AC_SUBST(ISTREAM_NO_FD)
AC_SUBST(ISTREAM_NO_MEM)

test "x${SAMPLING_RATE_MAX}" = "x" && SAMPLING_RATE_MAX=50000
test "x${SAMPLING_RATE_MIN}" = "x" && SAMPLING_RATE_MIN=6000
test "x${SAMPLING_RATE_DEF}" = "x" && SAMPLING_RATE_DEF=44100
test "x${REPLAY_RATE_MIN}"   = "x" && REPLAY_RATE_MIN=25
AC_SUBST(SAMPLING_RATE_MAX)
AC_SUBST(SAMPLING_RATE_MIN)
AC_SUBST(SAMPLING_RATE_DEF)
AC_SUBST(REPLAY_RATE_MIN)

# ######################################################################
# ###
# ##  Configuring tools
# #

# Libraries
SUBDIRS68="emu68 emu68debug io68 desa68 unice68 file68 api68"
# Programs
SUBDIRS68="$SUBDIRS68 as68 sc68 info68 sourcer68 debug68"
# Data
SUBDIRS68="$SUBDIRS68 data info man"

# C-flags
MY_CFLAGS="-DHAVE_CONFIG_CONFIG68_H=1 -DNO_FALLBACK_CONFIG=1"
if test "x$enable_debug" = "xyes"; then
    MY_CFLAGS="$MY_CFLAGS -Wall -g -O"
else
    MY_CFLAGS="$MY_CFLAGS -Wall -O3 -fomit-frame-pointer"
fi
CFLAGS="$CFLAGS $MY_CFLAGS"
CPPFLAGS="$CPPFLAGS $MY_CFLAGS"

# Compiler and companions
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
AC_PROG_INSTALL

# pkg-config

AC_PATH_PROG(PKGCONFIG, pkg-config, no)
PKGCONFIG_INSTALL_PATH='$(libdir)/pkgconfig'
AC_SUBST(PKGCONFIG_INSTALL_PATH)

# Doxygen (Documentation generation)

AC_PATH_PROG(DOXYGEN, doxygen, echo)
if test "x${doxygen}" = "xecho"; then
	enable_doc=no
fi
DOC_MAKEFILE=""
test "x$enable_doc" == "xyes" && DOC_MAKEFILE="doc/Makefile"

# Readline library (for debug68)
READLINE_LIB=""
if test "x$enable_readline" = "xyes"; then
AC_CHECK_LIB(readline,readline,
READLINE_LIB="-lreadline -lcurses",
enable_readline="no",
-lcurses)
fi
if test "x$enable_readline" = "xyes"; then
AC_CHECK_HEADERS(readline/readline.h,
HAVE_READLINE_READLINE_H=1,
HAVE_READLINE_READLINE_H=0;READLINE_LIB="")
AC_CHECK_HEADERS(readline/history.h,
HAVE_READLINE_HISTORY_H=1,
HAVE_READLINE_HISTORY_H=0)
fi


AC_SUBST(HAVE_READLINE_READLINE_H)
AC_SUBST(HAVE_READLINE_HISTORY_H)
AC_SUBST(READLINE_LIB)


# Zlib support
Z_LIB=""
if test "x$enable_zlib" = "xyes"; then
AC_CHECK_LIB(z,gzdopen,
Z_LIB="-lz",
enable_zlib="no")
fi
if test "x$enable_zlib" = "xyes"; then
AC_CHECK_HEADERS(zlib.h,HAVE_ZLIB_H=1,HAVE_ZLIB_H=0;Z_LIB="")
fi
AC_SUBST(HAVE_ZLIB_H)
AC_SUBST(Z_LIB)

SC68_LDFLAGS="-lapi68 -lfile68 -lunice68 -lemu68 -lio68 ${Z_LIB}"
AC_SUBST(SC68_LDFLAGS)

# Some functions

AC_CHECK_FUNCS(vsprintf,HAVE_VSPRINTF=1,HAVE_VSPRINTF=0)
AC_CHECK_FUNCS(vsnprintf,HAVE_VSNPRINTF=1,HAVE_VSNPRINTF=0)
AC_CHECK_FUNCS(getenv,HAVE_GETENV=1,HAVE_GETENV=0)
AC_SUBST(HAVE_VSPRINTF)
AC_SUBST(HAVE_VSNPRINTF)
AC_SUBST(HAVE_GETENV)

AC_SUBST(LIBTOOL_DEPS)
AC_SUBST(LIBCOMMAND)
AC_SUBST(MAKEDEPEND)
AC_SUBST(DEL)
AC_SUBST(SUBDIRS68)
AC_SUBST(DOXYGEN)

# ######################################################################
# ###
# ##  Configuring types
# #

AC_CHECK_SIZEOF(char, 1)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)
SIZEOF_CHAR=${ac_cv_sizeof_char}
SIZEOF_SHORT=${ac_cv_sizeof_short}
SIZEOF_INT=${ac_cv_sizeof_int}
SIZEOF_LONG=${ac_cv_sizeof_long}
SIZEOF_LONG_LONG=${ac_cv_sizeof_long_long}

all_chk_types=""
AC_MSG_CHECKING([Scanning all types])
for i in char short int long long_long; do
    size="`eval echo \\${ac_cv_sizeof_$i}`"
    j=$(echo "$i" | sed -e 'y/_/ /')
    if test -z "${size}"; then continue; fi
    case "${size}" in
	1)
	    test -z "${TYPE_U8}" && TYPE_U8="unsigned $j"
	    test -z "${TYPE_S8}" && TYPE_S8="  signed $j"
	    ;;
	2)
	    test -z "${TYPE_U16}" && TYPE_U16="unsigned $j"
	    test -z "${TYPE_S16}" && TYPE_S16="  signed $j"
	    ;;
	4)
	    test -z "${TYPE_U32}" && TYPE_U32="unsigned $j"
	    test -z "${TYPE_S32}" && TYPE_S32="  signed $j"
	    ;;
	*)
	    if test ${size} -ge 8; then
		test -z "${TYPE_U64}" && TYPE_U64="unsigned $j"
		test -z "${TYPE_S64}" && TYPE_S64="  signed $j"
	    else
		j=""
	    fi
	    ;;
    esac
    if test -n "$j"; then
	all_chk_types="${all_chk_types}'$j' "
    fi
done
AC_MSG_RESULT([${all_chk_types}])

all_chk_types=""
AC_MSG_CHECKING([Check for required types])
for i in 8 16 32 64; do
    eval test \"x\${TYPE_U$i}\" = x -o \"x\${TYPE_S$i}\" = x &&
    AC_MSG_ERROR([Could not find $i byte types])
    all_chk_types="${all_chk_types}$i "
done
AC_MSG_RESULT([${all_chk_types}])

AC_SUBST(SIZEOF_CHAR)
AC_SUBST(SIZEOF_SHORT)
AC_SUBST(SIZEOF_INT)
AC_SUBST(SIZEOF_LONG)
AC_SUBST(SIZEOF_LONG_LONG)

AC_SUBST(TYPE_U8)
AC_SUBST(TYPE_S8)
AC_SUBST(TYPE_U16)
AC_SUBST(TYPE_S16)
AC_SUBST(TYPE_U32)
AC_SUBST(TYPE_S32)
AC_SUBST(TYPE_U64)
AC_SUBST(TYPE_S64)


test "x${BREAKPOINT68}" = "x" &&
BREAKPOINT68="if (1) { *(int *)1 = 0x12345678; } else"
AC_SUBST(BREAKPOINT68)

AC_CONFIG_FILES([
 Makefile
 config_type68.h
 config_option68.h
 config_package68.h
 config_platform68.h
 sc68.spec
 sc68-${VERSION}.ebuild:sc68.ebuild.in
 sc68.pc
 sc68-config
 as68/Makefile
 file68/Makefile
 io68/Makefile
 desa68/Makefile
 emu68/Makefile
 emu68debug/Makefile
 api68/Makefile
 sc68/Makefile
 info68/Makefile
 debug68/Makefile
 sourcer68/Makefile
 unice68/Makefile
 man/Makefile
 info/Makefile
 doc/Makefile
 data/Makefile])
AC_OUTPUT

AC_MSG_NOTICE([])
AC_MSG_NOTICE([****])
AC_MSG_NOTICE([* Configuration summary])
AC_MSG_NOTICE([* ---------------------])
AC_MSG_NOTICE([*])
AC_MSG_NOTICE([*   doc-generation      : $enable_doc])
AC_MSG_NOTICE([*   debug-mode          : $enable_debug])
AC_MSG_NOTICE([*   file-stream         : $enable_file])
AC_MSG_NOTICE([*   fd-stream           : $enable_fd])
AC_MSG_NOTICE([*   memory-stream       : $enable_mem])
AC_MSG_NOTICE([*   readline support    : $enable_readline])
AC_MSG_NOTICE([*   zlib support        : $enable_zlib])
AC_MSG_NOTICE([*])
AC_MSG_NOTICE([****])
AC_MSG_NOTICE([])

dnl#
dnl# End of "$Id: configure.ac,v 2.20 2003/09/30 07:47:36 benjihan Exp $".
dnl#
