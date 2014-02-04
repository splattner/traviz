#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT(traviz, 0.1, sebastian.plattner@gmail.com)

AC_DEFINE([CODENAME], ["Traviz"], [Codename of package])
AC_DEFINE([RELEASE_TYPE], ["beta"], [Development status of package])

AM_INIT_AUTOMAKE(traviz, 0.1)
AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_SRCDIR([src/main.c])


AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_CC_C_O

AM_PATH_GTK_2_0
AC_CHECK_LIB([m], [pow])
AC_CHECK_LIB([gtk-x11-2.0], [gtk_init])
AC_CHECK_LIB([gdk-x11-2.0], [gdk_draw_drawable])
AC_CHECK_LIB([glib-2.0], [g_list_append])
AC_CHECK_LIB([gobject-2.0], [g_object_set_data])
AC_CHECK_LIB([gthread-2.0], [g_thread_init])
AC_CHECK_LIB([plplotd], [c_plinit])



AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h])

AC_FUNC_MALLOC
AC_CHECK_FUNCS([pow sqrt])


AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_CONFIG_FILES([Makefile src/Makefile])



AC_OUTPUT
