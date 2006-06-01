#ifndef _STR_H
#define _STR_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include "constants.h"
#include "sysdep.h"             /* BOOL */

/* simple macros */
#define IS_WHITE(c)     ((c)=='\t' || (c)=='\n' || (c)=='\f' || (c)=='\r' || (c)==' ')
#define IS_PERL_WHITE(c)\
                        ((c)==0x09 || (c)==0x0A || (c)==0x0C || (c)==0x0D || (c)==' ')
#define IS_POSIX_WHITE(c)\
                        ((c)==0x09 || (c)==0x0A || (c)==0x0B || (c)==0x0C || (c)==0x0D || (c)==' ')

/* private function declarations */

/* extern(al) function declarations */
extern char *str_char(const char *s, int c);
extern char *unescaped_char(const char *s, int c);
extern int remove_leading_ws(char *s);
extern int remove_trailing_ws(char *s);
extern BOOL is_absolute_path(const char *path);
#define _GET_INT(sign,size)\
extern sign##int##size##_t get_##sign##int##size(const char *word, char **endptr, BOOL warn);
_GET_INT(,8);
_GET_INT(u,8);
_GET_INT(,16);
_GET_INT(u,16);
_GET_INT(,32);
_GET_INT(u,32);
_GET_INT(,64);
_GET_INT(u,64);
#undef _GET_INT

/* C++ utils */
#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* std::stringstream, ... */
extern std::string i2str(int64_t value);
extern std::string r2str(double value);
extern std::string ssprintf(const char *fmt...);
extern std::string escape_chars(const char* s, const char c, BOOL q);
extern int get_dq_param(std::string &target, const char *source);
extern int get_ballanced_br_param(std::string &target, const char *source);

#endif /* _STR_H */
