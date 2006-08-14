#ifndef _PARSE_H
#define _PARSE_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include "n.h"          /* Node */
#include "sysdep.h"     /* BOOL */

/* parser constants */
#define CH_COMMENT	'%'			/* script comment character */
#define CH_PREPROC	'#'			/* script preprocessor character */
#define CH_EOL		-1			/* EOL character (parsing purposes) */
#define CH_INV_UGC	0			/* invalid unget() character */
#define MAX_ID		63			/* maximum length of an identifier */
#define MAX_IFS		63			/* maximum number of nested #if directives */
#define MAX_INCS	63			/* maximum number of files to include */

/* macros */
#define STRDUP(target,source)\
  do {\
    if(((target) = strdup(source)) == (char *)NULL) {\
      DM_ERR(ERR_SYSTEM, _("strdup failed\n"));\
      return ERR_SYSTEM;\
    }\
  } while(0)

#define NEW_STR(target,...)\
  do {\
    if(((target) = new std::string(__VA_ARGS__)) == (std::string *)NULL) {\
      DM_ERR(ERR_SYSTEM, _("new failed\n"));\
      exit(ERR_SYSTEM);\
    }\
  } while(0)

#define EAT(nonterminal, ...) \
  if((rval = (nonterminal(__VA_ARGS__)))) {\
    /* rval has to be locally defined */\
    return rval;        /* not good enough => throw up */\
  }

#define EAT_WS(nonterminal, ...) \
  if((rval = (nonterminal(__VA_ARGS__))))\
    /* rval has be locally defined */\
    return rval;	/* not good enough => throw up */\
  WS();			/* eat up remaining whitespace */

#define WS_COMMENT\
  do { WS(); if(line[col] == CH_COMMENT) { return ERR_OK; }; } while(0)

#define PARSE(f,t,s)\
  if((f)(t) != ERR_OK) {\
    DM_ERR_P("missing "s);			/* TODO: I18! */\
    return ERR_ERR;\
  }

#define P_DQ_PARAM(f,t,v,s)\
 {PARSE(f,t,s)\
  if((v) != NULL) {\
    DELETE(v);\
    DM_WARN_P("redefinition of "s);		/* TODO: I18! */\
  }\
  NEW_STR(v, t.c_str());}

#define P_DQ_PARAMa(f,t,v,s)\
 {PARSE(f,t,s)\
  if((v) != NULL) {\
    v->append(' ' + t);\
  }\
  NEW_STR(v, t.c_str());}

