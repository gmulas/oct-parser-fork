/* Test of the C API of liboct_parser (run by "make check"). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <liboct_parser.h>

static int failures = 0;

#define CHECK(cond) do { if (!(cond)) { \
  fprintf(stderr, "FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)
#define CLOSE(a, b) (fabs((a) - (b)) < 1e-12)

int main(void)
{
  const char *input = getenv("OCTP_INPUT_FILE");
  int rank = 0;
  sym_block *blk;
  char *s;
  gsl_complex z, zero;
  double d;
  int ival;
  parse_result res;
  char expr[64];

  if (input == NULL) input = "test.inp";
  GSL_SET_COMPLEX(&zero, 0.0, 0.0);

  CHECK(parse_init("test_c.log_parser", &rank) == 0);
  CHECK(parse_input(input, 0) == 0);
  parse_environment("OCTP_TEST_");

  CHECK(parse_int("CalculationMode", 0) == 2);
  CHECK(parse_int("NumberOfSteps", 0) == 100);
  CHECK(parse_int("NotInInput", -7) == -7);
  CHECK(CLOSE(parse_double("Spacing", 0.0), 0.25));
  CHECK(CLOSE(parse_double("BoxSize", 0.0), 5.0));
  CHECK(CLOSE(parse_double("LongExpression", 0.0), 10.0));
  /* set from the environment (OCTP_TEST_Radius=7.5) by make check */
  if (getenv("OCTP_TEST_PARSE_ENV") != NULL)
    CHECK(CLOSE(parse_double("Radius", 0.0), 7.5));
  CHECK(parse_int("Verbose", 0) == 1);
  CHECK(parse_int("Debug", 1) == 0);
  CHECK(parse_isdef("Title") && !parse_isdef("NotInInput"));
  /* names are case insensitive */
  CHECK(parse_int("numberofsteps", 0) == 100);

  z = parse_complex("Phase", zero);
  CHECK(fabs(GSL_REAL(z)) < 1e-12 && CLOSE(GSL_IMAG(z), 1.0));

  s = parse_string("Title", "none");
  CHECK(strcmp(s, "Water molecule") == 0);
  free(s);
  s = parse_string("NoTitle", "default");
  CHECK(strcmp(s, "default") == 0);
  free(s);

  /* blocks */
  CHECK(parse_block("Species", &blk) == 0);
  CHECK(parse_block_n(blk) == 2);
  CHECK(parse_block_cols(blk, 0) == 3);
  CHECK(parse_block_cols(blk, 1) == 4);
  CHECK(parse_block_string(blk, 1, 0, &s) == 0);
  CHECK(strcmp(s, "O") == 0);
  free(s);
  CHECK(parse_block_double(blk, 1, 1, &d) == 0 && CLOSE(d, 15.9994));
  CHECK(parse_block_int(blk, 1, 2, &ival) == 0 && ival == 8);
  CHECK(parse_block_complex(blk, 1, 3, &z) == 0 && CLOSE(GSL_REAL(z), 2.0) && CLOSE(GSL_IMAG(z), 3.0));
  parse_block_end(&blk);
  CHECK(blk == NULL);

  CHECK(parse_block("Lattice", &blk) == 0);
  CHECK(parse_block_double(blk, 0, 1, &d) == 0 && CLOSE(d, 0.5));
  parse_block_end(&blk);
  CHECK(parse_block("NoSuchBlock", &blk) != 0);

  /* expressions and symbols set from the program */
  parse_putsym_double("x", 2.0);
  strcpy(expr, "x^2 + sin(pi/2)");
  CHECK(parse_exp(expr, &res) == 0 && res.type == PR_CMPLX && CLOSE(GSL_REAL(res.value.c), 5.0));
  parse_result_free(&res);
  parse_putsym_int("NumberOfSteps", 3);
  CHECK(parse_int("NumberOfSteps", 0) == 3);

  /* input given as a string */
  CHECK(parse_input_string("FromString = 2*Spacing\n", 0) == 0);
  CHECK(CLOSE(parse_double("FromString", 0.0), 0.5));

  /* should list UnusedTypo (and a few others) on stderr */
  sym_output_table(1, 0);

  parse_end();

  if (failures == 0) printf("test_c: all checks passed\n");
  return failures == 0 ? 0 : 1;
}
