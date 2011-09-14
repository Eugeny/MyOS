#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT([fuse-umfuse-fat], [0.1], [osd@cs.unibo.it])
AM_INIT_AUTOMAKE([foreign dist-bzip2 1.10])
AC_CONFIG_SRCDIR([fusefat.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.
PKG_CHECK_MODULES(FUSE, fuse)
AC_SUBST(FUSE_LIBS)
AC_SUBST(FUSE_CFLAGS)

PKG_CHECK_MODULES(GLIB2, glib-2.0)
AC_SUBST(GLIB2_LIBS)
AC_SUBST(GLIB2_CFLAGS)

#AC_CHECK_LIB([umlib], [um_add_service], [UMLIB_LIBS=""],
#   [AC_MSG_ERROR([Can't find libumlib, please install it])])

AC_MSG_CHECKING([if FUSE on this system is too new for us])
AC_EGREP_CPP([yes], [
#include "fuse.h"
#if FUSE_VERSION > 26
yes
#endif
], AC_DEFINE([FUSE_USE_VERSION], [26], [Version of FUSE interface]) AC_MSG_RESULT([yes]),
   AC_DEFINE([FUSE_USE_VERSION], [FUSE_VERSION], [Version of FUSE interface]) AC_MSG_RESULT([no]))


# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/statvfs.h sys/vfs.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_STRUCT_ST_BLOCKS
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_STRUCT_TM
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_CHECK_FUNCS([memset strrchr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
