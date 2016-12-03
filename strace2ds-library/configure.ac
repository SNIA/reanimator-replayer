# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([strace2ds], [1.0], [ezk@fsl.cs.sunysb.edu])
AC_CONFIG_SRCDIR([DataSeriesOutputModule.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE

# look for DataSeries and Lintel libraries in special locations
CXXFLAGS="$CXXFLAGS -I/usr/local/dataseries/include \
		-I/usr/include/libxml2"

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
LT_INIT

# Checks for libraries.
AC_CHECK_LIB([DataSeries], [dataseriesVersion])
AC_CHECK_LIB([Lintel], [lintelVersion])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT32_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
