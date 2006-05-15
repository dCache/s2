#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "match.h"

#include "free.h"
#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */
#include "string.h"
#include "ctype.h"

#include "process.h"		/* Process (interface for writing variables) */

#include <signal.h>             /* signal() */
#include <stdlib.h>             /* exit() */
#include <stdio.h>              /* stderr */

/* a stub for debug */
extern void
write_variable(const char *name, const char *value, int vlen)
{
  fprintf(stderr, "Writing variable `%s' with value `%.*s'\n", name, vlen, value);
}

/* match options */
MATCH_opt PX_mopts[] = {
  { 'h', PX_MATCH_HBSPLIT, "compare head and body of a message separately", "do not distinguish head and body of a message while matching" },
  { 'l', PX_MATCH_LINESPLIT, "line-by-line matching", "do not compare line-by-line" },

#if 0
  { '0',  0x0000U, "all PX match options disabled", "all PX match options enabled" },
  { '1', ~0x0000U, "all PX match options enabled", "all PX match options disabled" },
#endif

  { 0,   0, NULL, NULL },                       /* array terminator */
};

/* PCRE match options */
MATCH_opt PCRE_mopts[] = {
  { 'i', PCRE_CASELESS, "case-insensitive matching", "case-sensitive matching" },
  { 'm', PCRE_MULTILINE, "^ and $ match newlines within data", "^ and $ match only at the start and the end of data" },
  { 's', PCRE_DOTALL, "dot matches anything including newlines", "dot does not matche newlines" },
  { 'x', PCRE_EXTENDED, "ignore whitespace and # --> '\n'", "extended patterns turned off" },
  { 'c', PCRE_ANCHORED, "whole-line matching on", "whole-line matching off" }, /* semantic change! */
  { 'E', PCRE_DOLLAR_ENDONLY, "$ matches only at the end of the subject string", "$ matches newline at end" },
  { 'X', PCRE_EXTRA, "additional PCRE functionality turned on", "additional PCRE functionality turned off" },
  { 'B', PCRE_NOTBOL, "first character of the subject string is not the beginning of a line", "first character of the subject string is the beginning of a line" },
  { 'Z', PCRE_NOTEOL, "the end of the subject string is not the end of a line", "the end of the subject string is the end of a line" },
  { 'U', PCRE_UNGREEDY, "inverts the \"greediness\" of the quantifiers", "standard \"greediness\" of the quantifiers" },
  { 'N', PCRE_NOTEMPTY, "an empty string is not considered to be a valid match", "an empty string is not considered to be a valid match" },
#if 0   /* not in pcre 3.9 (SL 3.0.4) */
  { '8', PCRE_UTF8, "UTF-8 character matching instead of single-byte", "single-byte character matching" },
  { 'n', PCRE_NO_AUTO_CAPTURE, "cannot use \\N to reference subpatterns", "can use \\N to reference subpatterns" },
  { '?', PCRE_NO_UTF8_CHECK, "no UTF-8 validity checks", "UTF-8 validity checks" },
#if 0   /* pcre library 5.0 and above */
  { 0,   PCRE_AUTO_CALLOUT, NULL, NULL },       /* not supported */
  { 0,   PCRE_PARTIAL, NULL, NULL },            /* not supported */
#endif
#endif

#if 0
  { '0',  0x0000U, "all PCRE match options disabled (PCRE default)", "all PCRE match options enabled" },
  { '1', ~0x0000U, "all PCRE match options enabled", "all PCRE match options disabled (PCRE default)" },
#endif

  { 0,   0, NULL, NULL },                       /* array terminator */
};

#ifdef PCRE_STUDY
/*
 * Study the regular expressions, as we will be running them may times
 *
 * Returns:
 *   ERR_OK:     normal execution
 *   ERR_ERR:    error studying a regular expression
 *   ERR_ASSERT: assertion failures
 */
