## Standalone build of liboct_parser (the Octopus input parser).

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/src $(GSL_CFLAGS)

# ----------------------------------------------------------------------
# The C library
# ----------------------------------------------------------------------
lib_LTLIBRARIES = liboct_parser.la
include_HEADERS = src/liboct_parser.h

liboct_parser_la_SOURCES = \
	src/parse.c \
	src/parse_exp.c \
	src/symbols.c \
	src/gsl_userdef.c \
	src/varinfo_stubs.c \
	src/oct_parser_fbind.c \
	src/symbols.h \
	src/gsl_userdef.h \
	src/error_handling.h
liboct_parser_la_LIBADD = $(GSL_LIBS)
liboct_parser_la_LDFLAGS = -version-info $(LT_VERSION_INFO) -no-undefined

# src/parse_exp.c #includes the bison output src/grammar.c, which is shipped
# pre-generated (as in Octopus): bison is not needed to build.
EXTRA_DIST = src/grammar.c src/grammar.y

# Maintainers: "make regen-grammar" rebuilds src/grammar.c from src/grammar.y.
regen-grammar:
	@if test "x$(BISON)" = xno; then echo "bison not found"; exit 1; fi
	$(BISON) --name-prefix=oct_parser_ -o $(srcdir)/src/grammar.c $(srcdir)/src/grammar.y
.PHONY: regen-grammar

# ----------------------------------------------------------------------
# Fortran 2008 module (optional)
# ----------------------------------------------------------------------
if ENABLE_FORTRAN
lib_LTLIBRARIES += liboct_parser_f.la
liboct_parser_f_la_SOURCES = fortran/oct_parser.f90
liboct_parser_f_la_LIBADD = liboct_parser.la
liboct_parser_f_la_LDFLAGS = -version-info $(LT_VERSION_INFO) -no-undefined

# The module file is written in the build directory; its name depends on the
# compiler (oct_parser.mod for most, OCT_PARSER.mod for some).
install-data-local:
	$(MKDIR_P) "$(DESTDIR)$(fmoddir)"
	@for f in oct_parser.mod OCT_PARSER.mod; do \
	  if test -f "$$f"; then \
	    echo " $(INSTALL_DATA) $$f '$(DESTDIR)$(fmoddir)'"; \
	    $(INSTALL_DATA) "$$f" "$(DESTDIR)$(fmoddir)/$$f" || exit 1; \
	  fi; \
	done

uninstall-local:
	rm -f "$(DESTDIR)$(fmoddir)/oct_parser.mod" "$(DESTDIR)$(fmoddir)/OCT_PARSER.mod"
endif
EXTRA_DIST += fortran/oct_parser.f90

# ----------------------------------------------------------------------
# pkg-config and CMake package files
# ----------------------------------------------------------------------
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = oct_parser.pc
if ENABLE_FORTRAN
pkgconfig_DATA += oct_parser_f.pc
HAVE_FORTRAN_VALUE = ON
else
HAVE_FORTRAN_VALUE = OFF
endif

cmakedir = $(libdir)/cmake/OctParser
cmake_DATA = OctParserConfig.cmake OctParserConfigVersion.cmake

# These files are generated at "make" time so that the installation
# directories are fully expanded (they may be changed at "make install"
# time with e.g. "make prefix=... install").
do_subst = sed \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	-e 's|@OCTOPUS_COMMIT[@]|$(OCTOPUS_COMMIT)|g' \
	-e 's|@prefix[@]|$(prefix)|g' \
	-e 's|@libdir[@]|$(libdir)|g' \
	-e 's|@includedir[@]|$(includedir)|g' \
	-e 's|@fmoddir[@]|$(fmoddir)|g' \
	-e 's|@GSL_CFLAGS[@]|$(GSL_CFLAGS)|g' \
	-e 's|@GSL_LIBS[@]|$(GSL_LIBS)|g' \
	-e 's|@GSL_PREFIX[@]|$(GSL_PREFIX)|g' \
	-e 's|@PC_REQUIRES[@]|$(PC_REQUIRES)|g' \
	-e 's|@PC_GSL_CFLAGS[@]|$(PC_GSL_CFLAGS)|g' \
	-e 's|@PC_GSL_LIBS[@]|$(PC_GSL_LIBS)|g' \
	-e 's|@LIBS[@]|$(LIBS)|g' \
	-e 's|@HAVE_FORTRAN[@]|$(HAVE_FORTRAN_VALUE)|g'

oct_parser.pc: $(srcdir)/pkgconfig/oct_parser.pc.in Makefile
	$(AM_V_GEN)$(do_subst) < $(srcdir)/pkgconfig/oct_parser.pc.in > $@
oct_parser_f.pc: $(srcdir)/pkgconfig/oct_parser_f.pc.in Makefile
	$(AM_V_GEN)$(do_subst) < $(srcdir)/pkgconfig/oct_parser_f.pc.in > $@
OctParserConfig.cmake: $(srcdir)/cmake/OctParserConfig.cmake.in Makefile
	$(AM_V_GEN)$(do_subst) < $(srcdir)/cmake/OctParserConfig.cmake.in > $@
OctParserConfigVersion.cmake: $(srcdir)/cmake/OctParserConfigVersion.cmake.in Makefile
	$(AM_V_GEN)$(do_subst) < $(srcdir)/cmake/OctParserConfigVersion.cmake.in > $@

EXTRA_DIST += pkgconfig/oct_parser.pc.in pkgconfig/oct_parser_f.pc.in \
	cmake/OctParserConfig.cmake.in cmake/OctParserConfigVersion.cmake.in

# ----------------------------------------------------------------------
# Tests ("make check")
# ----------------------------------------------------------------------
check_PROGRAMS = tests/test_c
tests_test_c_SOURCES = tests/test_c.c
tests_test_c_LDADD = liboct_parser.la $(GSL_LIBS)

if HAVE_CXX
check_PROGRAMS += tests/test_cxx
tests_test_cxx_SOURCES = tests/test_cxx.cc
tests_test_cxx_LDADD = liboct_parser.la $(GSL_LIBS)
endif

if ENABLE_FORTRAN
check_PROGRAMS += tests/test_f
tests_test_f_SOURCES = tests/test_f.f90
tests_test_f_LDADD = liboct_parser_f.la liboct_parser.la
# the test needs the module, which is built with the library
tests/test_f.$(OBJEXT): liboct_parser_f.la
endif

TESTS = $(check_PROGRAMS)
AM_TESTS_ENVIRONMENT = OCTP_INPUT_FILE='$(srcdir)/tests/test.inp'; export OCTP_INPUT_FILE; \
	OCTP_TEST_Radius=7.5; export OCTP_TEST_Radius; \
	OCTP_TEST_PARSE_ENV=1; export OCTP_TEST_PARSE_ENV;
EXTRA_DIST += tests/test.inp examples/c/example.c examples/c/Makefile \
	examples/cmake/CMakeLists.txt examples/cmake/main.c examples/cmake/main.f90 \
	examples/example.inp README.md COPYING PROVENANCE.md

CLEANFILES = oct_parser.pc oct_parser_f.pc OctParserConfig.cmake OctParserConfigVersion.cmake \
	*.mod test_c.log_parser test_cxx.log_parser test_f.log_parser
