## Process this file with automake to produce Makefile.in
AM_CFLAGS =
AM_LDFLAGS =

AUTOMAKE_OPTIONS = subdir-objects

CLEANFILES=
BUILT_SOURCES=

## librlimit
#############
lib_LTLIBRARIES = librlimit.la

librlimit_la_SOURCES = rlimit.c rlimit.h

#librlimit_la_CFLAGS =
#librlimit_la_CPPFLAGS =
librlimit_la_LDFLAGS =-release $(VERSION) -version-info 0:0:0

CLEANFILES +=

DISTCLEANFILES =


MAINTAINERCLEANFILES = \
        Makefile.in config.h.in compile.log
