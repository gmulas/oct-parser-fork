/*
 Copyright (C) 2002 - 2024 M. Marques, A. Castro, A. Rubio, G. Bertsch, D. Strubbe, A. Buccheri

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301, USA.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

extern char ** environ;

#include "error_handling.h"
#include "liboct_parser.h"
#include "symbols.h"
#include "gsl_userdef.h"

#define ROUND(x) ((x)<0 ? (int64_t)(x-0.5) : (int64_t)(x+0.5))

static FILE *fout;
static int disable_write = 0;
static const char block_delimiter = '%';


void str_trim(char *in)
{
  char *c, *s = in;

  for(c=s; isspace(*c); c++);
  for(; *c != '\0'; *s++=*c++);
  for(s--; s>=in && isspace(*s); s--);
  *(s+1) = '\0';
}

static int parse_get_line(FILE *f, char **s, int *length)
{
  int i, c;

  i = 0;
  do{
    c = getc(f);
    if(c == '#') /* skip comments */
      while(c!=EOF && c!='\n') c = getc(f);
    else if(c != EOF){
      if (i == *length - 1){
	*length *= 2;
	*s = (char *)realloc(*s, *length + 1);
//        memset(s + *length/2, 0, (*length - *length/2) + 1);
      }
      /* check for continuation lines */
      if(i > 0 && c == '\n' && (*s)[i-1] == '\\'){
        c = 'd'; /* dummy */
        i--;
      }else
        (*s)[i++] = (char)c;
    }
  }while(c != EOF && c != '\n');
  (*s)[i] = '\0';
	
  str_trim(*s);
  return c;
}

/**
 * @brief Initialise symbol table, and set file handle for parser log.
 *
 * The symbol table must be initialised for the parser to work.
 *
 * @param file_out   A character pointer to the file name. Return to stdout if '-' is passed.
 * @param mpiv_node  An integer pointer to the MPI process. This function only writes from process 0.
 *
 */
int parse_init(const char *file_out, const int *mpiv_node)
{
  sym_init_table();

  /* only enable writes for node 0 */
  disable_write = *mpiv_node;

  if(disable_write) return 0;

  if(strcmp(file_out, "-") == 0)
    fout = stdout;
  else {
    fout = fopen(file_out, "w");
    if(!fout)
      return -1; /* error opening file */
    setvbuf(fout, NULL, _IONBF, 0);
  }
  fprintf(fout, "# Octopus parser started\n");
  
  return 0;
}

/**
 * @brief Parses a data block from the input file.
 *
 * This function reads lines from the input file until a block delimiter is encountered.
 * It then parses each line within the block, splitting it into columns delimited by '|' or '\t'.
 * The parsed data is stored as a matrix-like structure in the symbol table.
 * If the block is already defined in the symbol table, it skips the block and moves the file pointer on.
 *
 * @param f          A pointer to the input file.
 * @param s          A pointer to a pointer to the current line buffer. The function may implicitly
 *                   reallocate this buffer if needed to accommodate longer lines, via `parse_get_line`.
 * @param length     A pointer to an integer representing the current size of the line buffer.
 *                   This value is updated if the buffer is reallocated, via `parse_get_line`.
 * @param pc         A pointer to a parse_result structure.
 *
 * @warning This function modifies the `*s` buffer and the `length` variable. The caller must ensure
 *          that the `*s` buffer is properly allocated and freed after use.
 */
