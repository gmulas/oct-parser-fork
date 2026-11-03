dnl Standalone build of liboct_parser, the input-file parser of Octopus
dnl (https://www.octopus-code.org).
dnl
dnl Version = date of the Octopus snapshot the sources were taken from.

AC_PREREQ([2.69])
AC_INIT([oct_parser], [2026.10.30.1], [], [oct_parser])
AC_CONFIG_SRCDIR([src/liboct_parser.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.13 foreign subdir-objects -Wall -Wno-portability dist-bzip2])
AM_SILENT_RULES([yes])

dnl Octopus commit the sources come from (informational).
OCTOPUS_COMMIT=80d00c1432a48a87245fd826b8dcf39ea681b06f
AC_SUBST([OCTOPUS_COMMIT])

dnl Libtool interface version current:revision:age (only change on API/ABI changes).
LT_VERSION_INFO=0:0:0
AC_SUBST([LT_VERSION_INFO])

dnl ------------------------------------------------------------------
dnl Compilers
dnl ------------------------------------------------------------------
AC_PROG_CC

dnl The Octopus sources use old-style function pointers, "gsl_complex (*f)()",
dnl called with arguments. In C23 (the default of GCC >= 15) "()" means "no
dnl arguments" and this is an error, so fall back to C17 when needed.
AC_DEFUN([OCTP_KR_FNPTR_TEST], [AC_LANG_PROGRAM([[
static int one(int x) { return x; }
static int (*fp)();]], [[fp = (int (*)())one; return fp(1) - 1;]])])
AC_MSG_CHECKING([whether $CC accepts calls through unprototyped function pointers])
AC_COMPILE_IFELSE([OCTP_KR_FNPTR_TEST], [AC_MSG_RESULT([yes])], [
  AC_MSG_RESULT([no])
  octp_kr_ok=no
  save_CFLAGS=$CFLAGS
  for octp_std in -std=gnu17 -std=gnu11 -std=c17 -std=c11; do
    CFLAGS="$save_CFLAGS $octp_std"
    AC_MSG_CHECKING([whether $CC accepts them with $octp_std])
    AC_COMPILE_IFELSE([OCTP_KR_FNPTR_TEST],
      [AC_MSG_RESULT([yes]); octp_kr_ok=yes; break], [AC_MSG_RESULT([no])])
  done
  AS_IF([test "x$octp_kr_ok" = xno], [
    CFLAGS=$save_CFLAGS
    AC_MSG_ERROR([$CC rejects old-style function pointers; set CFLAGS to select C17, e.g. CFLAGS="-O2 -std=gnu17"])])
])

AM_PROG_AR
LT_INIT

dnl C++ is only used to test that the header works from C++.
AC_PROG_CXX
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [have_cxx=yes], [have_cxx=no])
AC_LANG_POP([C++])
AM_CONDITIONAL([HAVE_CXX], [test "x$have_cxx" = xyes])

dnl Fortran module: --enable-fortran=yes|no|auto (default auto).
AC_ARG_ENABLE([fortran],
  [AS_HELP_STRING([--enable-fortran],
    [build the Fortran 2008 module oct_parser and liboct_parser_f (default: if a Fortran compiler is found)])],
  [], [enable_fortran=auto])

dnl AC_PROG_FC must be expanded unconditionally; it does not fail if no
dnl compiler is found (FC is then empty).
AC_PROG_FC([gfortran ifx ifort flang-new flang nvfortran xlf2008 ftn f95])
have_fc=no
AS_IF([test "x$enable_fortran" != xno], [
  AS_IF([test -n "$FC"], [
    AC_LANG_PUSH([Fortran])
    AC_FC_SRCEXT([f90])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
module conftest_m
  use, intrinsic :: iso_c_binding
  implicit none
  character(len=:), allocatable :: s
contains
  subroutine f(x) bind(c, name="conftest_f")
    complex(c_double_complex), intent(in) :: x
    s = "ok"
    if (real(x) < 0) error stop 1
  end subroutine f
end module conftest_m
]])], [have_fc=yes], [have_fc=no])
    AC_LANG_POP([Fortran])
  ])
  AS_IF([test "x$have_fc" = xyes], [LT_LANG([Fortran])])
  AS_IF([test "x$have_fc" = xno && test "x$enable_fortran" = xyes],
    [AC_MSG_ERROR([--enable-fortran given but no working Fortran 2008 compiler found (set FC=...)])])
])
AM_CONDITIONAL([ENABLE_FORTRAN], [test "x$have_fc" = xyes])

dnl Where to install the compiled Fortran module (.mod files are compiler specific).
AC_ARG_WITH([moduledir],
  [AS_HELP_STRING([--with-moduledir=DIR],
    [install the Fortran .mod file in DIR (default: INCLUDEDIR)])],
  [fmoddir="$withval"], [fmoddir='${includedir}'])
