/* Minimal example: read scalars and a block with liboct_parser.
   Build with:  make            (uses pkg-config, see Makefile)
   Run with:    ./example ../example.inp                          */
#include <stdio.h>
#include <stdlib.h>
#include <liboct_parser.h>

int main(int argc, char **argv)
{
  const char *input = argc > 1 ? argv[1] : "example.inp";
  int rank = 0;                      /* only rank 0 writes the log */
  sym_block *blk;

  /* "-" would send the log to stdout; "parser.log" records every value read */
  if (parse_init("parser.log", &rank) != 0) return 1;
  if (parse_input(input, 0) != 0) { fprintf(stderr, "cannot read %s\n", input); return 1; }

  long natoms   = (long)parse_int("NAtoms", 1);
  double cutoff = parse_double("Cutoff", 10.0);
  double temp   = parse_double("Temperature", 0.0);
  int restart   = (int)parse_int("Restart", 0);
  char *title   = parse_string("Title", "untitled");   /* malloc'ed: free it */
  gsl_complex zero, f;
  GSL_SET_COMPLEX(&zero, 0.0, 0.0);
  f = parse_complex("Field", zero);

  printf("%s: %ld atoms, cutoff %g, T = %g K, restart = %d, field = (%g, %g)\n",
         title, natoms, cutoff, temp, restart, GSL_REAL(f), GSL_IMAG(f));
  free(title);

  if (parse_block("Atoms", &blk) == 0) {
    for (int l = 0; l < parse_block_n(blk); l++) {
      char *sym; double x, y, z;
      parse_block_string(blk, l, 0, &sym);
      parse_block_double(blk, l, 1, &x);
      parse_block_double(blk, l, 2, &y);
      parse_block_double(blk, l, 3, &z);
      printf("  %-2s %8.3f %8.3f %8.3f\n", sym, x, y, z);
      free(sym);
    }
    parse_block_end(&blk);
  }

  sym_output_table(1, rank);   /* warn about assignments never read (typos) */
  parse_end();
  return 0;
}