int parse_input_block(FILE *f, char **s, int *length, parse_result *pc) {

    int c;
    **s = ' ';
    str_trim(*s);

    // If the block is already defined in the symbol table, skip it
    if (getsym(*s) != NULL) {
        fprintf(stderr, "Parser warning: Block \"%s\" already defined.\n", *s);
        do {
            c = parse_get_line(f, s, length);
        } while (c != EOF && **s != block_delimiter);
        return c;
    }

    symrec *rec = putsym(*s, S_BLOCK);
    rec->value.block = (sym_block *) malloc(sizeof(sym_block));
    if (!rec->value.block) {
        error_return(-1, "malloc failed to allocate rec->value.block");
    }
    rec->value.block->n = 0;
    rec->value.block->lines = NULL;

    do {
        c = parse_get_line(f, s, length);

        // If line is not empty, and not a block delimiter
        if (**s && **s != block_delimiter) {
            char *s1, *tok;
            int l, col;

            l = rec->value.block->n;
            rec->value.block->n++;
            rec->value.block->lines = (sym_block_line *)
                    realloc((void *) rec->value.block->lines, sizeof(sym_block_line) * (l + 1));
            rec->value.block->lines[l].n = 0;

            /* parse columns */
            for (s1 = *s; (tok = strtok(s1, "|")) != NULL; s1 = NULL) {
                char *tok2 = strdup(tok);
#ifndef NDEBUG
                if (!tok2) {
                    // Likely an error with the parsed line *s
                    error_abort("String tokenize failed parsing inp block");
                }
#endif
                str_trim(tok2);

                col = rec->value.block->lines[l].n;
                char *mtxel = (char *) calloc(strlen(tok2) + strlen(rec->name) + 20, sizeof(*mtxel));
                assert(mtxel);
                sprintf(mtxel, "%s[%i][%i] = %s", rec->name, l, col, tok2);
                parse_exp(mtxel, pc);
                free(mtxel);
                free(tok2);

                rec->value.block->lines[l].n++;
            }
        }
    } while (c != EOF && **s != block_delimiter);

    return c;
}

/**
 * @brief Parses an input file to extract and set a random seed value, if present.
 *
 * This function scans the input file, searching for a line that begins with the keyword "RandomSeed".
 * If found, the function parses the rest of the line to extract a numerical value, which is then used
 * to seed the GNU Scientific Library's (GSL) complex random number generator.
 *
 * @param f          A pointer to the input file to be parsed.
 * @param s          A pointer to a character pointer that stores the line read from the file.
 * @param length     A pointer to an integer representing the length of the line buffer.
 *
 * @warning This function has limited error handling. It does not validate the seed value's format
 *          or check if it's within the valid range for the GSL random number generator.
 *
 * @see parse_get_line
 * @see parse_exp
 * @see gsl_complex_rand_seed
 */
void parse_and_set_random_seed(FILE *f, char **s, int *length) {
    int c;
    do {
        c = parse_get_line(f, s, length);
        if (strncmp("RandomSeed", *s, 10) == 0) {
            parse_result rs;
            parse_exp(*s, &rs);
            printf("value %ld\n", lround(GSL_REAL(rs.value.c)));
            gsl_complex_rand_seed(lround(GSL_REAL(rs.value.c)));
            return;
        }
    } while (c != EOF);
}

/**
 * @brief Parse an input file.
 *
 * This function parses an input file and stores the data as a matrix-like structure in the symbol
 * table. It is capable of handling recursive nesting of files if a line begins with `include`.
 *
 * @param f          A pointer to a FILE object.
 * @param set_used   A integer indicating whether to mark symbols in the symbol table as used.
 *
**/
int parse_file_stream(FILE *f, const int set_used) {

    char *s;
    int c, length = 0;
    parse_result pc;
    const int initial_length = 40;

    // initial_length is just a starter length, not the maximum
    length = initial_length;
    s = (char *) calloc(length + 1, sizeof(*s));
    if (!s) {
        error_return(-1, "calloc failed to allocate initial length for line char");
    }

    // Parse the file once, for RandomSeed
    long start_pos = ftell(f);
    parse_and_set_random_seed(f, &s, &length);
    fseek(f, start_pos, SEEK_SET);

    // Parse the file
    do {
        c = parse_get_line(f, &s, &length);
        if (!*s) { continue; }

        // Include another file
        if (strncmp("include ", s, 8) == 0) {
            /* wipe out leading 'include' with blanks */
            memcpy(s, "       ", 7);
            str_trim(s);
            if (!disable_write) { fprintf(fout, "# including file '%s'\n", s); }
            c = parse_input(s, 0);
            if (c != 0) {
                fprintf(stderr, "Parser error: cannot open included file '%s'.\n", s);
                exit(1);
            }
        } else if (*s == block_delimiter) {
            // Parse a block
            c = parse_input_block(f, &s, &length, &pc);
        } else {
            // Parse a single line that does not start with include
            parse_exp(s, &pc);
        }

    } while (c != EOF);

    free(s);

    if (f != stdin) {
        fclose(f);
    }
    if (set_used == 1) {
        sym_mark_table_used();
    }

    return 0;
}

