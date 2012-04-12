dnl ********************************************************************
dnl Process this file with autoconf to produce a configure script.
dnl ********************************************************************
AC_INIT([librlimit],[0.6])
AC_CONFIG_AUX_DIR([autotools-files])
AC_CONFIG_MACRO_DIR([autotools-files/m4])
AC_CONFIG_HEADERS([src/config.h])

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

AC_LANG([C])

dnl ********************************************************************
dnl Checking for standard programs, headers and libraries
dnl ********************************************************************
AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CC
AM_PROG_CC_STDC
AM_PROG_CC_C_O
AM_PROG_LIBTOOL

dnl ********************************************************************
dnl Option and variable settings
dnl ********************************************************************

dnl Initial settings of flags variables
CFLAGS="-Wall -Wextra -std=c99 -D_FORTIFY_SOURCE=2"
CPPFLAGS="${CPPFLAGS}"
LDFLAGS="${LDFLAGS}"
LIBS="${LIBS}"

dnl Parsing and setting options on compile flags
AC_ARG_ENABLE(werror,
              AS_HELP_STRING([--enable-werror],
                             [compile with -Werror flag (default is no)]),
              ENABLE_WERROR=$enableval,
              ENABLE_WERROR=no)

if test x$ENABLE_WERROR = xyes ; then
    CFLAGS="${CFLAGS} -Werror"
    CXXFLAGS="${CXXFLAGS} -Werror"
fi

AC_ARG_ENABLE(debug,
              AS_HELP_STRING([--enable-debug],
                             [compile with debug (default is no)]),
              ENABLE_DEBUG=$enableval,
              ENABLE_DEBUG=no)

if test x$ENABLE_DEBUG = xyes ; then
    CFLAGS="${CFLAGS} -g -DDEBUG"
    CXXFLAGS="${CXXFLAGS} -g -DDEBUG"
fi

AC_ARG_ENABLE(optimize,
              AS_HELP_STRING([--enable-optimize],
                             [compile with optimization (default is no)]),
              ENABLE_OPTIMIZE=$enableval,
              ENABLE_OPTIMIZE=yes)

if test x$ENABLE_OPTIMIZE = xyes ; then
    CFLAGS="${CFLAGS} -O2"
    CXXFLAGS="${CXXFLAGS} -O2"
fi

dnl ********************************************************************
dnl Writing files
dnl ********************************************************************
AC_CONFIG_FILES([
        Makefile
	doc/Makefile
        src/Makefile
        test/Makefile
	test/utils/Makefile
], [ ], [ ])

AC_OUTPUT

AC_MSG_NOTICE([
=================================================================
                    librlimit ${VERSION}
=================================================================
Here is the configuration of the package:
    Operating System            : ${host_os}
    Architecture                : ${host_cpu}

    Installation prefix         : ${prefix}
    Source code location        : ${srcdir}

    C Compiler                  : ${CC}
    CFLAGS                      : ${CFLAGS}
    CPPFLAGS                    : ${CPPFLAGS}
    LDFLAGS                     : ${LDFLAGS}
    LIBS                        : ${LIBS}

Optional Features:
    Enable debug messages       : ${ENABLE_DEBUG}
    Enable optimization         : ${ENABLE_OPTIMIZE}
    Maintainer mode             : ${enable_maintainer_mode}
    Enable -Werror              : ${ENABLE_WERROR}
])
