/*
 Default (no-op) implementations of the Octopus "varinfo" hooks.

 Inside Octopus, parse.c and symbols.c call four functions defined in
 src/basic/varinfo_low.c, which look up variables in Octopus' own table of
 documented input variables. They are used only for cosmetic purposes:

   - parse_int() uses varinfo_getvar/varinfo_getopt/varinfo_opt_getinfo to
     write symbolic option names (e.g. "CalculationMode = gs") instead of
     plain integers into the parser log file;
   - sym_output_table(1, ...) uses varinfo_variable_exists() to avoid
     reporting as "unused" variables that Octopus knows about.

 In the standalone library there is no such table, so these stubs report
 "no information": parse_int() logs plain integers and sym_output_table()
 reports every assignment that was never read.

 The stubs are weak symbols (with GCC, Clang, Intel and compatible
 compilers): a program that wants the Octopus behaviour can provide its own
 strong definitions with the same prototypes and they will be used instead.

 This file is not part of Octopus. It is distributed under the same terms as
 the rest of the library (GNU GPL, version 2 or later).
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <stddef.h>

#if defined(HAVE_ATTRIBUTE_WEAK)
#define OCTP_WEAK __attribute__((weak))
#else
#define OCTP_WEAK
#endif

OCTP_WEAK void varinfo_getvar(const char *name_c, void **var)
{
  (void)name_c;
  *var = NULL;
}

OCTP_WEAK void varinfo_getopt(const void **var, void **opt)
{
  (void)var;
  *opt = NULL;
}

OCTP_WEAK void varinfo_opt_getinfo(const void **opt, char **name, int64_t *value, char **desc)
{
  (void)opt;
  *name = NULL;
  *value = 0;
  *desc = NULL;
}

OCTP_WEAK int varinfo_variable_exists(const char *var_name)
{
  (void)var_name;
  return 0;
}
