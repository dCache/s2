#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "str.h"

#include "i18.h"
#include "sysdep.h"     /* BOOL, STD_BUF, ... */
#include "errno.h"      /* errno, ERANGE, ... */

#include <stdarg.h>	/* va_start, ... */
#include <string.h>     /* strerror(), strlen(), ... */
#include <ctype.h>      /* isspace() */
#include <stdlib.h>     /* free() */
#include <limits.h>	/* (U)INT_MAX/MIN, ... */

/*
 * Returns pointer to a character `c' in string `s'.
 *   NULL:  no character `c' was found
 */
extern char *
str_char(const char *s, int c)
{
  int llen;
  int i;
  
  if(s == NULL)
    return NULL;

  llen = strlen(s);

  for(i = 0; i < llen; i++)
  {
    if(s[i] == c)
      return (char *)(s + i);
  }
    
  return NULL;
}

/*
 * Returns pointer to an unescaped character `c' in string `s'.
 *   NULL:  no unescaped character `c' was found
 */
extern char *
unescaped_char(const char *s, int c)
{
  BOOL esc = FALSE;       /* previous character was the '\\' character */
  int llen;
  int i;
  
  if(s == NULL)
    return NULL;

  llen = strlen(s);

  for(i = 0; i < llen; i++)
  {
    if(s[i] == '\\' && !esc) {
      esc = TRUE;
      continue;
    }
    if(s[i] == c && !esc)
      return (char *)(s + i);

    esc = FALSE;
  }
    
  return NULL;
}

/*
 * Remove leading whitespace from a string 
 */
extern int
remove_leading_ws(char *s)
{
  int i;
  int len;
  int offset;
  
  if(s == (char *)NULL)
    return 2;

  len = strlen(s);

  /* find the first non-whitespace character */
  for(offset = 0; offset < len && isspace(s[offset]); offset++)
    ;
  
  for(i = offset; i < len; i++) {
    s[i-offset] = s[i];
  }
  s[i-offset-1] = '\0';

  return 0;  
} /* remove_leading_ws */

/*
 * Remove trailing whitespace from a string 
 */
extern int
remove_trailing_ws(char *s)
{
  int i;
  
  if(s == (char *)NULL)
    return 2;

  i = strlen(s) - 1;

  for(; i >= 0 && isspace(s[i]); i--)
    s[i] = '\0';
    
  return 0;  
} /* remove_trailing_ws */

/*
 * Returns
 *   TRUE:  path is an absolute path
 *   FALSE: otherwise or path == NULL
 */
extern BOOL
is_absolute_path(const char *path)
{
  if(path == NULL)
    return FALSE;
  
  /* TODO: Other than *NIX system compatibility! */
  return (path[0] == PATH_CHAR);
}

/*
 * Returns
 *   TRUE:  s starts $[A-Z]*{
 *   false: otherwise
 */
inline BOOL
is_tag(const char *s)
{
  if(s == NULL || *s != '$')
    return FALSE;

  while(*++s != '\0')
  {
    if(IS_ASCII_ALPHA(*s)) continue;
    if(*s == '{') return TRUE;
    return FALSE;
  }
    
  return FALSE;
}

/*
 * Convert a number in a text to type and return the converted value.
 * A pointer to the last converted character is returned in *endptr.
 */
