#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([smart_robot], [1.0], [simon.yang@smartac.co])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
#AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([stdafx.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CC
#AC_PROG_RANLIB
#AC_PROG_LIBTOOL
# Checks for libraries.
# Check Poco
AC_CHECK_LIB(PocoFoundation, zlibVersion , [], [
	echo "Error! You need to have Poco installed!"
	exit -1])
# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

#ALL_LINGUAS

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([memset pow sqrt])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
