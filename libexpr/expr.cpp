#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include <stdlib.h>

#include "expr.h"

#include "i18.h"
#include "constants.h"
#include "sysdep.h"

/* simple macros */
#define EXPECT(s)\
  if (s == sym)\
    LEX();\
  else {\
    DM_ERR(ERR_ERR, _("Expected: %s; found: %s\n"), Lex::SymbolName(s), Lex::SymbolName(sym));\
  }

#define LEX()	do { if((sym = l.lex(lex_attr)) == InvalidSym) return attr; } while(0)

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

  switch (attr.type) {
    case INT:
      *e = attr.v.i;
    break;

    case REAL:
      *e = (int64_t)attr.v.r;
      if(*e != attr.v.r) {
        DM_WARN(ERR_WARN, "Truncating %f to %lld\n", attr.v.r, *e);
      }
    break;

    default:
      DM_ERR(ERR_ERR, _("Expression evaluation did not return a number.\n"));
      return ERR_ERR;
  }

  return ERR_OK;
}


types Expr::ConvTypes(Attr& a1, Attr& a2)
{
  static types table[COUNT][COUNT] = {
    /*            a2:  INV,    INT,   REAL */
    /* a1:                                 */
    /* INV   */       {INV,   INV,    INV},
    /* INT    */      {INV,   INT,    REAL},
    /* REAL   */      {INV,   REAL,   REAL},
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
          DM_ERR_ASSERT(_("switch: default: %s\n"), Lex::SymbolName(o));
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
          DM_ERR_ASSERT(_("switch: default: %s\n"), Lex::SymbolName(o));
      }
    break;

    default:
      DM_ERR(ERR_ERR, _("Cannot compare values of incompatible types\n"));
  }

  return 0; /* unable to compare */
}


/*** The Grammar ****************************************************/
#define RULE(a,b,c) \
Attr \
Expr::a()\
{\
  DM_DBG_I;\
\
  Attr attr;\
\
  attr = b();\
  attr = c(attr);\
\
  RETURN(attr);\
}

/* S -> X */
Attr
Expr::S()
{
  DM_DBG_I;
  Attr attr;

  LEX();
  attr = X();
  RETURN(attr);
}


/* X -> B A1 */
RULE(X,B,A1);

/* A1 -> '||' B A1 | e */
Attr
Expr::A1(Attr iattr)
{
  DM_DBG_I;
  Attr attr;

  if (sym == OrSym) {			/* A1 -> || B A1 */
    LEX();
    attr = B();
    switch (ConvTypes(iattr, attr)) {
      case INT: iattr.v.i = iattr.v.i || attr.v.i;
      break;

      default: DM_ERR(ERR_ERR, _("The %s operator requires INT type values\n"), Lex::SymbolName(sym));
    }
    attr = A1(iattr);
  }
  else {				/* A1 -> e */
    DM_DBG(DM_N(5), "A1 -> e\n");
    RETURN(iattr);
  }

  RETURN(attr);
}


/* B -> C B1 */
RULE(B,C,B1);

/* B1 -> '&&' C B1 | e */
Attr
Expr::B1(Attr iattr)
{
  DM_DBG_I;
  Attr attr;

  if (sym == AndSym) {			/* B1 -> '&&' C B1 */
    LEX();
    attr = C();
    switch (ConvTypes(iattr, attr)) {
      case INT: iattr.v.i = iattr.v.i && attr.v.i;
      break;

      default: DM_ERR(ERR_ERR, _("The %s operator requires INT type values\n"), Lex::SymbolName(sym));
    }
    attr = B1(iattr);
  }
  else {				/* B1 -> e */
    DM_DBG(DM_N(5), "B1 -> e\n");
    RETURN(iattr);
  }

  RETURN(attr);
}


/* C -> D C1 */
RULE(C,D,C1);

/* C1 -> '|' D C1 | e */
Attr
Expr::C1(Attr iattr)
{
  DM_DBG_I;
  Attr attr;

  if (sym == BitOrSym) {		/* C1 -> '|' D C1 */
    LEX();
    attr = D();
    switch (ConvTypes(iattr, attr)) {
      case INT: iattr.v.i |= attr.v.i;
      break;

      default: DM_ERR(ERR_ERR, _("The %s operator requires INT type values\n"), Lex::SymbolName(sym));
    }
    attr = C1(iattr);
  }
  else {				/* C1 -> e */
    DM_DBG(DM_N(5), "C1 -> e\n");
    RETURN(iattr);
  }

  RETURN(attr);
}


/* D -> E D1 */
RULE(D,E,D1);