static int
pcre_study_i
(const pcre *compiled_pcre, pcre_extra **studied_pcre)
{
  const char *error;

  if(compiled_pcre == NULL) {
    /* assertion failed */
    DM_ERR_ASSERT(_("NULL patern\n"));
    return ERR_ASSERT;
  }
  
  *studied_pcre = pcre_study(compiled_pcre, 0, &error);
  if (error != NULL)
  {
    DM_ERR(ERR_ERR, _("error while studying compiled regex: %s\n"), _(error));
    return ERR_ERR;
  }

  return ERR_OK;
} /* pcre_study_i */
#endif

/********************************************************************
 * Compile pattern # pattern_no.
 *
 * Returns:
 *   ERR_OK:     normal execution
 *   ERR_ERR:    error in regex
 *   ERR_ASSERT: assertion failures
 *   ERR_SYSTEM: malloc failed
 ********************************************************************/
static int
pcre_compile_i
(const char *regex, pcre **compiled_pcre, uint32_t pcre_compile_opts)
{
  int errptr;
  const char *error;

  if(compiled_pcre == NULL) {
    /* assertion failed */
    DM_ERR_ASSERT(_("NULL patern\n"));
    return ERR_ASSERT;
  }
  
  *compiled_pcre = pcre_compile(regex, pcre_compile_opts, &error, &errptr, NULL);
  if (*compiled_pcre == NULL)
  {
    DM_ERR(ERR_ERR, _("error in regex at offset %d: %s\n"), errptr, _(error));
    return ERR_ERR;
  }

  return ERR_OK;
} /* pcre_compile_i */

/********************************************************************
 * Free a compiled (and possibly studied) patterns
 ********************************************************************/
static void
free_pattern(M_pcre *mp)
{
  if(mp == NULL)
    return;
    
  if(mp->compiled_pcre)
    free(mp->compiled_pcre);            /* TODO: set pointer to NULL */
//  pcre_free(mp->compiled_pcre);       /* cross-compiler doesn't like this (Win) */
#ifndef FAST_CODE
  mp->studied_pcre = NULL;
#endif

  if(mp->studied_pcre)
    free(mp->studied_pcre);
//  pcre_free(mp->studied_pcre);        /* cross-compiler doesn't like this (Win) */
#ifndef FAST_CODE
  mp->studied_pcre = NULL;
#endif
} /* free_pattern */

static inline void
pcre_get_named
(pcre *re, pcre_extra *studied_pcre, const char *received, int *ovector,
 Process *proc)
{
  int i;
  int namecount;
  int name_entry_size;
  const char *tabptr;
  const char *name_table;

  pcre_fullinfo(re,                     /* the compiled pattern */
                studied_pcre,           /* extra data if we studied the regular expression */
                PCRE_INFO_NAMECOUNT,    /* get the number of named substrings */
                &namecount);            /* where to put the answer */

  if(namecount <= 0) {
    /* no named substrings */
    return;
  }
  /* We have come named substrings */

  /* Before we can access the substrings, we must extract the table for     *
   * translating names to numbers, and the size of each entry in the table. */
  pcre_fullinfo(re,                     /* the compiled pattern */
                studied_pcre,           /* extra data if we studied the regular expression */
                PCRE_INFO_NAMETABLE,    /* address of the table */
                &name_table);           /* where to put the answer */

  pcre_fullinfo(re,                     /* the compiled pattern */
                studied_pcre,           /* extra data if we studied the regular expression */
                PCRE_INFO_NAMEENTRYSIZE,/* get the size of each entry in the table */
                &name_entry_size);      /* where to put the answer */

  /* Now we can scan the table and, for each entry, print the number, the name, *
   * and the substring itself.                                                  */
  tabptr = name_table;
  for (i = 0; i < namecount; i++)
  {
    int n = (tabptr[0] << 8) | tabptr[1];
    int vlen = ovector[2*n+1] - ovector[2*n];

#if 1
      proc->WriteVariable(tabptr + 2, received + ovector[2*n], vlen);
#else
      write_variable(tabptr + 2, received + ovector[2*n], vlen);
#endif
    
    tabptr += name_entry_size;
  }
} /* pcre_get_named */

