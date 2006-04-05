#ifndef _EXPR_H
#define _EXPR_H

#include "lex.h"

class Expr {
public:
  Symbol sym;           /* current symbol */
  Attr   lex_attr;      /* symbol attribute */

  Expr();
  Expr(const char *);
  ~Expr();

  int parse(int64_t *e);

private:
  Lex l;

  void expect(Symbol s);

  types ConvTypes(Attr &a1, Attr &a2);
  int compare(Attr a1, Attr a2, Symbol o);
  Symbol lex();

  /* grammar */
  Attr S();
  Attr E();
  Attr R();
  Attr R1(Attr iattr);
  Attr A();
  Attr A1(Attr iattr);
  Attr B();
  Attr B1(Attr iattr);
  Attr Z();
};

#endif /* _EXPR_H */
