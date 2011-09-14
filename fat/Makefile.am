moddir = $(libdir)/umview/modules

DEVELFLAGS =

AM_CFLAGS = -ggdb $(GLIB2_CFLAGS)
AM_LDFLAGS = -module -avoid-version -export-dynamic
AM_LIBTOOLFLAGS = --tag=disable-static
AM_CPPFLAGS = -I$(includedir)/umview -DFATWRITE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE $(DEVELFLAGS)

bin_PROGRAMS = fusefat
mod_LTLIBRARIES = umfusefat.la

#dist_man1_MANS = fuseext2.1

COMMONSRC = fusefat.c \
	libfat.c libfat.h bits/libfat.h \
	unicode.h utf16be.c utf16le.c utf8.c \
	simple_fold.c \
	v2fuseutils.c v2fuseutils.h

fusefat_SOURCES = $(COMMONSRC)
fusefat_LDFLAGS = -lfuse -lpthread $(GLIB2_LIBS) $(FUSE_LIBS)
# Leave this also if it seems a no-op
fusefat_CFLAGS = $(AM_CFLAGS) $(FUSE_CFLAGS)

umfusefat_la_SOURCES = $(COMMONSRC)
umfusefat_la_LDFLAGS = $(AM_LDFLAGS) -lpthread $(GLIB2_LIBS)


install-data-hook:
	cd "$(DESTDIR)/$(moddir)" && rm -f $(mod_LTLIBRARIES)

extraclean: maintainer-clean
	rm -rf aclocal.m4 autom4te.cache autoscan.log configure.scan \
	config.h.in* configure depcomp install-sh Makefile.in \
	config.guess config.sub \
	ltmain.sh missing `find . -name Makefile.in` \
	fuse-umfuse-*.tar.gz \
	fuse-umfuse-*.tar.bz2