/*
 * Set PCRE options based on PX match options.
 */
static inline void
set_pcre_compile_options(uint32_t *pcre_compile_opts, const uint32_t pcre_opt)
{
  *pcre_compile_opts = pcre_opt;
  *pcre_compile_opts &= ~PCRE_EXEC_MASK;
} /* set_pcre_compile_options */

/*
 * PCRE 5.0; Only PCRE_ANCHORED, PCRE_NOTBOL, PCRE_NOTEOL, PCRE_NOTEMPTY,
 *                PCRE_NO_UTF8_CHECK, PCRE_PARTIAL bits may be set.
 */
static inline void
mask_pcre_exec_options(uint32_t *pcre_exec_options, const uint32_t pcre_opt)
{
  *pcre_exec_options = pcre_opt;
  *pcre_exec_options &= PCRE_EXEC_MASK;
} /* mask_pcre_exec_options */

/*
 * Returns maximum match (option) value.
 *
 * ttype
 *   0: PX table
 *   1: PCRE table
 */
extern uint32_t
get_match_opt_max(int ttype)
{
  uint32_t max = 0;
  uint32_t i;
  MATCH_opt *table = ttype? PCRE_mopts: PX_mopts;
  
  /* I know, counting like this is slooow, but it doesn't matter as it is just    *
   * a pretty-printer output; maximum value be defined, but this is maintainable. */
  for(i = 0; table[i].o != 0; i++)
    max += table[i].val;
    
  return max;
} /* get_match_opt_max */

/*
 * Prints match options in file fout.
 */
extern int
print_match_opts
(FILE *fout, const match_opts match_opt) 
{
  uint32_t i;
  BOOL printed = FALSE;
  uint32_t PX_MAX = get_match_opt_max(0);
  uint32_t PCRE_MAX = get_match_opt_max(1);

  if((match_opt.px == PX_MAX) && (match_opt.pcre == PCRE_MAX)) {
    /* all options set, print 1 */
    fputc('1', fout);
    return ERR_OK;
  }
  /* only some options set (or maybe none!), show them */

  OPT_PRINT(fout, PX_mopts, match_opt.px);
  OPT_PRINT(fout, PCRE_mopts, match_opt.pcre);

  if(!printed)
    /* no options => print 0 */
    fputc('0', fout);
    
  return ERR_OK;
} /* print_match_opts */

/*
 * Returns match options in text form.
 * This function should always succeed unless memory allocation fails.
 * In that case NULL i returned.
 *
 * Please remember to deallocate the returned string when no longer needed.
 */
extern char *
get_match_opts
(const match_opts match_opt) 
{
  uint32_t i;
  uint32_t opts = 1;    /* total number of options (allocate memory accordingly) */
  BOOL printed = FALSE;
  uint32_t PX_MAX = get_match_opt_max(0);
  uint32_t PCRE_MAX = get_match_opt_max(1);
  char *str = NULL;     /* string with match options */
  char *ptr = NULL;     /* pointer to the current character in s */

  /* work out the maximum length of str */
  for(i = 0; PX_mopts[i].o != 0; i++, opts++)
    ;
  for(i = 0; PCRE_mopts[i].o != 0; i++, opts++)
    ;

  ptr = str = (char *) malloc(opts + 1);
  if (ptr == NULL)
  {
    DM_ERR(ERR_SYSTEM, _("malloc failed\n"));
    return NULL;
  }

  if((match_opt.px == PX_MAX) && (match_opt.pcre == PCRE_MAX)) {
    /* all options set, print 1 */
    *ptr++ = '1';
    goto ok;
  }
  /* only some options set (or maybe none!), show them */

  OPT_SPRINT(ptr, PX_mopts, match_opt.px);
  OPT_SPRINT(ptr, PCRE_mopts, match_opt.pcre);

  if(!printed) {
    /* no options => print 0 */
    *ptr++ = '0';
    goto ok;
  }
    
ok:
  *ptr = '\0';
  return str;
} /* get_match_opts */

