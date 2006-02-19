#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "s2.h"

#include "constants.h"
#include "version.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "free.h"               /* FREE(), DELETE() */
#include "match.h"
#include "n.h"                  /* Node */
#include "parse.h"
#include "thread_pool.h"

#include <errno.h>              /* errno */
#include <signal.h>             /* signal() */
#include <stdio.h>              /* stderr */
#include <stdlib.h>             /* exit() */

#include <iostream>             /* std::string, cout, endl, ... */

using namespace std;

/* options */
struct opts_t opts;

/* private variables */
uint exit_val = 0;                      /* exit value (diagnose library: error-reporting functions) */

/* private function declarations */
static int parse_cmd_opts(int argc, char **argv);
static int handle_option(const char* short_name);
static void WarnCB(dg_callback cbData);
static void ErrCB(dg_callback cbData);
static int f_open(const char* fname, FILE **file);
static int f_close(const char* fname, FILE **file);

/********************************************************************
 * Constants
 ********************************************************************/
option_item optionlist[] = {
  { '-', "h[#]", "help[=#]",            _("display help and exit (#: help level)") },
  { '-', "0[#]", "e0-file[=#]",		NULL },
  { '-', "1[#]", "e1-file[=#]",		NULL },
  { '-', "2[#]", "e2-file[=#]",		NULL },
  { '-', "a[#]", "ansi[=#]",            NULL },
  { '-', "b[#]", "verbose[=#]",         NULL }, /* -2: no errors; -1: no warnings; 0: normal; 1+: verbose */
  { '-', "d<p>", "dbg-file=<p>",        NULL },
  { '-', "e<p>", "eval=<p>",            NULL },
  { '-', "i[#]", "pp-indent[=#]",       NULL },
  { '-', "l<p>", "log-file=<p>",        NULL },
  { '-', "p<p>", "pp-out-file=<p>",     NULL },
  { '-', "r<p>", "err-file=<p>",        NULL },
  { '-', "s[#]", "show-defaults[=#]",   NULL }, /* pretty-print defaults */
  { '-', "T[#]", "threads[=#]",         NULL },
  { '-', "t[#]", "timeout[=#]",         NULL },
  { '-', "V", "version",                _("print version information and exit") },
  { '-', "w<p>", "warn-file=<p>",       NULL },

  { 0, NULL, NULL,                      NULL }
};

/********************************************************************
 * Functions
 ********************************************************************/
/*
 * Return program name
 */
extern const char*
PNAME(void)
{
  return PGMNAME;
} /* PNAME */

/********************************************************************
 * Private C functions
 ********************************************************************/
/*
 * Open a file.
 *
 * Returns
 *   0:  success
 *   >0: errors
 */
static int
f_open(const char* fname, FILE **file)
{
  if(fname) {
    *file = fopen(fname, "a");		/* append */
    if(!(*file))
    {
      DM_ERR(ERR_SYSTEM, _("failed to open file `%s' for writing/append: %s\n"), fname, _(strerror(errno)));
      return ERR_SYSTEM;
    }
  }

  return ERR_OK;
} /* f_open */

/*
 * Close a file.
 *
 * Returns
 *   0:  success
 *   >0: errors
 */
static int
f_close(const char* fname, FILE **file)
{
  if(!file || !*file) return ERR_OK;

  if(fname && *file && (fclose(*file) != 0)) {
    DM_ERR(ERR_SYSTEM, _("failed to close file `%s': %s\n"), fname, _(strerror(errno)));
    return ERR_SYSTEM;
  }
  *file = NULL;

  return ERR_OK;
}

#if !defined(__WIN32__)
/* Signal handler. */
static void
sig_handler(int sig)
{
  exit(exit_val);       /* calls exit_handler() */
}

/* Set signal handlers. */
static void
set_signals(void)
{
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGHUP, sig_handler);
}
#endif

/* Exit handler. */
static void
exit_handler(void)
{
  /* open output files */
  f_close(opts.e0_fname, &opts.e0_file);
  f_close(opts.e1_fname, &opts.e1_file);
  f_close(opts.e2_fname, &opts.e2_file);

  /* close output files opened by the diagnose library */
  DM_CLOSE();
}

