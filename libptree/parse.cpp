#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "parse.h"

#ifdef HAVE_GSOAP
#include "n_srm.h"
#include "soapH.h"              /* soap_codes_srm__TSpaceType, ... */
#endif

#include "version.h"		/* for #require directive */
#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "free.h"               /* FREE(), DELETE() */
#include "io.h"                 /* file_ropen(), ... */
#include "n.h"                  /* Node */
#include "str.h"

#include <signal.h>             /* signal() */
#include <stdlib.h>             /* exit() */
#include <stdio.h>              /* stderr */
#include <errno.h>              /* errno */
#include <stdarg.h>             /* vprintf() */
#include <libgen.h>		/* dirname() */

#include <iostream>             /* std::string, cout, endl, ... */

using namespace std;

/* global variables */
TFunctions gl_fun_tab;		/* table of (global) functions */

struct Parser
{
  char *line_end;               /* pointer to a the end of the allocated memory of parser lines */
  char *line;                   /* pointer to a current parser line */
  int llen;			/* length of the parsed line */
  uint row;			/* row parser position in current file */
  uint rows;			/* total row number (all files included) */
  int col;			/* column parser position */
  uint curr_offset;		/* current (the last) branch offset value */
  Node *root_node;              /* first node of the parsed data list (first valid line) */
  Node *c0_node;                /* current root node (used for indentation checks) OFFSET=0 */
  Node *new_node;               /* pointer to the current node which is being parsed */
  Node parser_node;             /* node to collect pre-action values in such as offset, ... */

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
  int double_quoted_param(std::string &target, BOOL env_var);
  int dq_param(std::string &target);
  int dq_param_env(std::string &target);
  int ind_param(std::string &target);
  BOOL is_true_block(void);
  int set_include_dirname(char *target, const char *filename);

  /* The Grammar ******************************************************/
  /* special "symbols" */
  void WS(void);
  int WcW(int ch);              /* skips whitespace* `ch' whitespace* */
  inline int WEQW(void);        /* skips whitespace* '=' whitespace* */
  inline int LIND(void);        /* skips whitespace* '[' whitespace* */
  char *ENV_VAR(void);

  /* the grammar itself */
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
  int CMP(CMP_t cmp);
  int DEFUN(void);
  int FUN(void);
  int MATCH(void);
  int NOP(void);
  int SETENV(void);
  int SLEEP(void);
  int SYSTEM(void);
  int TEST(void);

#ifdef HAVE_GSOAP
  /* SRM2 related stuff */
  int ENDPOINT(std::string **target);
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
#endif /* HAVE_GSOAP */
};

/*
 * Parser constructor
 */
Parser::Parser()
{
  /* initialise line pointers */
  line_end = NULL;
  line = NULL;

  /* initialise row and column values */
  row = 0;
  rows = 0;
  col = 0;
  curr_offset = 0;

  /* preprocessor-related init */
  memset(&preproc, 0, sizeof(preproc));
  preproc.IF.p = -1;

  /* initialise the root node */
  root_node = NULL;
  c0_node = NULL;
  new_node = NULL;
} /* Parser */

/*
 * Parser destructor
 */
Parser::~Parser()
{
} /* ~Parser */

/*
 * Returns
 *   TRUE:  line starts with optional whitespace + and comment character
 *   FALSE: otherwise
 */
BOOL
Parser::is_comment_line(const char *s, const char comment_char)
{
  int i, slen;

  if(s == NULL)
    return FALSE;
  
  slen = strlen(s);
  
  for(i = 0; i < slen && isspace(s[i]); i++)
    ;

  return s[i] == comment_char;
} /* is_comment_line */

/*
 * Returns
 *   TRUE:  line is composed of only whitespace or is completely empty
 *   FALSE: otherwise
 */
BOOL
Parser::is_whitespace_line(const char *s)
{
  int i, slen;
  
  if(s == NULL)
    return TRUE;
    
  slen = strlen(s);
  
  for(i = 0; i < slen && isspace(s[i]); i++)
    ;

  return i == slen;
} /* is_whitespace_line */

/*
 * Returns
 *   TRUE:  line starts with [ ]* + a preprocessor character
 *   FALSE: otherwise
 */
BOOL
Parser::is_preprocessor_line(const char *s, const char preproc_char)
{
  int i, slen;
  
  if(s == NULL)
    return TRUE;
    
  slen = strlen(s);
  
  for(i = 0; i < slen && s[i] == ' '; i++)
    ;

  return s[i] == preproc_char;
} /* is_preprocessor_line */

/* 
 * Get the next character from the current input line and increases `col'
 * to point to a following character
 * 
 * Returns
 *   S2_EOL: if we are trying to get a character after the line's last character
 */
inline int
Parser::gc(void)
{
  if(col >= llen)
    return CH_EOL;

  return line[col++];
} /* gc */

/* unget a character */
inline int
Parser::ugc(void)
{
  if(col > 0) return --col;

  DM_ERR_ASSERT(_("cannot unget any more; returning 0x%X character)\n"), CH_INV_UGC);
  return CH_INV_UGC;
} /* ugc */

/*
 * Parse a (signed) integer or $ENV{VAR} which evaluates to a (signed) integer.
 * 
 * Environment variables $ENV{VAR} are interpreted if `env_var' is TRUE.
 */