/*
 * Text to match_opts (match options) conversion.
 * 
 * Returns
 *   ERR_OK: success
 *   ERR_WARN: on empty `options' string
 */
extern int
parse_match_opts(const char *options, match_opts *match)
{
  int c;
  int i;
  int opt_ptr;
  BOOL invert = FALSE;
  int rval = ERR_OK;

  if(options == NULL || *options == '\0') {
    DM_WARN(ERR_WARN, _("empty match options string; setting defaults\n"));
    match->px = 0;
    match->pcre = 0;
    
    return ERR_WARN;
  }

  /* get the actual `match' options */
  opt_ptr = 0;
loop:
  while(TRUE) {
    c = options[opt_ptr++];
    if(isspace(c) || c == '\0')
      /* enough, we have a separator (whitespace) */
      break;

    if(c == '-') {
      /* the meaning of the following options shall be inverted */
      invert = TRUE;
      goto loop;
    }
    if(c == '0') {
      if(invert)
        /* all options enabled */
        goto enable_all;
      else
        /* all options disabled */
        goto disable_all;
    }
    if(c == '1') {
      if(invert) {
disable_all:
        /* all options disabled */
        match->px = 0;
        match->pcre = 0;
      } else {
enable_all:
        /* all options enabled */
        match->px = get_match_opt_max(0);
        match->pcre = get_match_opt_max(1);
      }

      goto loop;
    }
    /* we have a match option, see if it is a legal one */

    OPT_SET(PX_mopts, match->px, c);
    OPT_SET(PCRE_mopts, match->pcre, c);
      
    /* not a PX or PCRE matching option => report this */
    DM_WARN(ERR_WARN, _("illegal match option `%c' a offset %d\n"), c, opt_ptr - 1);
    rval = ERR_WARN;
  }

  return rval;
} /* parse_match_opts */

/*
 * Returns
 *   TRUE:  messages `pattern' and `subject' match with match 
 *          level specified in `match'
 *   FALSE: otherwise
 */
