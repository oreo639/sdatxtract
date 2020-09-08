#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(sdatxtract, 0.2.0, https://github.com/oreo639/sdatxtract/issues)
AC_CONFIG_SRCDIR(src/main.cpp)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(subdir-objects foreign)

AC_CONFIG_HEADERS(src/config.h)

AC_DEFINE(APP_NAME, "SDATxtract", Name of application)
AC_DEFINE_UNQUOTED(APP_VERSION_MAJOR, 0, Major version of application)
AC_DEFINE_UNQUOTED(APP_VERSION_MINOR, 2, Minor version of application)
AC_DEFINE_UNQUOTED(APP_VERSION_MICRO, 0, Micro version of application)
AC_DEFINE(APP_VERSION_FULL, "APP_VERSION_MAJOR.APP_VERSION_MINOR.APP_VERSION_MICRO", Full version of application)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++], [C])

AX_CXX_COMPILE_STDCXX_17([noext],[mandatory])

AC_CONFIG_FILES(Makefile)
AC_OUTPUT