#define _GET_INT(sign,csign,d,size)\
extern sign##int##size##_t \
get_##sign##int##size(const char *word, char **endptr, BOOL warn)\
{\
  sign##int##64_t value = 0;\
\
  if(endptr == NULL) {\
    /* invalid use of function */\
    DM_ERR_ASSERT(_("endptr == NULL\n"));\
    return 0;\
  }\
\
  errno = 0;\
  value = strto##sign##ll(word, endptr, 0);\
\
  if(value < csign##INT##size##_MIN || value > csign##INT##size##_MAX) {\
    errno = ERANGE;\
    value = (sign##int##size##_t) value;        /* truncate */\
  }\
\
  if(errno == ERANGE) {\
    DM_ERR(ERR_ERR, _("truncating input value to %ll" #d "\n"), value);\
  }\
\
  if(word == *endptr && warn) {\
    DM_WARN(ERR_WARN, _("no integer characters converted from `%s', returning %ll" #d "\n"), word, value);\
  }\
\
  return (sign##int##size##_t)value;\
}

_GET_INT(,,d,);
_GET_INT(u,U,u,);
_GET_INT(,,d,8);
_GET_INT(u,U,u,8);
_GET_INT(,,d,16);
_GET_INT(u,U,u,16);
_GET_INT(,,d,32);
_GET_INT(u,U,u,32);
_GET_INT(,,d,64);
_GET_INT(u,U,u,64);
#undef _GET_INT

extern std::string
i2str(int64_t value)
{
  std::stringstream ss;
  ss << value << std::flush;
  return ss.str();
} /* i2str */

extern std::string
r2str(double value)
{
  std::stringstream ss;
  ss << value << std::flush;
  return ss.str();
} /* r2str */

extern std::string
ssprintf(const char *fmt...)
{
  int buf_size = 128;
  char buffer[buf_size+1];

  va_list arg_list;
  va_start(arg_list, fmt);

  int printed_chars = vsnprintf(buffer, buf_size, fmt, arg_list);

  if (printed_chars != -1 && printed_chars <= buf_size)
    return std::string(buffer);

  char* heap_buffer = NULL;

  while (printed_chars >= buf_size) {
    buf_size *= 2;
    heap_buffer = (char*)realloc(heap_buffer, buf_size + 1);
    printed_chars = vsnprintf(heap_buffer, buf_size, fmt, arg_list);
  }

  std::string result = std::string(heap_buffer);
  free(heap_buffer);

  va_end(arg_list);

  return result;
}

/*
 * If esc == TRUE create a string with escaped `c' characters 
 * unless already escaped by the backslash character '\\'.
 * 
 * e.g.: 
 * if esc && c == '"' => string: 'hello "world"' is printed as 'hello \"world\"'
 * if !esc            => string: 'hello "world"' is printed as 'hello "world"'
 */
extern std::string
escape_chars(const char* s, const char c, BOOL esc)
{
  DM_DBG_I;
  std::stringstream ss;
  BOOL bslash = FALSE;  /* we had the '\\' character */
  int llen;
  int i;

  if(s == NULL)
    return ss.str();

  llen = strlen(s);

  DM_DBG(DM_N(6), "complete string=|%s|\n", s);

  for(i = 0; i < llen; i++)
  {
    if(s[i] == '\\' && bslash) {
      /* two backslashes => no escaping */
      bslash = FALSE;
      goto out;
    }

    
    if(esc && s[i] == c && !bslash) {
      /* we need to escape unescaped double quotes */
      ss << '\\';
    }
    bslash = s[i] == '\\';
out:
    ss << s[i];
  }

  DM_DBG(DM_N(6), "escaped string=|%s|\n", ss.str().c_str());

  return ss.str();
}

/* 
 * Create a string parameter enclosed by double quotes depending on
 * characters in the string and its length.
 */
extern std::string
dq_param(const char *s, BOOL quote)
{
  DM_DBG_I;
  std::stringstream ss;
  BOOL q = FALSE;

  if(!quote) RETURN(std::string(s));

//#define DELIMIT_PARAM
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

  if(s == NULL) {
    ss << S2_NULL_STR;
    goto out;
  }

  if(!is_tag(s)) {
    q = str_char(s, ' ') != NULL ||	/* constains a space */
        str_char(s, '\t') != NULL ||	/* constains a tabulator */
        str_char(s, '\n') != NULL ||	/* constains a newline character */
        str_char(s, '\r') != NULL ||	/* constains a carriage return */
        *s == 0;			/* empty string */
  }

  if(q) ss << '"';

  ss << escape_chars(s, '"', q);

  if(q) ss << '"';

out:
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

#undef DELIMIT_PARAM    /* for debugging only */

  RETURN(ss.str());
} /* dq_param */

extern std::string
dq_param(const std::string &s, BOOL quote)
{
  if(!quote) return s.c_str();

  return dq_param(s.c_str(), quote);
}

extern std::string
dq_param(const std::string *s, BOOL quote)
{
  if(s == NULL) return std::string(S2_NULL_STR);

  if(!quote) return s->c_str();

  return dq_param(s->c_str(), quote);
}

extern std::string
dq_param(const bool b, BOOL quote)
{
  std::string s = b? std::string("1"): std::string("0");
  
  return s;
}

extern std::string
dq_param(const unsigned char c, BOOL quote)
{
  std::string s = i2str(c);
  
  return s;
}

/* 
 * Create a string parameter enclosed by double quotes depending on
 * characters in the string and its length.
 */
extern std::string
ind_param(const char *s)
{
  std::stringstream ss;

//#define DELIMIT_PARAM
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

  if(s == NULL) {
    ss << S2_NULL_STR;
    goto out;
  }

  ss << escape_chars(s, ']', TRUE);

out:
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

  return ss.str();

#undef DELIMIT_PARAM    /* for debugging only */
} /* ind_param */

extern std::string
ind_param(const std::string &s)
{
  return ind_param(s.c_str());
}

extern std::string
ind_param(const std::string *s)
{
  if(s == NULL) return std::string(S2_NULL_STR);

  return ind_param(s->c_str());
}

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
 * Returns the number of characters parsed.
 */
#if 0
extern int
get_dq_param(std::string &target, const char *source, BOOL &ws_only)
{
#define TERM_CHAR(c)    (dq? (c == '"'): IS_WHITE(c))
/* <= is important to make for gc/ugc() behaviour consistent; don't change to < */
#define gc(c)		(col <= source_len) ? source[col++] : '\0'
#define ugc()		if(col > 0) col--

  int i, c;
  unsigned col = 0;
  unsigned source_len;
  BOOL dq;		/* quotation mark at the start of a string */
  BOOL bslash = FALSE;	/* we had the '\\' character */
  
  if(source == NULL) {
    DM_ERR_ASSERT(_("source == NULL\n"));\
    return 0;
  }

  /* initialisation */
  ws_only = TRUE;	/* whitespace or empty string only */
  target.clear();
  source_len = strlen(source);

  DM_DBG(DM_N(5), "complete string=|%s|\n", source);
  do { c = gc(); } while(IS_WHITE(c));
  dq = (c == '"');
  DM_DBG(DM_N(6), "|%c|\n", c);
  if(c != '\0') ws_only = FALSE;
  if(!dq) ugc();
  DM_DBG(DM_N(5), "1) col=%d, dq=%d\n", col, dq);
  
  for(i = 0; (c = gc()) != '\0'; i++) {
    if(c == '\\' && bslash) {
      /* two backslashes => no quoting */
      bslash = FALSE;
      target.push_back(c);
      continue;
    }

    if(TERM_CHAR(c) && !bslash) {
      if(c != '"') ugc();

      DM_DBG(DM_N(5), "|%s|; col=|%d|\n", target.c_str(), col);
      return col;	/* found a string terminator */
    }

    if(!dq) {
      /* we have an unquoted string => remove escaping of whitespace and "s */
      if(c == '\\' && !bslash && 
         (IS_WHITE(source[col]) || source[col] == '"')) /* look ahead */
      {
        /* single backslash => the following character is escaped */
        goto esc_out;
      }
    }

    target.push_back(c);
esc_out:
    bslash = c == '\\';
  }

  if(dq && c == '\0')
    DM_WARN(ERR_WARN, "'\\0' terminated double-quoted parameter\n");

  return col - 1; /* do not count the terminating '\0' */

#undef TERM_CHAR
#undef gc
#undef ugc
} /* get_dq_param */
#else
extern int
get_dq_param(std::string &target, const char *source, BOOL &ws_only)
{
#define TERM_CHAR(c)    (dq? (c == '"'): (IS_WHITE(c) && (!tag || (tag && !brackets))))
/* <= is important to make for gc/ugc() behaviour consistent; don't change to < */
#define gc(c)		(col <= source_len) ? source[col++] : '\0'
#define ugc()		if(col > 0) col--

  int i, c;
  unsigned col = 0;
  unsigned source_len;
  BOOL dq;		/* quotation mark at the start of a string */
  BOOL bslash = FALSE;	/* we had the '\\' character */
  BOOL string = FALSE;	/* we had an opening " */
  BOOL tag;		/* $[A-Za-z]{ */
  int brackets = 0;
  
  if(source == NULL) {
    DM_ERR_ASSERT(_("source == NULL\n"));\
    return 0;
  }

  /* initialisation */
  ws_only = TRUE;	/* whitespace or empty string only */
  target.clear();
  source_len = strlen(source);

  DM_DBG(DM_N(5), "complete string=|%s|\n", source);
  do { c = gc(); } while(IS_WHITE(c));
  dq = (c == '"');
//  tag = is_tag(source + col);
  tag = c == '$';	/* speed things up */
  DM_DBG(DM_N(6), "|%c|\n", c);
  if(c != '\0') ws_only = FALSE;
  if(!dq) ugc();
  DM_DBG(DM_N(5), "1) col=%d, dq=%d\n", col, dq);
  
  for(i = 0; (c = gc()) != '\0'; i++) {
    DM_DBG(DM_N(5), "'%c'; bslash=%d, string=%d, brackets=%d\n", c, bslash, string, brackets);
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

    DM_DBG(DM_N(5), "'%c'; bslash=%d, string=%d, brackets=%d\n", c, bslash, string, brackets);

    if(TERM_CHAR(c) && !bslash) {
      if(c != '"') ugc();

      DM_DBG(DM_N(5), "|%s|; col=|%d|\n", target.c_str(), col);
      return col;	/* found a string terminator */
    }

    if(c == '"') {
      string = string? FALSE: TRUE;
      target.push_back(c);
      continue;
    }

    if(!dq) {
      /* we have an unquoted string => remove escaping of whitespace and "s */
      if(c == '\\' && !bslash && 
         (IS_WHITE(source[col]) || source[col] == '"')) /* look ahead */
      {
        /* single backslash => the following character is escaped */
        goto esc_out;
      }
    }
    
    target.push_back(c);
esc_out:
    bslash = c == '\\';
  }

  if(dq && c == '\0')
    DM_WARN(ERR_WARN, "'\\0' terminated double-quoted parameter\n");

  return col - 1; /* do not count the terminating '\0' */

#undef gc
#undef ugc
} /* get_dq_param */

#endif

/*
 * See 3 parameter get_dq_param above.
 */
extern int
get_dq_param(std::string &target, const char *source)
{
  BOOL ws_only;
  return get_dq_param(target, source, ws_only);
} /* get_dq_param */

/*
 * Get a parameter enclosed in curly (ballanced) brackets and ignore brackets in
 * double quoted strings.
 */
extern int
get_ballanced_br_param(std::string &target, const char *source)
{
/* <= is important to make for gc/ugc() behaviour consistent; don't change to < */
#define gc(c)		(col <= source_len) ? source[col++] : '\0'
#define ugc()		if(col > 0) col--

  int i, c;
  unsigned col = 0;
  unsigned source_len;
  BOOL bslash = FALSE;	/* we had the '\\' character */
  BOOL string = FALSE;	/* we had an opening " */
  int brackets = 1;
  
  if(source == NULL) {
    DM_ERR_ASSERT(_("source == NULL\n"));\
    return 0;
  }

  /* initialisation */
  target.clear();
  source_len = strlen(source);

  DM_DBG(DM_N(5), "complete string=|%s|\n", source);
  
  for(i = 0; (c = gc()) != '\0'; i++) {
    DM_DBG(DM_N(5), "'%c'; bslash=%d, string=%d\n", c, bslash, string);
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
    
    if(!brackets) {
      DM_DBG(DM_N(5), "|%s|; col=|%d|\n", target.c_str(), col);
      return col;	/* found a string terminator */
    }

    if(c == '"') {
      string = string? FALSE: TRUE;
    }

    target.push_back(c);
    bslash = c == '\\';
  }

  if(c == '\0')
    DM_WARN(ERR_WARN, "'\\0' terminated {} parameter\n");

  return col - 1; /* do not count the terminating '\0' */

#undef gc
#undef ugc
} /* get_ballanced_br_param */
