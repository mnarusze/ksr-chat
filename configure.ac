#
# Preparation
#

m4_define([ksr_chat_version], [0.1])

AC_INIT([ksr chat], [ksr_chat_version], maciek.naruszewicz@gmail.com)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC

#
# Libraries
#

PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.31.13])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(GIO, [gio-unix-2.0 >= 2.31.13])
AC_SUBST(GIO_CFLAGS)
AC_SUBST(GIO_LIBS)

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_OUTPUT