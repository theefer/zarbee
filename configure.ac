#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])

AC_INIT([zarbee], [1.0], [seb@cine7.net])

AM_INIT_AUTOMAKE([1.0 subdir-objects foreign])

AC_CONFIG_HEADERS([config.h])


# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET

# Checks for libraries (using pkg-config).

dnl Check for libxml2 and libpng
PKG_CHECK_MODULES([DEPS], [libxml-2.0 libpng])

dnl Check for GL, GLU
PKG_CHECK_MODULES([GL], [gl glu])

dnl Check for SDL
PKG_CHECK_MODULES([SDL], [sdl SDL_mixer])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(math.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STRTOD
AC_CHECK_FUNCS([pow select strtol])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