/**
 * @brief Parse an input file.
 *
 * This function parses an input file and stores the data as a matrix-like structure in the symbol
 * table. It is capable of handling recursive nesting of files, via the `include` key.
 *
 * @param file_in    File name.
 * @param set_used   An integer indicating whether to mark symbols in the symbol table as used.
 *
**/
int parse_input(const char *file_in, int set_used) {

    FILE *f = (strcmp(file_in, "-") == 0) ? stdin : fopen(file_in, "r");

    if (!f) {
        error_return(-1, "There was an error reading \"%s\"", file_in);
    }
    return parse_file_stream(f, set_used);
}

/**
 * @brief Parse an input string.
 *
 * This function parses an input string and stores the data as a matrix-like structure in the symbol
 * table. It is capable of handling recursive nesting of files, via the `include` key.
 *
 * @param file_contents Contents of the input file.
 * @param set_used      An integer indicating whether to mark symbols in the symbol table as used.
 *
**/
int parse_input_string(const char *file_contents, int set_used) {

    FILE *f = fmemopen((void *)file_contents, strlen(file_contents), "r");

    if (!f) {
        error_return(-1, "There was an error reading an input string");
    }
    return parse_file_stream(f, set_used);
}

/* If *_PARSE_ENV is set, then environment variables beginning with prefix are read and set,
overriding input file if defined there too. */
void parse_environment(const char* prefix)
{
  char* flag;
  parse_result pc;

  flag = (char *)calloc(strlen(prefix) + 11, sizeof(*flag));
  strcpy(flag, prefix);
  strcat(flag, "PARSE_ENV");
  
  if( getenv(flag)!=NULL ) {
    if(!disable_write)
      fprintf(fout, "# %s is set, parsing environment variables\n", flag);
    
    /* environ is an array of C strings with all the environment
       variables, the format of the string is NAME=VALUE, which
       is directly recognized by parse_exp */
    
    char **env = environ;    
    while(*env) {
      /* Only consider variables that begin with the prefix, except the PARSE_ENV flag */
      if( strncmp(flag, *env, strlen(flag)) != 0 && strncmp(prefix, *env, strlen(prefix)) == 0 ){	
	if(!disable_write)
	  fprintf(fout, "# parsed from environment: %s\n", (*env) + strlen(prefix));
	parse_exp( (*env) + strlen(prefix), &pc);
      }
      
      env++;
    }
  }

  free(flag);
}

char* get_mtxel_name(const char* block, int l, int col)
{
  char *mtxel = (char *)calloc(strlen(block) + 20, sizeof(*mtxel));
  sprintf(mtxel, "%s[%i][%i]", block, l, col);
  return mtxel;
}

/**
 * @brief Finalise the parser.
 *
 * This function frees the symbol table entries, and closes the globally-declared file handle fout.
 *
 * @param file_contents Contents of the input file.
 * @param set_used      An integer indicating whether to mark symbols in the symbol table as used.
 *
**/
void parse_end()
{
  sym_end_table();
  if(!disable_write) {
    fprintf(fout, "# Octopus parser ended\n");
    if(fout != stdout)
      fclose(fout);
  }
}

int parse_isdef(const char *name)
{
  if(getsym(name) == NULL)
    return 0;
  return 1;
}

/* these functions are defined in src/basic/varinfo_low.c */
void varinfo_getvar(const char * name_c, void **var);
void varinfo_getopt(const void **var, void **opt);
void varinfo_opt_getinfo(const void **opt, char **name, int64_t *value, char **desc);