static void
init_s2(void)
{
  /* s2 options */
  opts.help = 0;			/* 0: no help; 1: basic help; ... */
  opts.ansi = FALSE;			/* use ansi colors */
  opts.verbose = 0;			/* -2: no errors; -1: no warnings; 0: normal; 1: verbose */
  opts.pp_indent = PP_INDENT;		/* pretty-printer indentation value */
  opts.show_defaults = FALSE;		/* show default values (pretty-printer, evaluator) */
  opts.pp_fname = NULL;			/* pretty-printer output filename */
  opts.pp_file = PP_DEFAULT_OUTPUT;	/* pretty-printer output file */
  opts.log_fname = NULL;		/* log messages output filename */
  opts.dbg_fname = NULL;		/* debug messages output filename */
  opts.warn_fname = NULL;		/* warning messages output filename */
  opts.err_fname = NULL;		/* error messages output filename */
  opts.e0_fname = NULL;			/* before-execution log messages filename */
  opts.e0_file = E0_DEFAULT_OUTPUT;	/* before-execution log messages file */
  opts.e1_fname = NULL;			/* after-execution log messages filename */
  opts.e1_file = E1_DEFAULT_OUTPUT;	/* after-execution log messages file */
  opts.e2_fname = NULL;			/* after-evaluation log messages filename */
  opts.e2_file = E2_DEFAULT_OUTPUT;	/* after-evaluation log messages file */
  opts.s2_eval = S2_EVAL;		/* default evaluation threshold for branches */
  opts.s2_timeout = S2_TIMEOUT;		/* default timeout for branches in milliseconds */
  opts.tp_size = TP_THREADS_DEF;	/* number of threads to use in the thread pool */

  /* diagnose library */
  DM_PGMNAME_SET(PNAME());
  DM_ANSI_SET(opts.ansi);		/* environment variable DM_ANSI takes preference over this */
  DM_LOG_OPEN(opts.log_fname);
  DM_DBG_OPEN(opts.dbg_fname);
  DM_WARN_OPEN(opts.warn_fname);
  DM_ERR_OPEN(opts.err_fname);

  /* callback functions */
  DM_WARN_CB(WarnCB);
  DM_ERR_CB(ErrCB);

  /* open output files */
  f_open(opts.pp_fname, &opts.pp_file);
  f_open(opts.e0_fname, &opts.e0_file);
  f_open(opts.e1_fname, &opts.e1_file);
  f_open(opts.e2_fname, &opts.e2_file);
}

#if defined(DG_WARN) && defined(DG_DIAGNOSE)
static void
WarnCB(dg_callback cbData)
{
  UPDATE_MAX(exit_val, cbData.level);   /* increase the return value */
} /* WarnCB */
#endif

#if defined(DG_ERR) && defined(DG_DIAGNOSE)
static void
ErrCB(dg_callback cbData)
{
  switch(cbData.level) {
    case ERR_SYSTEM:
      UPDATE_MAX(exit_val, ERR_SYSTEM);         /* increase the return value */
      exit(exit_val);   /* better than SEGv (e.g.: phreads) */
    break;

    case DG_ERR_ASSERT: /* don't change to ERR_ASSERT! (see dg.h) */
      UPDATE_MAX(exit_val, ERR_ASSERT);         /* increase the return value */
      exit(exit_val);   /* better than SEGv */
    break;

    default:
      UPDATE_MAX(exit_val, cbData.level);       /* increase the return value */
  }
} /* ErrCB */
#endif

/*
 * Pretty-prints parsed tree to file `pp_fname' starting with node `node'.
 * If `pp_fname' is NULL, prints to PP_DEFAULT_OUTPUT.
 *
 * Returns
 *   0:  success
 *   >0: errors
 */
static int
pp_print(Node *node)
{
  int rval = ERR_OK;

  f_open(opts.pp_fname, &opts.pp_file);

  if(node) rval = node->print_tree(0, opts.pp_file, TRUE, FALSE, FALSE);

  f_close(opts.pp_fname, &opts.pp_file);

  return rval;
} /* pp_print */

/*
 * Execution/Evaluation values print of the tree.  Node which were not
 * evaluated are not printed.
 * If `e2_fname' is NULL, prints to E2_DEFAULT_OUTPUT.
 *
 * Returns
 *   0:  success
 *   >0: errors
 */
