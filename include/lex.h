#ifndef _LEX_H
#define _LEX_H

enum Symbol {
// EOF     +        -         *        /       %
  EofSym, PlusSym, MinusSym, MultSym, DivSym, ModSym,
// (       )
  LprSym, RprSym,
// <<      >>
  ShlSym, ShrSym,
// <      <=      >     >=    =,        ==,     !       !=
  LtSym, LeSym, GtSym, GeSym, AsgnSym, EqSym, NotSym, NeSym,
// ~
  BitNotSym,
// |         ||
  BitOrSym, OrSym,
// &          &&
  BitAndSym, AndSym,
// INT     REAL
  IntSym, RealSym
};

enum types {
  NONE, INT, REAL,
  COUNT /* number of types */
};

struct Attr {
  types type;		/* type    */
  union {
    int64_t i;		/* integer */
    double  r;		/* real    */
  } v;			/* value   */
};

class Lex {
public:
  Lex();
  Lex(const char *s);
  ~Lex();

  Symbol lex(Attr& attr);
  static const char* SymbolName(Symbol s);
  char gc(void);
  void ugc(void);

private:
  const char *expr;
  int i_expr;

};

#endif /* _LEX_H */
