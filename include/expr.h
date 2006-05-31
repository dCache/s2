#ifndef _EXPR_H
#define _EXPR_H

#include "lex.h"

struct Expr {
  Process *proc;	/* pointer to the Process with relevant variable tables, etc. */

public:
  Symbol sym;           /* current symbol */
  Attr   lex_attr;      /* symbol attribute */

  Expr();
  Expr(const char *s, Process *p);
  ~Expr();

  Attr parse();

private:
  Lex l;
  std::vector <std::string *> strings;

  void expect(Symbol s);

  Types ConvTypes(Attr &a1, Attr &a2);
  int compare(Attr a1, Attr a2, Symbol o);
  void normalize(Attr &attr);

  /* grammar */
  Attr S();
  Attr X();
  Attr A();
  Attr A1(Attr iattr);
  Attr B();
  Attr B1(Attr iattr);
  Attr C();
  Attr C1(Attr iattr);
  Attr D();
  Attr D1(Attr iattr);
  Attr E();
  Attr E1(Attr iattr);
  Attr F();
  Attr F1(Attr iattr);
  Attr G();
  Attr G1(Attr iattr);
  Attr H();
  Attr H1(Attr iattr);
  Attr I();
  Attr I1(Attr iattr);
  Attr J();
  Attr J1(Attr iattr);
  Attr K();
  Attr K1();
  Attr L();
};

#endif /* _EXPR_H */