#if FAST_CODE
#define P_DQ_PARAMv(f,t,v,s)\
 {PARSE(f,t,s)\
  DM_DBG(DM_N(1), ""#v ".push_back(%s)\n", t.c_str());\
  v.push_back(new std::string(t));}
#else
#define P_DQ_PARAMv(f,t,v,s)\
 {PARSE(f,t,s)\
  DM_DBG(DM_N(1), ""#v ".push_back(%s)\n", t.c_str());\
  v.push_back(new std::string(t));\
  if(!(void *)v.back()) {\
    DM_ERR(ERR_SYSTEM, _("new failed\n"));\
    exit(ERR_SYSTEM);\
  }}
#endif

#define P_EQ_PARAM(t,v,s)\
 {EAT(WEQW);            /* eat whitespace* '=' whitespace* */\
  P_DQ_PARAM(dq_param,t,v,s" value\n");}		/* TODO: I18! */

#define P_EQ_PARAM_ENV(t,v,s)\
 {EAT(WEQW);            /* eat whitespace* '=' whitespace* */\
  P_DQ_PARAM(dq_param_env,t,v,s" value\n");}		/* TODO: I18! */

#define P_IND_PARAM(t,v,s)\
 {P_DQ_PARAMv(dq_param_ind,t,v,s" value\n");}		/* TODO: I18! */

#define P_INT(f,s,i)      /* start_col must be locally defined */\
  do {\
    if((f) != ERR_OK) {\
      DM_ERR_P(s" is not an"i" integer constant\n");	/* TODO: I18! */\
      return ERR_ERR;\
    }\
  } while(0)

#define P_INT8(s,v)	P_INT(parse_int8(&(v),TRUE),s,"")
#define P_INT16(s,v)	P_INT(parse_int16(&(v),TRUE),s,"")
#define P_INT32(s,v)	P_INT(parse_int32(&(v),TRUE),s,"")
#define P_INT64(s,v)	P_INT(parse_int64(&(v),TRUE),s,"")
#define P_UINT8(s,v)	P_INT(parse_uint8(&(v),TRUE),s," unsigned")
#define P_UINT16(s,v)	P_INT(parse_uint16(&(v),TRUE),s," unsigned")
#define P_UINT32(s,v)	P_INT(parse_uint32(&(v),TRUE),s," unsigned")
#define P_UINT64(s,v)	P_INT(parse_uint64(&(v),TRUE),s," unsigned")

/* Parser OPL */
#define P_OPL(s)\
  /* opt, end have to be locally defined */\
  ((!s || !opt || !end)? FALSE: (end > opt && (strlen(s) == (size_t)(end - opt)) && strncmp(opt, (s), strlen(s)) == 0)? TRUE: FALSE)

#define P_OPL_ERR\
  /* opt, end have to be locally defined */\
 {if(*opt) \
    DM_ERR_P(_("unknown parameter `%.*s'\n"), end - opt, opt);\
  else \
    DM_ERR_P(_("unknown parameter\n"));\
\
  return ERR_ERR;}

#define P_OPL_EQ_PARAM(s,r) \
  if(P_OPL(s)) {\
    P_EQ_PARAM(_val, r, s);\
    DM_DBG(DM_N(1), "%s=|%s|\n", s, r->c_str());\
  }

#define P_OPL_EQ_PARAM_ENV(s,r) \
  if(P_OPL(s)) {\
    P_EQ_PARAM_ENV(_val, r, s);\
    DM_DBG(DM_N(1), "%s=|%s|\n", s, r->c_str());\
  }

/* vector */
#define P_OPL_EQ_PARAM_ENVv(s,r) \
  if(P_OPL(s)) {\
    P_EQ_PARAM_ENVv(_val,r,s);\
  }

#define P_OPL_INT32(s,r) \
  if(P_OPL(s)) {\
    EAT(WEQW);\
    P_INT32(s,r);\
  }

#define P_OPL_INT64(s,r) \
  if(P_OPL(s)) {\
    EAT(WEQW);\
    P_INT64(s,r);\
  }

#define P_OPL_UINT32(s,r) \
  if(P_OPL(s)) {\
    EAT(WEQW);\
    P_UINT32(s, r);\
  }

#define P_OPL_UINT64(s,r) \
  if(P_OPL(s)) {\
    EAT(WEQW);\
    P_UINT64(s, r);\
  }

#define P_OPL_ARRAY(s,r)\
  if(P_OPL(s)) { /* s[dq_param...] */\
    EAT(LIND);\
    while(col < llen && line[col] != ']') {\
      _val.clear();\
      P_IND_PARAM(_val,r,s);\
      WS();\
    }\
    EAT(RIND);\
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
    dgERR_Msg(DG_MSG_T, level, __VA_ARGS__);\
  } while(0)
#define dgPWARN(t, level, ...)\
  do {\
    dgMsg##t(DG_MSG_S, DT_WARN, level);\
    dgWARN_Msg(DG_MSG_B, level, "pwarning: " dgLOC##t);\
    if(preproc.INC.p > 0 && preproc.INC.fullname[preproc.INC.p]) {\
      /* nested includes */\
      dgWARN_Msg(DG_MSG_B, level, "[%u/%d]: %s ", preproc.INC.row[0], preproc.INC.offset[preproc.INC.p], preproc.INC.name[preproc.INC.p]);\
    }\
    dgWARN_Msg(DG_MSG_B, level, "[%u/%d]: ", row, col+1);\
    dgWARN_Msg(DG_MSG_T, level, __VA_ARGS__);\
  } while(0)

#define DM_ERR_P(...)	DM_BLOCK(ERR, ERR_ERR, dgPERR(P, ERR_ERR, __VA_ARGS__))
#define DM_WARN_P(...)	DM_BLOCK(WARN, ERR_WARN, dgPWARN(P, ERR_WARN, __VA_ARGS__))

/* typedefs */
typedef std::map<std::string, struct nDefun *> TFunctions;
typedef std::map<std::string, std::string> TDefines;

struct Parser
{
  char *line_end;		/* pointer to a the end of the allocated memory of parser lines */
  char *line;			/* pointer to a current parser line */
  int llen;			/* length of the parsed line */
  uint row;			/* row parser position in current file */
  uint rows;			/* total row number (all files included) */
  int col;			/* column parser position */
  uint curr_offset;		/* current (the last) branch offset value */
  Node *root_node;		/* first node of the parsed data list (first valid line) */
  Node *c0_node;		/* current root node (used for indentation checks) OFFSET=0 */
  Node *new_node;		/* pointer to the current node which is being parsed */
  Node parser_node;		/* node to collect pre-action values in such as offset, ... */

  /* preprocessor directives */
  struct {
    struct {				/* #if */
      uint8_t b[MAX_IFS+1];		/* nested #if block */
      int p;				/* "nested" #if pointer (to the current block) */
    } IF;
    struct {				/* #include */
      char *name[MAX_INCS+1];		/* filename table (as seen by #include) */
      char *fullname[MAX_INCS+1];	/* path/filename table */
      char *dir[MAX_INCS+1];		/* canonical basename of the S2 script filename ($PWD if stdin) */
      FILE *fd[MAX_INCS+1];		/* file descriptor table */
      uint row[MAX_INCS+1];		/* row parser position in the included file */
      uint offset[MAX_INCS+1];		/* offset/row_indentation in this #included file */
      int p;				/* pointer to the currently included file */
    } INC;
  } preproc;

public:
  Parser();
  ~Parser();

  /* public methods */
  int start(const char *filename, Node **root);

private:
  BOOL is_comment_line(const char *s, const char comment_char);
  BOOL is_whitespace_line(const char *s);
  BOOL is_preprocessor_line(const char *s, const char preproc_char);

  /* some private parsing functions */
  int gc(void);
  int ugc(void);

#define _GET_INT(sign,size)\
  int parse_##sign##int##size(sign##int##size##_t *target, BOOL env_var);
_GET_INT(,8);
_GET_INT(u,8);
_GET_INT(,16);
_GET_INT(u,16);
_GET_INT(,32);
_GET_INT(u,32);
_GET_INT(,64);
_GET_INT(u,64);
#undef _GET_INT
    
  int AZaz_(const char *l, char **end);         /* [A-Za-z_]+ */
  int AZaz_dot(const char *l, char **end);      /* [A-Za-z_.]+ */
  int AZaz_09(const char *l, char **end);       /* [A-Za-z_][A-Za-z0-9_] */
  int double_quoted_param(std::string &target, BOOL env_var, const char* term_chars);
  int dq_param(std::string &target);
  int dq_param_env(std::string &target);
  int dq_param_x(std::string &target);
  int dq_param_ind(std::string &target);
  BOOL is_true_block(void);
  int set_include_dirname(char *target, const char *filename);

  /* The Grammar ******************************************************/
  /* special "symbols" */
  void WS(void);
  int WcW(int ch);		/* skips whitespace* `ch' whitespace* */
  int WEQW(void);		/* skips whitespace* '=' whitespace* */
  int LIND(void);		/* skips whitespace* '[' whitespace* */
  int RIND(void);		/* skips whitespace* ']' whitespace* */
  char *ENV_VAR(void);

  /* the grammar for the language itself */
  int S(void);
  int PREPROCESSOR(void);
  int BRANCH(void);
  int OFFSET(void);
  int BRANCH_PREFIX(void);
  int BRANCH_OPT(void);
  int MATCH_OPTS(void);
  int COND(void);
  int REPEAT(void);
  int REPEAT_FIXED(void);
  int WHILE(void);
  int ACTION(void);

  int ASSIGN(void);
  int DEFUN(void);
  int FUN(void);
  int NOP(void);
  int SETENV(void);
  int SLEEP(void);
  int SYSTEM(void);
  int TEST(void);

  /* the grammar for protocols */
#if defined(HAVE_SRM21) || defined(HAVE_SRM22)
  /* SRM2 related stuff */
  int ENDPOINT(std::string **target);
#endif
#ifdef HAVE_SRM21
  int srmAbortFilesR(void);
  int srmAbortRequestR(void);
  int srmChangeFileStorageTypeR(void);
  int srmCheckPermissionR(void);
  int srmCompactSpaceR(void);
  int srmCopyR(void);
  int srmExtendFileLifeTimeR(void);
  int srmGetRequestIDR(void);
  int srmGetRequestSummaryR(void);
  int srmGetSpaceMetaDataR(void);
  int srmGetSpaceTokenR(void);
  int srmLsR(void);
  int srmMkdirR(void);
  int srmMvR(void);
  int srmPrepareToGetR(void);
  int srmPrepareToPutR(void);
  int srmPutDoneR(void);
  int srmReassignToUserR(void);
  int srmReleaseFilesR(void);
  int srmReleaseSpaceR(void);
  int srmRemoveFilesR(void);
  int srmReserveSpaceR(void);
  int srmResumeRequestR(void);
  int srmRmR(void);
  int srmRmdirR(void);
  int srmSetPermissionR(void);
  int srmStatusOfCopyRequestR(void);
  int srmStatusOfGetRequestR(void);
  int srmStatusOfPutRequestR(void);
  int srmSuspendRequestR(void);
  int srmUpdateSpaceR(void);
#endif	/* HAVE_SRM21 */
#ifdef HAVE_SRM22
  int srmAbortFilesR(void);
  int srmAbortRequestR(void);
  int srmBringOnlineR(void);
  int srmChangeSpaceForFilesR(void);
  int srmCheckPermissionR(void);
  int srmCopyR(void);
  int srmExtendFileLifeTimeR(void);
  int srmExtendFileLifeTimeInSpaceR(void);
  int srmGetPermissionR(void);
  int srmGetRequestSummaryR(void);
  int srmGetRequestTokensR(void);
  int srmGetSpaceMetaDataR(void);
  int srmGetSpaceTokensR(void);
  int srmGetTransferProtocolsR(void);
  int srmLsR(void);
  int srmMkdirR(void);
  int srmMvR(void);
  int srmPingR(void);
  int srmPrepareToGetR(void);
  int srmPrepareToPutR(void);
  int srmPurgeFromSpaceR(void);
  int srmPutDoneR(void);
  int srmReleaseFilesR(void);
  int srmReleaseSpaceR(void);
  int srmReserveSpaceR(void);
  int srmResumeRequestR(void);
  int srmRmR(void);
  int srmRmdirR(void);
  int srmSetPermissionR(void);
  int srmStatusOfBringOnlineRequestR(void);
  int srmStatusOfCopyRequestR(void);
  int srmStatusOfGetRequestR(void);
  int srmStatusOfChangeSpaceForFilesRequestR(void);
  int srmStatusOfLsRequestR(void);
  int srmStatusOfPutRequestR(void);
  int srmStatusOfReserveSpaceRequestR(void);
  int srmStatusOfUpdateSpaceRequestR(void);
  int srmSuspendRequestR(void);
  int srmUpdateSpaceR(void);
#endif	/* HAVE_SRM22 */
};

/* extern(al) functions (defined by other modules) */
extern const char* PNAME(void);

/* extern(al) function declarations */
extern int parse(const char *filename, Node **root);

/* global variables */
extern TFunctions gl_fun_tab;	/* table of (global) functions */

#endif /* _PARSE_H */
