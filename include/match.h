#ifndef _MATCH_H
#define _MATCH_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#ifdef HAVE_PCRE
#include <pcre.h>
#endif

#include "sysdep.h"             /* BOOL */

#include "constants.h"
#include "i18.h"

/* PCRE constants */
#define PCRE_NAMED              32              /* maximum number of named substrings supported (on one line) */
#define PCRE_OVECCOUNT          3+3*PCRE_NAMED  /* should be a multiple of 3 */
#define MAX_MATCH_LINES         512             /* maximum number of lines of a textual message to match */

#define PCRE_EXEC_MASK  (PCRE_ANCHORED | PCRE_NOTBOL | PCRE_NOTEOL | PCRE_NOTEMPTY | PCRE_NO_UTF8_CHECK)

/* macros */
#define OPT_SET(table, store, c) /* `i' needs to be locally defined */\
    for(i = 0; table[i].o != 0; i++) {\
      /* go through match options */\
      if(table[i].o == (c)) {\
        if(invert)\
          store &= ~table[i].val;\
        else\
          store |= table[i].val;\
\
        goto loop;\
      }\
    }

#define OPT_PRINT(fout, table, mval) /* `i', `printed' needs to be locally defined */\
  for(i = 0; table[i].o != 0; i++) {\
    /* DM_DBG(DM_N(0), "table[i].val=%d\n", table[i].val); */\
    if((table[i].val & mval)) {\
      /* DM_DBG(DM_N(0), "match (%c)\n", table[i].o); */\
      fputc(table[i].o, fout);\
      printed = TRUE;\
    }\
  }

#define OPT_SPRINT(ptr, table, mval) /* `i', `printed' needs to be locally defined */\
  for(i = 0; table[i].o != 0; i++) {\
    /* DM_DBG(DM_N(0), "table[i].val=%d\n", table[i].val); */\
    if((table[i].val & mval)) {\
      /* DM_DBG(DM_N(0), "match (%c)\n", table[i].o); */\
      *ptr++ = table[i].o;\
      printed = TRUE;\
    }\
  }

typedef struct MATCH_opt {
  uint8_t o;            /* short option name */
  uint32_t val;         /* constant value of the option */
  const char *st;       /* "true" string (if the option is set) */
  const char *sf;       /* "false" string (if the option is not set) */
} MATCH_opt;

typedef struct match_opts {
  uint32_t px;                          /* PX match options */
  uint32_t pcre;                        /* PCRE match options */

  #define PX_MATCH_HBSPLIT      (0x01U << 0)
  #define PX_MATCH_LINESPLIT    (0x01U << 1)

  #define PX_MATCH_DEFAULT      (0x01U << 31)           /* ignore bits 0--31 and set default matching */

  #define PX_MATCH_BIN          (0x00U)                 /* default BIN match value */
  #define PX_MATCH_RTP          (0x00U)                 /* default RTP match value */
  #define PX_MATCH_STP          PX_MATCH_RTP            /* default STP match value */
  #define PX_MATCH_TXT          (0x00)                  /* default TXT match value */
  #define PX_MATCH_SIP          PX_MATCH_HBSPLIT|PX_MATCH_LINESPLIT
                                                        /* default SIP match value */

  #define PCRE_MATCH_BIN        PCRE_ANCHORED|PCRE_CASELESS|PCRE_EXTENDED
                                                        /* default PCRE BIN match value */
  #define PCRE_MATCH_RTP        PCRE_MATCH_BIN          /* default PCRE RTP match value */
  #define PCRE_MATCH_STP        PCRE_MATCH_BIN          /* default PCRE STP match value */
  #define PCRE_MATCH_TXT        PCRE_MULTILINE          /* default PCRE TXT match value */
  #define PCRE_MATCH_SIP        PCRE_MULTILINE          /* default PCRE TXT match value */
} match_opts;

typedef struct M_pcre {
  pcre *compiled_pcre;          /* compiled regular expression */
  pcre_extra *studied_pcre;     /* compiled and studied regular expression */
} M_pcre;

/* extern(al) function declarations (provided for other modules) */
extern uint32_t get_match_opt_max(int ttype);
extern int parse_match_opts(const char *options, match_opts *match_opt);
extern int print_match_opts(FILE *fout, const match_opts match_opt);
extern char *get_match_opts(const match_opts match_opt);
extern BOOL pcre_matches(const char *expected, const char *received, const uint32_t pcre_opt,
                         void WriteVariable(const char *, const char *, int));
extern BOOL pcre_match_linesplit(char *pattern, const char *subject, const match_opts match_opt,
                               void WriteVariable(const char *, const char *, int));
extern BOOL pcre_match_hbsplit(char *pattern, char *subject, const match_opts match_opt,
                               void WriteVariable(const char *, const char *, int));

/* extern(al) variable declarations (provided for other modules) */
extern MATCH_opt PX_mopts[];    /* PX match options, defined in match.cpp */
extern MATCH_opt PCRE_mopts[];  /* PCRE match options, defined in match.cpp */

#endif /* _MATCH_H */
