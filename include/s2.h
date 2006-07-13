#ifndef _S2_H
#define _S2_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include <stdio.h>              /* FILE */

#include "sysdep.h"	/* BOOL, STD_BUF, ... */

#define PGMNAME         "s2"
#define VERSION         "v"MK_VERSION" build: "BUILD_DATE

/* pretty-printer constants */
#define PP_INDENT       3                       /* default pretty-printer indentation value */
#define PP_INDENT_MIN   1                       /* minimum pretty-printer indentation value */
#define PP_INDENT_MAX   16                      /* maximum pretty-printer indentation value */
#define PP_DEFAULT_OUTPUT stdout                /* pretty-printer default output file */
#define E0_DEFAULT_OUTPUT stderr		/* before-execution tree default output file */
#define E1_DEFAULT_OUTPUT stderr		/* after-execution tree default output file */
#define E2_DEFAULT_OUTPUT stderr		/* after-evaluation tree default output file */

/* options defaults */
#define VERBOSE_MIN     -2                      /* minimum verbosity level */
#define VERBOSE_MAX     3                       /* maximum verbosity level */

/* structure for options and list of them */
typedef struct option_item {
  const char sw;                /* switch (-/+) */
  const char *short_name;
  const char *long_name;
  const char *help_text;        /* can be NULL */
} option_item;

/* option values */
typedef struct opts_t {
  unsigned int help;			/* 0: no help; 1: basic help; ... */
  BOOL ansi;				/* use ansi colors */
  int verbose;				/* -2: no errors; -1: no warnings; 0: normal; 1: verbose */
  int pp_indent;			/* pretty-printer indentation value */
  BOOL show_defaults;			/* show default values (pretty-printer, evaluator) */
  BOOL progress_bar;			/* show progres bar */
  char *pp_fname;			/* pretty-printer output filename */
  FILE *pp_file;			/* pretty-printer output file */
  char *log_fname;			/* log messages output filename */
  char *dbg_fname;			/* debug messages output filename */
  char *warn_fname;			/* warning messages output filename */
  char *err_fname;			/* error messages output filename */
  char *e0_fname;			/* before-execution log messages filename */
  FILE *e0_file;			/* before-execution log messages file */
  char *e1_fname;			/* after-execution log messages filename */
  FILE *e1_file;			/* after-execution log messages file */
  char *e2_fname;			/* after-evaluation log messages filename */
  FILE *e2_file;			/* after-evaluation log messages file */
  int s2_eval;				/* default evaluation threshold for branches */
  uint64_t s2_timeout;			/* default timeout for branches in milliseconds */
  int tp_size;				/* number of threads to use in the threadpool */
} opts_t;

/* extern(al) function declarations */
extern void progress(int show);

#endif /* _S2_H */