static int
e2_print(Node *node)
{
  int rval = ERR_OK;

  f_open(opts.e2_fname, &opts.e2_file);

  if(node) rval = node->print_tree(0, opts.e2_file, FALSE, TRUE, TRUE);

  f_close(opts.e2_fname, &opts.e2_file);

  return rval;
} /* e2_print */

/********************************************************************
 * Usage function
 ********************************************************************/
static int
usage(int ret_val)
{
  option_item *op;
  const char *PG = PNAME();
 
  fprintf(stderr, _("Usage: %s [-+"), PG);

  for (op = optionlist; op->short_name != 0; op++)
  {
    if(op->sw == '+') continue;
    putc(op->short_name[0], stderr);
  }

  fprintf(stderr, "] [LONG-OPT]... [FILE1 FILE2...]\n");

  fprintf(stderr, _("Example: %s eval01.s2\n"), PG);
  fprintf(stderr, _("Type `%s --help' for more information.\n"), PG);

  return ret_val;
}

/* General help */
static void
hlp_gen(const char *PG)
{
  fprintf(stderr, _("Usage: %s [OPTION]... [FILE1 FILE2 ...]\n"), PG);
}

static void
hlp_head(const char *PG)
{
  hlp_gen(PG);
  fprintf(stderr, _("Evaluate FILEs in S2 language.\n"));
  fprintf(stderr, _("With no FILE, read standard input.\n\n"));
  fprintf(stderr, _("Example: %s eval01.s2 eval02.s2\n"), PG);
  fprintf(stderr, _("         %s < eval01.s2\n"), PG);
}

static void
hlp_tail(void)
{
  fprintf(stderr, _("\nExit status is %d success, %d warning(s) and >=%d if false evaluation/trouble.\n"), ERR_OK, ERR_WARN, ERR_ERR);
}

/********************************************************************
 * Help function
 *
 * Params
 *   l: help level
 ********************************************************************/
