# oct_parser — the Octopus input parser as a standalone library

This is `liboct_parser`, the input-file parser of the
[Octopus](https://www.octopus-code.org) code, extracted so that it can be
built and installed on its own, without Octopus and its dependencies.

It reads input files like this one:

```
# comments start with '#'
NAtoms      = 3
Cutoff      = 20*Units + sqrt(2)        # full arithmetic, functions, constants
Field       = 0.01*exp(i*pi/4)          # complex numbers
Title       = "My simulation"
Restart     = no                        # yes/no, true/false
LongExpr    = 1 + 2 + \
              3                         # continuation lines

%Atoms                                  # blocks: lines of '|'-separated columns
 "O" |  0.000 | 0.000 | 0.000
 "H" |  0.757 | 0.586 | 0.000
%
```

and provides C (also usable from C++) and Fortran 2008 interfaces to query it.

## Origin, credits and why this fork exists

- **Origin.** The parser sources come from Octopus
  (https://gitlab.com/octopus-code/octopus), directory `liboct_parser/` of
  the `main` branch at commit `80d00c1432a48a87245fd826b8dcf39ea681b06f`
  (2026-10-30), in the 16.x version line (Octopus 16.4 was released on
  2026-04-21). `PROVENANCE.md` lists exactly which files were taken
  unchanged, which were changed (one header, with `extern "C"` guards and
  one more declaration) and which were added.
- **Credits.** The parser is the work of the Octopus authors: M. Marques,
  A. Castro, A. Rubio, G. Bertsch, D. Strubbe, A. Buccheri and H. Menke, as
  stated in the headers of its files. Octopus as a whole is developed by
  the team listed on its
  ["About Octopus"](https://octopus-code.org/documentation/16/manual/about_octopus/)
  page. If you use this library in published work, please cite Octopus,
  for example M. A. L. Marques, A. Castro, G. F. Bertsch and A. Rubio,
  "octopus: a first-principles tool for excited electron-ion dynamics",
  Comput. Phys. Commun. **151**, 60 (2003), and the more recent papers
  listed on that page.
- **Why a fork.** Only to make the parser usable in other projects with a
  compatible licence without building the whole of Octopus. It adds a
  standalone build (autotools, pkg-config and CMake files), a Fortran 2008
  interface and tests, and changes nothing in the parsing itself. It is not
  a replacement for, nor a development branch of, Octopus: improvements to
  the parser belong upstream. It is used, for example, by
  [AnharmoniCaOs](https://sourceforge.net/projects/anharmonica/).
- **In the spirit of Octopus.** This fork exists with gratitude to the
  Octopus developers, whose work it relies on entirely, and with no intent
  of competing with or diverging from their project. Its only aim is to
  make their parser available to more users, under the same free licence.
  Bug reports that concern the parsing itself are forwarded upstream.
- **Following upstream.** The fork follows the development of the parser in
  Octopus and brings in its fixes and improvements. A weekly GitHub
  workflow (`.github/workflows/upstream-check.yml`) lists the Octopus
  commits that changed `liboct_parser/` after the one this copy comes from,
  and opens an issue here when there are any; the update itself is done by
  hand, as described in `PROVENANCE.md`.

## Requirements

- a C compiler (C99);
- [GSL](https://www.gnu.org/software/gsl/) (headers and library), the only
  dependency: `libgsl-dev` (Debian/Ubuntu), `gsl-devel` (Fedora/RHEL),
  `brew install gsl` (macOS);
- optional: a Fortran 2008 compiler (gfortran, ifx, flang, nvfortran, ...)
  for the Fortran module; a C++ compiler is only used by `make check`.

Bison and the autotools are **not** needed to build from the tarball.

## Build and install

```sh
tar xjf oct_parser-2026.10.30.tar.bz2
cd oct_parser-2026.10.30
./configure --prefix=$HOME/.local        # default prefix: /usr/local
make
make check                               # optional, runs the tests
make install
```

Out-of-tree builds work as usual (`mkdir build && cd build && ../configure ...`).

Useful `configure` options:

| option | meaning |
|---|---|
| `--prefix=DIR` | installation prefix |
| `--with-gsl=DIR` | GSL installed in `DIR` (default: found via `pkg-config`, then `gsl-config`, then the default search path) |
| `--enable-fortran` / `--disable-fortran` | build the Fortran module (default: if a Fortran compiler is found) |
| `--with-moduledir=DIR` | where to install `oct_parser.mod` (default: `PREFIX/include`) |
| `--disable-shared` / `--disable-static` | build only the static / shared library |
| `CC=... FC=... CFLAGS=... FCFLAGS=...` | compilers and flags, as usual |

What gets installed:

```
PREFIX/include/liboct_parser.h
PREFIX/include/oct_parser.mod                      (Fortran module, if enabled)
PREFIX/lib/liboct_parser.{so,a}                    (C library)
PREFIX/lib/liboct_parser_f.{so,a}                  (Fortran module library, if enabled)
PREFIX/lib/pkgconfig/oct_parser.pc, oct_parser_f.pc
PREFIX/lib/cmake/OctParser/OctParserConfig.cmake, OctParserConfigVersion.cmake
```

`make uninstall` removes them. If the prefix is not a system one, programs
linked to the shared library need `LD_LIBRARY_PATH=PREFIX/lib` at run time
(or link with `-Wl,-rpath,PREFIX/lib`, or use the static library).

## Using it from your projects

### Plain Makefile / command line (pkg-config)

```sh
export PKG_CONFIG_PATH=$HOME/.local/lib/pkgconfig:$PKG_CONFIG_PATH   # if needed
cc -c myprog.c $(pkg-config --cflags oct_parser)
cc -o myprog myprog.o $(pkg-config --libs oct_parser)
# Fortran:
gfortran -c myprog.f90 $(pkg-config --cflags oct_parser_f)
gfortran -o myprog myprog.o $(pkg-config --libs oct_parser_f)
```

See `examples/c/` for a complete example with its Makefile.

### Autoconf/automake projects

```m4
PKG_CHECK_MODULES([OCT_PARSER], [oct_parser >= 2026.10])
dnl and, for Fortran:
PKG_CHECK_MODULES([OCT_PARSER_F], [oct_parser_f])
```

then use `$(OCT_PARSER_CFLAGS)`/`$(OCT_PARSER_LIBS)` (or the `_F_` ones, with
`FCFLAGS`) in `Makefile.am`.

### CMake projects

```cmake
find_package(OctParser REQUIRED)                       # C / C++
target_link_libraries(myprog PRIVATE OctParser::oct_parser)

find_package(OctParser REQUIRED COMPONENTS Fortran)    # Fortran
target_link_libraries(myprog_f PRIVATE OctParser::oct_parser_f)
```

Configure with `-DCMAKE_PREFIX_PATH=<prefix>` if the prefix is not a
standard one. `-DOctParser_USE_STATIC=ON` selects the static libraries.
See `examples/cmake/`.

## C API (also for C++)

```c
#include <liboct_parser.h>

int rank = 0;                         /* only rank 0 writes the log (MPI) */
parse_init("parser.log", &rank);      /* log of every value read; "-" = stdout */
parse_input("input.inp", 0);          /* 0 on success; "-" = stdin */
parse_input_string("A = 1\n", 0);     /* input from a string */
parse_environment("MYCODE_");         /* if MYCODE_PARSE_ENV is set, MYCODE_X=... sets X */

int64_t     n = parse_int    ("NAtoms", 1);         /* the 2nd argument is the default */
double      x = parse_double ("Cutoff", 10.0);
gsl_complex z = parse_complex("Field",  zero);
char       *s = parse_string ("Title",  "none");    /* malloc'ed: free() it */
int defined   = parse_isdef  ("Title");

sym_block *blk;
if (parse_block("Atoms", &blk) == 0) {              /* 0 = block found */
  int nl = parse_block_n(blk);                      /* number of lines */
  int nc = parse_block_cols(blk, 0);                /* columns of line 0 */
  double v; char *str;
  parse_block_double(blk, 1, 2, &v);                /* line 1, column 2 (0-based) */
  parse_block_string(blk, 1, 0, &str); free(str);
  /* also parse_block_int, parse_block_int8, parse_block_complex */
  parse_block_end(&blk);
}

parse_putsym_double("x", 2.0);                      /* define symbols from the program */
parse_result r;  char e[] = "x^2 + 1";
parse_exp(e, &r);                                   /* evaluate an expression */
parse_result_free(&r);

sym_output_table(1, rank);                          /* list assignments never read (typos!) */
parse_end();
```

Variable names are case insensitive. Predefined symbols include `pi`, `e`,
`i`, `true`/`false`, `yes`/`no` and many functions (`sqrt`, `exp`, `log`,
`sin`..., `erf`, `abs`, `arg`, `min`, `max`, `atan2`, `floor`, `ceiling`,
`rand`, ...).

## Fortran API (`use oct_parser`)

```fortran
use oct_parser
type(oct_block) :: blk
character(len=:), allocatable :: title, sym

call oct_parser_init("parser.log")          ! optional: rank=, ierr=
call oct_parser_input_file("input.inp")     ! optional: set_used=, ierr=
call oct_parse("NAtoms", 1, natoms)         ! integer, integer(int64), real(c_double),
call oct_parse("Cutoff", 10.0_dp, cutoff)   ! complex(c_double_complex), logical,
call oct_parse("Title", "none", title)      ! character (allocatable result)

if (oct_block_open("Atoms", blk)) then
  do l = 0, oct_block_n(blk) - 1            ! 0-based, as in C and in Octopus
    call oct_block_get(blk, l, 0, sym)      ! same types as oct_parse; optional ierr=
    call oct_block_get(blk, l, 1, x)
  end do
  call oct_block_close(blk)
end if

z = oct_parser_eval("exp(-r^2)", ["r"], [1.5_dp])   ! expression with temporary variables
call oct_parser_putsym("Nk", 8)
call oct_parser_output_table(only_unused=.true.)
call oct_parser_end()
```

Also: `oct_parser_input_string`, `oct_parser_environment`, `oct_parser_isdef`,
`oct_block_cols`, `oct_block_is_open`. This module is new (not the Octopus
`parser_oct_m` module, which depends on the rest of Octopus); it uses only
standard `iso_c_binding`. A `.mod` file can only be used with the compiler
(and usually the compiler version) that built it.

## Behaviour to be aware of (inherited from Octopus)

- **Errors in the input terminate the program** (`exit(1)`) with a
  message on stderr, e.g. a string given where a number is expected.
- The library has **global state**: one input at a time, not thread-safe.
  Call `parse_end()` before parsing another input with `parse_init()`.
- The log file starts with `# Octopus parser started` and records every
  value read, marking defaults with `# default`.
- In Octopus, integer options are logged with their symbolic names (e.g.
  `CalculationMode = gs`) via Octopus' table of input variables. Here that
  table does not exist and plain numbers are logged; for the same reason
  `sym_output_table(1, ...)` lists every assignment never read. A program
  may provide its own `varinfo_*` functions to restore that behaviour (see
  `src/varinfo_stubs.c`).
- Internal symbols (`putsym`, `getsym`, `str_trim`, `sym_*`, some
  `gsl_complex_*` extensions) are exported, as in Octopus; avoid defining
  functions with the same names in your programs.
- `parse_input_string` uses `fmemopen` (POSIX 2008; on macOS from 10.13).

## Licence and authors

The parser is part of Octopus. Its source files are copyright of the
Octopus authors (M. Marques, A. Castro, A. Rubio, G. Bertsch, D. Strubbe,
A. Buccheri, H. Menke; see the header of each file) and are distributed
under the GNU General Public License, "version 2, or (at your option) any
later version" (SPDX `GPL-2.0-or-later`), as their headers state. The Octopus
code as a whole is distributed under the GPL version 2 (Octopus manual,
"Copying"). `src/grammar.c` is the output of GNU Bison, under the GPL
version 3 or later with the Bison exception.

The files added here (`src/oct_parser_fbind.c`, `src/varinfo_stubs.c`,
`fortran/oct_parser.f90`, the build system, tests and examples) are by
Giacomo Mulas and have the same licence, `GPL-2.0-or-later`.

The text of the GPL version 3 is in `COPYING`, that of version 2 in
`COPYING.GPL-2`. Programs you distribute linked with this library must
comply with the GPL.

## Maintainer notes

`src/grammar.c` is generated by bison from `src/grammar.y` and is shipped
pre-generated, as in Octopus. To regenerate it: `make regen-grammar`.
After changing `configure.ac` or `Makefile.am`: `autoreconf -fi`.
To make a release tarball: `make distcheck`.