static void check_is_numerical(const char * name, const symrec * ptr){
  if( ptr->type != S_CMPLX){
    fprintf(stderr, "Parser error: expecting a numerical value for variable '%s' and found a string.\n", name);
    exit(1);
  }
}

int64_t parse_int(const char *name, int64_t def)
{
  symrec *ptr;
  int64_t ret;
  void *var = NULL;
  void *opt = NULL;
  char *opt_name = NULL;
  int64_t value = 0;
  char *desc = NULL; // unused, only to satisfy API

  ptr = getsym(name);	
  if(ptr){
    check_is_numerical(name, ptr);
    ret = ROUND(GSL_REAL(ptr->value.c));
    if(!disable_write) {
      varinfo_getvar(name, &var);
      const void *cvar = var;
      varinfo_getopt(&cvar, &opt);
      while (opt) {
        const void *copt = opt;
        varinfo_opt_getinfo(&copt, &opt_name, &value, &desc);
        if (ret == value) {
          fprintf(fout, "%s = %s\n", name, opt_name);
          break;
        }
        varinfo_getopt(&cvar, &opt);
      }
      if (!opt) {
        fprintf(fout, "%s = %ld\n", name, ret);
      }
    }
    if(fabs(GSL_IMAG(ptr->value.c)) > 1e-10) {
       fprintf(stderr, "Parser error: complex value passed for integer variable '%s'.\n", name);
       exit(1);
    }
    if(fabs(ret - GSL_REAL(ptr->value.c)) > 1e-10) {
       fprintf(stderr, "Parser error: non-integer value passed for integer variable '%s'.\n", name);
       exit(1);
    }
  }else{
    ret = def;
    if(!disable_write) {
      varinfo_getvar(name, &var);
      const void *cvar = var;
      varinfo_getopt(&cvar, &opt);
      while (opt) {
        const void *copt = opt;
        varinfo_opt_getinfo(&copt, &opt_name, &value, &desc);
        if (ret == value) {
          fprintf(fout, "%s = %s\t\t# default\n", name, opt_name);
          break;
        }
        varinfo_getopt(&cvar, &opt);
      }
      if (!opt) {
        fprintf(fout, "%s = %ld\t\t# default\n", name, ret);
      }
    }
  }
  return ret;
}

double parse_double(const char *name, double def)
{
  symrec *ptr;
  double ret;

  ptr = getsym(name);	
  if(ptr){
    check_is_numerical(name, ptr);
    ret = GSL_REAL(ptr->value.c);
    if(!disable_write) {
      fprintf(fout, "%s = %g\n", name, ret);
    }
    if(fabs(GSL_IMAG(ptr->value.c)) > 1e-10) {
       fprintf(stderr, "Parser error: complex value passed for real variable '%s'.\n", name);
       exit(1);
    }
  }else{
    ret = def;
    if(!disable_write) {
      fprintf(fout, "%s = %g\t\t# default\n", name, ret);
    }
  }
  return ret;
}

gsl_complex parse_complex(const char *name, gsl_complex def)
{
  symrec *ptr;
  gsl_complex ret;

  ptr = getsym(name);	
  if(ptr){
    check_is_numerical(name, ptr);
    ret = ptr->value.c;
    if(!disable_write) {
      fprintf(fout, "%s = (%g, %g)\n", name, GSL_REAL(ret), GSL_IMAG(ret));
    }
  }else{
    ret = def;
    if(!disable_write) {
      fprintf(fout, "%s = (%g, %g)\t\t# default\n", name, GSL_REAL(ret), GSL_IMAG(ret));
    }
  }
  return ret;
}