extern void
hlp(int l)
{
  option_item *op;
  const char *PG = PNAME();

  switch(l) {
  case 0:
    hlp_head(PG);
    fprintf(stderr, "\n");
    fprintf(stderr,_("Options:\n"));
    
    for (op = optionlist; op->sw != 0; op++)
    {
      int n;
      char s[32];
      s[0] = 0;
      if (op->short_name) 
        sprintf(s, "  %c%s%s%n", op->sw, op->short_name, op->long_name? ",": "", &n);
      else n = 0;
      n = 9 - n;                                        /* 9 */
      if (n < 1) n = 1;
      fprintf(stderr, "%s%.*s", s, n, "         ");     /* 9 spaces */
      s[0] = 0;
      if (op->long_name) sprintf(s,"%c%c%s%n", op->sw, op->sw, op->long_name, &n);
      else n = 0;
      n = 23 - n;
      if (n < 1) n = 1;
      fprintf(stderr, "%s%.*s", s, n, "                           ");
      if(op->short_name)
        switch(op->short_name[0]) {
        case '0':
          /* before-execution tree output filename (<p>: path) */
          fprintf(stderr,_("before-evaluation tree output filename (%s)\n"), opts.e0_fname? opts.e0_fname : 
                         E0_DEFAULT_OUTPUT == stdout? "stdout":
                         E0_DEFAULT_OUTPUT == stderr? "stderr": "?");
        break;
          
        case '1':
          /* after-execution tree output filename (<p>: path) */
          fprintf(stderr,_("after-execution tree output filename (%s)\n"), opts.e1_fname? opts.e1_fname : 
                         E1_DEFAULT_OUTPUT == stdout? "stdout":
                         E1_DEFAULT_OUTPUT == stderr? "stderr": "?");
        break;
          
        case '2':
          /* after-evaluation tree output filename (<p>: path) */
          fprintf(stderr,_("after-evaluation tree output filename (%s)\n"), opts.e2_fname? opts.e2_fname : 
                         E2_DEFAULT_OUTPUT == stdout? "stdout":
                         E2_DEFAULT_OUTPUT == stderr? "stderr": "?");
        break;
          
        case 'a':
          fprintf(stderr,_("ANSI colors 0/1 (%s)\n"), opts.ansi ? _("on") : _("off"));
        break;
        
        case 'b':
          fprintf(stderr,_("verbose execution %d..%d (%d)\n"), VERBOSE_MIN, VERBOSE_MAX, opts.verbose);
        break;

        case 'd':
          /* debug messages output filename (<p>: path) */
          fprintf(stderr, _("debug messages output filename (%s)\n"), opts.dbg_fname? opts.dbg_fname: 
                  DS_DBG_FOUT == stdout? "stdout":
                  DS_DBG_FOUT == stderr? "stderr": "?");
        break;

        case 'e':
          fprintf(stderr,_("default evaluation threshold for branches (%d)\n"), opts.s2_eval);
        break;

        case 'i':
          fprintf(stderr,_("pretty-printer indentation value (%d)\n"), opts.pp_indent);
        break;

        case 'l':
          /* log messages output filename (<p>: path) */
          fprintf(stderr, _("log messages output filename (%s)\n"), opts.log_fname? opts.log_fname: 
                  DS_LOG_FOUT == stdout? "stdout":
                  DS_LOG_FOUT == stderr? "stderr": "?");
        break;

        case 'p':
          /* pretty-printer output filename (<p>: path) */
          fprintf(stderr,_("pretty-printer output filename (%s)\n"), opts.pp_fname? opts.pp_fname : 
                         PP_DEFAULT_OUTPUT == stdout? "stdout":
                         PP_DEFAULT_OUTPUT == stderr? "stderr": "?");
        break;
          
        case 'r':
          /* error messages output filename (<p>: path) */
          fprintf(stderr, _("error messages output filename (%s)\n"), opts.err_fname? opts.err_fname: 
                  DS_ERR_FOUT == stdout? "stdout":
                  DS_ERR_FOUT == stderr? "stderr": "?");
        break;

        case 's':
          /* show default values (pretty-printer, evaluator) ON/OFF */
          fprintf(stderr,_("show default values (%s)\n"), opts.show_defaults? "on": "off");
        break;
            
        case 'T':
          fprintf(stderr,_("threads in the thread pool %d..%d (%d)\n"), TP_THREADS_MIN, TP_THREADS_MAX, opts.tp_size);
        break;

        case 't':
          fprintf(stderr,_("default timeout for branches in milliseconds (%"PRIu64")\n"), opts.s2_timeout);
        break;

        case 'w':
          /* warning messages output filename (<p>: path) */
          fprintf(stderr, _("warning messages output filename (%s)\n"), opts.warn_fname? opts.warn_fname: 
                  DS_WARN_FOUT == stdout? "stdout":
                  DS_WARN_FOUT == stderr? "stderr": "?");
        break;

        default:
          fprintf(stderr,"%s\n", op->help_text);
        } else {
          /* we ran out of short options..., distinguish by the long one */
        }
    }

    hlp_tail();
  break;

  case 1:
  default: {
  } /* end default */
  } /* end switch */
}

/********************************************************************
 * Handle an option
 * 
 * Return:
 *   0: if match
 *   1: no match
 ********************************************************************/
extern int
handle_option(const char* short_name)
{

  switch(short_name[0])
  {
//    case 'p': opts.pretty_print = TRUE; return 0;
  }

  /* no short option found */
  return 1;
}

/********************************************************************
 * Parse command-line option (global)
 *
 * Parameters:
 *  cfg_file:  TRUE  if called from a configuration file
 *             FALSE if command-line option
 *
 * Returns:
 *  -1: parameter was found (opt doesn't start with -+)
 *   0: option was found
 *   1: option was NOT found
 *   2: an error
 ********************************************************************/
