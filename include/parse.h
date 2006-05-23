#ifndef _PARSE_H
#define _PARSE_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include "n.h"          /* Node */
#include "sysdep.h"     /* BOOL */

/* parser constants */
#define CH_COMMENT      ';'                     /* script comment character */
#define CH_PREPROC      '#'                     /* script preprocessor character */
#define CH_EOL          -1                      /* EOL character (parsing purposes) */
#define CH_INV_UGC      0                       /* invalid unget() character */
#define MAX_ID          63                      /* maximum length of an identifier */
#define MAX_IFS         63                      /* maximum number of nested #if directives */
#define MAX_INCS	63			/* maximum number of files to include */

/* simple macros */
#define OPT(s)          (((s) == NULL)? FALSE: (strcmp(opt, (s)) == 0))
#define OPN(s, n)       (((s) == NULL)? FALSE: (strncmp(opt, (s), (n)) == 0))
#define OPI(s)          (((s) == NULL)? FALSE: (strcasecmp(opt, (s)) == 0))
#define OPL(s)          (((s) == NULL)? FALSE: (strncmp(opt, (s), (opt_off = strlen(s))) == 0))

#define UPDATE_MAX(m1, m2)      ((m1) < (m2))? (m1) = (m2): (m1) = (m1)
#define UPDATE_MAXF(m1, m2, f)  if((m1) < (m2 = f)) (m1) = (m2)
#define STRDUP(target,source)\
  if(((target) = strdup(source)) == (char *)NULL) {\
    DM_ERR(ERR_SYSTEM, _("strdup failed\n"));\
    return ERR_SYSTEM;\
  }

#define EAT(nonterminal, ...) \
  if((rval = (nonterminal(__VA_ARGS__)))) {\
    /* rval has to be locally defined */\
    return rval;        /* not good enough => throw up */\
  }

#define EATW(nonterminal, ...) \
  if((rval = (nonterminal(__VA_ARGS__))))\
    /* rval has be locally defined */\
    return rval;        /* not good enough => throw up */\
  WS();         /* eat up remaining whitespace */

#define WS_COMMENT\
  do { WS(); if(line[col] == CH_COMMENT) { return ERR_OK; }; } while(0)

#define NEW_STR(target,...)\
  if(((target) = new std::string(__VA_ARGS__)) == (std::string *)NULL) {\
    DM_ERR(ERR_SYSTEM, _("new failed\n"));\
    exit(ERR_SYSTEM);\
  }

#define PARSE(f,t,s)\
  if((f)(t) != ERR_OK) {\
    DM_PERR("missing "s);               /* TODO: I18! */\
    return ERR_ERR;\
  }

#define DQ_PARAM(f,t,v,s)\
 {PARSE(f,t,s)\
  if((v) != NULL) {\
    DELETE(v);\
    DM_PWARN("redefinition of "s);              /* TODO: I18! */\
  }\
  NEW_STR(v, t.c_str())}

