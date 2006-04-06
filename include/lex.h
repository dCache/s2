#ifndef _LEX_H
#define _LEX_H

enum Symbol {
// EOF     INV_SYM
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
// INT     REAL
  IntSym, RealSym
};

enum types {
  INV, INT, REAL,
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
  int expr_col;

};

#endif /* _LEX_H */