static int
parse_cmd_opt(char *opt, BOOL cfg_file)
{
  option_item *op;
  int opt_off;
  char *p_err;

  if(opt == NULL) {
    /* internal error */
    DM_ERR_ASSERT("opt == NULL\n");
    return 0;
  }

  /* Process the options */

  /* Parameters */
  if (!(*opt == '-' || *opt == '+')) 
    return -1;

  if (OPL("-0") || OPL("--e0-file"))
  { /* before-execution log messages filename */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;
      
    f_close(opts.e0_fname, &opts.e0_file);
    opts.e0_fname = opt + opt_off;

    if(strcmp(opts.e0_fname, "-") == 0)
      /* standard output */
      opts.e0_fname = NULL;

    f_open(opts.e0_fname, &opts.e0_file);
    return 0;
  }

  if (OPL("-1") || OPL("--e1-file"))
  { /* after-execution log messages filename */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;
      
    f_close(opts.e1_fname, &opts.e1_file);
    opts.e1_fname = opt + opt_off;

    if(strcmp(opts.e1_fname, "-") == 0)
      /* standard output */
      opts.e1_fname = NULL;

    f_open(opts.e1_fname, &opts.e1_file);
    return 0;
  }

  if (OPL("-2") || OPL("--e2-file"))
  { /* after-evaluation log messages filename */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;
      
    f_close(opts.e2_fname, &opts.e2_file);
    opts.e2_fname = opt + opt_off;

    if(strcmp(opts.e2_fname, "-") == 0)
      /* standard output */
      opts.e2_fname = NULL;

    f_open(opts.e2_fname, &opts.e2_file);
    return 0;
  }

  if (OPL("-a") || OPL("--ansi"))
  { /* ANSI colors */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;

    opts.ansi = strtol(opt + opt_off, &p_err, 0);
    if (p_err == opt + opt_off || *p_err) {
      /* no option value given || value contains invalid/non-digit char */
      opts.ansi = TRUE;
    }
    DM_ANSI_SET(opts.ansi);

    return 0;
  }

  if (OPL("-b") || OPL("--verbose"))
  { /* verbose/quiet execution */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;

    opts.verbose = strtol(opt + opt_off, &p_err, 0);
    if (p_err == opt + opt_off || *p_err)
      /* no option value given || value contains invalid/non-digit char */
      opts.verbose = 1;

    /* limit verbosity */
    if(opts.verbose > VERBOSE_MAX)
      opts.verbose = VERBOSE_MAX;
    else if(opts.verbose < VERBOSE_MIN)
      opts.verbose = VERBOSE_MIN;

    if(opts.verbose > 0)
      DM_LOG_SET_L((1 << opts.verbose) - 1);

    if(opts.verbose <= -1)
      /* disable warning messages */
      DM_WARN_SET_L(0);
                   
    if(opts.verbose <= -2)
      /* disable error messages */
      DM_ERR_SET_L(0);
      
    return 0;
  }

  if (OPL("-h") || OPL("-?") || OPL("--help"))
  { /* help */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;
      
    opts.help = strtol(opt + opt_off, &p_err, 0);
    if (p_err == opt + opt_off || *p_err) {
      /* no option value given || value contains invalid/non-digit char */
      opts.help = 0;
    }

//    hlp(opts.help); exit(0);
    hlp(0); /* only level 0 help so far */ exit(0);
    return 0;
  }

  if (OPL("-d") || OPL("--dbg-file"))
  { /* debug messages output filename */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;
      
    opts.dbg_fname = opt + opt_off;
    DM_DBG_OPEN(opts.dbg_fname);     /* old stream is automatically closed */

    return 0;
  }

  if (OPL("-e") || OPL("--eval"))
  { /* default evaluation threshold for branches */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;

    opts.s2_eval = strtol(opt + opt_off, &p_err, 0);
    if (p_err == opt + opt_off || *p_err)
      /* no option value given || value contains invalid/non-digit char */
      opts.s2_eval = S2_EVAL;

    return 0;
  }

  if (OPL("-i") || OPL("--pp-indent"))
  { /* pretty-printer indentation value */
    BOOL vrb_msg = FALSE;

    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;

    opts.pp_indent = strtol(opt + opt_off, &p_err, 0);

    /* limit pretty-printer indentation */
    if (opts.pp_indent > PP_INDENT_MAX) {
      opts.pp_indent = PP_INDENT_MAX;
      vrb_msg = TRUE;
    } else
    if (opts.pp_indent < PP_INDENT_MIN) {
      opts.pp_indent = PP_INDENT_MIN;
      vrb_msg = TRUE;
    }
    
    if (p_err == opt + opt_off || *p_err || 
        opts.pp_indent < PP_INDENT_MIN || opts.pp_indent > PP_INDENT_MAX) {
      /* no option value given || value contains invalid/non-digit char */
      opts.pp_indent = PP_INDENT;
      vrb_msg = TRUE;
    }

    if(vrb_msg)
      DM_DBG(DM_N(1),_("pretty-printer indentation set to %d spaces\n"), opts.pp_indent);

    return 0;
  }

  if (OPL("-l") || OPL("--log-file"))
  { /* default log filename */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;
      
    opts.log_fname = opt + opt_off;
    DM_LOG_OPEN(opts.log_fname);     /* old stream is automatically closed */

    return 0;
  }

  if (OPL("-p") || OPL("--pp-out-file"))
  { /* pretty-printer output file */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;
      
    f_close(opts.pp_fname, &opts.pp_file);
    opts.pp_fname = opt + opt_off;

    if(strcmp(opts.pp_fname, "-") == 0)
      /* standard output */
      opts.pp_fname = NULL;

    f_open(opts.pp_fname, &opts.pp_file);
    return 0;
  }

  if (OPL("-r") || OPL("--err-file"))
  { /* error messages output filename */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;
      
    opts.err_fname = opt + opt_off;
    DM_ERR_OPEN(opts.err_fname);     /* old stream is automatically closed */

    return 0;
  }

  if (OPL("-s") || OPL("--show-defaults"))
  { /* show default values (pretty-printer, evaluator) ON/OFF */
  
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;

    opts.show_defaults = strtol(opt + opt_off, &p_err, 0);
    if (p_err == opt + opt_off || *p_err) {
      /* no option value given || value contains invalid/non-digit char */
      opts.show_defaults = TRUE;
    }

    return 0;
  }

  if (OPL("-T") || OPL("--threads"))
  { /* threads in the thread pool */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;

    opts.tp_size = strtol(opt + opt_off, &p_err, 0);
    if (p_err == opt + opt_off || *p_err)
      /* no option value given || value contains invalid/non-digit char */
      opts.tp_size = TP_THREADS_DEF;

    /* limit verbosity */
    if(opts.tp_size > TP_THREADS_MAX)
      opts.tp_size = TP_THREADS_MAX;
    else if(opts.tp_size < TP_THREADS_MIN)
      opts.tp_size = TP_THREADS_MIN;

    return 0;
  }

  if (OPL("-t") || OPL("--timeout"))
  { /* default timeout for branches */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;

    opts.s2_timeout = strtol(opt + opt_off, &p_err, 0);
    if (p_err == opt + opt_off || *p_err)
      /* no option value given || value contains invalid/non-digit char */
      opts.s2_timeout = S2_TIMEOUT;

    return 0;
  }

  if (OPL("-V") || OPL("--version"))
  { /* print the version and exit */
    fprintf(stderr,_("%s version %s\n"), PNAME(), VERSION);
    exit(ERR_OK);
  }

  if (OPL("-w") || OPL("--warn-file"))
  { /* warning messages output filename */
    if(opt_off > 2 && *(opt + opt_off) == '=')
      /* long option, ignore '=' */
      opt_off++;
      
    opts.warn_fname = opt + opt_off;
    DM_WARN_OPEN(opts.warn_fname);   /* old stream is automatically closed */

    return 0;
  }

  /* default option handling */
  if (opt[1] == '-' || opt[1] == '+') {
    int ret_val = 1;
    /* try long options */
    for (op = optionlist; op->short_name; op++)
    {
      if (op->long_name && strcmp(opt+2, op->long_name) == 0)
      {
        ret_val = 0;
        handle_option(op->short_name);
        break;
      }
    }
    return ret_val;
  } else
    /* try short options */
    return handle_option(opt+1);

  return 1;
}

