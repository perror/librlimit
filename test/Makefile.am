## Process this file with automake to produce Makefile.in
AM_CFLAGS = -I../src/
AM_LDFLAGS = $(top_builddir)/src/librlimit.la -lpthread -lrt

AUTOMAKE_OPTIONS=subdir-objects

bin_PROGRAMS = 01_stdout

01_stdout_SOURCES = 01_stdout.c

check: $(bin_PROGRAMS)
	@./test-runner.sh

MAINTAINERCLEANFILES = \
	Makefile.in
