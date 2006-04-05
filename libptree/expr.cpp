#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include <stdlib.h>

#include "expr.h"

#include "i18.h"
#include "constants.h"
#include "sysdep.h"

/* constructor */
Expr::Expr()
{
}


/* constructor */
Expr::Expr(const char *s)
{
  l = Lex(s);
}


/* destructor */
Expr::~Expr()
{
}


int
Expr::parse(int64_t *e)
{
  Attr attr;
  attr = S();
  *e = attr.v.i;
  return 0;
}


Symbol
Expr::lex()
{
  sym = l.lex(lex_attr);
  return sym;
}


void
Expr::expect(Symbol s)
{
  if (s == sym)
    lex();
  else {
    DM_ERR(ERR_ERR, _("Expected %s, found %s\n"), Lex::SymbolName(s), Lex::SymbolName(sym));
  }
}


types Expr::ConvTypes(Attr& a1, Attr& a2)
{
  static types table[COUNT][COUNT] = {
    /*            a2:  NONE,    INT,   REAL */
    /* a1:                                  */
    /* NONE   */      {NONE,   NONE,   NONE},
    /* INT    */      {NONE,    INT,   REAL},
    /* REAL   */      {NONE,   REAL,   REAL},
  };
  types type;

  type = table[a1.type][a2.type];
  if (type == REAL) {			/* conversion int na real */
    if (a1.type == INT) {		/* conversion a1 (int -> real) */
      a1.type = REAL;
      a1.v.r = a1.v.i;
    }
    else
    if (a2.type == INT) {		/* conversion a2 (int -> real) */
      a2.type = REAL;
      a2.v.r  = a2.v.i;
    }
  }
  return type;
}


int
Expr::compare(Attr a1, Attr a2, Symbol o)
{
  switch (ConvTypes(a1, a2)) {
    case INT:
      switch (o) {
        case EqSym: return a1.v.i == a2.v.i;
        case LtSym: return a1.v.i <  a2.v.i;
        case GtSym: return a1.v.i >  a2.v.i;
        case NeSym: return a1.v.i != a2.v.i;
        case LeSym: return a1.v.i <= a2.v.i;
        case GeSym: return a1.v.i >= a2.v.i;
        default:
          DM_ERR_ASSERT(_("switch: default\n"));
      }
    break;

    case REAL:
      switch (o) {
        case EqSym: return a1.v.r == a2.v.r;
        case LtSym: return a1.v.r <  a2.v.r;
        case GtSym: return a1.v.r >  a2.v.r;
        case NeSym: return a1.v.r != a2.v.r;
        case LeSym: return a1.v.r <= a2.v.r;
        case GeSym: return a1.v.r >= a2.v.r;
        default:
          DM_ERR_ASSERT(_("switch: default\n"));
      }
    break;

    default:
      DM_ERR(ERR_ERR, _("Cannot compare values of incompatible types\n"));
  }

  return 0; /* unable to compare */
}


/*** The Grammar ****************************************************/
/* S -> E */
Attr
Expr::S()
{
  DM_DBG_I;
  Attr attr;

  lex();
  attr = E();
  RETURN(attr);
}


/* E -> R */
Attr
Expr::E()
{
  DM_DBG_I;
  Attr attr;

  attr = R();
  RETURN(attr);
}


/* R -> A R1 */
Attr
Expr::R()
{
  DM_DBG_I;

  Attr attr;

  attr = A();
  attr = R1(attr);

  RETURN(attr);
}


/* R1 -> RelOp A R1 | e */
Attr
Expr::R1(Attr iattr)
{
#define IS_RELOP(sym) ((sym) == LtSym || (sym) == LeSym || (sym) == GtSym || (sym) == GeSym || (sym) == AsgnSym || (sym) == EqSym || (sym) == NotSym || (sym) == NeSym)
  DM_DBG_I;

  Attr attr;
  if (IS_RELOP(sym)) {			/* R1 -> RelOp A R1 */
    Symbol PomSym = sym;
    lex();
    attr = A();
    iattr.v.i = compare(iattr, attr, PomSym);
    iattr.type = INT;
    attr = R1(iattr);
  }
  else {				/* R1 -> e */
    DM_DBG(DM_N(3), "A1 -> e\n");
    RETURN(iattr);
  }

  RETURN(attr);

#undef IS_RELOP
}


/* A -> B A1 */
Attr
Expr::A()
{
  DM_DBG_I;

  Attr attr;

  attr = B();
  attr = A1(attr);

  RETURN(attr);
}


/* A1 -> + B A1 | - B A1 | '||' B A1 | e */
Attr
Expr::A1(Attr iattr)
{
  DM_DBG_I;

  Attr attr;

  if (sym == PlusSym) {			/* A1 -> + B A1 */
    lex();
    attr = B();
    switch (ConvTypes(iattr, attr)) {
      case INT:     iattr.v.i += attr.v.i; break;
      case REAL:    iattr.v.r += attr.v.r; break;

      default: DM_ERR(ERR_ERR, _("Addition is not supported for these types\n"));
    }
    attr = A1(iattr);
  }
  else
  if (sym == MinusSym) {		/* A1 -> - B A1 */
    lex();
    attr = B();
    switch (ConvTypes(iattr, attr)) {
      case INT:     iattr.v.i -= attr.v.i; break;
      case REAL:    iattr.v.r -= attr.v.r; break;

      default: DM_ERR(ERR_ERR, _("Subtraction is not supported for these types\n"));
    }
    attr = A1(iattr);
  }
  else
  if (sym == ShlSym) {			/* A1 -> << B A1 */
    lex();
    attr = B();
    switch (ConvTypes(iattr, attr)) {
      case INT:     iattr.v.i <<= attr.v.i; break;

      default: DM_ERR(ERR_ERR, _("SHL is not supported for these types\n"));
    }
    attr = A1(iattr);
  }
  else
  if (sym == ShrSym) {			/* A1 -> >> B A1 */
    lex();
    attr = B();
    switch (ConvTypes(iattr, attr)) {
      case INT:     iattr.v.i >>= attr.v.i; break;

      default: DM_ERR(ERR_ERR, _("SHR is not supported for these types\n"));
    }
    attr = A1(iattr);
  }
  else
  if (sym == OrSym) {			/* A1 -> || B A1 */
    lex();
    attr = B();
    switch (ConvTypes(iattr, attr)) {
      case INT: iattr.v.i |= attr.v.i;
      break;

      default: DM_ERR(ERR_ERR, _("The || operator requires INT type values\n"));
    }
    attr = A1(iattr);
  }
  else {				/* A1 -> e */
    DM_DBG(DM_N(3), "A1 -> e\n");
    RETURN(iattr);
  }

  RETURN(attr);
}