/********************************************************************
 * Parse command-line options
 ********************************************************************/
static int
parse_cmd_opts(int argc, char **argv)
{
  int i;

  if(argv == NULL) {
    /* internal error */
    DM_ERR_ASSERT("argv == NULL\n");
    return 0;
  }
  
  for (i = 1; i < argc; i++)
  {
    char *opt = argv[i];
    int ret_val;
  
    if((ret_val = parse_cmd_opt(opt, FALSE)) == -1)
      /* non -+ option */
      break;
    else if(ret_val == 1) {
      /* option not found */
      DM_ERR(ERR_ERR, _("unknown option %s\n"), opt);
      exit(usage(ERR_ERR));
    }
  }

  return i;
}

/********************************************************************
 * Parse command-line arguments (non -+ options)
 *
 * Returns: 
 *   ERR_OK: if success
 *   >= ERR_OK: otherwise
 ********************************************************************/
static int
s2_run(int argc, char **argv, int i)
{
  int rval = ERR_OK, lval;
  Node *root = NULL;
  BOOL tp_created = FALSE;

  if(i >= argc) {
    /* there are no further arguments do the business on stdin and exit */
    goto evaluate;
  } else {
    /* work through the remaining arguments as files */
    for (; i < argc; i++)
    {
evaluate:
      lval = parse(argv[i], &root);
      DM_DBG(DM_N(1), "parser return value=%d\n", lval);
      UPDATE_MAX(rval, lval);

      /* create thread pool */
      if(!tp_created) tp_init(opts.tp_size);
      tp_created = TRUE;
    
      /* Pretty-print S2 tree ($ENV{VAR} are evaluated) */
      if(opts.pp_fname) lval = pp_print(root);
      DM_DBG(DM_N(1), "pretty-printer return value=%d\n", lval);
      UPDATE_MAX(rval, lval);
    
      if (root) {
        Node::threads_init();
        lval = root->eval();
        Node::threads_destroy();
        DM_DBG(DM_N(1), "evaluation return value=%d\n", lval);
        UPDATE_MAX(rval, lval);
      }
    
      /* Execution/Evaluation print of the S2 tree */
      if(opts.e2_fname) lval = e2_print(root);
      DM_DBG(DM_N(1), "evaluation print return value=%d\n", lval);
      UPDATE_MAX(rval, lval);

      /* Cleanup */
      DELETE(root);
    }
  }

  /* destroy thread pool */
  if(tp_created) tp_cleanup();

  DM_DBG(DM_N(1), "s2_run return value=%d\n", rval);
  return rval;
}