/* D1 -> ^ E D1 | e */
Attr
Expr::D1(Attr iattr)
{
  DM_DBG_I;
  Attr attr;

  if (sym == BitXorSym) {		/* D1 -> ^ E D1 */
    LEX();
    attr = E();
    switch (ConvTypes(iattr, attr)) {
      case INT: iattr.v.i ^= attr.v.i;
      break;

      default: DM_ERR(ERR_ERR, _("The %s operator requires INT type values\n"), Lex::SymbolName(sym));
    }
    attr = D1(iattr);
  }
  else {				/* D1 -> e */
    DM_DBG(DM_N(5), "D1 -> e\n");
    RETURN(iattr);
  }

  RETURN(attr);
}


/* E -> F E1 */
RULE(E,F,E1);

/* E1 -> & F E1 | e */
Attr
Expr::E1(Attr iattr)
{
  DM_DBG_I;
  Attr attr;

  if (sym == BitAndSym) {		/* E1 -> & F E1 */
    LEX();
    attr = F();
    switch (ConvTypes(iattr, attr)) {
      case INT: iattr.v.i &= attr.v.i;
      break;

      default: DM_ERR(ERR_ERR, _("The %s operator requires INT type values\n"), Lex::SymbolName(sym));
    }
    attr = E1(iattr);
  }
  else {				/* E1 -> e */
    DM_DBG(DM_N(5), "E1 -> e\n");
    RETURN(iattr);
  }

  RETURN(attr);
}


/* F -> G F1 */
RULE(F,G,F1);

/* F1 -> == G F1 | != G F1 | e */
Attr
Expr::F1(Attr iattr)
{
#define IS_CMPSYM(sym) ((sym) == EqSym || (sym) == NeSym)
  DM_DBG_I;
  Attr attr;

  if (IS_CMPSYM(sym)) {			/* F1 -> == G F1 | != G F1 */
    Symbol PomSym = sym;
    LEX();
    attr = G();
    iattr.v.i = compare(iattr, attr, PomSym);
    iattr.type = INT;
    attr = F1(iattr);
  }
  else {				/* F1 -> e */
    RETURN(iattr);
  }

  RETURN(attr);
#undef IS_CMPSYM
}


/* G -> H G1 */
RULE(G,H,G1);

/* G1 -> < H G1 | <= H G1 | > H G1 | >= H G1 | e */
Attr
Expr::G1(Attr iattr)
{
#define IS_CMPSYM(sym) ((sym) == LtSym || (sym) == LeSym || (sym) == GtSym || (sym) == GeSym)
  DM_DBG_I;
  Attr attr;

  if (IS_CMPSYM(sym)) {			/* G1 -> < H G1 | <= H G1 | > H G1 | >= H G1 */
    Symbol PomSym = sym;
    LEX();
    attr = H();
    iattr.v.i = compare(iattr, attr, PomSym);
    iattr.type = INT;
    attr = F1(iattr);
  }
  else {				/* G1 -> e */
    RETURN(iattr);
  }

  RETURN(attr);
#undef IS_CMPSYM
}


/* H -> I H1 */
RULE(H,I,H1);

/* H1 -> << I H1 | >> I H1 | e */
Attr
Expr::H1(Attr iattr)
{
  DM_DBG_I;
  Attr attr;

  switch(sym) {
    /* H1 -> << I H1 */
    case ShlSym:
      LEX();
      attr = I();
      switch (ConvTypes(iattr, attr)) {
        case INT: iattr.v.i <<= attr.v.i;
        break;
  
        default: DM_ERR(ERR_ERR, _("The %s operator requires INT type values\n"), Lex::SymbolName(sym));
      }
      attr = H1(iattr);
    break;

    /* H1 -> >> I H1 */
    case ShrSym:
      LEX();
      attr = I();
      switch (ConvTypes(iattr, attr)) {
        case INT: iattr.v.i >>= attr.v.i;
        break;
  
        default: DM_ERR(ERR_ERR, _("The %s operator requires INT type values\n"), Lex::SymbolName(sym));
      }
      attr = H1(iattr);
    break;

    /* H1 -> e */
    default:
      DM_DBG(DM_N(5), "H1 -> e\n");
      RETURN(iattr);
  }

  RETURN(attr);
}

/* I -> J I1 */
RULE(I,J,I1);