#define _GET_INT(sign,csign,d,size)\
int \
Parser::parse_##sign##int##size(sign##int##size##_t *target, BOOL env_var)\
{\
  sign##int##64_t value = 0;\
  char *endptr = NULL;\
  char *ptr_num = NULL;\
  int start_col = col;\
  BOOL have_env_var = FALSE;\
  int c;\
\
  /* see if we have $ENV{<VAR>} */\
  if(((c = gc()) == '$') && env_var) {\
    ptr_num = ENV_VAR();\
    if(ptr_num == NULL) {\
      /* ENV didn't follow $, error or environment variable not set */\
      col = start_col;\
      goto err;\
    }\
    have_env_var = TRUE;\
  } else {\
    if(c != CH_EOL) ugc();\
  }\
\
  if(!ptr_num) {\
    /* we don't have a valid $ENV{<VAR>} => set number pointer to the current parser offset */\
    ptr_num = line + col;\
  } else if(*ptr_num == '\0') {\
    /* (content of environment variable is empty or we just have EOL) */\
    col = start_col;\
\
    return ERR_OK;\
  }\
\
  /* see if we have a number in ptr_num */\
  errno = 0;\
  value = strto##sign##ll(ptr_num, &endptr, 0);\
  if(!have_env_var) \
    /* change the column value only if we don't parse a number from $ENV{VAR} */\
    col += endptr - ptr_num;\
\
  if(value < csign##INT##size##_MIN || value > csign##INT##size##_MAX) {\
    errno = ERANGE;\
    value = (sign##int##size##_t) value;        /* truncate */\
  }\
\
  *target = value;\
  if(errno == ERANGE) {\
    DM_PWARN(_("truncating input value to %ll" #d "\n"), value);\
\
    /* ignore the truncation error */\
    return ERR_OK;\
  }\
\
  if(ptr_num == endptr) {\
err:\
    *target = value;\
    DM_PERR(_("no integer characters converted, returning %ll" #d "\n"), value);\
\
    return ERR_ERR;\
  }\
\
  return ERR_OK;\
}

_GET_INT(,,d,8);
_GET_INT(u,U,u,8);
_GET_INT(,,d,16);
_GET_INT(u,U,u,16);
_GET_INT(,,d,32);
_GET_INT(u,U,u,32);
_GET_INT(,,d,64);
_GET_INT(u,U,u,64);
#undef _GET_INT

/*
 * Parse [A-Za-z_]*
 * + also some national characters depending on locale set
 */
int
Parser::AZaz_(const char *l, char **end)
{
  int c;
  int i;

  for(i = 0; (c = gc()) != CH_EOL && (isalpha(c) || c == '_'); i++)
    ;

  if(c != CH_EOL) ugc();

  if(end != NULL)
    *end = (char *)l + i;

  return ERR_OK;
} /* AZaz_ */

/*
 * Parse [A-Za-z_.]*
 * + also some national characters depending on locale set
 */
int
Parser::AZaz_dot(const char *l, char **end)
{
  int c;
  int i;

  for(i = 0; (c = gc()) != CH_EOL && (isalpha(c) || c == '_' || c == '.'); i++)
    ;

  if(c != CH_EOL) ugc();

  if(end != NULL)
    *end = (char *)l + i;

  return ERR_OK;
} /* AZaz_dot */

/*
 * Parse an identifier [A-Za-z_][A-Za-z0-9_].
 */
int
Parser::AZaz_09(const char *l, char **end)
{
  int i, c;

  if(col >= llen) {
    DM_PERR(_("identifier must start with [A-Za-z_] (found EOL)\n"));
    return ERR_ERR;
  }

  for(i = 0; (c = gc()) != CH_EOL; i++) {
    if(i == 0 && !(isalpha(c) || c == '_')) {
      ugc();
      DM_PERR(_("identifier must start with [A-Za-z_] (found %c)\n"), c);
      return ERR_ERR;
    }
    if(!(isalnum(c) || c == '_'))
      break;
  };
  if(c != CH_EOL) ugc();
  
  if(end != NULL)
    *end = (char *)l + i;

  return ERR_OK;
} /* AZaz_09 */

/*
 * Parse one of the following strings:
 * 1) [^ \t\r\n]* 
 * 2) '"' [^"]* '"'
 * Note: ad 1) space can be part of the parsed string if it is escaped
 *       ad 2) "     can be part of the parsed string if it is escaped.
 * 
 * - De-escaping of spaces and "s is performed: 'a\ string' => 'a string'
 *                                              '"a \"string\""' => 'a "string"'.
 * - \\ is left unchanged: '\\"' => '\\"'
 * - ad 1) the meaning of \" and " is equivalent
 * 
 * Environment variables $ENV{VAR} are interpreted if `env_var' is TRUE.
 *
 * Returns
 *   ERR_OK:  found a parameter (even an empty one "")
 *   ERR_ERR: the first character is a whitespace character or _only_ S2_EOL or
 *            we were unable to get the value of an environment variable.
 */
int
Parser::double_quoted_param(std::string &target, BOOL env_var)
{
#define TERM_CHAR(c)    (q? (c == '"'): IS_WHITE(c))
  int i, c;
  BOOL q;               /* quotation mark at the start of the filename */
  BOOL bslash = FALSE;  /* we had the '\\' character */
  int esc = 0;          /* number of escaped characters */
  char *ptr_env = NULL;
  
  q = (c = gc()) == '"';
  if(!q && c != CH_EOL) ugc();
  if(IS_WHITE(c) || c == CH_EOL) return ERR_ERR;
  
  for(i = 0; (c = gc()) != CH_EOL; i++) {
    int start_col = col;
    /* see if we have $ENV{<VAR>} */
    if(c == '$' && env_var) {
      if(!bslash) {
        ptr_env = ENV_VAR();
        if(ptr_env == NULL) {
          /* ENV didn't follow $, error or environment variable not set */
          col = start_col;
          /* ignore this and copy the string as it is */
        } else {
          /* we have ptr_env pointing at ENV variable => overwrite the line with its value */
          int env_len = strlen(ptr_env);
          target.append(ptr_env);
          i += env_len - 1;
          continue;
        }
      }
    }

    if(c == '\\' && bslash) {
      /* two backslashes => no quoting */
      bslash = FALSE;
      target.push_back(c);
      continue;
    }

    if(TERM_CHAR(c) && !bslash) {
      if(c != '"') ugc();

      return ERR_OK;            /* found a string terminator */
    }

    if(!q) {
      /* we have an unquoted string => remove escaping of whitespace and "s */
      if(c == '\\' && !bslash && 
         (IS_WHITE(line[col]) || line[col] == '"')) /* look ahead */
      {
        /* single backslash => the following character is escaped */
        esc++;
        goto out;
      }
    }

    target.push_back(c);
out:
    bslash = c == '\\';
  }

  if(q && c == CH_EOL)
    DM_PWARN("'\\0' terminated double-quoted parameter\n");

  return ERR_OK;

#undef TERM_CHAR
} /* double_quoted_param */

int
Parser::dq_param(std::string &target)
{
  return double_quoted_param(target, FALSE);
} /* dq_param */

int
Parser::dq_param_env(std::string &target)
{
  return double_quoted_param(target, TRUE);
} /* dq_param_env */

/*
 * Parse \[ [^\]]* \].
 * 
 * - De-escaping of ] is performed: 'a [string\]' => 'a [string]'
 * 
 * Returns
 *   ERR_OK:      found a parameter
 *   ERR_ERR: the first character is S2_EOL
 */
int
Parser::ind_param(std::string &target)
{
#define TERM_CHAR(c)    (c == ']')
  int i, c;
  BOOL bslash = FALSE;  /* we had the '\\' character */
  int esc = 0;          /* number of escaped characters */

  if((c = gc()) != CH_EOL) ugc();
  else return ERR_ERR;
  
  for(i = 0; (c = gc()) != CH_EOL; i++) {
    if(c == '\\' && bslash) {
      /* two backslashes => no quoting */
      bslash = FALSE;
      target.push_back(c);
      continue;
    }

    if(TERM_CHAR(c) && !bslash) {
      return ERR_OK;            /* found a string terminator */
    }

    /* remove escaping of ']' */
    if(c == '\\' && !bslash && 
       line[col] == ']') /* look ahead */
    {
      /* single backslash => the following character is escaped */
      esc++;
      goto out;
    }

    target.push_back(c);
out:
    bslash = c == '\\';
  }

  if(c == CH_EOL)
    DM_PWARN("'\\0' terminated index parameter\n");

  return ERR_OK;

#undef TERM_CHAR
} /* ind_param */

/* 
 * Returns
 *   TRUE:  we are in #if 1 block or #else branch of #if 0 block
 *   FALSE: we are in #if 0 block or #else branch of #if 1 block
 */
BOOL
Parser::is_true_block(void)
{
  int i;
  for(i = 0; i <= preproc.IF.p; i++)
    if(!preproc.IF.b[i]) return FALSE;

  return TRUE;
} /* is_true_block */

/*
 * Sets include directory name based on the value of the S2 script file
 * `filename' to be processed.
 */
int
Parser::set_include_dirname(char *target, const char *filename)
{
  if(filename == NULL) {
    /* standard input */
    if(getcwd(target, PATH_MAX - 1) != (char *)NULL)
    {
      strcat(target, PATH_STR);
      return ERR_OK;
    }
    
    DM_ERR(ERR_SYSTEM, _("could not get current working directory: `%s'\n"), _(strerror(errno)));
    return ERR_SYSTEM;
  }

  /* the S2 script is standard input */ 
  if(!realpath(filename, target))
  {
    DM_ERR(ERR_SYSTEM, _("canonicalised absolute pathname for `%s': `%s'\n"), filename, _(strerror(errno)));
    return ERR_SYSTEM;
  }

  dirname(target);
  return ERR_OK;
} /* set_include_dirname */

/*
 * The Grammar ******************************************************
 */
void
Parser::WS(void)
{
  int c;

  do {
    c = gc();
  } while (isspace(c) && c != CH_EOL);
  if(c != CH_EOL) ugc();
} /* WS */

/*
 * Skips whitespace* `ch' whitespace*
 */
int
Parser::WcW(int ch)
{
  int c;

  WS();                 /* skip whitespace (if any) */
  if((c = gc()) != ch) {
    ugc();
    if(c != CH_EOL) DM_PERR("expected '%c', found '%c'\n", ch, c);
    else DM_PERR("found EOL while expecting '%c'\n", ch);
    return ERR_ERR;
  }
  WS();                 /* skip whitespace (if any) */
  
  return ERR_OK;
} /* WcW */

/*
 * Skips whitespace* '=' whitespace*
 */
inline int
Parser::WEQW(void)
{
  return WcW('=');
} /* WEQW */

/*
 * Skips whitespace* '[' whitespace*
 */
inline int
Parser::LIND(void)
{
  return WcW('[');
} /* LIND */

/*
 * Parse ENV{<VAR>} and return a value of environment variable <VAR>.
 */
char *
Parser::ENV_VAR(void)
{
  char *env_var = NULL;
  int c;
  char *opt;
  char *end = NULL;
  char end_char;

  AZaz_(opt = line + col, &end);        /* try to get ENV, move col */

  if(POPL("ENV")) {
    WS();       /* allow whitespace after $ENV */
    if((c = gc()) != '{') {
      DM_PERR(_("$ENV with missing '{'\n"));
      return NULL;
    }
    WS();       /* allow whitespace after $ENV{ */
  
    /* get variable name */
    AZaz_09(opt = line + col, &end);

    WS();       /* allow whitespace after $ENV{<VAR> */
    if((c = gc()) != '}') {
      ugc();
      DM_PERR(_("$ENV{<VAR> with missing '}'\n"));
      return NULL;
    }
    /* we have a variable name in opt */
    end_char = *end;
    *end = '\0';
    env_var = getenv(opt);
    if(!env_var)
      DM_PWARN(_("environment variable %s is unset\n"), opt);
    *end = end_char;
  }

  /* we have a value of environment variable (possibly NULL) => return it */
  return env_var;
}

int
Parser::S(void)
{
  int rval;

  /* initialise column value */
  col = 0;
  
  rval = BRANCH();

  /* check any remaining tokens to be parsed (none should be left) apart from comments */
  if(!rval && col < llen && !is_comment_line(line+col, CH_COMMENT)) {
    /* show this warning only if there was no error (do not overwhelm users) */
    DM_PWARN("unparsed tokens remain (%d)\n", llen);
  }

  return rval;
} /* S */

int
Parser::PREPROCESSOR()
{
  int rval;
  char *opt;
  char *end = NULL;
  char dir[PATH_MAX+1];		/* temporary storage for preproc.INC.dir[preproc.INC.p] */
  char fullname[PATH_MAX+1];	/* temporary storage for preproc.INC.fullname[preproc.INC.p] */
  std::string _val;
  uint preproc_offset;
  
  if((opt = str_char(line, CH_PREPROC)) == NULL) {
    /* no preprocessor character present */
    DM_ERR_ASSERT(_("missing preprocessor character\n"));
    return ERR_ERR;
  }
  col = opt - line + 1; /* move 1 behind CH_PREPROC */
  preproc_offset = col - 1;

  /* we have a preprocessor character */
  WS();                         /* allow whitespace after CH_PREPROC ('#') */
  AZaz_(opt = line + col, &end);        /* get preprocessor directive name */

  if(POPL("if")) {
    WS();       /* allow whitespace after 'if' */
    if(++preproc.IF.p >= MAX_IFS) {
      DM_PERR(_("too many nested #if directives (max %u)\n"), MAX_IFS);
      preproc.IF.p--;
      return ERR_OK;
    }
    UINT8(_("#if directive"),preproc.IF.b[preproc.IF.p]);
  } else if(POPL("else")) {
    WS();       /* allow whitespace after 'else' */
    if(preproc.IF.p < 0) {
      DM_PWARN(_("unexpected #else directive (no matching #if)\n"));
      /* be lenient, just ignore it */
      return ERR_OK;    /* don't return ERR_ERR, try to ignore it */
    }
    preproc.IF.b[preproc.IF.p] = !preproc.IF.b[preproc.IF.p];
  } else if(POPL("endif")) {
    WS();       /* allow whitespace after 'endif' */
    if(--preproc.IF.p < -1) {
      DM_PWARN(_("unexpected #endif preprocessor directive (no matching #if)\n"));
      /* be lenient, just ignore it */
      return ERR_OK;    /* don't return ERR_ERR, try to ignore it */
    }
  } else if(POPL("require")) {		/* require a specific s2 version */
    WS();		/* allow whitespace after 'require' */
    PARSE(dq_param,_val,"require directive parameter\n");	/* parse require version into _val */
    if(strcmp(MK_VERSION, _val.c_str()) < 0) {
      /* TODO (major/minor versions) */
      DM_ERR(ERR_ERR, _("S2 script requires S2 client version `%s' or higher\n"), _val.c_str());
      return ERR_ERR;
    }
  } else if(POPL("include")) {
    if(!is_true_block()) 
      /* #if 0 or #else branch of #if 1 */
      return ERR_OK;

    WS();	/* allow whitespace after 'include' */

    if(++preproc.INC.p >= MAX_INCS) {
      DM_PERR(_("too many nested #include(s) (max %u); #include directive ignored\n"), MAX_INCS);
      preproc.INC.p--;
      return ERR_OK;	/* don't return ERR_ERR, try to ignore this error */
    }
    preproc.INC.offset[preproc.INC.p] = preproc.INC.offset[preproc.INC.p-1] + preproc_offset;
    DM_DBG(DM_N(5),"INC.offset[%d]=%u, INC.offset[%d]=%u, preproc_offset=%u\n", preproc.INC.p, preproc.INC.offset[preproc.INC.p], preproc.INC.p-1, preproc.INC.offset[preproc.INC.p-1], preproc_offset);
    PARSE(dq_param,_val,"include directive filename\n");	/* parse include filename into _val */
    if(is_absolute_path(_val.c_str())) {
      /* we have an absolute pathname */
      DM_DBG(DM_N(3),"absolute include directive filename=|%s|\n", _val.c_str());
      strncpy(dir, _val.c_str(), PATH_MAX);
    } else {
      /* a relative pathname */
      DM_DBG(DM_N(3),"relative include directive filename=|%s|\n", _val.c_str());
      strncpy(dir, preproc.INC.dir[preproc.INC.p-1], PATH_MAX);
      strncat(dir, PATH_STR, PATH_MAX-strlen(dir));
      strncat(dir, _val.c_str(), PATH_MAX-strlen(dir));
    }
    if(!realpath(dir, fullname))
    { /* canonicalisation failed */
      DM_ERR(ERR_SYSTEM, _("canonicalisation of #include filename `%s' failed: `%s'\n"), dir, _(strerror(errno)));
      return ERR_SYSTEM;
    }
    STRDUP(preproc.INC.name[preproc.INC.p], _val.c_str());
    DM_DBG(DM_N(3),"INC.name[preproc.INC.p]=|%s|\n", preproc.INC.name[preproc.INC.p]);
    STRDUP(preproc.INC.fullname[preproc.INC.p], fullname);
    DM_DBG(DM_N(3),"INC.fullname[preproc.INC.p]=|%s|\n", preproc.INC.fullname[preproc.INC.p]);
    dirname(fullname);
    STRDUP(preproc.INC.dir[preproc.INC.p], fullname);
    DM_DBG(DM_N(3),"INC.dir[preproc.INC.p]=|%s|\n", preproc.INC.dir[preproc.INC.p]);
    rval = file_ropen(preproc.INC.fullname[preproc.INC.p], &(preproc.INC.fd[preproc.INC.p]));
    if(rval) {
      /* open failed */
      FREE(preproc.INC.name[preproc.INC.p]);
      FREE(preproc.INC.fullname[preproc.INC.p]);
      FREE(preproc.INC.dir[preproc.INC.p]);
      return rval;
    }
    /* open succeeded */
  } else {
    if(*opt)
      DM_PERR(_("unknown preprocessor directive `%.*s'; use: #(if|else|endif)\n"), end - opt, opt);
    else
      DM_PERR(_("unknown preprocessor directive; use: #(if|else|endif)\n"));
      
    return ERR_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* PREPROCESSOR */

int
Parser::BRANCH(void)
{
  int rval;
  
  if(llen == 0)
    /* empty line */
    return ERR_OK;

  EATW(OFFSET);         /* compulsory, but may be empty */
  EATW(BRANCH_PREFIX);  /* optional stuff common to all branches */
  EAT(ACTION);          /* compulsory; don't skip whitespace (error reporting) */

  /* parsing succeeded */
  return ERR_OK;
} /* BRANCH */

/* compulsory offset (may be empty) */
int
Parser::OFFSET(void)
{
  int c;
  uint o;
  
  /* calculate the offset */
  for(o = 0; ((c = gc()) == ' ') && (c != CH_EOL); o++)
    ;
  if(c != CH_EOL) ugc();
  if(preproc.INC.p != 0) o += preproc.INC.offset[preproc.INC.p];
  DM_DBG(DM_N(3), "OFFSET()=%d/o=%d\n", preproc.INC.offset[preproc.INC.p], o);

  if(c == '\t') {
    DM_PERR(_("horizontal tab character is not allowed to indent a branch\n"));
    return ERR_ERR;
  }
  
  if(c == CH_EOL) {
    /* this should not happen due to the removal of trailing whitespace */
    DM_ERR_ASSERT(_("invalid S2 line (composed completely of spaces)\n"));
    return ERR_ASSERT;
  }

  /* check indentation of the first (root node) */
  if(root_node == NULL && o != 0) {
    DM_PERR(_("indentation of the root node must be 0\n"), o);
    return ERR_ERR;
  }

  /* valid indentation check */
  if(c0_node != NULL && o < curr_offset && Node::get_node_with_offset(c0_node, o) == NULL) {
    DM_PERR(_("invalid indentation: no previous offset %d\n"), o);
    return ERR_ERR;
  }

  parser_node.OFFSET = o;

  /* parsing succeeded */
  return ERR_OK;
} /* OFFSET */

/* optional stuff common to all branches */
int
Parser::BRANCH_PREFIX(void)
{
  int rval;

  EATW(COND);           /* optional */
  EATW(REPEAT);         /* optional */
  EATW(BRANCH_OPT);     /* optional */

  /* parsing succeeded */
  return ERR_OK;
} /* BRANCH_PREFIX */

/* optional stuff common to all branches (no order dependencies) */
int
Parser::BRANCH_OPT(void)
{
  int rval;
  char *opt;
  char *end = NULL;

  while(col < llen) {
//  eval                                /* evaluation level */

    WS();
    AZaz_(opt = line + col, &end);      /* get options (or <ACTION>) */

    POPL_INT32("eval",parser_node.EVAL) else
    POPL_UINT64("timeout",parser_node.TIMEOUT) else
    if (POPL("match")) {
      EAT(WEQW);
      EAT(MATCH_OPTS);
    } else
    {
      col -= end - opt;
      return ERR_OK;    /* no other options, we've hit something else; proceed with <ACTION> */
    }
  }

  /* parsing succeeded */
  return ERR_OK;
} /* BRANCH_OPT */

/* optional */
int
Parser::MATCH_OPTS(void)
{
  int c;
  int i;
  BOOL invert = FALSE;

  /* get the actual `match' options */
loop:
  while(TRUE) {
    c = gc();
    if(isspace(c))
      /* enough, we have a separator (whitespace) */
      break;

    if(c == CH_EOL) {
      DM_PERR(_("expecting a match option, found EOL\n"));
      return ERR_ERR;
    }

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
        parser_node.match_opt.px = 0;
        parser_node.match_opt.pcre = 0;
      } else {
enable_all:
        /* all options enabled */
        parser_node.match_opt.px = get_match_opt_max(0);
        parser_node.match_opt.pcre = get_match_opt_max(1);
      }

      goto loop;
    }
    /* we have a match option, see if it is a legal one */

    DM_DBG(DM_N(0), "char (%c); px=%d; pcre=%d\n", c, parser_node.match_opt.px, parser_node.match_opt.pcre);
    OPT_SET(PX_mopts, parser_node.match_opt.px, c);
    OPT_SET(PCRE_mopts, parser_node.match_opt.pcre, c);
    
    /* not a PX or PCRE matching option => report this */
    ugc();      /* show the correct error location */
    DM_PWARN(_("illegal match option `%c'\n"), c);
    gc();
  }
  if(c != CH_EOL) ugc();

  /* parsing succeeded */
  return ERR_OK;
} /* MATCH_OPTS */

/* optional condition */
int
Parser::COND(void)
{
  int c = gc();
  
  switch(c) {
    case '|': /* OR condition */
      if((c = gc()) != '|') {
        ugc();
        DM_PERR(_("invalid OR condition character %c\n"), c);
        return ERR_ERR;
      }
      if(Node::get_node_with_offset(c0_node, parser_node.OFFSET) == NULL) {
        DM_PERR(_("no previous branch with the same offset (%u) to join || condition to\n"), parser_node.OFFSET);
        return ERR_ERR;
      }

      /* we have a valid OR condition */
      parser_node.COND = S2_COND_OR;
    break;
    
    case '&': /* AND condition */
      if((c = gc()) != '&') {
        ugc();
        DM_PERR(_("invalid AND condition character %c\n"), c);
        return ERR_ERR;
      }
      if(Node::get_node_with_offset(c0_node, parser_node.OFFSET) == NULL) {
        DM_PERR(_("no previous branch with the same offset (%u) to join && condition to\n"), parser_node.OFFSET);
        return ERR_ERR;
      }

      /* we have a valid AND condition */
      parser_node.COND = S2_COND_AND;
      
    break;
    
    default:
      if(c != CH_EOL) ugc();
  }

  /* parsing succeeded */
  return ERR_OK;
} /* COND */
    
/* optional repeat */
int
Parser::REPEAT(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  int c = gc();

  switch(c) {
    case '>':
      EATW(REPEAT_FIXED);
    break;

    case 'W': 
      ugc();
      AZaz_(opt = line + col, &end);
      if(POPL("WHILE")) parser_node.REPEAT.type = S2_REPEAT_WHILE;
      else col -= end - opt;

      return ERR_OK;
    break;
        
    default: 
      /* no repeat present */
      parser_node.REPEAT.type = S2_REPEAT_NONE;
      ugc();
      return ERR_OK;
  }
 
  /* parsing succeeded */
  return ERR_OK;
} /* REPEAT */

/* optional repeat */
int
Parser::REPEAT_FIXED(void)
{
  int c;
  int start_col;

  /* we have a repeat operator */
  WS();         /* allow whitespace after '>' */

  INT64(_("start value of the repeat operator"),parser_node.REPEAT.X);

  start_col = col;
  WS();         /* allow whitespace after X */

  if(start_col != col) {
    /* it can be a parallel ' ' repeat operator (we had some whitespace) */
    parser_node.REPEAT.type = S2_REPEAT_PAR;
  }

  c = gc();
  switch(c) {
    case '|':
      /* we have found '|', check the second one */
      c = gc();
      if(c != '|') {
        ugc();
        DM_PERR(_("invalid sequential '||' repeat operator %c\n"), c);
        return ERR_ERR;
      }
      /* a sequential '||' repeat */
//      ugc();
      parser_node.REPEAT.type = S2_REPEAT_OR;
    break;

    case '&':
      /* we have found '&', check the second one */
      c = gc();
      if(c != '&') {
        ugc();
        DM_PERR(_("invalid sequential '||' repeat operator %c\n"), c);
        return ERR_ERR;
      }
      /* a sequential '&&' repeat */
//      ugc();
      parser_node.REPEAT.type = S2_REPEAT_AND;
    break;
    
    default:
      ugc();

      /* we hit a non-whitespace or '|' or '&' character */
      if(parser_node.REPEAT.type == S2_REPEAT_PAR) 
        break;

      DM_PERR(_("invalid repeat operator\n"));
      return ERR_ERR;
  }
  
  WS();         /* allow whitespace '||' or '&&' */

  INT64(_("end value of the repeat operator"), parser_node.REPEAT.Y);
  
  /* parsing succeeded */
  return ERR_OK;
} /* REPEAT_FIXED */


int
Parser::ACTION(void)
{
  int rval;
  char *opt;
  char *end = NULL;

  WS();
  AZaz_(opt = line + col, &end);        /* get a ACTION method name */

#define POPL_EAT(s,r,...)\
  if (POPL(""#s)) {\
    EAT(s##r,__VA_ARGS__);		/* don't eat whitespace (error reporting) */\
  }
#define POPL_CMP(op)\
  if (POPL(""#op)) {\
    EAT(CMP,CMP_##op);		/* don't eat whitespace (error reporting) */\
  }

  POPL_EAT(ASSIGN,,) else
  POPL_EAT(DEFUN,,) else
  POPL_EAT(FUN,,) else
  POPL_EAT(MATCH,,) else
  POPL_EAT(NOP,,) else
  POPL_EAT(SETENV,,) else
  POPL_EAT(SLEEP,,) else
  POPL_EAT(SYSTEM,,) else
  POPL_EAT(TEST,,) else

  POPL_CMP(EQ) else
  POPL_CMP(NE) else
  POPL_CMP(LT) else
  POPL_CMP(GT) else
  POPL_CMP(LE) else
  POPL_CMP(GE) else

#ifdef HAVE_GSOAP
  POPL_EAT(srmAbortFiles,R,) else
  POPL_EAT(srmAbortRequest,R,) else
  POPL_EAT(srmChangeFileStorageType,R,) else
  POPL_EAT(srmCheckPermission,R,) else
  POPL_EAT(srmCompactSpace,R,) else
  POPL_EAT(srmCopy,R,) else
  POPL_EAT(srmExtendFileLifeTime,R,) else
  POPL_EAT(srmGetRequestID,R,) else
  POPL_EAT(srmGetRequestSummary,R,) else
  POPL_EAT(srmGetSpaceMetaData,R,) else
  POPL_EAT(srmGetSpaceToken,R,) else
  POPL_EAT(srmLs,R,) else
  POPL_EAT(srmMkdir,R,) else
  POPL_EAT(srmMv,R,) else
  POPL_EAT(srmPrepareToGet,R,) else
  POPL_EAT(srmPrepareToPut,R,) else
  POPL_EAT(srmPutDone,R,) else
  POPL_EAT(srmReassignToUser,R,) else
  POPL_EAT(srmReleaseFiles,R,) else
  POPL_EAT(srmReleaseSpace,R,) else
  POPL_EAT(srmRemoveFiles,R,) else
  POPL_EAT(srmReserveSpace,R,) else
  POPL_EAT(srmResumeRequest,R,) else
  POPL_EAT(srmRm,R,) else
  POPL_EAT(srmRmdir,R,) else
  POPL_EAT(srmSetPermission,R,) else
  POPL_EAT(srmStatusOfCopyRequest,R,) else
  POPL_EAT(srmStatusOfGetRequest,R,) else
  POPL_EAT(srmStatusOfPutRequest,R,) else
  POPL_EAT(srmSuspendRequest,R,) else
  POPL_EAT(srmUpdateSpace,R,) else
#endif /* HAVE_GSOAP */

  POPL_ERR;

  /* parsing succeeded */
  return ERR_OK;

#undef POPL_EAT
} /* ACTION */

int
Parser::ASSIGN(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  nAssign *r = new nAssign(parser_node);
  new_node = r;

  WS(); /* allow whitespace before an option or ASSIGN variable (or comment char) */
  AZaz_dot(opt = line + col, &end);   /* get options */
  POPL_EQ_PARAM("overwrite",r->overwrite) else
  {
    col -= end - opt;
  }

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace before the ASSIGN variable, leave if comment char hit */

    DQ_PARAMv(dq_param,_val,r->var,"ASSIGN variable\n");
    DM_DBG(DM_N(3), "var=|%s|\n", _val.c_str());
  
    _val.clear();
    WS(); /* allow whitespace before the ASSIGN value */
    DQ_PARAMv(dq_param,_val,r->val,"ASSIGN value\n");
    DM_DBG(DM_N(3), "val=|%s|\n", _val.c_str());
  }

  /* parsing succeeded */
  return ERR_OK;
} /* ASSIGN */

int
Parser::CMP(CMP_t cmp)
{
  std::string _val;
  
  nCmp *r = new nCmp(parser_node);
  new_node = r;

  r->cmp = cmp;

  WS(); /* allow whitespace before LOP */
  DQ_PARAM(dq_param,_val,r->lop,"LOP variable\n");

  _val.clear();
  WS(); /* allow whitespace before ROP */
  DQ_PARAM(dq_param,_val,r->rop,"ROP value\n");

  return ERR_OK;
} /* CMP */

int
Parser::DEFUN(void)
{
  TFunctions::iterator funIter;	/* name/function object pair */
  std::string _val;
  int c;

  nDefun *r = new nDefun(parser_node);
  new_node = r;
  r->TYPE = N_DEFUN;		/* tell the defun process it should not evaluate its children */

  WS(); /* allow whitespace before name of the function */
  DQ_PARAM(dq_param,_val,r->name,"function name\n");

  if ((funIter = gl_fun_tab.find(r->name->c_str())) != gl_fun_tab.end()) {
    /* function `name' already defined, issue a warning and re-define it */
    DM_PWARN("function `%s' already defined, re-defining\n", r->name->c_str());
    funIter->second = r;
  } else {
    /* we have a definition of a new function */
    gl_fun_tab.insert(std::pair<std::string, struct nDefun *>(r->name->c_str(), r));
  }
  
  /* parameters */
  while(col < llen) {
    _val.clear();

    WS_COMMENT;	 /* allow whitespace before function parameters, leave if comment char hit */

    if((c = gc()) == ':') break;
    else ugc();

    DQ_PARAMv(dq_param,_val,r->params,"function call by value parameter\n");
    DM_DBG(DM_N(3), "param=|%s|\n", _val.c_str());
  }

  /* : params_ref part */
  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace before call by reference values, leave if comment char hit */

    DQ_PARAMv(dq_param,_val,r->params_ref,"function call by reference parameter\n");
    DM_DBG(DM_N(3), "retval=|%s|\n", _val.c_str());
  }

  return ERR_OK;
} /* DEFUN */

int
Parser::FUN(void)
{
  std::string _val;
  int c;

  nFun *r = new nFun(parser_node);
  new_node = r;
  r->TYPE = N_FUN;

  WS(); /* allow whitespace before name of the function */
  DQ_PARAM(dq_param,_val,r->name,"function name\n");
  
  /* arguments */
  while(col < llen) {
    _val.clear();

    WS_COMMENT;	 /* allow whitespace before function arguments, leave if comment char hit */

    if((c = gc()) == ':') break;
    else ugc();

    DQ_PARAMv(dq_param,_val,r->args,"function argument\n");
    DM_DBG(DM_N(3), "arg=|%s|\n", _val.c_str());
  }

  /* : args_ref part */
  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace before call by reference values, leave if comment char hit */

    DQ_PARAMv(dq_param,_val,r->args_ref,"function return value\n");
    DM_DBG(DM_N(3), "retval=|%s|\n", _val.c_str());
  }

  return ERR_OK;
} /* FUN */

int
Parser::MATCH(void)
{
  std::string _val;
  
  nMatch *r = new nMatch(parser_node);
  new_node = r;

  WS(); /* allow whitespace before the MATCH pattern/expected string */
  DQ_PARAM(dq_param,_val,r->expected,"MATCH pattern\n");

  _val.clear();
  WS(); /* allow whitespace before the MATCH received/subject string */
  DQ_PARAM(dq_param,_val,r->received,"MATCH subject\n");

  /* parsing succeeded */
  return ERR_OK;
} /* MATCH */

int
Parser::NOP(void)
{
  char c;
  std::string _val;
  
  nNop *r = new nNop(parser_node);
  new_node = r;

  WS(); /* allow whitespace before the NOP value */
  
  c = gc();
  if(c == CH_COMMENT) {
    /* we have a comment character (no NOP parameter, use the default value) */
    ugc();
    goto zero;
  }
  if(c == CH_EOL)
    /* we have an EOL (no NOP parameter, use the default value) */
    goto zero;

  ugc();

  DQ_PARAM(dq_param,_val,r->val,"NOP value\n");

  /* parsing succeeded */
  return ERR_OK;

zero:
  /* default NOP value */
  NEW_STR(r->val, "0");

  return ERR_OK;
} /* NOP */

int
Parser::SETENV(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  nSetenv *r = new nSetenv(parser_node);
  new_node = r;

  WS(); /* allow whitespace before an option or SETENV variable (or comment char) */
  AZaz_dot(opt = line + col, &end);   /* get options */
  POPL_EQ_PARAM("overwrite",r->overwrite) else
  {
    col -= end - opt;
  }

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace before the ASSIGN variable, leave if comment char hit */

    DQ_PARAMv(dq_param,_val,r->var,"SETENV variable\n");
    DM_DBG(DM_N(3), "var=|%s|\n", _val.c_str());
  
    _val.clear();
    WS(); /* allow whitespace before the SETENV value */
    DQ_PARAMv(dq_param,_val,r->val,"SETENV value\n");
    DM_DBG(DM_N(3), "val=|%s|\n", _val.c_str());
  }

  /* parsing succeeded */
  return ERR_OK;
} /* SETENV */

int
Parser::SLEEP(void)
{
  char c;
  std::string _val;
  
  nDelay *r = new nDelay(parser_node);
  new_node = r;

#define SLEEP_VAL(label,store,err_text)\
  _val.clear();\
  WS(); /* allow whitespace before the DELAY value */\
  c = gc();\
  if(c == CH_COMMENT) {\
    /* we have a comment character (no DELAY parameter, use the default value) */\
    ugc();\
    goto label;\
  }\
  if(c == CH_EOL)\
    /* we have an EOL (no DELAY parameter, use the default value) */\
    goto label;\
\
  ugc();\
\
  DQ_PARAM(dq_param,_val,store,err_text);

  SLEEP_VAL(szero,  r->sec, "SLEEP seconds value\n");
  DM_DBG(DM_N(2), "sleep seconds=%s\n", r->sec->c_str());
  SLEEP_VAL(nszero, r->nsec, "SLEEP nanoseconds value\n");
  DM_DBG(DM_N(2), "sleep nanoseconds=%s\n", r->nsec->c_str());

  /* parsing succeeded */
  return ERR_OK;

szero:
  /* default SLEEP seconds value */
  NEW_STR(r->sec, "0");
    
nszero:
  /* default SLEEP nanoseconds value */
  NEW_STR(r->nsec, "0");

  return ERR_OK;
#undef SLEEP_VAL
} /* SLEEP */

int
Parser::SYSTEM(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  nSystem *r = new nSystem(parser_node);
  new_node = r;

  WS(); /* allow whitespace before an option or ASSIGN variable (or comment char) */
  AZaz_dot(opt = line + col, &end);   /* get options */
  POPL_EQ_PARAM("out",r->out) else
  { 
    col -= end - opt;
  };

  /* the actual system call */
  _val.clear();

  WS(); /* don't do WS_COMMENT, CH_COMMENT can be part of the system call */
  NEW_STR(r->cmd, line + col, llen - col);
  col = llen;       /* make parser happy */
  DM_DBG(DM_N(3), "system=|%s|\n", r->cmd->c_str());

  /* parsing succeeded */
  return ERR_OK;
} /* SYSTEM */

int
Parser::TEST(void)
{
  std::string _val;
  
  nTest *r = new nTest(parser_node);
  new_node = r;
  int c;

  WS(); /* allow whitespace before the TEST expression */
  
  c = gc();
  if(c == CH_COMMENT) {
    /* we have a comment character (no TEST parameter, use the default value -- empty string) */
    ugc();
    goto empty;
  }
  if(c == CH_EOL)
    /* we have an EOL (no TEST parameter, use the default value -- empty string) */
    goto empty;

  ugc();

  DQ_PARAM(dq_param,_val,r->expr,"TEST expression\n");

  /* parsing succeeded */
  return ERR_OK;

empty:
  /* default TEST expression */
  NEW_STR(r->expr, "");

  /* parsing succeeded */
  return ERR_OK;
} /* TEST */

#ifdef HAVE_GSOAP
int
Parser::ENDPOINT(std::string **target)
{
  std::string _val;

  WS();
  DQ_PARAM(dq_param,_val,*target,"endpoint\n");
  
  /* parsing succeeded */
  return ERR_OK;
}

int
Parser::srmAbortFilesR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmAbortFiles *r = new srmAbortFiles(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("requestToken",r->requestToken) else
    POPL_ARRAY("surlArray",r->surlArray) else

    /* response */
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmAbortFilesR */

int
Parser::srmAbortRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmAbortRequest *r = new srmAbortRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("requestToken",r->requestToken) else

    /* response */
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmAbortRequestR */

int
Parser::srmChangeFileStorageTypeR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmChangeFileStorageType *r = new srmChangeFileStorageType(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_ARRAY("path.SURLOrStFN",r->path.SURLOrStFN) else
    POPL_ARRAY("path.storageSystemInfo",r->path.storageSystemInfo) else
    POPL_EQ_PARAM("desiredStorageType",r->desiredStorageType) else

    /* response */
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmChangeFileStorageTypeR */

int
Parser::srmCheckPermissionR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmCheckPermission *r = new srmCheckPermission(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_ARRAY("path.SURLOrStFN",r->path.SURLOrStFN) else
    POPL_ARRAY("path.storageSystemInfo",r->path.storageSystemInfo) else
    POPL_EQ_PARAM("checkInLocalCacheOnly",r->checkInLocalCacheOnly) else

    /* response */
    POPL_EQ_PARAM("permissions",r->permissions) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmCheckPermissionR */

int
Parser::srmCompactSpaceR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmCompactSpace *r = new srmCompactSpace(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("spaceToken",r->spaceToken) else
    POPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    POPL_EQ_PARAM("doDynamicCompactFromNowOn",r->doDynamicCompactFromNowOn) else

    /* response */
    POPL_EQ_PARAM("newSizeOfThisSpace",r->newSizeOfThisSpace) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmCompactSpaceR */

int
Parser::srmCopyR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmCopy *r = new srmCopy(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_ARRAY("arrayOfFileRequests.allLevelRecursive",r->arrayOfFileRequests.allLevelRecursive) else
    POPL_ARRAY("arrayOfFileRequests.isSourceADirectory",r->arrayOfFileRequests.isSourceADirectory) else
    POPL_ARRAY("arrayOfFileRequests.numOfLevels",r->arrayOfFileRequests.numOfLevels) else
    POPL_ARRAY("arrayOfFileRequests.fileStorageType",r->arrayOfFileRequests.fileStorageType) else
    POPL_ARRAY("arrayOfFileRequests.fromSURLOrStFN",r->arrayOfFileRequests.fromSURLOrStFN) else
    POPL_ARRAY("arrayOfFileRequests.fromStorageSystemInfo",r->arrayOfFileRequests.fromStorageSystemInfo) else
    POPL_ARRAY("arrayOfFileRequests.lifetime",r->arrayOfFileRequests.lifetime) else
    POPL_ARRAY("arrayOfFileRequests.overwriteMode",r->arrayOfFileRequests.overwriteMode) else
    POPL_ARRAY("arrayOfFileRequests.spaceToken",r->arrayOfFileRequests.spaceToken) else
    POPL_ARRAY("arrayOfFileRequests.toSURLOrStFN",r->arrayOfFileRequests.toSURLOrStFN) else
    POPL_ARRAY("arrayOfFileRequests.toStorageSystemInfo",r->arrayOfFileRequests.toStorageSystemInfo) else
    POPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else
    POPL_EQ_PARAM("overwriteOption",r->overwriteOption) else
    POPL_EQ_PARAM("removeSourceFiles",r->removeSourceFiles) else
    POPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    POPL_EQ_PARAM("totalRetryTime",r->totalRetryTime) else

    /* response */
    POPL_EQ_PARAM("requestToken",r->requestToken) else
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmCopyR */

int
Parser::srmExtendFileLifeTimeR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmExtendFileLifeTime *r = new srmExtendFileLifeTime(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("requestToken",r->requestToken) else
    POPL_EQ_PARAM("siteURL",r->siteURL) else
    POPL_EQ_PARAM("newLifeTime",r->newLifeTime) else

    /* response */
    POPL_EQ_PARAM("newTimeExtended",r->newTimeExtended) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmExtendFileLifeTimeR */

int
Parser::srmGetRequestIDR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetRequestID *r = new srmGetRequestID(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else

    /* response */
    POPL_EQ_PARAM("requestTokens",r->requestTokens) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetRequestSummaryR */

int
Parser::srmGetRequestSummaryR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetRequestSummary *r = new srmGetRequestSummary(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_ARRAY("arrayOfRequestToken",r->arrayOfRequestToken) else

    /* response */
    POPL_EQ_PARAM("requestSummary",r->requestSummary) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetRequestSummaryR */

int
Parser::srmGetSpaceMetaDataR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetSpaceMetaData *r = new srmGetSpaceMetaData(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_ARRAY("arrayOfSpaceToken",r->arrayOfSpaceToken) else

    /* response */
    POPL_EQ_PARAM("spaceDetails",r->spaceDetails) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetSpaceMetaDataR */

int
Parser::srmGetSpaceTokenR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetSpaceToken *r = new srmGetSpaceToken(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("userSpaceTokenDescription",r->userSpaceTokenDescription) else

    /* response */
    POPL_EQ_PARAM("possibleSpaceTokens",r->possibleSpaceTokens) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetSpaceTokenR */

int
Parser::srmLsR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmLs *r = new srmLs(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_ARRAY("path.SURLOrStFN",r->path.SURLOrStFN) else
    POPL_ARRAY("path.storageSystemInfo",r->path.storageSystemInfo) else
    POPL_EQ_PARAM("fileStorageType",r->fileStorageType) else
    POPL_EQ_PARAM("fullDetailedList",r->fullDetailedList) else
    POPL_EQ_PARAM("allLevelRecursive",r->allLevelRecursive) else
    POPL_EQ_PARAM("numOfLevels",r->numOfLevels) else
    POPL_EQ_PARAM("offset",r->offset) else
    POPL_EQ_PARAM("count",r->count) else

    /* response */
    POPL_EQ_PARAM("pathDetails",r->pathDetails) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmLsR */

int
Parser::srmMkdirR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmMkdir *r = new srmMkdir(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("SURLOrStFN",r->SURLOrStFN) else
    POPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else

    /* response */
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmMkdirR */

int
Parser::srmMvR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmMv *r = new srmMv(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("fromSURLOrStFN",r->fromSURLOrStFN) else
    POPL_EQ_PARAM("fromStorageSystemInfo",r->fromStorageSystemInfo) else
    POPL_EQ_PARAM("toSURLOrStFN",r->toSURLOrStFN) else
    POPL_EQ_PARAM("toStorageSystemInfo",r->toStorageSystemInfo) else

    /* response */
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmMvR */

int
Parser::srmPrepareToGetR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmPrepareToGet *r = new srmPrepareToGet(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_ARRAY("arrayOfFileRequests.allLevelRecursive",r->arrayOfFileRequests.allLevelRecursive) else
    POPL_ARRAY("arrayOfFileRequests.isSourceADirectory",r->arrayOfFileRequests.isSourceADirectory) else
    POPL_ARRAY("arrayOfFileRequests.numOfLevels",r->arrayOfFileRequests.numOfLevels) else
    POPL_ARRAY("arrayOfFileRequests.fileStorageType",r->arrayOfFileRequests.fileStorageType) else
    POPL_ARRAY("arrayOfFileRequests.SURLOrStFN",r->arrayOfFileRequests.SURLOrStFN) else
    POPL_ARRAY("arrayOfFileRequests.storageSystemInfo",r->arrayOfFileRequests.storageSystemInfo) else
    POPL_ARRAY("arrayOfFileRequests.lifetime",r->arrayOfFileRequests.lifetime) else
    POPL_ARRAY("arrayOfFileRequests.spaceToken",r->arrayOfFileRequests.spaceToken) else
    POPL_ARRAY("arrayOfTransferProtocols",r->arrayOfTransferProtocols) else
    POPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else
    POPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    POPL_EQ_PARAM("totalRetryTime",r->totalRetryTime) else

    /* response */
    POPL_EQ_PARAM("requestToken",r->requestToken) else
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmPrepareToGetR */

int
Parser::srmPrepareToPutR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmPrepareToPut *r = new srmPrepareToPut(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_ARRAY("arrayOfFileRequests.fileStorageType",r->arrayOfFileRequests.fileStorageType) else
    POPL_ARRAY("arrayOfFileRequests.knownSizeOfThisFile",r->arrayOfFileRequests.knownSizeOfThisFile) else
    POPL_ARRAY("arrayOfFileRequests.lifetime",r->arrayOfFileRequests.lifetime) else
    POPL_ARRAY("arrayOfFileRequests.spaceToken",r->arrayOfFileRequests.spaceToken) else
    POPL_ARRAY("arrayOfFileRequests.SURLOrStFN",r->arrayOfFileRequests.SURLOrStFN) else
    POPL_ARRAY("arrayOfFileRequests.storageSystemInfo",r->arrayOfFileRequests.storageSystemInfo) else
    POPL_ARRAY("arrayOfTransferProtocols",r->arrayOfTransferProtocols) else
    POPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else
    POPL_EQ_PARAM("overwriteOption",r->overwriteOption) else
    POPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    POPL_EQ_PARAM("totalRetryTime",r->totalRetryTime) else

    /* response */
    POPL_EQ_PARAM("requestToken",r->requestToken) else
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmPrepareToPutR */

int
Parser::srmPutDoneR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmPutDone *r = new srmPutDone(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("requestToken",r->requestToken) else
    POPL_ARRAY("surlArray",r->surlArray) else

    /* response */
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmPutDoneR */

int
Parser::srmReassignToUserR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmReassignToUser *r = new srmReassignToUser(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("assignedUser",r->assignedUser) else
    POPL_EQ_PARAM("lifeTimeOfThisAssignment",r->lifeTimeOfThisAssignment) else
    POPL_EQ_PARAM("SURLOrStFN",r->SURLOrStFN) else
    POPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else

    /* response */
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmReassignToUserR */

int
Parser::srmReleaseFilesR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmReleaseFiles *r = new srmReleaseFiles(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("requestToken",r->requestToken) else
    POPL_ARRAY("surlArray",r->surlArray) else
    POPL_EQ_PARAM("keepFiles",r->keepFiles) else

    /* response */
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmReleaseFilesR */

int
Parser::srmReleaseSpaceR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmReleaseSpace *r = new srmReleaseSpace(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("spaceToken",r->spaceToken) else
    POPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    POPL_EQ_PARAM("forceFileRelease",r->forceFileRelease) else

    /* response */
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmReleaseSpaceR */

int
Parser::srmRemoveFilesR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmRemoveFiles *r = new srmRemoveFiles(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("requestToken",r->requestToken) else
    POPL_ARRAY("surlArray",r->surlArray) else

    /* response */
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmRemoveFilesR */

int
Parser::srmReserveSpaceR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmReserveSpace *r = new srmReserveSpace(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("typeOfSpace",r->typeOfSpace) else
    POPL_EQ_PARAM("userSpaceTokenDescription",r->userSpaceTokenDescription) else
    POPL_EQ_PARAM("sizeOfTotalSpaceDesired",r->sizeOfTotalSpaceDesired) else
    POPL_EQ_PARAM("sizeOfGuaranteedSpaceDesired",r->sizeOfGuaranteedSpaceDesired) else
    POPL_EQ_PARAM("lifetimeOfSpaceToReserve",r->lifetimeOfSpaceToReserve) else
    POPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else

    /* response */
    POPL_EQ_PARAM("typeOfReservedSpace",r->typeOfReservedSpace) else
    POPL_EQ_PARAM("sizeOfTotalReservedSpace",r->sizeOfTotalReservedSpace) else
    POPL_EQ_PARAM("sizeOfGuaranteedReservedSpace",r->sizeOfGuaranteedReservedSpace) else
    POPL_EQ_PARAM("lifetimeOfReservedSpace",r->lifetimeOfReservedSpace) else
    POPL_EQ_PARAM("referenceHandleOfReservedSpace",r->referenceHandleOfReservedSpace) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmReserveSpaceR */

int
Parser::srmResumeRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmResumeRequest *r = new srmResumeRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("requestToken",r->requestToken) else

    /* response */
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmResumeRequestR */

int
Parser::srmRmR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmRm *r = new srmRm(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_ARRAY("path.SURLOrStFN",r->path.SURLOrStFN) else
    POPL_ARRAY("path.storageSystemInfo",r->path.storageSystemInfo) else

    /* response */
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmRmR */

int
Parser::srmRmdirR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmRmdir *r = new srmRmdir(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("SURLOrStFN",r->SURLOrStFN) else
    POPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    POPL_EQ_PARAM("recursive",r->recursive) else

    /* response */
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmRmdirR */

int
Parser::srmSetPermissionR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmSetPermission *r = new srmSetPermission(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("SURLOrStFN",r->SURLOrStFN) else
    POPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    POPL_EQ_PARAM("permissionType",r->permissionType) else
    POPL_EQ_PARAM("ownerPermission",r->ownerPermission) else
    POPL_ARRAY("userPermissionArray.mode",r->userPermissionArray.mode) else
    POPL_ARRAY("userPermissionArray.ID",r->userPermissionArray.ID) else
    POPL_ARRAY("groupPermissionArray.mode",r->groupPermissionArray.mode) else
    POPL_ARRAY("groupPermissionArray.ID",r->groupPermissionArray.ID) else
    POPL_EQ_PARAM("otherPermission",r->otherPermission) else

    /* response */
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmSetPermissionR */

int
Parser::srmStatusOfCopyRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmStatusOfCopyRequest *r = new srmStatusOfCopyRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("requestToken",r->requestToken) else
    POPL_ARRAY("fromSurlArray",r->fromSurlArray) else
    POPL_ARRAY("toSurlArray",r->toSurlArray) else

    /* response */
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmStatusOfCopyRequestR */

int
Parser::srmStatusOfGetRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmStatusOfGetRequest *r = new srmStatusOfGetRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("requestToken",r->requestToken) else
    POPL_ARRAY("surlArray",r->surlArray) else

    /* response */
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmStatusOfGetRequestR */

int
Parser::srmStatusOfPutRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmStatusOfPutRequest *r = new srmStatusOfPutRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("requestToken",r->requestToken) else
    POPL_ARRAY("surlArray",r->surlArray) else

    /* response */
    POPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmStatusOfPutRequestR */

int
Parser::srmSuspendRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmSuspendRequest *r = new srmSuspendRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("requestToken",r->requestToken) else

    /* response */
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmSuspendRequestR */

int
Parser::srmUpdateSpaceR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmUpdateSpace *r = new srmUpdateSpace(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    POPL_EQ_PARAM("userID",r->userID) else
    POPL_EQ_PARAM("spaceToken",r->spaceToken) else
    POPL_EQ_PARAM("newSizeOfTotalSpaceDesired",r->newSizeOfTotalSpaceDesired) else
    POPL_EQ_PARAM("newSizeOfGuaranteedSpaceDesired",r->newSizeOfGuaranteedSpaceDesired) else
    POPL_EQ_PARAM("newLifeTimeFromCallingTime",r->newLifeTimeFromCallingTime) else
    POPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else

    /* response */
    POPL_EQ_PARAM("sizeOfTotalSpace",r->sizeOfTotalSpace) else
    POPL_EQ_PARAM("sizeOfGuaranteedSpace",r->sizeOfGuaranteedSpace) else
    POPL_EQ_PARAM("lifetimeGranted",r->lifetimeGranted) else
    POPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    POPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    POPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmUpdateSpaceR */
#endif /* HAVE_GSOAP */

/*
 * Returns
 *   0:  success
 *   >0: errors
 */
int
Parser::start(const char *filename, Node **root)
{
  char dir[PATH_MAX+1];		/* temporary storage for preproc.INC.dir[preproc.INC.p] */
  int rval = 0;                 /* return value */

  if(root == NULL)
    DM_ERR_ASSERT(_("root == NULL\n"));

  if(filename != NULL) {
    /* we don't have a standard input as the input file */
    STRDUP(preproc.INC.name[preproc.INC.p], filename);		/* copy the original filename to the INC structure */
    STRDUP(preproc.INC.fullname[preproc.INC.p], filename);	/* copy the original filename to the INC structure */
  }
    
  rval = file_ropen(filename, &(preproc.INC.fd[0]));
  if(rval) return rval;         /* open failed */

  *dir = '\0';				/* first S2 script file, no directory prefix */
  set_include_dirname(dir, filename);	/* initialise #include directive directory name */
  STRDUP(preproc.INC.dir[0], dir);
  
loop:
  while(dfreads(&line, &line_end, preproc.INC.fd[preproc.INC.p], (uint32_t *)&llen)) {
    int lval;
    rows++; row++;
    parser_node.init();
    parser_node.row = row;

    /* ignore empty lines and comments */
    if(is_whitespace_line(line) || is_comment_line(line, CH_COMMENT))
      continue;

    /* remove trailing whitespace */
    remove_trailing_ws(line);
    llen = strlen(line);        /* line length might have changed (trailing whitespace removed) */

    if(is_preprocessor_line(line, CH_PREPROC)) {
      int INCp = preproc.INC.p;
      lval = PREPROCESSOR();
      UPDATE_MAX(rval, lval);

      if(INCp != preproc.INC.p) {
        /* we have a new #include */
        if(lval) {
          /* there were errors; decrease INCp and continue where we left off */
          --preproc.INC.p;
          continue;
        }
        /* we have a legal #include directive */
        preproc.INC.row[preproc.INC.p - 1] = row;	/* save the row number */
        row = 0;					/* we're about to start parsing a new file */
      }
      /* any directive has done its job now, continue */
      continue;
    }

    if(!is_true_block())
      /* #if 0 or #else branch of #if 1 */
      continue;
    
    /* decide which SRM function we're using */
    if ((lval = S()) != 0) {
      /* parser error */
      UPDATE_MAX(rval, lval);   /* increase the return value */
      DELETE(new_node);
      continue;
    } else {
      if(root_node == NULL) {
        /* we have a parser root node */
        root_node = new_node;
        *root = root_node;
      } else {
        /* append this new node to the parser tree */
        if(root_node != new_node) {
          /* do not append the first node (circular dependency) */
          if(Node::append_node(root_node, new_node)) {
            /* could not append node for some reason, try to continue */
            DELETE(new_node);
            continue;
          }
          /* update the current offset variable */
          DM_DBG(DM_N(3), "curr_offset=%d\n", curr_offset);
          curr_offset = new_node->OFFSET;
          DM_DBG(DM_N(3), "updating curr_offset=%d\n", curr_offset);
        }
      }

      if(new_node->OFFSET == 0) {
        /* we have a new c0 node */
        c0_node = new_node;
      }

      /* make sure there are no references to the already attached node */
      new_node = NULL;
    }
  }
  
  /* free included filename and close its file descriptor */
  file_close(preproc.INC.fullname[preproc.INC.p], &preproc.INC.fd[preproc.INC.p]);
  FREE(preproc.INC.name[preproc.INC.p]);
  FREE(preproc.INC.fullname[preproc.INC.p]);
  FREE(preproc.INC.dir[preproc.INC.p]);
  if(preproc.INC.p-- > 0) {
    row = preproc.INC.row[preproc.INC.p];	/* retrieve the row number */
    goto loop;
  }

  if(line) {
    FREE(line);
    line_end = NULL;
  }

  return rval;
} /* start */

/*
 * C interface ******************************************************
 */
int
parse(const char *filename, Node **root)
{
  int rval;

  Parser *parser = new Parser();

  rval = parser->start(filename, root);

  if(parser) delete parser;

  return rval;
} /* parse */
