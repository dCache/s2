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
#include "expr.h"

#include "constants.h"
#include "i18.h"
#include "limits.h"
#include "str.h"
#include "free.h"		/* FREE(), DELETE() */
#include "max.h"		/* UPDATE_MAX() */

#include <ctype.h>		/* isspace(), ... */
#include <math.h>		/* pow(), ... */
#include <errno.h>		/* errno */

#define IS_HEX_CHAR(c) ((c) >= 'A' && (c) <= 'F')

#define NEW_STR(v,s)\
  if((v = new std::string(s)) == NULL) {\
    DM_ERR_ASSERT(_("new failed\n"));\
  } else strings.push_back(v);		/* free the allocated string on exit */

/* constructor */
Attr::Attr()
{
}


/* destructor */
Attr::~Attr()
{
}

std::string
Attr::toString()
{
  std::string str;

  switch(type) {
    case INV:    str = "(invalid)" ; break;
    case INT:    str = i2str(v.i); break;
    case REAL:   str = r2str(v.r); break;
    case STRING: str = std::string(v.s->c_str()); break;
    default:  /* never reached */
      DM_ERR_ASSERT(_("switch: default: %d\n"), type);
      str = "(default)";
  }

  return str;
}

/* constructor */
Lex::Lex()
{
}


/* constructor */
Lex::Lex(const char *s, Process *p)
{
  if(!s) {
    DM_ERR_ASSERT(_("s == NULL\n"));
  }
  source = s;
  col = 0;
  source_len = strlen(source);

  proc = p;
}


/* desructor */
Lex::~Lex()
{
  DELETE_VEC(strings);
}


char
Lex::gc(void)
{
  /* <= is important to make for gc/ugc() behaviour consistent; don't change to < */
  return (col <= source_len)? source[col++]: '\0';
}


void
Lex::ugc(void)
{
  if(col > 0) col--;
}

BOOL
Lex::eof(void)
{
  DM_DBG(DM_N(5), "%d/%d\n", col, source_len);
  return col >= source_len;
}