/* I1 -> + J I1 | - J I1 | e */
Attr
Expr::I1(Attr iattr)
{
  DM_DBG_I;
  Attr attr;

  switch(sym) {
    /* I1 -> + J I1 */
    case PlusSym:
      LEX();
      attr = J();
      switch (ConvTypes(iattr, attr)) {
        case INT:     iattr.v.i += attr.v.i; break;
        case REAL:    iattr.v.r += attr.v.r; break;
  
        default: DM_ERR(ERR_ERR, _("Addition is not supported for these types\n"));
      }
      attr = I1(iattr);
    break;

    /* I1 -> - J I1 */
    case MinusSym:
      LEX();
      attr = J();
      switch (ConvTypes(iattr, attr)) {
        case INT:     iattr.v.i -= attr.v.i; break;
        case REAL:    iattr.v.r -= attr.v.r; break;
  
        default: DM_ERR(ERR_ERR, _("Subtraction is not supported for these types\n"));
      }
      attr = I1(iattr);
    break;

    /* I1 -> e */
    default:
      DM_DBG(DM_N(5), "I1 -> e\n");
      RETURN(iattr);
  }

  RETURN(attr);
}

/* J -> K J1 */
RULE(J,K,J1);

/* J1 -> * K J1 | / K J1 | % K J1 | e */
Attr
Expr::J1(Attr iattr)
{
  DM_DBG_I;
  Attr attr;

  switch(sym) {
    /* J1 -> * K J1 */
    case MultSym:
      LEX();
      attr = K();
      switch (ConvTypes(iattr, attr)) {
        case INT:     iattr.v.i *= attr.v.i; break;
        case REAL:    iattr.v.r *= attr.v.r; break;
  
        default: DM_ERR(ERR_ERR, _("Multiplication is not supported for these types\n"));
      }
      attr = J1(iattr);
    break;

    /* J1 -> / K J1 */
    case DivSym:
      LEX();
      attr = K();
      switch (ConvTypes(iattr, attr)) {
        case INT:
          if (attr.v.i) {		/* division INT/INT => REAL */
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
      attr = J1(iattr);
    break;

    /* J1 -> % K J1 */
    case ModSym:
      LEX();
      attr = K();
      switch (ConvTypes(iattr, attr)) {
        case INT: 
          if (attr.v.i) iattr.v.i %= attr.v.i;
          else {
            iattr.v.i = 0;
            DM_ERR(ERR_ERR, _("Modulo division by zero\n"));
          };
        break;
  
        default: DM_ERR(ERR_ERR, _("Modulo division is not supported for these types\n"));
      }
      attr = J1(iattr);
    break;

    /* J1 -> e */
    default:
      DM_DBG(DM_N(5), "J1 -> e\n");
      RETURN(iattr);
  }

  RETURN(attr);
}

/* K -> K1 */
Attr
Expr::K()
{
  DM_DBG_I;

  Attr attr;
  attr = K1();

  RETURN(attr);
}

/* K1 -> + K1 | - K1 | ! K1 | ~ K1
      |  ( X ) | INT | REAL */
Attr
Expr::K1()
{
  DM_DBG_I;
  Attr attr;

  attr.type = INV;
  switch(sym) {
    /* K1 -> + K1 */
    case PlusSym:
      LEX();				/* unary plus */
      attr = K1();
      switch (attr.type) {
        case INT:
        case REAL: break;

        default: DM_ERR(ERR_ERR, _("Illegal use of unary minus\n"));
      }
    break;

    /* K1 -> - K1 */
    case MinusSym:
      LEX();				/* unary minus */
      attr = K1();
      switch (attr.type) {
        case INT:  attr.v.i = -attr.v.i; break;
        case REAL: attr.v.r = -attr.v.r; break;
  
        default: DM_ERR(ERR_ERR, _("Illegal use of unary minus\n"));
      }
    break;

    /* K1 -> ! K1 */
    case NotSym:
      LEX();
      attr = K1();
      switch (attr.type) {
        case INT: attr.v.i = attr.v.i ? FALSE : TRUE;
        break;
  
        default: DM_ERR(ERR_ERR, _("The %s operator requires INT type values\n"), Lex::SymbolName(sym));
      }
    break;

    /* K1 -> ~ K1 */
    case BitNotSym:
      LEX();
      attr = K1();
      switch (attr.type) {
        case INT: attr.v.i = ~attr.v.i;
        break;
  
        default: DM_ERR(ERR_ERR, _("The %s operator requires INT type values\n"), Lex::SymbolName(sym));
      }
    break;

    /* K1 -> ( X ) */
    case LprSym:
      LEX();
      attr = X();
      EXPECT(RprSym);
    break;

    /* K1 -> INT */
    case IntSym:
      attr.type = INT;
      attr.v.i = lex_attr.v.i;		/* get the value from lex */
      LEX();
    break;

    /* K1 -> REAL */
    case RealSym:
      attr.type = REAL;
      attr.v.r = lex_attr.v.r;		/* get the value from lex */
      LEX();
    break;

    default:
      DM_ERR_ASSERT(_("switch: default: %s\n"), Lex::SymbolName(sym));
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
