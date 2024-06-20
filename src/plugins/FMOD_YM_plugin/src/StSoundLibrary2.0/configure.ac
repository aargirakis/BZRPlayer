# Author Romain Giot <giot.romain@gmail.com>

# Init
AC_INIT([StSound], [2.0], [krusty@cpcscene.com])
AM_INIT_AUTOMAKE([ -Wall -Werror subdir-objects])

m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

# Check programs
AC_PROG_CXX
#Enable building DLLs on windows
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL


#Output file
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
        Makefile
])
AC_CONFIG_MACRO_DIR([m4])

AC_ARG_ENABLE(integer,
        [  --enable-integer        Integer emulation mode [default=no]],
        integer_only=yes,
        integer_only=no)

AC_ARG_ENABLE(simple-filter,
        [  --enable-simple-filter  Low computation filter [default=no]],
        simple_filter=yes,
        simple_filter=no)

AM_CONDITIONAL([YM_INTEGER_ONLY], [test "$integer_only" = "yes"])
AM_CONDITIONAL([YM_SIMPLIFIED_FILTER], [test "$simple_filter" = "yes"])

AC_OUTPUT

AC_MSG_NOTICE([
libstsound compilation options :

    Use integer only mode.............. ${integer_only}
    Use simplified filter.............. ${simple_filter}
])