Symbol
Lex::lex(Attr& attr)
{
#define IS_SEP(c)	(c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '(' || c == ')' || c == '^' || c == '~' || c == '<' || c == '>' || c == '=' || c == '!' || c == '|' || c == '&' || c == ';')
#define TERM_CHAR(c)    (dq? (c == '"'): (IS_WHITE(c) || IS_SEP(c)) && !brackets)

  DM_DBG_I;

  Symbol sym;
  int c;
  enum { sInit,
         sNum, sNumber,
         sReal01, sReal02, sReal03, sReal04, 
         sLt, sGt, sEq, sNe, sOr, sAnd, sString, sComment } state = sInit;
  int64_t inum = 0;	/* for overflows/underflows */
  double power10 = 1.0;
  int expon = 0, sign = 1, base = 10;
  BOOL bslash = FALSE;	/* we had the '\\' character */
  BOOL dq = FALSE;	/* quotation mark at the start of a string */
  int brackets = 0;

  attr.type = INV;
  do {
    c = gc();
    DM_DBG(DM_N(5), "gc()='%c'\n", c);
 
    switch (state) {
      case sInit:
        if (IS_WHITE(c))
          ;
        else if (isdigit(c) && c != '0') {
          /* decimal numbers */
          state = sNumber;
          base = 10;
          attr.v.i = c - '0';
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
          case ';': state = sComment; break;

          default: /* string */
            state = sString;
            attr.type = STRING;
            NEW_STR(attr.v.s,);
            dq = (c == '"');	/* string enclosed in quotes */
            if(!dq) attr.v.s->push_back(c);
        }
      break;

      case sNum:
        /* octal, hexadecimal or decimal numbers */
        if(isdigit(c)) {
          state = sNumber;
          base = 8;
          attr.v.i = c - '0';
        } else
        if(c == 'x' || c == 'X') {
          state = sNumber;
          base = 16;
          attr.v.i = 0;
        } else
        if(c == '.') {
          state = sReal02;
          base = 10;
          attr.v.r = 0;
        }
        else {
          /* 0 */
          attr.type = INT;
          attr.v.i = 0;
          ugc();
          DM_DBG(DM_N(4), "we have a 0\n");
          return IntSym;
        }
      break;

      case sNumber:
        /* base is defined (octal, decimal, hexadecimal) */
        DM_DBG(DM_N(4), "number (%"PRIi64"), base=%d\n", attr.v.i, base);
        if (isalpha(c)) c = toupper(c);
        if (isdigit(c)) {
          inum = attr.v.i;
          attr.v.i = inum * base + (c - '0');
#ifdef CHECK_OVERFLOWS
          if (inum > (INT64_MAX - (c - '0'))/base) {
            /* attr.v.i overflow => use real */
            attr.v.r = (double) inum * base + (c - '0');
            state = sReal01;
            DM_DBG(DM_N(3), "integer too large, using real (%f)\n", attr.v.r);
          }
#endif
        }
        else if (IS_HEX_CHAR(c) && base == 16) {
          inum = attr.v.i;
          attr.v.i = inum * base + ((c - 'A') + 10);
#ifdef CHECK_OVERFLOWS
          if (inum > (INT64_MAX - ((c - 'A') + 10))/base) {
            /* attr.v.i overflow => use real */
            attr.v.r = (double) inum * base + ((c - 'A') + 10);
            state = sReal01;
            DM_DBG(DM_N(3), "integer too large, using real (%f)\n", attr.v.r);
          }
#endif
        }
        else if (c == '.') {
          attr.v.r = attr.v.i;
          state = sReal02;
        }
        else if (c == 'E') {	/* 'e' is handled by toupper() */
          /* exponent */
          attr.v.r = attr.v.i;
          state = sReal03;
        }
        else {
          state = sInit;
          ugc();
          attr.type = INT;
          DM_DBG(DM_N(4), "we have an integer (%"PRIi64")\n", attr.v.i);
          return IntSym;
        }
      break;

      case sReal01:		/* real number before . */
        DM_DBG(DM_N(4), "real number (%f), c='%c', base=%d, exponent=%d, sign=%d, power10=%f\n", attr.v.r, c, base, sign, expon, power10);
        if (isalpha(c)) c = toupper(c);
        if (isdigit(c))
          attr.v.r = attr.v.r * base + (c - '0');
        else if (IS_HEX_CHAR(c) && base == 16)
          attr.v.r = attr.v.r * base + ((c - 'A') + 10);
        else if (c == '.')
          state = sReal02;
        else
        if(c == 'e' || c == 'E')
          state = sReal03;
        else {
          state = sInit;
          ugc();
          attr.type = REAL;
          DM_DBG(DM_N(4), "we have an real number (%f)\n", attr.v.r);
          return RealSym;
        }
      break;

      case sReal02:		/* real number after . */
        DM_DBG(DM_N(4), "real number (%f), c='%c', base=%d, exponent=%d, sign=%d, power10=%f\n", attr.v.r, c, base, sign, expon, power10);
        if (isalpha(c)) c = toupper(c);
        if (isdigit(c))
          attr.v.r += (c - '0') * (power10 /= base);
        else if (IS_HEX_CHAR(c) && base == 16)
          /* note e/E is ignored for hexadecimal real numbers */
          attr.v.r += ((c - 'A') + 10) * (power10 /= base);
        else if (c == 'E')	/* 'e' is handled by toupper() */
          state = sReal03;
        else {
          state = sInit;
          ugc();
          attr.type = REAL;
          DM_DBG(DM_N(4), "we have a real number (%f)\n", attr.v.r);
          return RealSym;
        }
      break;

      case sReal03:		/* real number (exponent) */
        DM_DBG(DM_N(4), "real number (%f), c='%c', base=%d, exponent=%d, sign=%d, power10=%f\n", attr.v.r, c, base, sign, expon, power10);
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
          DM_DBG(DM_N(4), "we have a real number (%f)\n", attr.v.r);
          return RealSym;
        }
        state = sReal04;
      break;

      case sReal04:		/* real number (exponent, finish) */
        DM_DBG(DM_N(4), "real number (%f), c='%c', base=%d, exponent=%d, sign=%d, power10=%f\n", attr.v.r, c, base, sign, expon, power10);
        if (isdigit(c)) {
          expon = expon * base + (c - '0');
        } else {
          /* not a digit, calculate the final real number */
          power10 = pow(base, sign > 0 ? expon : -expon);
          double rnum = attr.v.r * power10;
          DM_DBG(DM_N(4), "huge_val=%f; real number (%f), c='%c', base=%d, exponent=%d, sign=%d, power10=%f\n", HUGE_VALF, attr.v.r, c, base, sign, expon, power10);
#ifdef CHECK_OVERFLOWS
          if (rnum >= HUGE_VALF) {	/* = is necessary, HUGE_VALF is infinity... */
            if (sign > 0) {
              attr.v.r = rnum;
              UPDATE_MAX(proc->executed, ERR_WARN);
              DM_WARN(ERR_WARN, _("real number too big, using %f\n"), attr.v.r);
            } else {
              attr.v.r = 0;
              UPDATE_MAX(proc->executed, ERR_WARN);
              DM_WARN(ERR_WARN, _("real number too small, using %f\n"), attr.v.r);
            }
          }
          else
#endif
            attr.v.r *= power10;
  
          state = sInit;
          ugc();
          attr.type = REAL;
          DM_DBG(DM_N(4), "we have a real number (%f)\n", attr.v.r);
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

      case sString:		/* string */
        if(c == '\\' && bslash) {
          /* two backslashes => no quoting */
          bslash = FALSE;
          attr.v.s->push_back('\\');
          continue;
        }

        if(!bslash) {
          /* brackets within a string must be ballanced for non-quoted strings; the reason *
           * being such strings may contain/be tags, e.g. $MATCH{"a ." "a b"}              */
          if(c == '}') brackets--;
          if(c == '{') brackets++;
        }

        if ((c == '\0'
             || TERM_CHAR(c) && !bslash)
           ) {
          /* we have a string terminator */
          state = sInit;
          if(c != '"') {
            ugc();
            if(dq) {
              UPDATE_MAX(proc->executed, ERR_WARN);
              DM_WARN(ERR_WARN, "'%s%c' terminated double-quoted parameter\n", (c == 0)? "\\": "", c);
            }
          }
          return StringSym;
        }

        attr.v.s->push_back(c);
        bslash = c == '\\';
      break;

      case sComment:		/* ; */
        /* ignore everything until '\0' */
      break;

    } /* switch */
  } while (c != '\0');

  RETURN(EofSym);

#undef TERM_CHAR
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
    "INT", "REAL", "STRING",
  };
  return symbs[s];
}
