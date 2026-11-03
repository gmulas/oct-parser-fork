# Provenance

The parser sources in `src/` come from the Octopus code
(https://www.octopus-code.org, https://gitlab.com/octopus-code/octopus),
directory `liboct_parser/`, branch `main`:

- commit `80d00c1432a48a87245fd826b8dcf39ea681b06f` (2026-10-30)
- Octopus version line: 16.x (16.4 released 2026-04-21)

## Files taken unchanged

    src/parse.c  src/parse_exp.c  src/symbols.c  src/symbols.h
    src/gsl_userdef.c  src/gsl_userdef.h  src/error_handling.h
    src/grammar.y  src/grammar.c (bison 3.4 output, as shipped by Octopus)

## Files changed

- `src/liboct_parser.h`: added `extern "C"` guards (C++ use) and the
  declaration of `sym_output_table()`, which was only in the private
  `symbols.h`. No other change.

## Files not taken

- `liboct_parser/parser_f.c`: the Octopus Fortran glue. It needs Octopus'
  `config.h` and Fortran name-mangling macros, and is designed for Octopus'
  own `parser.F90`, which depends on many other Octopus modules. It is
  replaced by `src/oct_parser_fbind.c` + `fortran/oct_parser.f90`.
- `liboct_parser/CMakeLists.txt`: replaced by the autotools build.

## Files added (not part of Octopus)

- `src/varinfo_stubs.c`: weak no-op versions of the four `varinfo_*`
  functions that Octopus defines in `src/basic/varinfo_low.c` and that the
  parser calls (see the comments in the file).
- `src/oct_parser_fbind.c`, `fortran/oct_parser.f90`: Fortran 2008 interface.
- Build system (`configure.ac`, `Makefile.am`), pkg-config and CMake package
  files, tests, examples, documentation.

## Updating to a newer Octopus

Copy the files listed above from the new `liboct_parser/` into `src/`,
re-apply the two small header changes, check whether new external symbols
appeared (`nm -u .libs/liboct_parser.so`), update `OCTOPUS_COMMIT` and the
version (the snapshot date) in `configure.ac`, then run `autoreconf -fi`
and `make distcheck`.
