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

#include "version.h"		/* for #require directive */
#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "free.h"               /* FREE(), DELETE() */
#include "max.h"		/* UPDATE_MAX() */
#include "opt.h"		/* OPT() */
#include "io.h"                 /* file_ropen(), ... */
#include "n.h"                  /* Node */
#include "str.h"

#include <stdlib.h>             /* exit() */
#include <stdio.h>              /* stderr */
#include <errno.h>              /* errno */
#include <stdarg.h>             /* vprintf() */
#include <libgen.h>		/* dirname() */
#include <unistd.h>		/* getcwd() on RedHat */

#include <iostream>             /* std::string, cout, endl, ... */

using namespace std;

/* private macros */

/* global variables */
TFunctions gl_fun_tab;		/* table of (global) functions */
TDefines defines;		/* defines */

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

  return 0;
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
    DM_WARN_P(_("truncating input value to %ll" #d "\n"), value);\
\
    /* ignore the truncation error */\
    return ERR_OK;\
  }\
\
  if(ptr_num == endptr) {\
err:\
    *target = value;\
    DM_ERR_P(_("no integer characters converted, returning %ll" #d "\n"), value);\
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
    DM_ERR_P(_("identifier must start with [A-Za-z_] (found EOL)\n"));
    return ERR_ERR;
  }

  for(i = 0; (c = gc()) != CH_EOL; i++) {
    if(i == 0 && !(isalpha(c) || c == '_')) {
      ugc();
      DM_ERR_P(_("identifier must start with [A-Za-z_] (found %c)\n"), c);
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

#if 1
/*
 * Parse one of the following strings:
 * 1) [^ \t\r\n]* 
 * 2) '"' [^"]* '"'
 * Note: ad 1) space can be part of the parsed string if it is escaped
 *       ad 2) "     can be part of the parsed string if it is escaped.
 * 
 * - De-escaping of " and \ is performed: 'a\ string' => 'a string'
 *                                        '"a \"string\""' => 'a "string"'.
 * 
 * Environment variables $ENV{VAR} are interpreted if `env_var' is TRUE.
 *
 * Returns
 *   ERR_OK:  found a parameter (even an empty one "")
 *   ERR_ERR: the first character is a whitespace character or _only_ S2_EOL or
 *            we were unable to get the value of an environment variable.
 */
int
Parser::double_quoted_param(std::string &target, BOOL env_var, const char* term_chars)
{
#define TERM_CHAR(c)    (dq? (c == '"'): (term_chars)? strchr(term_chars, c) != NULL : (IS_WHITE(c) && (!tag || (tag && !brackets))))
  int i, c;
  BOOL dq;		/* quotation mark at the start of the parameter */
  BOOL bslash = FALSE;	/* we had the '\\' character */
  BOOL string = FALSE;	/* we had an opening " */
  int esc = 0;		/* number of escaped characters */
  char *ptr_env = NULL;
  BOOL tag;		/* $[A-Za-z]{ */
  int brackets = 0;
  
  dq = (c = gc()) == '"';
  tag = c == '$';
  if(!dq && c != CH_EOL) ugc();
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

    if(!bslash && !string) {
      if(c == '}') brackets--;
      if(c == '{') brackets++;
    }

    if(TERM_CHAR(c) && !bslash) {
      if(c != '"') ugc();

      return ERR_OK;            /* found a string terminator */
    }

    if(c == '"') {
      string = string? FALSE: TRUE;
      target.push_back(c);
      goto esc_out;
    }

    if(dq) {
      /* we have a double-quoted string => remove escaping of "\ */
      if(c == '\\' && !bslash
         && (line[col] == '"' || line[col] == '\\')) /* look ahead */
      {
        /* single backslash => the following character is escaped */
        esc++;
        goto esc_out;
      }
    } else {
      /* we have an unquoted string => remove escaping of "\ and whitespace */
      if(c == '\\' && !bslash
         && (IS_WHITE(line[col]) || line[col] == '"' || line[col] == '\\')) /* look ahead */
      {
        /* single backslash => the following character is escaped */
        esc++;
        goto esc_out;
      }
    }

    target.push_back(c);
esc_out:
    bslash = c == '\\';
  }

  if(dq && c == CH_EOL)
    DM_WARN_P("'\\0' terminated double-quoted parameter\n");

  return ERR_OK;

#undef TERM_CHAR
} /* double_quoted_param */
#else
/*
 * Parse one of the following strings:
 * 1) [^ \t\r\n]* 
 * 2) " [^"]* "
 * 3) ' [^']* '
 * Note: ad 1) space can be part of the parsed string if it is escaped
 *       ad 2) "     can be part of the parsed string if it is escaped.
 *       ad 3) '     can be part of the parsed string if it is escaped.
 * 
 * - De-escaping of spaces is performed: 'a\ string' => 'a string'
 * - \\ is left unchanged: '\\"' => '\\"'
 * 
 * Environment variables $ENV{VAR} are interpreted if `env_var' is TRUE.
 *
 * Returns
 *   ERR_OK:  found a parameter
 *   ERR_ERR: the first character is a whitespace character or _only_ S2_EOL or
 *            we were unable to get the value of an environment variable.
 */
int
Parser::double_quoted_param(std::string &target, BOOL env_var)
{
  DM_DBG_I;
  
#define TERM_CHAR(c)    (term_char? (c) == term_char : IS_WHITE(c))
  int i, c = gc();
  BOOL q;			/* ' at the start of target */
  BOOL dq;			/* " at the start of target */
  BOOL bslash = FALSE;		/* we had the '\\' character */
  char term_char = '\0';	/* parameter terminator */
  char *ptr_env = NULL;

  q  = c == '\'';
  dq = c == '"';
  if(q || dq) {
    term_char = c;
    target.push_back(c);
  } else ugc();

  DM_DBG(DM_N(3), "col=%d; dq_char='%c'\n", col, c);

  if(IS_WHITE(c) || c == CH_EOL) return ERR_ERR;
  
  for(i = 0; (c = gc()) != CH_EOL; i++) {
    int start_col = col;

    DM_DBG(DM_N(3), "col=%d; dq_char='%c'\n", col, c);

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
      if(term_char) {
        term_char = c;
        target.push_back(c);
      } else ugc();

      return ERR_OK;            /* found a string terminator */
    }

    target.push_back(c);
out:
    bslash = c == '\\';
  }

  if(term_char && c == CH_EOL) {
    DM_WARN_P("'\\0' terminated %squoted parameter\n", dq? "double-": "");
    target.push_back(term_char);
  }

  return ERR_OK;

#undef TERM_CHAR
} /* double_quoted_param */
#endif

int
Parser::dq_param(std::string &target)
{
  return double_quoted_param(target, FALSE, NULL);
} /* dq_param */

int
Parser::dq_param_env(std::string &target)
{
  return double_quoted_param(target, TRUE, NULL);
} /* dq_param_env */

int
Parser::dq_param_x(std::string &target)
{
  return double_quoted_param(target, FALSE, "|& \t");
} /* dq_param_x */

int
Parser::dq_param_ind(std::string &target)
{
  return double_quoted_param(target, FALSE, "] \t");
} /* dq_param_ind */

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
    if(c != CH_EOL) DM_ERR_P("expected '%c', found '%c'\n", ch, c);
    else DM_ERR_P("found EOL while expecting '%c'\n", ch);
    return ERR_ERR;
  }
  WS();                 /* skip whitespace (if any) */
  
  return ERR_OK;
} /* WcW */

/*
 * Skips whitespace* '=' whitespace*
 */
int
Parser::WEQW(void)
{
  return WcW('=');
} /* WEQW */

/*
 * Skips whitespace* '[' whitespace*
 */
int
Parser::LIND(void)
{
  return WcW('[');
} /* LIND */

/*
 * Skips whitespace* ']' whitespace*
 */
int
Parser::RIND(void)
{
  return WcW(']');
} /* RIND */

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

  if(P_OPL("ENV")) {
    WS();       /* allow whitespace after $ENV */
    if((c = gc()) != '{') {
      DM_ERR_P(_("$ENV with missing '{'\n"));
      return NULL;
    }
    WS();       /* allow whitespace after $ENV{ */
  
    /* get variable name */
    AZaz_09(opt = line + col, &end);

    WS();       /* allow whitespace after $ENV{<VAR> */
    if((c = gc()) != '}') {
      ugc();
      DM_ERR_P(_("$ENV{<VAR> with missing '}'\n"));
      return NULL;
    }
    /* we have a variable name in opt */
    end_char = *end;
    *end = '\0';
    env_var = getenv(opt);
    if(!env_var)
      DM_WARN_P(_("environment variable %s is unset\n"), opt);
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
    DM_WARN_P("unparsed tokens remain (%d)\n", llen);
  }

  return rval;
} /* S */

