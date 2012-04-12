## Process this file with automake to produce Makefile.in
SUBDIRS = . src test doc

ACLOCAL_AMFLAGS = -I autotools-files/m4

EXTRA_DIST =      \
	AUTHORS   \
	COPYING   \
	ChangeLog \
	NEWS      \
	README

MAINTAINERCLEANFILES = \
	Makefile.in    \
	aclocal.m4     \
	compile		\
	config.guess   \
	config.sub     \
	depcomp		\
	install-sh     \
	ltmain.sh      \
	missing		\
	ylwrap

maintainer-clean-local:
	-rm -fr m4 autotools-files
