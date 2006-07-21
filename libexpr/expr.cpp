#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "expr.h"
#include "str.h"		/* i2str */
#include "free.h"		/* FREE(), DELETE() macros */
#include "max.h"		/* UPDATE_MAX() */

#include "i18.h"
#include "constants.h"
#include "sysdep.h"

#include <stdlib.h>
#include <errno.h>		/* errno */

/* simple macros */
#define EXPECT(s)\
  if (s == sym)\
    LEX();\
  else {\
    DM_ERR(ERR_ERR, _("expected: %s; found: %s\n"), Lex::SymbolName(s), Lex::SymbolName(sym));\
  }

#define LEX()	do { if((sym = l.lex(lex_attr)) == InvalidSym) return attr; } while(0)

#define NEW_STR(v,s)\
  if((v = new std::string(s)) == NULL) {\
    DM_ERR_ASSERT(_("new failed\n"));\
  } else strings.push_back(v);		/* free the allocated string on exit */

/* constructor */
Expr::Expr()
{
}


/* constructor */
Expr::Expr(const char *s, Process *p)
{
  l = Lex(s, p);
  proc = p;
}


/* destructor */
Expr::~Expr()
{
  DELETE_VEC(strings);
}


Attr
Expr::parse()
{
  DM_DBG_I;

  Attr attr;
  attr = S();
 
  RETURN(attr);
}


Types
Expr::ConvTypes(Attr& a1, Attr& a2)
{
  static Types table[COUNT][COUNT] = {
    /*            a2:  INV,    INT,   REAL,   STRING */
    /* a1:                                           */
    /* INV    */      {INV,   INV,    INV,    INV   },
    /* INT    */      {INV,   INT,    REAL,   STRING},
    /* REAL   */      {INV,   REAL,   REAL,   STRING},
    /* STRING */      {INV,   STRING, STRING, STRING},
  };
  Types type;

  type = table[a1.type][a2.type];
  if (type == REAL) {			/* conversion int na real */
    if (a1.type == INT) {		/* conversion a1 (int -> real) */
      a1.type = REAL;
      a1.v.r = a1.v.i;
    } else
    if (a2.type == INT) {		/* conversion a2 (int -> real) */
      a2.type = REAL;
      a2.v.r = a2.v.i;
    }
  }
  else if(type == STRING) {
    if (a1.type == INT) {		/* conversion a1 (int -> string) */
      a1.type = STRING;
      NEW_STR(a1.v.s,i2str(a1.v.i).c_str());
    } else
    if (a1.type == REAL) {		/* conversion a1 (real -> string) */
      a1.type = STRING;
      NEW_STR(a1.v.s,r2str(a2.v.r).c_str());
    } else
    if (a2.type == INT) {		/* conversion a2 (int -> string) */
      a2.type = STRING;
      NEW_STR(a2.v.s,i2str(a2.v.i).c_str());
    } else
    if (a2.type == REAL) {		/* conversion a2 (real -> string) */
      a2.type = STRING;
      NEW_STR(a2.v.s,r2str(a2.v.r).c_str());
    } 
  }
  return type;
}


int
Expr::compare(Attr a1, Attr a2, Symbol o)
{
  switch (ConvTypes(a1, a2)) {
    case INT:{
      DM_DBG(DM_N(4), "%"PRIi64" %s %"PRIi64"\n", a1.v.i, Lex::SymbolName(o), a2.v.i);
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
    }
    break;

    case REAL:{
      DM_DBG(DM_N(4), "%f %s %f\n", a1.v.r, Lex::SymbolName(o), a2.v.r);
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
    }
    break;

    case STRING:{
      int cmp = strcmp(a1.v.s->c_str(), a2.v.s->c_str());
      DM_DBG(DM_N(4), "%s %s %s\n", a1.v.s->c_str(), Lex::SymbolName(o), a2.v.s->c_str());
      switch (o) {
        case EqSym: return cmp == 0 ? TRUE : FALSE;
        case LtSym: return cmp <  0 ? TRUE : FALSE;
        case GtSym: return cmp >  0 ? TRUE : FALSE;
        case NeSym: return cmp != 0 ? TRUE : FALSE;
        case LeSym: return cmp <= 0 ? TRUE : FALSE;
        case GeSym: return cmp >= 0 ? TRUE : FALSE;
        default:
          DM_ERR_ASSERT(_("switch: default: %s\n"), Lex::SymbolName(o));
      }
    }
    break;

    default:
      DM_ERR(ERR_ERR, _("cannot compare values of incompatible types\n"));
  }

  return 0; /* unable to compare */
}

