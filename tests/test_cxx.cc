// Checks that liboct_parser.h can be used from C++ (extern "C" linkage).
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>
#include <liboct_parser.h>

int main()
{
  const char *input = std::getenv("OCTP_INPUT_FILE");
  if (!input) input = "test.inp";
  int rank = 0;
  if (parse_init("test_cxx.log_parser", &rank) != 0) return 1;
  if (parse_input(input, 0) != 0) return 1;

  int ok = 1;
  ok &= parse_int("NumberOfSteps", 0) == 100;
  ok &= std::fabs(parse_double("BoxSize", 0.0) - 5.0) < 1e-12;
  char *t = parse_string("Title", "");
  ok &= std::string(t) == "Water molecule";
  std::free(t);

  sym_block *blk;
  if (parse_block("Species", &blk) == 0) {
    ok &= parse_block_n(blk) == 2;
    parse_block_end(&blk);
  } else ok = 0;

  parse_end();
  std::cout << (ok ? "test_cxx: all checks passed" : "test_cxx: FAILED") << std::endl;
  return ok ? 0 : 1;
}