extern BOOL
pcre_matches(const char *pattern, const char *subject, const uint32_t pcre_opt,
             Process *proc)
{
  BOOL match;
  M_pcre mp;

  int ovector[PCRE_OVECCOUNT];
  int subject_len;
  int rc;
  uint32_t pcre_compile_opts = 0;       /* pcre compile options */
  uint32_t pcre_exec_opts = 0;          /* pcre exec options */
//#define SHOW_SUBSTRINGS
#ifdef SHOW_SUBSTRINGS
  int i;
#endif

 {char *opt_str = NULL;
  match_opts match_opt; match_opt.px = 0; match_opt.pcre = pcre_opt;
  DM_LOG(DM_N(4), "match options=%s\n", opt_str = get_match_opts(match_opt));
  if(opt_str) FREE(opt_str);
  DM_LOG(DM_N(1), "pattern=\n|%s|\nreceived=\n|%s|\n", pattern, subject);}

  if(pattern == NULL || subject == NULL) {
    match = !(pcre_opt & PCRE_NOTEMPTY);

    if(match)
      DM_LOG(DM_N(0), "Match\n");
    else
      DM_LOG(DM_N(0), "Match miss\n");

    return match;
  }
  subject_len=strlen(subject);
  
  /* initialisation of the pcre structure */
  memset(&mp, 0, sizeof(mp));

  /* set PCRE compile options */
  set_pcre_compile_options(&pcre_compile_opts, pcre_opt);

  pcre_compile_i(pattern, &mp.compiled_pcre, pcre_compile_opts);
  
#ifdef PCRE_STUDY
  pcre_study_i(mp.compiled_pcre, &mp.studied_pcre);
#endif

  /* set PCRE compile options */
  mask_pcre_exec_options(&pcre_exec_opts, pcre_opt);
  DM_LOG(DM_N(4), "pcre_exec_opts=0x%02x\n", pcre_exec_opts);
  
  rc = pcre_exec(
    mp.compiled_pcre,   /* the compiled pattern */
    mp.studied_pcre,    /* no extra data - we didn't study the pattern */
    subject,            /* the subject string */
    subject_len,        /* the length of the subject */
    0,                  /* start at offset 0 in the subject */
    pcre_exec_opts,     /* pcre_exec options */
    ovector,            /* output vector for substring information */
    PCRE_OVECCOUNT);    /* number of elements in the output vector */

  if(rc < 0) {
    switch(rc) {
      case PCRE_ERROR_NOMATCH: break;

      /* Handle other special cases if you like */
      default: DM_ERR(ERR_ERR, _("matching error %d\n"), rc); break;
    }
    /* no match or error */
    goto not_found;
  }
  /**************************************************************************
   * We have found the first match within the subject string.               *
   * If the output vector wasn't big enough, set its size to the maximum.   *
   * Then output any substrings that were captured.                         *
   **************************************************************************/

  DM_LOG(DM_N(4),"ovector[0]=%d\n", ovector[0]);
  DM_LOG(DM_N(4),"ovector[1]=%d\n", ovector[1]);

  if((pcre_exec_opts & PCRE_ANCHORED) && (ovector[1] != subject_len)) {
    /* PCRE_ANCHORED: The pattern is forced to be "anchored", that is,       *
     *                it is constrained to match only at the  first matching *
     *                point in the string that is being searched             *
     *                (the "received string").                               *
     *                We need to check whetherline lengths are equal!        */
    goto not_found;
  }
  
  if(rc == 0) {
    /* The output vector wasn't big enough */
    rc = PCRE_OVECCOUNT/3;
    DM_ERR(ERR_ERR, _("number of captured substrings is limited to %d\n"), rc - 1);
  }

#ifdef SHOW_SUBSTRINGS
  /* Show substrings stored in the output vector by number. */
  for(i = 0; i < rc; i++)
  {
    const char *substring_start = subject + ovector[2*i];
    int substring_length = ovector[2*i+1] - ovector[2*i];
    printf("%2d: %.*s\n", i, substring_length, substring_start);
  }
#endif
  
  /* See if there are any named substrings, and if so, show them by name.      *
   * First we have to extract the count of named parentheses from the pattern. */
  pcre_get_named(mp.compiled_pcre, mp.studied_pcre, subject, ovector, proc);

  DM_LOG(DM_N(0), "Match\n");

  free_pattern(&mp);
  return TRUE;

not_found:
  DM_LOG(DM_N(0), "Match miss\n");

  free_pattern(&mp);
  return FALSE;
} /* pcre_matches */

/*
 * Line-by-line comparison of pattern against the subject.
 * 
 * Returns
 *   TRUE:  messages `pattern' and `subject' match with match 
 *          level specified in `match'
 *   FALSE: otherwise
 */
extern BOOL
pcre_match_linesplit(char *pattern, const char *subject, const match_opts match_opt,
                     Process *proc)
{ 
  BOOL match;
  char *pattern_ptr = NULL;     /* pattern offset */
  char *newline = NULL;
  char null_char;


  char *opt_str = NULL;
  DM_LOG(DM_N(1), "match options=%s\n", opt_str = get_match_opts(match_opt));
  if(opt_str) FREE(opt_str);
  DM_LOG(DM_N(1), "pattern=\n|%s|\nreceived=\n|%s|\n", pattern, subject);

  if(pattern == NULL || subject == NULL) {
    match = !(match_opt.pcre & PCRE_NOTEMPTY);

    if(match)
      DM_LOG(DM_N(0), "Match\n");
    else
      DM_LOG(DM_N(0), "Match miss\n");

    return match;
  }

  if(!(match_opt.px & PX_MATCH_LINESPLIT))
    /* linesplit option not set => perform a simple PCRE match */
    return pcre_matches((const char *)pattern, subject, match_opt.pcre, proc);

  DM_WARN(ERR_WARN, _("not fully implemented (no instantiation of variables and evaluation of expressions)\n"));
  
  do {
    newline = strchr((const char *)pattern, '\n');
    if(newline) {
      pattern_ptr = newline + 1;
      null_char = *pattern_ptr;
      *pattern_ptr = '\0';

      if(!pcre_matches((const char *)pattern, subject, match_opt.pcre, proc)) {
        *pattern_ptr = null_char;
        return FALSE;
      }

      /* we have a match, try the following line */
      pattern = pattern_ptr;
      *pattern_ptr = null_char;
    } else {
      /* we didn't find the newline character => the last match */
      if(!pcre_matches((const char *)pattern, subject, match_opt.pcre, proc)) {
        return FALSE;
      }
    }
    
  } while(newline);

  return TRUE;
} /* pcre_match_linesplit */