#if FAST_CODE
#define DQ_PARAMv(f,t,v,s)\
 {PARSE(f,t,s)\
  DM_DBG(DM_N(1), ""#v ".push_back(%s)\n", t.c_str());\
  v.push_back(new std::string(t));}
#else
#define DQ_PARAMv(f,t,v,s)\
 {PARSE(f,t,s)\
  DM_DBG(DM_N(1), ""#v ".push_back(%s)\n", t.c_str());\
  v.push_back(new std::string(t));\
  if(!(void *)v.back()) {\
    DM_ERR(ERR_SYSTEM, _("new failed\n"));\
    exit(ERR_SYSTEM);\
  }}
#endif

#define EQ_PARAM(t,v,s)\
 {EAT(WEQW);            /* eat whitespace* '=' whitespace* */\
  DQ_PARAM(dq_param,t,v,s" value\n");}          /* TODO: I18! */

#define EQ_PARAM_ENV(t,v,s)\
 {EAT(WEQW);            /* eat whitespace* '=' whitespace* */\
  DQ_PARAM(dq_param_env,t,v,s" value\n");}      /* TODO: I18! */

#define IND_PARAM(t,v,s)\
 {EAT(LIND);            /* eat whitespace* '[' whitespace* */\
  DQ_PARAMv(ind_param,t,v,s" value\n");}        /* TODO: I18! */

#define INT(f,s,i)      /* start_col must be locally defined */\
  if((f) != ERR_OK) {\
    DM_PERR(s" is not an"i" integer constant\n");       /* TODO: I18! */\
    return ERR_ERR;\
  }

#define INT8(s,v)       INT(parse_int8(&(v),TRUE),s,"")
#define INT16(s,v)      INT(parse_int16(&(v),TRUE),s,"")
#define INT32(s,v)      INT(parse_int32(&(v),TRUE),s,"")
#define INT64(s,v)      INT(parse_int64(&(v),TRUE),s,"")
#define UINT8(s,v)      INT(parse_uint8(&(v),TRUE),s," unsigned")
#define UINT16(s,v)     INT(parse_uint16(&(v),TRUE),s," unsigned")
#define UINT32(s,v)     INT(parse_uint32(&(v),TRUE),s," unsigned")
#define UINT64(s,v)     INT(parse_uint64(&(v),TRUE),s," unsigned")

/* Parser OPL */
#define POPL(s)\
  /* opt, end have to be locally defined */\
  ((!s || !opt || !end)? FALSE: (end > opt && (strlen(s) == (size_t)(end - opt)) && strncmp(opt, (s), strlen(s)) == 0)? TRUE: FALSE)

#define POPL_ERR\
  /* opt, end have to be locally defined */\
 {if(*opt) \
    DM_PERR(_("unknown parameter `%.*s'\n"), end - opt, opt);\
  else \
    DM_PERR(_("unknown parameter\n"));\
\
  return ERR_ERR;}

#define POPL_EQ_PARAM(s,r) \
  if (POPL(s)) {\
    EQ_PARAM(_val, r, s);\
    DM_DBG(DM_N(1), "%s=|%s|\n", s, r->c_str());\
  }

#define POPL_EQ_PARAM_ENV(s,r) \
  if (POPL(s)) {\
    EQ_PARAM_ENV(_val, r, s);\
    DM_DBG(DM_N(1), "%s=|%s|\n", s, r->c_str());\
  }

/* vector */
#define POPL_EQ_PARAM_ENVv(s,r) \
  if (POPL(s)) {\
    EQ_PARAM_ENVv(_val,r,s);\
  }

#define POPL_INT32(s,r) \
  if (POPL(s)) {\
    EAT(WEQW);\
    INT32(s,r);\
  }

#define POPL_INT64(s,r) \
  if (POPL(s)) {\
    EAT(WEQW);\
    INT64(s,r);\
  }

#define POPL_UINT32(s,r) \
  if (POPL(s)) {\
    EAT(WEQW);\
    UINT32(s, r);\
  }

#define POPL_UINT64(s,r) \
  if (POPL(s)) {\
    EAT(WEQW);\
    UINT64(s, r);\
  }

#define POPL_ARRAY(s,r)\
  if(POPL(s)) { /* s[][]... */\
    EAT(LIND); col--;   /* nicer diagnostics */\
    while(col < llen && line[col] == '[') {\
      _val.clear();\
      IND_PARAM(_val,r,s);\
      WS();\
    }\
  }

/* Extending diagnose library macros */
#define dgPERR(t, level, ...)\
  do {\
    dgMsg##t(DG_MSG_S, DT_ERR, level);\
    dgERR_Msg(DG_MSG_B, level, "perror: " dgLOC##t);\
    if(preproc.INC.p > 0 && preproc.INC.fullname[preproc.INC.p]) {\
      /* nested includes */\
      dgERR_Msg(DG_MSG_B, level, "[%u/%d]: %s ", preproc.INC.row[0], preproc.INC.offset[preproc.INC.p], preproc.INC.name[preproc.INC.p]);\
    }\
    dgERR_Msg(DG_MSG_B, level, "[%u/%d]: ", row, col+1);\
    dgERR_Msg(DG_MSG_T, level, __VA_ARGS__);} while(0)
#define dgPWARN(t, level, ...)\
  do {\
    dgMsg##t(DG_MSG_S, DT_WARN, level);\
    dgWARN_Msg(DG_MSG_B, level, "pwarning: " dgLOC##t);\
    if(preproc.INC.p > 0 && preproc.INC.fullname[preproc.INC.p]) {\
      /* nested includes */\
      dgWARN_Msg(DG_MSG_B, level, "[%u/%d]: %s ", preproc.INC.row[0], preproc.INC.offset[preproc.INC.p], preproc.INC.name[preproc.INC.p]);\
    }\
    dgWARN_Msg(DG_MSG_B, level, "[%u/%d]: ", row, col+1);\
    dgWARN_Msg(DG_MSG_T, level, __VA_ARGS__);} while(0)

#define DM_PERR(...)    DM_BLOCK(ERR, ERR_ERR, dgPERR(P, ERR_ERR, __VA_ARGS__))
#define DM_PWARN(...)   DM_BLOCK(WARN, ERR_WARN, dgPWARN(P, ERR_WARN, __VA_ARGS__))

/* typedefs */
typedef std::map<std::string, struct nDefun *> TFunctions;

/* extern(al) functions (defined by other modules) */
extern const char* PNAME(void);

/* extern(al) function declarations */
extern int parse(const char *filename, Node **root);

/* global variables */
extern TFunctions gl_fun_tab;	/* table of (global) functions */

#endif /* _PARSE_H */
