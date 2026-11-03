#include <stdio.h>
#include <liboct_parser.h>

int main(int argc, char **argv)
{
  int rank = 0;
  if (parse_init("-", &rank) != 0) return 1;          /* log to stdout */
  if (parse_input(argc > 1 ? argv[1] : "example.inp", 0) != 0) return 1;
  printf("NAtoms = %ld\n", (long)parse_int("NAtoms", 0));
  parse_end();
  return 0;
}