/* B -> Z B1 */
Attr
Expr::B()
{
  DM_DBG_I;

  Attr attr;

  attr = Z();
  attr = B1(attr);

  RETURN(attr);
}


/* B1 -> * Z B1 | / Z B1 | % Z B1 | && Z B1 | e */
Attr
Expr::B1(Attr iattr)
{
  DM_DBG_I;

  Attr attr;

  if (sym == MultSym) {			/* B1 -> * Z B1 */
    lex();
    attr = Z();
    switch (ConvTypes(iattr, attr)) {
      case INT:  iattr.v.i *= attr.v.i; break;
      case REAL: iattr.v.r *= attr.v.r; break;

      default: DM_ERR(ERR_ERR, _("Division is not supported for these types\n"));
    }
    attr = B1(iattr);
  }
  else
  if (sym == DivSym) {			/* B1 -> / Z B1 */
    lex();
    attr = Z();
    switch (ConvTypes(iattr, attr)) {
      case INT:
        if (attr.v.i) {			/* division INT/INT => REAL */
          iattr.v.r = iattr.v.i;
          iattr.type = REAL;
          iattr.v.r /= attr.v.i;
        }
        else {
          iattr.v.i = 0;
          DM_ERR(ERR_ERR, _("Division by zero\n"));
        };
      break;

      case REAL:
        if (attr.v.r) iattr.v.r /= attr.v.r;
        else {
          iattr.v.r = 0.0;
          DM_ERR(ERR_ERR, _("Division by zero\n"));
        };
      break;

      default: DM_ERR(ERR_ERR, _("Division is not supported for these types\n"));
    }
    attr = B1(iattr);
  }
  else
  if (sym == ModSym) {			/* B1 -> % Z B1 */
    lex();
    attr = Z();
    switch (ConvTypes(iattr, attr)) {
      case INT: 
        if (attr.v.i) iattr.v.i %= attr.v.i;
        else {
          iattr.v.i = 0;
          DM_ERR(ERR_ERR, _("Division by zero\n"));
        };
      break;

      default: DM_ERR(ERR_ERR, _("Division is not supported for these types\n"));
    }
    attr = B1(iattr);
  }
  else
  if (sym == AndSym) {			/* B1 -> && Z B1 */
    lex();
    attr = Z();
    switch (ConvTypes(iattr, attr)) {
      case INT: iattr.v.i &= attr.v.i;
      break;

      default: DM_ERR(ERR_ERR, _("The && operator requires INT type values\n"));
    }
    attr = B1(iattr);
  }
  else {				/* B1 -> e */
    DM_DBG(DM_N(3), "B1 -> e\n");
    return iattr;
  }
  
  RETURN(attr);
}


/* Z -> ( Z ) | + Z | - Z | ! Z | INT | REAL */
Attr
Expr::Z()
{
  DM_DBG_I;

  Attr attr;

  attr.type = NONE;
  if (sym == LprSym) {			/* Z -> ( E ) */
    lex();
    attr = E();
    expect(RprSym);
  }
  else
  if (sym == PlusSym) {			/* Z -> + Z */
    lex();				/* unary plus */
    attr = Z();
    switch (attr.type) {
      case INT:
      case REAL: break;

      default: DM_ERR(ERR_ERR, _("Illegal use of unary minus\n"));
    }
  }
  else
  if (sym == MinusSym) {		/* Z -> - Z */
    lex();				/* unary minus */
    attr = Z();
    switch (attr.type) {
      case INT:  attr.v.i = -attr.v.i; break;
      case REAL: attr.v.r = -attr.v.r; break;

      default: DM_ERR(ERR_ERR, _("Illegal use of unary minus\n"));
    }
  }
  else
  if (sym == NotSym) {			/* Z -> ! Z */
    lex();
    attr = Z();
    switch (attr.type) {
      case INT: attr.v.i = attr.v.i ? FALSE : TRUE;
      break;

      default: DM_ERR(ERR_ERR, _("The ! operator requires INT type values\n"));
    }
  }
  else
  if (sym == IntSym) {			/* Z -> INT */
    attr.type = INT;
    attr.v.i = lex_attr.v.i;		/* get the value from lex */
    lex();
  }
  else
  if (sym == RealSym) {			/* Z -> REAL */
    attr.type = REAL;
    attr.v.r = lex_attr.v.r;		/* get the value from lex */
    lex();
  }
  RETURN(attr);
}

/*
 * C-like operator priority table 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * LOWEST
 * ==============================
 * ||
 * &&
 * |
 * ^
 * &
 * == !=
 * < <= > >=
 * << >>
 * + -
 * * / %
 * + - ! ~                (unary)
 * ()
 * ==============================
 * HIGHEST
 */