void
Expr::normalize(Attr &attr)
{
  switch(attr.type) {
    case STRING:{
      Attr a, eof_a;
      Lex l = Lex(attr.v.s->c_str(), proc);
      Symbol s = l.lex(a);
      if((s == IntSym || s == RealSym) && l.eof()) {
        DM_DBG(DM_N(4), "normalizing string type to %s\n", Lex::SymbolName(s));
        attr = a;
      }
    }
    break;

    case INV:	/* fall through */
    case INT:	/* fall through */
    break;

    case REAL:{	/* see if an integer can take the real number */
      int64_t inum = (int64_t)attr.v.r;
      if(inum == attr.v.r) {
        attr.type = INT;
        attr.v.i = inum;
      }
    }
    break;

    default:
      ;		/* yeah, needed */
  }
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

  switch(sym) {
    case OrSym:{		/* A1 -> || B A1 */
      LEX();
      attr = B();
      switch (ConvTypes(iattr, attr)) {
        case INT:{
          iattr.v.i = iattr.v.i || attr.v.i;
        }
        break;
  
        default:
          DM_ERR(ERR_ERR, _("the %s binary operator requires INT type values\n"), Lex::SymbolName(sym));
      }
      attr = A1(iattr);
    }
    break;
    
    case EofSym:{ 		/* A1 -> e */
      DM_DBG(DM_N(5), "A1 -> e\n");
      RETURN(iattr);
    }
    break;
    
    default:
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
      case INT:{
        iattr.v.i = iattr.v.i && attr.v.i;
      }
      break;

      default:
        DM_ERR(ERR_ERR, _("the %s binary operator requires INT type values\n"), Lex::SymbolName(sym));
    }
    attr = B1(iattr);
  }
  else {				/* B1 -> e */
    if(sym == EofSym) DM_DBG(DM_N(5), "B1 -> e\n");
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
      case INT:{
        iattr.v.i |= attr.v.i;
      }
      break;

      default:
        DM_ERR(ERR_ERR, _("the %s binary operator requires INT type values\n"), Lex::SymbolName(sym));
    }
    attr = C1(iattr);
  }
  else {				/* C1 -> e */
    if(sym == EofSym) DM_DBG(DM_N(5), "C1 -> e\n");
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
      case INT:{
        iattr.v.i ^= attr.v.i;
      }
      break;

      default:
        DM_ERR(ERR_ERR, _("the %s operator requires INT type values\n"), Lex::SymbolName(sym));
    }
    attr = D1(iattr);
  }
  else {				/* D1 -> e */
    if(sym == EofSym) DM_DBG(DM_N(5), "D1 -> e\n");
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
      case INT:{
        iattr.v.i &= attr.v.i;
      }
      break;

      default:
        DM_ERR(ERR_ERR, _("the %s binary operator requires INT type values\n"), Lex::SymbolName(sym));
    }
    attr = E1(iattr);
  }
  else {				/* E1 -> e */
    if(sym == EofSym) DM_DBG(DM_N(5), "E1 -> e\n");
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
    if(sym == EofSym) DM_DBG(DM_N(5), "F1 -> e\n"); 
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
    if(sym == EofSym) DM_DBG(DM_N(5), "G1 -> e\n"); 
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
    case ShlSym:{
      LEX();
      attr = I();
      switch (ConvTypes(iattr, attr)) {
        case INT:{
          int64_t inum = iattr.v.i << attr.v.i;
#ifdef CHECK_OVERFLOWS
          if(attr.v.i < 0) {
            UPDATE_MAX(proc->executed, ERR_WARN);
            DM_WARN(ERR_WARN, "left shift is negative %"PRIi64"\n", attr.v.i);
          } else if(attr.v.i > 0) {
            if(iattr.v.i >= inum || attr.v.i >= 63) {
              UPDATE_MAX(proc->executed, ERR_WARN);
              DM_WARN(ERR_WARN, "integer overflow using left shift (%"PRIi64" << %"PRIi64")\n", iattr.v.i, attr.v.i);
            }
          }
#endif
          iattr.v.i = inum;
        }
        break;
  
        default:
          DM_ERR(ERR_ERR, _("the %s binary operator requires INT type values\n"), Lex::SymbolName(sym));
      }
      attr = H1(iattr);
    }
    break;

    /* H1 -> >> I H1 */
    case ShrSym:{
      LEX();
      attr = I();
      switch (ConvTypes(iattr, attr)) {
        case INT:{
          iattr.v.i >>= attr.v.i;
        }
        break;
  
        default:
          DM_ERR(ERR_ERR, _("the %s binary operator requires INT type values\n"), Lex::SymbolName(sym));
      }
      attr = H1(iattr);
    }
    break;

    /* H1 -> e */
    case EofSym:
      DM_DBG(DM_N(5), "H1 -> e\n");
    /* fall through */

    default:
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
        case INT:{
          int64_t inum = iattr.v.i + attr.v.i;
#ifdef CHECK_OVERFLOWS
          double rnum = (double)iattr.v.i + attr.v.i;
          if(inum != rnum) {
            DM_DBG(DM_N(3), "integer overflow detected, using double instead of integer\n");
            iattr.type = REAL;
            iattr.v.r = rnum;
            break;
          }
#endif
          iattr.v.i = inum;
        }
        break;

        case REAL:{
          iattr.v.r += attr.v.r;
        }
        break;
  
        default:
          DM_ERR(ERR_ERR, _("addition is not supported for these types\n"));
      }
      attr = I1(iattr);
    break;

    /* I1 -> - J I1 */
    case MinusSym:{
      LEX();
      attr = J();
      switch (ConvTypes(iattr, attr)) {
        case INT:{
          int64_t inum = iattr.v.i - attr.v.i;
#ifdef CHECK_OVERFLOWS
          double rnum = (double)iattr.v.i - attr.v.i;
          if(inum != rnum) {
            DM_DBG(DM_N(3), "integer overflow detected, using double instead of integer\n");
            iattr.type = REAL;
            iattr.v.r = rnum;
            break;
          }
#endif
          iattr.v.i = inum;
        }
	break;

        case REAL:{
          iattr.v.r -= attr.v.r;
        }
	break;
  
        default:
	  DM_ERR(ERR_ERR, _("subtraction is not supported for these types\n"));
      }
      attr = I1(iattr);
    }
    break;

    /* I1 -> e */
    case EofSym:
      DM_DBG(DM_N(5), "I1 -> e\n");
    /* fall through */

    default:
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
    case MultSym:{
      LEX();
      attr = K();
      switch (ConvTypes(iattr, attr)) {
        case INT:{
          int64_t inum = iattr.v.i * attr.v.i;
#ifdef CHECK_OVERFLOWS
          double rnum = (double)iattr.v.i * attr.v.i;
          if(inum != rnum) {
            DM_DBG(DM_N(3), "integer overflow detected, using double instead of integer\n");
            iattr.type = REAL;
            iattr.v.r = rnum;
            break;
          }
#endif
          iattr.v.i = inum;
        }
	break;

        case REAL:{
          iattr.v.r *= attr.v.r;
        }
	break;
  
        default:
          DM_ERR(ERR_ERR, _("multiplication is not supported for these types\n"));
      }
      attr = J1(iattr);
    }
    break;

    /* J1 -> / K J1 */
    case DivSym:{
      LEX();
      attr = K();
      switch (ConvTypes(iattr, attr)) {
        case INT:{
          if (attr.v.i) {		/* division INT/INT => REAL */
            iattr.v.r = iattr.v.i;
            iattr.type = REAL;
            iattr.v.r /= attr.v.i;
          }
          else {
            iattr.v.i = 0;
            DM_ERR(ERR_ERR, _("division by zero\n"));
          }
        }
        break;
  
        case REAL:{
          if (attr.v.r) iattr.v.r /= attr.v.r;
          else {
            iattr.v.r = 0.0;
            DM_ERR(ERR_ERR, _("division by zero\n"));
          }
        }
        break;
  
        default:
          DM_ERR(ERR_ERR, _("division is not supported for these types\n"));
      }
      attr = J1(iattr);
    }
    break;

    /* J1 -> % K J1 */
    case ModSym:{
      LEX();
      attr = K();
      switch (ConvTypes(iattr, attr)) {
        case INT:{
          if (attr.v.i) iattr.v.i %= attr.v.i;
          else {
            iattr.v.i = 0;
            DM_ERR(ERR_ERR, _("modulo division by zero\n"));
          }
        }
        break;
  
        default:
          DM_ERR(ERR_ERR, _("modulo division is not supported for these types\n"));
      }
      attr = J1(iattr);
    }
    break;

    case EofSym:
    /* J1 -> e */
      DM_DBG(DM_N(5), "J1 -> e\n");
    /* fall through */

    default:
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
      |  ( X ) | INT | REAL | STRING */
