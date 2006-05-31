#ifndef _LEX_H
#define _LEX_H

#include "process.h"		/* Process */

/* C++ utils */
#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* std::stringstream, ... */

enum Symbol {
// EOF    INV_SYM
  EofSym, InvalidSym,
// +        -         *        /       %
  PlusSym, MinusSym, MultSym, DivSym, ModSym,
// (       )
  LprSym, RprSym,
// <<      >>
  ShlSym, ShrSym,
// <      <=      >     >=    =,        ==,     !       !=
  LtSym, LeSym, GtSym, GeSym, AsgnSym, EqSym, NotSym, NeSym,
// ^
  BitXorSym,
// ~
  BitNotSym,
// |         ||
  BitOrSym, OrSym,
// &          &&
  BitAndSym, AndSym,
// INT     REAL    STRING
  IntSym, RealSym, StringSym
};

enum Types {
  INV, INT, REAL, STRING,
  COUNT /* number of types */
};

struct Attr {
public:
  Types type;		/* type    */
  union {
    int64_t	i;	/* integer */
    double	r;	/* real    */
    std::string	*s;	/* string  */
  } v;			/* value   */

  Attr();
  ~Attr();
  std::string toString();

};

struct Lex {
public:
  Lex();
  Lex(const char *s);
  ~Lex();

  Symbol lex(Attr& attr);
  static const char* SymbolName(Symbol s);
  char gc(void);
  void ugc(void);
  BOOL eof(void);

private:
  const char *source;
  int col;
  int source_len;
  std::vector <std::string *> strings;

};

#endif /* _LEX_H */
