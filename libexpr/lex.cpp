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

#define IS_HEX_CHAR(c) ((c) >= 'A' && (c) <= 'F')

/* constructor */
Lex::Lex()
{
}


/* constructor */
Lex::Lex(const char *s)
{
  expr = s;
  expr_col = 0;
}


/* desructor */
Lex::~Lex()
{
}


char
Lex::gc(void)
{
  return expr[expr_col++];
}


void
Lex::ugc(void)
{
  expr_col--;
}


Symbol
Lex::lex(Attr& attr)
{
  DM_DBG_I;

  Symbol sym;
  int c;
  enum { sInit,
         sNum, sNumber,
         sReal01, sReal02, sReal03, sReal04, 
         sLt, sGt, sEq, sNe, sOr, sAnd } state = sInit;
  int64_t inum = 0;
  double power10;
  int expon, sign, base;

  attr.type = INV;
  do {
    c = gc();
 
    switch (state) {
      case sInit:
        if (isspace(c))
          ;
        else if (isdigit(c) && c != '0') {
          state = sNumber;
          base = 10;
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
          case '^': return BitXorSym;
          case '~': return BitNotSym;
          case '0': state = sNum; break;
          case '<': state = sLt; break;
          case '>': state = sGt; break;
          case '=': state = sEq; break;
          case '!': state = sNe; break;
          case '|': state = sOr; break;
          case '&': state = sAnd; break;

          default:
            DM_ERR(ERR_ERR, _("invalid character '%c'\n"), c);
            return InvalidSym;
        }
      break;

      case sNum:
        /* octal or hexadecimal number */
        if(isdigit(c)) {
          state = sNumber;
          base = 8;
          inum = c - '0';
        } else
        if(c == 'x' || c == 'X') {
          state = sNumber;
          base = 16;
          inum = 0;
        }  
        else {
          /* 0 */
          state = sInit;
          ugc();
          attr.v.i = 0;
          attr.type = INT;
          return IntSym;
        }
        attr.v.i = inum;
      break;

      case sNumber:
        /* base is defined (octal, decimal, hexadecimal) */
        if (isalpha(c)) c = toupper(c);
        if (isdigit(c)) {
          attr.v.i = inum * base + (c - '0');
          if (inum > (INT64_MAX - (c - '0'))/base) {
            /* attr.v.i overflow => use real */
            attr.v.r = (double) inum * base + (c - '0');
            state = sReal01;
            DM_DBG(DM_N(3), "integer too large, using real (%d)\n", attr.v.r);
          } else inum = attr.v.i;
        }
        else if (IS_HEX_CHAR(c) && base == 16) {
          attr.v.i = inum * base + ((c - 'A') + 10);
          if (inum > (INT64_MAX - ((c - 'A') + 10))/base) {
            /* attr.v.i overflow => use real */
            attr.v.r = (double) inum * base + ((c - 'A') + 10);
            state = sReal01;
            DM_DBG(DM_N(3), "integer too large, using real (%d)\n", attr.v.r);
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
        if (isalpha(c)) c = toupper(c);
        if (isdigit(c))
          attr.v.r = attr.v.r * base + (c - '0');
        else if (IS_HEX_CHAR(c) && base == 16)
          attr.v.r = attr.v.r * base + ((c - 'A') + 10);
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
        if (isalpha(c)) c = toupper(c);
        if (isdigit(c))
          attr.v.r += (c - '0') * (power10 /= base);
        else if (IS_HEX_CHAR(c) && base == 16)
          /* note e/E is ignored for hexadecimal real numbers */
          attr.v.r += ((c - 'A') + 10) * (power10 /= base);
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
          expon = expon * base + (c - '0');
        else {
          power10 = pow(sign > 0 ? expon : -expon, base);
          if (attr.v.r * power10 > HUGE_VAL) {
            if (sign > 0) DM_ERR(ERR_ERR, _("real number too big\n"));
            else DM_ERR(ERR_ERR, _("real number too small\n"));
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
    "EOF", "INV_SYM",
    "+", "-", "*", "/", "%",
    "<<", ">>",
    "(", ")",
    "<", "<=", ">", ">=", "=", "==", "!", "!=",
    "^",
    "~",
    "|", "||",
    "&", "&&",
    "INT", "REAL",
  };
  return symbs[s];
}