char *parse_string(const char *name, const char *def)
{
  symrec *ptr;
  char *ret;
  
  ptr = getsym(name);
  if(ptr){
    if( ptr->type != S_STR){
      fprintf(stderr, "Parser error: expecting a string for variable '%s'.\n", name);
      exit(1);
    }
    ret = (char *)calloc(strlen(ptr->value.str) + 1, sizeof(*ret));
    strcpy(ret, ptr->value.str);
    if(!disable_write) {
      fprintf(fout, "%s = \"%s\"\n", name, ret);
    }
  }else{
    ret = (char *)calloc(strlen(def) + 1, sizeof(*ret));
    strcpy(ret, def);
    if(!disable_write) {
      fprintf(fout, "%s = \"%s\"\t\t# default\n", name, ret);
    }
  }
  return ret;
}

int parse_block (const char *name, sym_block **blk)
{
  symrec *ptr;

  ptr = getsym(name);
  if(ptr && ptr->type == S_BLOCK){
    *blk = ptr->value.block;
    (*blk)->name = (char *)calloc(strlen(name) + 1, sizeof(*((*blk)->name)));
    strcpy((*blk)->name, name);
    if(!disable_write) {
      fprintf(fout, "Opened block '%s'\n", name);
    }
    return 0;
  }else{
    *blk = NULL;
    return -1;
  }
}

int parse_block_end (sym_block **blk)
{
  if(!disable_write) {
     fprintf(fout, "Closed block '%s'\n", (*blk)->name);
  }
  free((*blk)->name);
  *blk = NULL;
  return 0;
}

int parse_block_n(const sym_block *blk)
{
  assert(blk != NULL);

  return blk->n;
}

int parse_block_cols(const sym_block *blk, int l)
{
  assert(blk!=NULL);
  if(l < 0 || l >= blk->n){
    fprintf(stderr, "Parser error: row %i out of range [0,%i] when parsing block '%s'.\n", l, blk->n-1, blk->name);
    exit(1);
  }
  
  return blk->lines[l].n;
}

int parse_block_int(const sym_block *blk, int l, int col, int *r)
{
  char *mtxel_name = get_mtxel_name(blk->name, l, col);
  *r = parse_int(mtxel_name, 0);
  free(mtxel_name);
  return 0;
}

int parse_block_int8(const sym_block *blk, int l, int col, int64_t *r)
{
  char *mtxel_name = get_mtxel_name(blk->name, l, col);
  *r = parse_int(mtxel_name, 0);
  free(mtxel_name);
  return 0;
}

int parse_block_double(const sym_block *blk, int l, int col, double *r)
{
  char *mtxel_name = get_mtxel_name(blk->name, l, col);
  *r = parse_double(mtxel_name, 0.0);
  free(mtxel_name);
  return 0;
}

int parse_block_complex(const sym_block *blk, int l, int col, gsl_complex *r)
{
  char *mtxel_name = get_mtxel_name(blk->name, l, col);
  gsl_complex zero;
  GSL_SET_COMPLEX(&zero, 0.0, 0.0);
  *r = parse_complex(mtxel_name, zero);
  free(mtxel_name);
  return 0;
}

int parse_block_string(const sym_block *blk, int l, int col, char **r)
{
  char *mtxel_name = get_mtxel_name(blk->name, l, col);
  *r = parse_string(mtxel_name, "");
  free(mtxel_name);
  return 0;
}

void parse_result_free(parse_result *t)
{
  if(t->type == PR_STR)
    free(t->value.s);
  t->type = PR_NONE;
}

void parse_putsym_int(const char *s, int i)
{
  symrec *rec = putsym(s, S_CMPLX);
  GSL_SET_COMPLEX(&rec->value.c, (double)i, 0);
  rec->def = 1;
  rec->used = 1;
}

void parse_putsym_double(const char *s, double d)
{
  symrec *rec =  putsym(s, S_CMPLX);
  GSL_SET_COMPLEX(&rec->value.c, d, 0);
  rec->def = 1;
  rec->used = 1;
}

void parse_putsym_complex(const char *s, gsl_complex c)
{
  symrec *rec =  putsym(s, S_CMPLX);
  rec->value.c = c;
  rec->def = 1;
  rec->used = 1;
}
