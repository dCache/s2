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

#include <string.h>     /* strerror(), strlen(), ... */
#include <ctype.h>      /* isspace() */
#include <stdlib.h>     /* free() */

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
  BOOL q = FALSE;       /* previous character was the '\\' character */
  int llen;
  int i;
  
  if(s == NULL)
    return NULL;

  llen = strlen(s);

  for(i = 0; i < llen; i++)
  {
    if(s[i] == '\\' && !q) {
      q = TRUE;
      continue;
    }
    if(s[i] == c && !q)
      return (char *)(s + i);

    q = FALSE;
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
    DM_WARN(ERR_WARN, _("no integer characters converted, returning %ll" #d "\n"), value);\
  }\
\
  return (sign##int##size##_t)value;\
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

extern std::string
i2str(int64_t value)
{
  std::stringstream ss;
  ss << value << std::flush;
  return ss.str();
} /* i2str */

/*
 * If q == TRUE create a string with escaped `c' characters 
 * unless already escaped by the backslash character '\\'.
 * 
 * e.g.: 
 * if q && c == '"' => string: 'hello "world"' is printed as 'hello \"world\"'
 * if !q            => string: 'hello "world"' is printed as 'hello "world"'
 */
extern std::string
escape_chars(const char* s, const char c, BOOL q)
{
  std::stringstream ss;
  BOOL bslash = FALSE;  /* we had the '\\' character */
  int llen;
  int i;
  
  if(s == NULL)
    return ss.str();

  llen = strlen(s);

  for(i = 0; i < llen; i++)
  {
    if(s[i] == '\\' && bslash) {
      /* two backslashes => no quoting */
      bslash = FALSE;
      goto out;
    }

    if(q && s[i] == c && !bslash) {
      /* we need to escape unescaped double quotes */
      ss << '\\';
    }
    bslash = s[i] == '\\';
out:
    ss << s[i];
  }

  return ss.str();
}
