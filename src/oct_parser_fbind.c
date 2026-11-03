/*
 Small C helpers used by the Fortran module oct_parser (fortran/oct_parser.f90).

 Most of the C API of liboct_parser is called directly from Fortran through
 BIND(C) interfaces. The functions below exist only where a direct binding
 is not portable: the C API passes gsl_complex by value, and some
 operations need internal symbols (rmsym) that are not part of the public
 header. Everything here takes pointers, so it can be bound from Fortran
 with standard ISO_C_BINDING.

 These functions are pure C and add no dependencies: they live in
 liboct_parser itself, so the Fortran module only needs the Fortran
 compiler and nothing else.

 This file is not part of Octopus. It replaces, for standalone use, the
 Octopus-specific glue in liboct_parser/parser_f.c. Distributed under the
 GNU GPL, version 2 or later.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_complex.h>

#include "liboct_parser.h"
#include "symbols.h"

void octp_f_parse_complex(const char *name, const gsl_complex *def, gsl_complex *res);
void octp_f_putsym_complex(const char *name, const gsl_complex *c);
int  octp_f_eval(const char *expr, gsl_complex *res);
int  octp_f_rmsym(const char *name);
void octp_f_free(void *p);

void octp_f_parse_complex(const char *name, const gsl_complex *def, gsl_complex *res)
{
  *res = parse_complex(name, *def);
}

void octp_f_putsym_complex(const char *name, const gsl_complex *c)
{
  parse_putsym_complex(name, *c);
}

/* Evaluate an expression with the current symbol table.
   Returns 0 on success, 1 on a syntax error, 2 if the expression evaluates
   to a string instead of a number. */
int octp_f_eval(const char *expr, gsl_complex *res)
{
  parse_result r;
  char *buf;
  int status;

  buf = strdup(expr);
  if (buf == NULL) return 1;

  status = parse_exp(buf, &r);
  free(buf);
  if (status != 0) return 1;

  if (r.type != PR_CMPLX) {
    parse_result_free(&r);
    return 2;
  }
  *res = r.value.c;
  return 0;
}

int octp_f_rmsym(const char *name)
{
  return rmsym(name);
}

void octp_f_free(void *p)
{
  free(p);
}
