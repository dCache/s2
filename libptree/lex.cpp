#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "lex.h"

#include "constants.h"
#include "i18.h"
#include "limits.h"

#include <ctype.h>	/* isspace(), ... */
#include <math.h>	/* pow(), ... */

/* constructor */
Lex::Lex()
{
}


/* constructor */
Lex::Lex(const char *s)
{
  expr = s;
  i_expr = 0;
}


/* desructor */
Lex::~Lex()
{
}


char
Lex::gc(void)
{
  return expr[i_expr++];
}


void
Lex::ugc(void)
{
  i_expr--;
}


Symbol
Lex::lex(Attr& attr)
{
  DM_DBG_I;

  Symbol sym;
  int c;
  enum { sInit, sNum, sReal01, sReal02, sReal03, sReal04, 
         sLt, sGt, sEq, sNe, sOr, sAnd } state = sInit;
  long double power10;
  long inum = 0;
  int expon, sign;

  attr.type = NONE;
  do {
    c = gc();
 
    switch (state) {
      case sInit:
        if (isspace(c))
          ;
        else if (isdigit(c)) {
          state = sNum;
          inum = c - '0';
          attr.v.i = inum;
        }
        else switch (c) {
          case 0:   return EofSym;
          case '+': return PlusSym;
          case '-': return MinusSym;
          case '*': return MultSym;
          case '/': return DivSym;
          case '%': return ModSym;
          case '(': return LprSym;
          case ')': return RprSym;
          case '~': return BitNotSym;
          case '<': state = sLt; break;
          case '>': state = sGt; break;
          case '=': state = sEq; break;
          case '!': state = sNe; break;
          case '|': state = sOr; break;
          case '&': state = sAnd; break;

          default: DM_ERR(ERR_ERR, _("Invalid character '%c'\n"), c);
        }
      break;

      case sNum:
        if (isdigit(c)) {
          attr.v.i = inum * 10 + (c - '0');
          if (inum > (LONG_MAX - (c - '0'))/10) {
            /* pøeteèení attr.v.i => konverze na real */
            printf("%li", LONG_MAX);
            attr.v.r = (double) inum * 10 + (c - '0');
            state = sReal01;
          } else inum = attr.v.i;
        }
        else if (c == '.') {
          attr.v.r = inum;
          power10 = 1.0;
          sign = 1;
          expon = 0;
          state = sReal02;
        }
        else if (c == 'e' || c == 'E') {
          /* exponent */
          attr.v.r = inum;
          power10 = 1.0;
          sign = 1;
          expon = 0;
          state = sReal03;
        }
        else {
          state = sInit;
          ugc();
          attr.type = INT;
          return IntSym;
        }
        break;

      case sReal01:		/* real number before . */
        if (isdigit(c))
          attr.v.r = attr.v.r * 10 + (c - '0');
        else if (c == '.') {
          power10 = 1.0;
          sign = 1;
          expon = 0;
          state = sReal02;
        } else
        if(c == 'e' || c == 'E')
          state = sReal03;
        else {
          state = sInit;
          ugc();
          attr.type = REAL;
          return RealSym;
        }
      break;

      case sReal02:		/* real number after . */
        if (isdigit(c))
          attr.v.r += (c - '0') * (power10 /= 10);
        else if (c == 'e' || c == 'E')
          state = sReal03;
        else {
          state = sInit;
          ugc();
          attr.type = REAL;
          return RealSym;
        }
      break;

      case sReal03:		/* real number (exponent) */
        state = sReal04;
        if (isdigit(c))
          expon = c - '0';
        else if (c == '+')
          sign = 1;
        else if (c == '-')
          sign = -1;
        else {
          state = sInit;
          ugc();
          attr.type = REAL;
          return RealSym;
        }
      break;

      case sReal04:		/* real number (exponent, finish) */
        if (isdigit(c))
          expon = expon * 10 + (c - '0');
        else {
          power10 = pow(sign > 0 ? expon : -expon, 10);
          if (attr.v.r * power10 >= HUGE_VAL || power10 == 0.0) {
            if (sign > 0) DM_ERR(ERR_ERR, _("Real number too big"));
            else DM_ERR(ERR_ERR, _("Real number too small"));
            attr.v.r = 0;
          }
          else
            attr.v.r *= power10;
  
          state = sInit;
          ugc();
          attr.type = REAL;
          return RealSym;
        }
      break;

      case sLt:			/* <, <<, <= */
        state = sInit;
        if (c == '=')		/* <= */
          sym = LeSym;
        else if (c == '<')	/* << */
          sym = ShlSym;
        else {			/* < */
          ugc();
          sym = LtSym;
        }
      return sym;

      case sGt:			/* >, >>, >= */
        state = sInit;
        if (c == '=')		/* >= */
          sym = GeSym;
        else if (c == '>')	/* >> */
          sym = ShrSym;
        else {
          ugc();		/* > */
          sym = GtSym;
        }
      return sym;

      case sEq:			/* =, == */
        state = sInit;
        if (c == '=')		/* == */
          sym = EqSym;
        else {
          ugc();		/* = */
          sym = AsgnSym;
        }
      return sym;

      case sNe:			/* !, != */
        state = sInit;
        if (c == '=')		/* != */
          sym = NeSym;
        else {
          ugc();		/* ! */
          sym = NotSym;
        }
      return sym;

      case sOr:			/* |, || */
        state = sInit;
        if (c == '|')		/* || */
          sym = OrSym;
        else {
          ugc();		/* | */
          sym = BitOrSym;
        }
      return sym;

      case sAnd:		/* &, && */
        state = sInit;
        if (c == '&')		/* && */
          sym = AndSym;
        else {
          ugc();		/* & */
          sym = BitAndSym;
        }
      return sym;

    } /* switch */
  } while (c != EOF);

  return EofSym;
}


const char*
Lex::SymbolName(Symbol s)
{
  static const char* symbs[] = {
    "EOF", "+", "-", "*", "/", "%",
    "<<", ">>",
    "(", ")",
    "<", "<=", ">", ">=", "=", "==", "!", "!=",
    "~",
    "|", "||",
    "&", "&&",
    "INT", "REAL",
  };
  return symbs[s];
}