#include "str.h"

#if 1
int
main(int argc, char *argv[])
{
  int i;
  uint rval;

  /* Initialisation */
  init_s2();

  /* Close all open diagnose streams at exit */
  atexit(exit_handler);         /* normal exit */
#if !defined(__WIN32__)
  set_signals();                /* deal with some signals sent to s2 */
#endif

  /* Parse command line options */
  i = parse_cmd_opts(argc, argv);

  /* Parse and evaluate S2 language file(s) */
  rval = s2_run(argc, argv, i);

  /* Update the return value (warnings/errors) */
  UPDATE_MAX(rval, exit_val);

  return (int)rval;
}
#else

/**************** Thread-pool tests *****************************/

/*
 * function tp_handle_request(): handle a single given request.
 * algorithm: prints a message stating that the given thread handled
 *            the given request.
 * input:     request pointer
 * output:    none
 */
extern void *
tp_handle_request(void *request)
{
  DM_DBG_I;
  tp_request *p_request = (tp_request *)request;

  if (p_request) {
    sleep(*((int *)p_request->data));
    DM_DBG(DM_N(3), "thread (%d): handled request '%d', timeout=%llu\n",
	   p_request->tp_tid, *((int *)p_request->data), p_request->timeout_usec);
  }
  
  RETURN(NULL);
}

int
main(int argc, char *argv[])
{
  int i;					/* loop counter */

  /* Initialisation */
  init_s2();

  /* Close all open diagnose streams at exit */
  atexit(exit_handler);         /* normal exit */
#if !defined(__WIN32__)
  set_signals();                /* deal with some signals sent to s2 */
#endif

  /* Parse command line options */
  i = parse_cmd_opts(argc, argv);

  tp_init(opts.tp_size);

#define MAX_REQUESTS	5
  int data[MAX_REQUESTS];
#if 1
  /* run a loop that generates requests */
  for (i = 0; i < MAX_REQUESTS; i++) {
    data[i] = i;
    tp_enqueue(&data[i], NULL, NULL);
  }
#endif
//  int a = 1, b = 2, c = 3;
  
//  sleep(1);
//  tp_enqueue(&a, NULL, NULL);
//  sleep(2);
//  tp_enqueue(&b, NULL, NULL);
//  sleep(2);
//  tp_enqueue(&c, NULL, NULL);

//  sleep(10);

  tp_cleanup();

  /* Parse and evaluate S2 language file(s) */
  if(0) s2_run(argc, argv, i);

  return 0;
}
#endif
