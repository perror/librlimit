## Process this file with automake to produce Makefile.in
SUBDIRS = utils .

AM_CFLAGS = -I../src/
AM_LDFLAGS = $(top_builddir)/src/librlimit.la -lpthread -lrt

AUTOMAKE_OPTIONS=subdir-objects

bin_PROGRAMS = \
	01_io  			\
	02_timeout 		\
	03_notimeout 		\
	04_forbid_syscall 	\
	05_profile_sleep  	\
	06_profile_stack_alloc 	\
	07_profile_malloc	\
	08_forbid_syscall_timeouted

01_io_SOURCES = 01_io.c
02_timeout_SOURCES = 02_timeout.c
03_notimeout_SOURCES = 03_notimeout.c
04_forbid_syscall_SOURCES = 04_forbid_syscall.c
05_profile_sleep_SOURCES = 05_profile_sleep.c
06_profile_stack_alloc_SOURCES = 06_profile_stack_alloc.c
07_profile_malloc_SOURCES = 07_profile_malloc.c
08_forbid_syscall_timeouted_SOURCES = 08_forbid_syscall_timeouted.c

check: $(bin_PROGRAMS)
	@./test-runner.sh

MAINTAINERCLEANFILES = \
	Makefile.in