Attr
Expr::K1()
{
  DM_DBG_I;
  Attr attr;

  attr.type = INV;
  switch(sym) {
    /* K1 -> + K1 */
    case PlusSym:{
      LEX();				/* unary plus */
      attr = K1();
      switch (attr.type) {
        case INT:
        case REAL: break;

        default:
          DM_ERR(ERR_ERR, _("illegal use of unary minus\n"));
      }
    }
    break;

    /* K1 -> - K1 */
    case MinusSym:{
      LEX();				/* unary minus */
      attr = K1();
      switch (attr.type) {
        case INT:{
          int64_t inum = -attr.v.i;
#ifdef CHECK_OVERFLOWS
          if(attr.v.i == inum) {
            UPDATE_MAX(proc->executed, ERR_WARN);
            DM_WARN(ERR_WARN, "integer overflow using unary minus (%"PRIi64" == %"PRIi64")\n", attr.v.i, inum);
          }
#endif
          attr.v.i = inum;
        }
	break;

        case REAL:{
	  attr.v.r = -attr.v.r;
	}
	break;
  
        default:
	  DM_ERR(ERR_ERR, _("illegal use of unary minus\n"));
      }
    }
    break;

    /* K1 -> ! K1 */
    case NotSym:{
      LEX();
      attr = K1();
      switch (attr.type) {
        case INT:{
          attr.v.i = attr.v.i ? FALSE : TRUE;
        }
        break;
  
        default:
          DM_ERR(ERR_ERR, _("the %s unary operator requires INT type values\n"), Lex::SymbolName(sym));
      }
    }
    break;

    /* K1 -> ~ K1 */
    case BitNotSym:{
      LEX();
      attr = K1();
      switch (attr.type) {
        case INT:{
          attr.v.i = ~attr.v.i;
        }
        break;
  
        default:
          DM_ERR(ERR_ERR, _("the %s unary operator requires INT type values\n"), Lex::SymbolName(sym));
      }
    }
    break;

    /* K1 -> ( X ) */
    case LprSym:{
      LEX();
      attr = X();
      EXPECT(RprSym);
    }
    break;

    /* K1 -> INT */
    case IntSym:{
      attr.type = INT;
      attr.v.i = lex_attr.v.i;		/* get the value from lex */
      LEX();
    }
    break;

    /* K1 -> REAL */
    case RealSym:{
      attr.type = REAL;
      attr.v.r = lex_attr.v.r;		/* get the value from lex */
      LEX();
    }
    break;

    /* K1 -> STRING */
    case StringSym:{
      std::string s;			/* string with no $TAGs */
      DM_DBG(DM_N(4),"s=|%s|\n",lex_attr.v.s->c_str());
      s = Process::eval_str(lex_attr.v.s->c_str(), proc);
      DM_DBG(DM_N(4),"s=|%s|\n",s.c_str());
      if(strcmp(s.c_str(), lex_attr.v.s->c_str())) {
        /* string evaluation produced a different string; *
         * evaluate the expression again                  */
        Expr e = Expr(s.c_str(), proc);
        Attr a = e.parse();
        attr = a;
        if(a.type == STRING) {
          NEW_STR(attr.v.s,a.v.s->c_str());
          DM_DBG(DM_N(4),"attr.v.s=|%s|\n",attr.v.s->c_str());
        }
        DM_DBG(DM_N(4),"attr.type=%d\n",attr.type);
      } else {
        /* string evaluation didn't produce a different string; *
         * it is a string                                       */
        attr.type = STRING;
        NEW_STR(attr.v.s,s.c_str());
        DM_DBG(DM_N(4),"s=|%s|\n",s.c_str());
      }
      LEX();
    }
    break;

    /* K1 -> e */
    case EofSym:{
      /* we have an empty string */
      DM_DBG(DM_N(5), "K1 -> e\n");
      attr.type = STRING;
      NEW_STR(attr.v.s,"");
    }
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