int
Parser::PREPROCESSOR()
{
#define CHECK_IF_NESTING\
  if(++preproc.IF.p >= MAX_IFS) {\
    DM_ERR_P(_("too many nested #if((n)def) directives (max %u)\n"), MAX_IFS);\
    preproc.IF.p--;\
    return ERR_ERR;\
  }
  
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

  if(P_OPL("ifdef")) {
    WS();       /* allow whitespace after 'ifdef' */
    CHECK_IF_NESTING;
    PARSE(dq_param,_val,"ifdef directive parameter\n");		/* parse variable name into _val */
    preproc.IF.b[preproc.IF.p] = defines.find(_val) != defines.end();
  } else if(P_OPL("ifndef")) {
    WS();       /* allow whitespace after 'ifndef' */
    CHECK_IF_NESTING;
    PARSE(dq_param,_val,"ifndef directive parameter\n");	/* parse variable name into _val */
    preproc.IF.b[preproc.IF.p] = defines.find(_val) == defines.end();
  } else if(P_OPL("define")) {
    WS();       /* allow whitespace after 'define' */
    PARSE(dq_param,_val,"define directive parameter\n");	/* parse variable name into _val */
    defines.insert(std::pair<std::string, std::string>(_val, ""));
  } else if(P_OPL("if")) {
    WS();       /* allow whitespace after 'if' */
    CHECK_IF_NESTING;
    P_UINT8(_("#if directive"),preproc.IF.b[preproc.IF.p]);
  } else if(P_OPL("else")) {
    WS();       /* allow whitespace after 'else' */
    if(preproc.IF.p < 0) {
      DM_WARN_P(_("unexpected #else directive (no matching #if)\n"));
      /* be lenient, just ignore it */
      return ERR_OK;    /* don't return ERR_ERR, try to ignore it */
    }
    preproc.IF.b[preproc.IF.p] = !preproc.IF.b[preproc.IF.p];
  } else if(P_OPL("endif")) {
    WS();       /* allow whitespace after 'endif' */
    if(--preproc.IF.p < -1) {
      DM_WARN_P(_("unexpected #endif preprocessor directive (no matching #if)\n"));
      /* be lenient, just ignore it */
      return ERR_OK;    /* don't return ERR_ERR, try to ignore it */
    }
  } else if(P_OPL("require")) {		/* require a specific s2 version */
    WS();		/* allow whitespace after 'require' */
    PARSE(dq_param,_val,"require directive parameter\n");	/* parse require version into _val */

    const char *MK_VERSION_ptr = MK_VERSION;
    const char *_val_ptr = _val.c_str();
    char *MK_VERSION_endptr, *_val_endptr;

    do {
      uint MK_VERSION_u, _val_u;
      MK_VERSION_u = get_uint(MK_VERSION_ptr, &MK_VERSION_endptr, FALSE);
      if(MK_VERSION_ptr != MK_VERSION_endptr)
        MK_VERSION_ptr = *MK_VERSION_endptr? MK_VERSION_endptr + 1 : MK_VERSION_endptr;	/* skip a dot */

      _val_u = get_uint(_val_ptr, &_val_endptr, FALSE);
      if (MK_VERSION_u < _val_u) {
        DM_ERR(ERR_ERR, _("S2 script requires S2 client version `%s' or higher, have `%s'\n"), _val.c_str(), MK_VERSION);
        return ERR_ERR;
      } else if (MK_VERSION_u > _val_u) break;
      
      if(_val_ptr != _val_endptr)
        _val_ptr = *_val_endptr? _val_endptr + 1 : _val_endptr;	/* skip a dot */

    } while(*MK_VERSION_ptr && *_val_ptr);
  } else if(P_OPL("include")) {
    if(!is_true_block()) 
      /* #if 0 or #else branch of #if 1 */
      return ERR_OK;

    WS();	/* allow whitespace after 'include' */

    if(++preproc.INC.p >= MAX_INCS) {
      DM_ERR_P(_("too many nested #include(s) (max %u); #include directive ignored\n"), MAX_INCS);
      preproc.INC.p--;
      return ERR_ERR;
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
      DM_ERR_P(_("unknown preprocessor directive `%.*s'; use: #(ifdef|ifndef|define|if|else|endif|require)\n"), end - opt, opt);
    else
      DM_ERR_P(_("unknown preprocessor directive; use: #(ifdef|ifndef|define|if|else|endif|require)\n"));
      
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

  EAT_WS(OFFSET);         /* compulsory, but may be empty */
  EAT_WS(BRANCH_PREFIX);  /* optional stuff common to all branches */
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
    DM_ERR_P(_("horizontal tab character is not allowed to indent a branch\n"));
    return ERR_ERR;
  }
  
  if(c == CH_EOL) {
    /* this should not happen due to the removal of trailing whitespace */
    DM_ERR_ASSERT(_("invalid S2 line (composed completely of spaces)\n"));
    return ERR_ASSERT;
  }

  /* check indentation of the first (root node) */
  if(root_node == NULL && o != 0) {
    DM_ERR_P(_("indentation of the root node must be 0\n"), o);
    return ERR_ERR;
  }

  /* valid indentation check */
  if(c0_node != NULL && o < curr_offset && Node::get_node_with_offset(c0_node, o) == NULL) {
    DM_ERR_P(_("invalid indentation: no previous offset %d\n"), o);
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

  EAT_WS(COND);           /* optional */
  EAT_WS(REPEAT);         /* optional */
  EAT_WS(BRANCH_OPT);     /* optional */

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

    P_OPL_INT32("eval",parser_node.EVAL) else
    P_OPL_UINT64("timeout",parser_node.TIMEOUT) else
    if (P_OPL("match")) {
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
      DM_ERR_P(_("expecting a match option, found EOL\n"));
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
    DM_WARN_P(_("illegal match option `%c'\n"), c);
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
        DM_ERR_P(_("invalid OR condition character %c\n"), c);
        return ERR_ERR;
      }
      if(Node::get_node_with_offset(c0_node, parser_node.OFFSET) == NULL) {
        DM_ERR_P(_("no previous branch with the same offset (%u) to join || condition to\n"), parser_node.OFFSET);
        return ERR_ERR;
      }

      /* we have a valid OR condition */
      parser_node.COND = S2_COND_OR;
    break;
    
    case '&': /* AND condition */
      if((c = gc()) != '&') {
        ugc();
        DM_ERR_P(_("invalid AND condition character %c\n"), c);
        return ERR_ERR;
      }
      if(Node::get_node_with_offset(c0_node, parser_node.OFFSET) == NULL) {
        DM_ERR_P(_("no previous branch with the same offset (%u) to join && condition to\n"), parser_node.OFFSET);
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
      EAT_WS(REPEAT_FIXED);
    break;

    case 'W': 
      ugc();
      AZaz_(opt = line + col, &end);
      if(P_OPL("WHILE")) parser_node.REPEAT.type = S2_REPEAT_WHILE;
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
  std::string _val;

  /* we have a repeat operator */
  WS();         /* allow whitespace after '>' */

  P_DQ_PARAM(dq_param_x,_val,parser_node.REPEAT.X,"function name\n");
  _val.clear();

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
        DM_ERR_P(_("invalid sequential '||' repeat operator %c\n"), c);
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
        DM_ERR_P(_("invalid sequential '||' repeat operator %c\n"), c);
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

      DM_ERR_P(_("invalid repeat operator\n"));
      return ERR_ERR;
  }
  
  WS();         /* allow whitespace '||' or '&&' */

  P_DQ_PARAM(dq_param,_val,parser_node.REPEAT.Y,"end value of the repeat operator\n");
  
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

#define P_OPL_EAT(s,r,...)\
  if (P_OPL(""#s)) {\
    EAT(s##r,__VA_ARGS__);		/* don't eat whitespace (error reporting) */\
  }

  P_OPL_EAT(ASSIGN,,) else
  P_OPL_EAT(DEFUN,,) else
  P_OPL_EAT(FUN,,) else
  P_OPL_EAT(NOP,,) else
  P_OPL_EAT(SETENV,,) else
  P_OPL_EAT(SLEEP,,) else
  P_OPL_EAT(SYSTEM,,) else
  P_OPL_EAT(TEST,,) else

#ifdef HAVE_SRM21
  P_OPL_EAT(srmAbortFiles,R,) else
  P_OPL_EAT(srmAbortRequest,R,) else
  P_OPL_EAT(srmChangeFileStorageType,R,) else
  P_OPL_EAT(srmCheckPermission,R,) else
  P_OPL_EAT(srmCompactSpace,R,) else
  P_OPL_EAT(srmCopy,R,) else
  P_OPL_EAT(srmExtendFileLifeTime,R,) else
  P_OPL_EAT(srmGetRequestID,R,) else
  P_OPL_EAT(srmGetRequestSummary,R,) else
  P_OPL_EAT(srmGetSpaceMetaData,R,) else
  P_OPL_EAT(srmGetSpaceToken,R,) else
  P_OPL_EAT(srmLs,R,) else
  P_OPL_EAT(srmMkdir,R,) else
  P_OPL_EAT(srmMv,R,) else
  P_OPL_EAT(srmPrepareToGet,R,) else
  P_OPL_EAT(srmPrepareToPut,R,) else
  P_OPL_EAT(srmPutDone,R,) else
  P_OPL_EAT(srmReassignToUser,R,) else
  P_OPL_EAT(srmReleaseFiles,R,) else
  P_OPL_EAT(srmReleaseSpace,R,) else
  P_OPL_EAT(srmRemoveFiles,R,) else
  P_OPL_EAT(srmReserveSpace,R,) else
  P_OPL_EAT(srmResumeRequest,R,) else
  P_OPL_EAT(srmRm,R,) else
  P_OPL_EAT(srmRmdir,R,) else
  P_OPL_EAT(srmSetPermission,R,) else
  P_OPL_EAT(srmStatusOfCopyRequest,R,) else
  P_OPL_EAT(srmStatusOfGetRequest,R,) else
  P_OPL_EAT(srmStatusOfPutRequest,R,) else
  P_OPL_EAT(srmSuspendRequest,R,) else
  P_OPL_EAT(srmUpdateSpace,R,) else
#endif	/* HAVE_SRM21 */

#ifdef HAVE_SRM22
  P_OPL_EAT(srmAbortFiles,R,) else
  P_OPL_EAT(srmAbortRequest,R,) else
  P_OPL_EAT(srmBringOnline,R,) else
  P_OPL_EAT(srmChangeSpaceForFiles,R,) else
  P_OPL_EAT(srmCheckPermission,R,) else
  P_OPL_EAT(srmCopy,R,) else
  P_OPL_EAT(srmExtendFileLifeTime,R,) else
  P_OPL_EAT(srmExtendFileLifeTimeInSpace,R,) else
  P_OPL_EAT(srmGetPermission,R,) else
  P_OPL_EAT(srmGetRequestSummary,R,) else
  P_OPL_EAT(srmGetRequestTokens,R,) else
  P_OPL_EAT(srmGetSpaceMetaData,R,) else
  P_OPL_EAT(srmGetSpaceTokens,R,) else
  P_OPL_EAT(srmGetTransferProtocols,R,) else
  P_OPL_EAT(srmLs,R,) else
  P_OPL_EAT(srmMkdir,R,) else
  P_OPL_EAT(srmMv,R,) else
  P_OPL_EAT(srmPing,R,) else
  P_OPL_EAT(srmPrepareToGet,R,) else
  P_OPL_EAT(srmPrepareToPut,R,) else
  P_OPL_EAT(srmPurgeFromSpace,R,) else
  P_OPL_EAT(srmPutDone,R,) else
  P_OPL_EAT(srmReleaseFiles,R,) else
  P_OPL_EAT(srmReleaseSpace,R,) else
  P_OPL_EAT(srmReserveSpace,R,) else
  P_OPL_EAT(srmResumeRequest,R,) else
  P_OPL_EAT(srmRm,R,) else
  P_OPL_EAT(srmRmdir,R,) else
  P_OPL_EAT(srmSetPermission,R,) else
  P_OPL_EAT(srmStatusOfBringOnlineRequest,R,) else
  P_OPL_EAT(srmStatusOfCopyRequest,R,) else
  P_OPL_EAT(srmStatusOfChangeSpaceForFilesRequest,R,) else
  P_OPL_EAT(srmStatusOfGetRequest,R,) else
  P_OPL_EAT(srmStatusOfLsRequest,R,) else
  P_OPL_EAT(srmStatusOfPutRequest,R,) else
  P_OPL_EAT(srmStatusOfReserveSpaceRequest,R,) else
  P_OPL_EAT(srmStatusOfUpdateSpaceRequest,R,) else
  P_OPL_EAT(srmSuspendRequest,R,) else
  P_OPL_EAT(srmUpdateSpace,R,) else
#endif	/* HAVE_SRM22 */

  P_OPL_ERR;

  /* parsing succeeded */
  return ERR_OK;

#undef P_OPL_EAT
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
  P_OPL_EQ_PARAM("overwrite",r->overwrite) else
  {
    col -= end - opt;
  }

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace before the ASSIGN variable, leave if comment char hit */

    P_DQ_PARAMv(dq_param,_val,r->var,"ASSIGN variable\n");
    DM_DBG(DM_N(3), "var=|%s|\n", _val.c_str());
  
    _val.clear();
    WS(); /* allow whitespace before the ASSIGN value */
    P_DQ_PARAMv(dq_param,_val,r->val,"ASSIGN value\n");
    DM_DBG(DM_N(3), "val=|%s|\n", _val.c_str());
  }

  /* parsing succeeded */
  return ERR_OK;
} /* ASSIGN */

int
Parser::DEFUN(void)
{
  TFunctions::iterator iter;	/* name/function object pair */
  std::string _val;
  int c;

  nDefun *r = new nDefun(parser_node);
  new_node = r;
  r->TYPE = N_DEFUN;		/* tell the defun process it should not evaluate its children */

  WS(); /* allow whitespace before name of the function */
  P_DQ_PARAM(dq_param,_val,r->name,"function name\n");

  if ((iter = gl_fun_tab.find(r->name->c_str())) != gl_fun_tab.end()) {
    /* function `name' already defined, issue a warning and re-define it */
    DM_WARN_P("function `%s' already defined, re-defining\n", r->name->c_str());
    iter->second = r;
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

    P_DQ_PARAMv(dq_param,_val,r->params,"function call by value parameter\n");
    DM_DBG(DM_N(3), "param=|%s|\n", _val.c_str());
  }

  /* : params_ref part */
  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace before call by reference values, leave if comment char hit */

    P_DQ_PARAMv(dq_param,_val,r->params_ref,"function call by reference parameter\n");
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
  P_DQ_PARAM(dq_param,_val,r->name,"function name\n");
  
  /* arguments */
  while(col < llen) {
    _val.clear();

    WS_COMMENT;	 /* allow whitespace before function arguments, leave if comment char hit */

    if((c = gc()) == ':') break;
    else ugc();

    P_DQ_PARAMv(dq_param,_val,r->args,"function argument\n");
    DM_DBG(DM_N(3), "arg=|%s|\n", _val.c_str());
  }

  /* : args_ref part */
  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace before call by reference values, leave if comment char hit */

    P_DQ_PARAMv(dq_param,_val,r->args_ref,"function return value\n");
    DM_DBG(DM_N(3), "retval=|%s|\n", _val.c_str());
  }

  return ERR_OK;
} /* FUN */

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

  P_DQ_PARAM(dq_param,_val,r->val,"NOP value\n");

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
  P_OPL_EQ_PARAM("overwrite",r->overwrite) else
  {
    col -= end - opt;
  }

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace before the ASSIGN variable, leave if comment char hit */

    P_DQ_PARAMv(dq_param,_val,r->var,"SETENV variable\n");
    DM_DBG(DM_N(3), "var=|%s|\n", _val.c_str());
  
    _val.clear();
    WS(); /* allow whitespace before the SETENV value */
    P_DQ_PARAMv(dq_param,_val,r->val,"SETENV value\n");
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
  P_DQ_PARAM(dq_param,_val,store,err_text);

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
  P_OPL_EQ_PARAM("out",r->out) else
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
  /* comments are handled within the <expr>ession itself */
  if(c == CH_EOL)
    /* we have an EOL (no TEST parameter, use the default value -- empty string) */
    goto empty;

  ugc();

  NEW_STR(r->expr, line + col, llen - col);
  col = llen;       /* make parser happy */
  DM_DBG(DM_N(3), "test=|%s|\n", r->expr->c_str());

  /* parsing succeeded */
  return ERR_OK;

empty:
  /* default TEST expression */
  NEW_STR(r->expr, "");

  /* parsing succeeded */
  return ERR_OK;
} /* TEST */

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
    DELETE(parser_node.REPEAT.X); DELETE(parser_node.REPEAT.Y); parser_node.init();
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
      DM_DBG(DM_N(5), "PREPROC: rval=%d/lval=%d\n", rval, lval);

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