/*
 * Separate header/body comparison of pattern against the subject.
 * 
 * Returns
 *   TRUE:  messages `pattern' and `subject' match with match 
 *          level specified in `match'
 *   FALSE: otherwise
 */
extern BOOL
pcre_match_hbsplit(char *pattern, char *subject, const match_opts match_opt,
                   Process *proc)
{
  BOOL match;
  char *ptr;
  char *ptr_nl = NULL;
  char *pattern_head_end = NULL;
  char *subject_head_end = NULL;
  char *pattern_body_start = NULL;
  char *subject_body_start = NULL;
  char pattern_null_char;
  char subject_null_char;

  char *opt_str = NULL;
  DM_LOG(DM_N(1), "match options=%s\n", opt_str = get_match_opts(match_opt));
  if(opt_str) FREE(opt_str);
  DM_LOG(DM_N(1), "pattern=\n|%s|\nreceived=\n|%s|\n", pattern, subject);

  if(pattern == NULL || subject == NULL) {
    match = !(match_opt.pcre & PCRE_NOTEMPTY);

    if(match)
      DM_LOG(DM_N(0), "Match\n");
    else
      DM_LOG(DM_N(0), "Match miss\n");

    return match;
  }

#define FIND_BODY(string, head_end, body_start)\
  ptr = string;\
  while(*ptr) {\
    if(*ptr == '\n' && ptr_nl != NULL) {\
      if((ptr - ptr_nl) == 1) {\
        /* we have found message body at ptr+1 */\
        head_end = ptr;\
        body_start = ptr + 1;\
        break;\
      } else if((ptr - ptr_nl) == 2 && *(ptr - 1) == '\r') {\
        /* we have found message body at ptr+1 */\
        head_end = ptr - 1;\
        body_start = ptr + 1;\
        break;\
      }\
    }\
    if(*ptr == '\n')\
      ptr_nl = ptr;\
\
    ptr++;\
  }\
\
  if(!head_end)\
    /* maybe one wants to see some warning message here? */\
    goto compare_not_split;

  /* find start of the pattern body */
  FIND_BODY(pattern, pattern_head_end, pattern_body_start);
  /* find start of the subject body */
  FIND_BODY(subject, subject_head_end, subject_body_start);
#undef FIND_BODY
  
  /* we have found pattern and subject bodies => compare headers and bodies separately */
  pattern_null_char = *pattern_head_end;
  *pattern_head_end = '\0';
  subject_null_char = *subject_head_end;
  *pattern_head_end = '\0';
  
  DM_LOG(DM_N(1), "pattern=\n|%s|\nsubject=\n|%s|\n", pattern, subject);
  if(!pcre_match_linesplit(pattern, (const char *)subject, match_opt, proc)) {
    DM_LOG(DM_N(0), "headers don't match\n");
    return FALSE;
  }
  DM_LOG(DM_N(0), "headers match\n");

  /* restore the original values */
  *pattern_head_end = pattern_null_char;
  *subject_head_end = subject_null_char;
  /* set the pointers to start comparison of bodies */
  pattern = pattern_body_start;
  subject = subject_body_start;

compare_not_split:
  DM_LOG(DM_N(1), "matching bodies\n");
  DM_LOG(DM_N(1), "pattern=\n|%s|\nsubject=\n|%s|\n", pattern, subject);

  return pcre_match_linesplit(pattern, (const char *)subject, match_opt, proc);
} /* pcre_match_hbsplit */