AC_SUBST([fmoddir])

dnl ------------------------------------------------------------------
dnl Compiler features
dnl ------------------------------------------------------------------
AC_MSG_CHECKING([whether the C compiler supports __attribute__((weak))])
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([[__attribute__((weak)) int conftest_weak(void) { return 0; }]],
                   [[return conftest_weak();]])],
  [AC_MSG_RESULT([yes])
   AC_DEFINE([HAVE_ATTRIBUTE_WEAK], [1], [Define if the compiler supports weak symbols])],
  [AC_MSG_RESULT([no])])

AC_SEARCH_LIBS([cos], [m])

dnl ------------------------------------------------------------------
dnl GSL (the only external dependency): --with-gsl=PREFIX, pkg-config,
dnl gsl-config, or the default search path, in this order.
dnl ------------------------------------------------------------------
AC_ARG_WITH([gsl],
  [AS_HELP_STRING([--with-gsl=PREFIX], [GSL installation prefix (default: use pkg-config or gsl-config)])],
  [], [with_gsl=check])

gsl_via_pkgconfig=no
GSL_PREFIX=
AS_IF([test "x$with_gsl" = xno],
        [AC_MSG_ERROR([GSL is required by liboct_parser])],
      [test "x$with_gsl" != xcheck && test "x$with_gsl" != xyes], [
        GSL_CFLAGS="-I$with_gsl/include"
        AS_IF([test -d "$with_gsl/lib64" && test ! -d "$with_gsl/lib"],
          [GSL_LIBS="-L$with_gsl/lib64 -lgsl -lgslcblas"],
          [GSL_LIBS="-L$with_gsl/lib -lgsl -lgslcblas"])
        GSL_PREFIX="$with_gsl"
      ], [
        PKG_PROG_PKG_CONFIG
        PKG_CHECK_MODULES([GSL], [gsl], [
          gsl_via_pkgconfig=yes
          GSL_PREFIX=`$PKG_CONFIG --variable=prefix gsl`
        ], [
          AC_PATH_PROG([GSL_CONFIG], [gsl-config], [no])
          AS_IF([test "x$GSL_CONFIG" != xno], [
            GSL_CFLAGS=`$GSL_CONFIG --cflags`
            GSL_LIBS=`$GSL_CONFIG --libs`
            GSL_PREFIX=`$GSL_CONFIG --prefix`
          ], [
            GSL_CFLAGS=
            GSL_LIBS="-lgsl -lgslcblas"
          ])
        ])
      ])

save_CPPFLAGS=$CPPFLAGS; save_LIBS=$LIBS
CPPFLAGS="$CPPFLAGS $GSL_CFLAGS"
LIBS="$GSL_LIBS $LIBS"
AC_CHECK_HEADER([gsl/gsl_complex_math.h], [],
  [AC_MSG_ERROR([GSL headers not found; install GSL (e.g. libgsl-dev / gsl-devel / brew install gsl) or use --with-gsl=PREFIX])])
AC_MSG_CHECKING([whether GSL links])
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([[#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_sf_erf.h>]],
    [[gsl_complex z = gsl_complex_rect(1.0, 0.0); z = gsl_complex_sin(z); return (int)gsl_sf_erf(GSL_REAL(z));]])],
  [AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no]); AC_MSG_ERROR([cannot link with GSL using: $GSL_LIBS])])
CPPFLAGS=$save_CPPFLAGS; LIBS=$save_LIBS
AC_SUBST([GSL_CFLAGS])
AC_SUBST([GSL_LIBS])
AC_SUBST([GSL_PREFIX])

dnl What goes in oct_parser.pc: depend on gsl.pc when we found GSL that way.
AS_IF([test "x$gsl_via_pkgconfig" = xyes],
  [PC_REQUIRES="gsl"; PC_GSL_CFLAGS=""; PC_GSL_LIBS=""],
  [PC_REQUIRES=""; PC_GSL_CFLAGS="$GSL_CFLAGS"; PC_GSL_LIBS="$GSL_LIBS"])
AC_SUBST([PC_REQUIRES])
AC_SUBST([PC_GSL_CFLAGS])
AC_SUBST([PC_GSL_LIBS])

dnl Optional: regenerate src/grammar.c from src/grammar.y (maintainers only).
AC_PATH_PROG([BISON], [bison], [no])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

octp_short_commit=`echo $OCTOPUS_COMMIT | cut -c1-12`
AC_MSG_NOTICE([
  oct_parser $PACKAGE_VERSION (from Octopus commit $octp_short_commit)
  prefix .............. $prefix
  C compiler .......... $CC $CFLAGS
  GSL ................. $GSL_CFLAGS $GSL_LIBS
  Fortran module ...... $have_fc${FC:+ ($FC)}
  C++ header test ..... $have_cxx
])
