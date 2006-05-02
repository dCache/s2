#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "parse.h"
#include "n.h"

#include "md5f.h"
#include "free.h"
#include "date.h"
#include "constants.h"
#include "i18.h"
#include "timeout.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "expr.h"
#include "printf.h"
#include "s2.h"			/* opts (s2 options) */
#include "str.h"
#include "io.h"                 /* file_ropen(), ... */

#include <sys/time.h>		/* gettimeofday() */
#include <time.h>		/* gettimeofday() */
#include <errno.h>		/* errno */
#include <signal.h>             /* signal() */
#include <unistd.h>		/* sleep() */
#include <stdlib.h>             /* exit() */
#include <stdio.h>              /* stderr */

#include <iostream>             /* std::string, cout, endl, ... */

using namespace std;

/*
 * Constructor
 */
Node::Node()
{
  init();
}

/*
 * Destructor (virtual) + recursive postorder traversal
 */
Node::~Node()
{
  DM_DBG_I;

  /* first free the children */
  if(this->child) DELETE(this->child);

  /* then parallel branches */
  if(this->par) DELETE(this->par);

  DM_DBG_O;
}

/*
 * Initialise the node.
 */
void
Node::init()
{
  OFFSET = 0;
  COND = S2_COND_NONE;
  REPEAT.type = S2_REPEAT_NONE;
  REPEAT.X = 0;
  REPEAT.Y = 0;
  REPEAT.I = 0;
  EVAL = opts.s2_eval;
  TIMEOUT = opts.s2_timeout;

  /* PCRE options */
  match_opt.px = MATCH_PX;
  match_opt.pcre = MATCH_PCRE;
  
  /* debugging information */
  row = 0;
  has_executed = FALSE;
  executed = ERR_OK;
  evaluated = ERR_OK;

  /* tree overhead */
  parent = NULL;
  child = NULL;
  par = NULL;
  rpar = NULL;
}

/*
 * Initialise the node based on the value of another node.
 */
void
Node::init(Node &node)
{
  OFFSET = node.OFFSET;
  COND = node.COND;
  REPEAT.type = node.REPEAT.type;
  REPEAT.X = node.REPEAT.X;
  REPEAT.Y = node.REPEAT.Y;
  REPEAT.I = node.REPEAT.X;
  EVAL = node.EVAL;
  TIMEOUT = node.TIMEOUT;

  /* PCRE options */
  match_opt.px = node.match_opt.px;
  match_opt.pcre = node.match_opt.pcre;

  /* debugging information */
  row = node.row;
  has_executed = node.has_executed;
  executed = node.executed;
  evaluated = node.evaluated;

  /* tree overhead */
  parent = NULL;
  child = NULL;
  par = NULL;
  rpar = NULL;
}

/* 
 * Returns
 *   NULL: if no node with offset `offset' is found in the `append/bottom-most' branch
 *         pointer to the first node with offset `offset' otherwise
 */
Node *
Node::get_node_with_offset(Node *ptr_node, const uint offset)
{
  if(ptr_node == NULL)
    return NULL;

last_par:  
  if(ptr_node->OFFSET == offset)
    return ptr_node;


  /* descend to the last node on this level */
  while(ptr_node->par)
  {
    /* the same level, no need to check */
    ptr_node = ptr_node->par;
  }
//  DM_DBG(DM_N(0),"ptr_node->OFFSET=%u; n->OFFSET=%u\n", ptr_node->OFFSET, node->OFFSET);
  /* descend one level */
  if(ptr_node->child) {
    ptr_node = ptr_node->child;
    goto last_par;
  };

  /* node with offset `offset' was not found */
  return NULL;
} /* get_node_with_offset */

/* find an appropriate node to attach this node to + do the attachent */
int
Node::append_node(Node *root_node, Node *n)
{
  Node *ptr_node = root_node;

  if(ptr_node == n) {
    DM_ERR_ASSERT("trying to append the root node to itself\n");
    return ERR_ASSERT;
  }

  if(ptr_node == NULL) {
    DM_ERR_ASSERT("the root node is NULL\n");
    return ERR_ASSERT;
  }

last_par:  
  /* descend to the last node on this level */
  while(ptr_node->par)
  {
    ptr_node = ptr_node->par;
  }
//  DM_DBG(DM_N(0),"ptr_node->OFFSET=%d; n->OFFSET=%d\n", ptr_node->OFFSET, n->OFFSET);
  if(ptr_node->OFFSET < n->OFFSET)
  { /* descend one level */
    if(ptr_node->child) {
      ptr_node = ptr_node->child;
      goto last_par;
    };
    /* found the furthermost child => append */
    ptr_node->child = n;
    n->parent = ptr_node;
  } else {
    if(ptr_node->OFFSET != n->OFFSET) {
      /* 8-{} this should have been caught earlier in OFFSET()! */
      DM_ERR_ASSERT(_("invalid indentation, ignoring node %p\n"), n);
      return ERR_ASSERT;
    }
    
    /* offsets are equal => append the node after the last one on this level */
    ptr_node->par = n;
    n->parent = ptr_node->parent;
    n->rpar = ptr_node;
  }
  
  return ERR_OK;
} /* append_node */

/* 
 * Create a string parameter enclosed by double quotes depending on
 * characters in the string and its length.
 */
std::string
Node::dq_param(const char *s, BOOL quote)
{
  DM_DBG_I;
  std::stringstream ss;
  BOOL q;

  if(!quote) RETURN(std::string(s));

//#define DELIMIT_PARAM
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

  if(s == NULL) {
    ss << S2_NULL_STR;
    goto out;
  }

  q = str_char(s, ' ') != NULL ||       /* constains a space */
      str_char(s, '\t') != NULL ||      /* constains a tabulator */
      str_char(s, '\n') != NULL ||      /* constains a newline character */
      str_char(s, '\r') != NULL ||      /* constains a carriage return */
      *s == 0;                          /* empty string */

  if(q) ss << '"';

  ss << escape_chars(s, '"', q);

  if(q) ss << '"';

out:
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

#undef DELIMIT_PARAM    /* for debugging only */

  RETURN(ss.str());
} /* print_dq_param */

std::string
Node::dq_param(const std::string &s, BOOL quote)
{
  if(!quote) return s.c_str();

  return dq_param(s.c_str(), quote);
}

std::string
Node::dq_param(const std::string *s, BOOL quote)
{
  if(s == NULL) return std::string(S2_NULL_STR);

  if(!quote) return s->c_str();

  return dq_param(s->c_str(), quote);
}

std::string
Node::dq_param(const bool b, BOOL quote)
{
  std::string s = b? std::string("1"): std::string("0");
  
  return s;
}

std::string
Node::dq_param(const unsigned char c, BOOL quote)
{
  std::string s = i2str(c);
  
  return s;
}

/* 
 * Create a string parameter enclosed by double quotes depending on
 * characters in the string and its length.
 */
std::string
Node::ind_param(const char *s)
{
  std::stringstream ss;

//#define DELIMIT_PARAM
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

  if(s == NULL) {
    ss << S2_NULL_STR;
    goto out;
  }

  ss << escape_chars(s, ']', TRUE);

out:
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

  return ss.str();

#undef DELIMIT_PARAM    /* for debugging only */
} /* ind_param */

std::string
Node::ind_param(const std::string &s)
{
  return ind_param(s.c_str());
}

std::string
Node::ind_param(const std::string *s)
{
  if(s == NULL) return std::string(S2_NULL_STR);

  return ind_param(s->c_str());
}

std::string
Node::nodeToString(Node *n, uint indent, BOOL eval)
{
/* a pretty-printer macro to decide on printing default values */
#define IS_DEFAULT(v,def_val_indicator)\
  (!(opts.show_defaults) && (v) == (def_val_indicator))

  std::stringstream ss;
  int i;

  /* offset */
  i = indent;
  while(i-- > 0)
    ss << ' ';

  /* branch options */
  switch(n->COND) {
    case S2_COND_NONE: break;
    case S2_COND_OR: ss << "|| "; break;
    case S2_COND_AND: ss << "&& "; break;
  }

  if(n->REPEAT.type != S2_REPEAT_NONE) {
    ss << '>' << n->REPEAT.X;
    switch(n->REPEAT.type) {
      case S2_REPEAT_NONE: DM_ERR_ASSERT(_("S2_REPEAT_NONE\n")); break;
      case S2_REPEAT_OR: ss << "||"; break;
      case S2_REPEAT_AND: ss << "&&"; break;
      case S2_REPEAT_PAR: ss << ' '; break;
    }
    ss << n->REPEAT.Y << ' ';
  }

  if(!IS_DEFAULT(n->EVAL, S2_EVAL))
    ss << "eval=" << n->EVAL << ' ';

  if(!IS_DEFAULT(n->TIMEOUT, S2_TIMEOUT))
    ss << "timeout=" << n->TIMEOUT << ' ';

  if(!(IS_DEFAULT(n->match_opt.px, MATCH_PX) && 
       IS_DEFAULT(n->match_opt.pcre, MATCH_PCRE)))
  {
    /* we have non-default match options => print them */
    char *cstr = get_match_opts(n->match_opt);
    ss << "match=";
    ss << cstr; FREE(cstr);
    ss << ' ';
  }

  /* the action itself */
  ss << n->toString(eval);

  return ss.str();

#undef IS_DEFAULT
}

int
Node::print_node
(uint indent, FILE *file, BOOL eval, BOOL show_executed, BOOL show_evaluated)
{
  Node::print_node(this, indent, file, eval, show_executed, show_evaluated);

  return ERR_OK;
} /* print_node */

int
Node::print_node
(Node *n, uint indent, FILE *file, BOOL eval, BOOL show_executed, BOOL show_evaluated)
{
  if(show_executed && !n->has_executed)
    /* don't show node which hasn't executed */
    return ERR_OK;

//  S_P(&thread.print_mtx);
  if(n->has_executed && show_executed) {
    if(show_evaluated)
      fprintf(file, "%d/%d: ", n->executed, n->evaluated);
    else
      fprintf(file, "%d: ", n->executed);
  }

  fputs(nodeToString(n, indent, eval).c_str(), file);
  fputc('\n', file);
//  S_V(&thread.print_mtx);

  return ERR_OK;
} /* print_node */

/* 
 * Recursive preorder traversal of the tree.
 */
int
Node::print_tree(uint indent, FILE *file, BOOL pp_indent, BOOL eval, BOOL exec_eval)
{
  print_node(pp_indent? indent: OFFSET, file, eval, exec_eval, exec_eval);

  /* first print the children */
  if(child) {
    child->print_tree(pp_indent? (indent+opts.pp_indent): OFFSET, file, pp_indent, eval, exec_eval);
  }

  /* then parallel branches */
  if(par) {
    par->print_tree(pp_indent? indent: OFFSET, file, pp_indent, eval, exec_eval);
  }
  
  return ERR_OK;
} /* print_tree */

/*
 * Evaluate the expected string before PCRE matching.
 */
BOOL
Node::e_match(const std::string *expected, const char *received)
{
  BOOL match;
  std::string s_expected;

  if(expected == NULL || received == NULL) {
    DM_DBG(DM_N(1), "expected == %p || received == %p\n", expected, received);
    RETURN(!(match_opt.pcre & PCRE_NOTEMPTY));
  }

  s_expected = eval_str(expected, TRUE);
  match = pcre_matches(s_expected.c_str(), received, match_opt.pcre, WriteVariable);
  
  return match;
} /* e_match */

BOOL
Node::e_match(const char *expected, const char *received)
{
  BOOL match;
  std::string s_expected = eval_str(expected, TRUE);
  match = pcre_matches(s_expected.c_str(), received, match_opt.pcre, WriteVariable);

  return match;
} /* e_match */

/*** Private functions **********************************************/

/*
 * Try to evaluate an expression to a 64-bit integer.
 * 
 * Returns: 0 on success
 *          1 on failure and e is not modified
 */
extern int
str_expr2i(const char *cstr, int64_t *e)
{
  Expr expr(cstr);
  return expr.parse(e);
}

/*
 * Evaluate cstr and return evaluated std::string.
 */
std::string
Node::eval_str(const char *cstr, BOOL eval)
{
  int i, c;
  int slen;
  BOOL bslash = FALSE;  /* we had the '\\' character */
  std::string s;
  std::string var;
  enum s_eval { 
    sInit, sDollar, sVar, sEvar, sCounter, sExpr,  sRnd,  sDate,  sPrintf,
    sMd5,
  } state = sInit;
  static const char* state_name[] = {
    "0",   "",      "",   "ENV", "I",      "EXPR", "RND", "DATE", "PRINTF",
    "MD5", 
  };
  const char *opt;
  int opt_off;
  int brackets = 0;

  if(cstr == NULL) {
    DM_ERR_ASSERT(_("c_str == NULL\n"));
    return std::string(S2_NULL_STR);
  }

  if(!eval) return std::string(cstr);

  s.clear();
  slen = strlen(cstr);
  for(i = 0; i < slen; i++) {
//    DM_DBG(DM_N(0), "------>%d\n", i);
    c = cstr[i];
    opt = cstr + i;
    if(c == '{') brackets++;
    if(c == '}') brackets--;
//    DM_DBG(DM_N(5), "brackets=%d\n", brackets);

    switch (state) {
      case sInit:{
        if(c == '$') {
          if(!bslash) {
            state = sDollar;
            continue;
          }
        }
      }
      break;

      case sDollar:{
        if(c == '{') {
          /* we have a reference to a variable */
          var.clear();
          state = sVar;
          continue;
        } else if(OPL("ENV{")) {
          /* we have a reference to environment variable */
          var.clear();
          state = sEvar;
          i += opt_off - 1;
        } else if(OPL("I{")) {
          /* we have a reference to a repeat loop counter */
          var.clear();
          state = sCounter;
          i += opt_off - 1;
        } else if(OPL("EXPR{")) {
          /* expression evaluation */
          var.clear();
          state = sExpr;
          i += opt_off - 1;
        } else if(OPL("RND{")) {
          /* random values */
          var.clear();
          state = sRnd;
          i += opt_off - 1;
        } else if(OPL("DATE{")) {
          /* date/time */
          var.clear();
          state = sDate;
          i += opt_off - 1;
        } else if(OPL("PRINTF{")) {
          /* date/time */
          var.clear();
          state = sPrintf;
          i += opt_off - 1;
        } else if(OPL("MD5{")) {
          /* MD5 sum of a string */
          var.clear();
          state = sMd5;
          i += opt_off - 1;
        } else {
          /* return the borrowed dollar */
          s.push_back('$');
          state = sInit;
          break;
        }
        brackets++;
      }
      continue;

      case sVar:{
        if(c == '}' && !brackets) {
          /* we have a complete variable */
          if(var == "?") {	/* ${?} */
            /* return parent's execution value */
            int parent_executed = parent? parent->executed: 0;
            DM_DBG(DM_N(4), "returning parent's execution value %d\n", parent_executed);
            s.append(i2str(parent_executed));
            state = sInit;
            continue;
          }
	  
          if(var == "!") {	/* ${!} */
            /* return evaluation value of the branch at the same offset */
            int rpar_evaluated = rpar? rpar->evaluated: 0;
            DM_DBG(DM_N(4), "returning rpar evaluation value %d\n", rpar_evaluated);
            s.append(i2str(rpar_evaluated));
            state = sInit;
            continue;
          }
	  
          /* classic variable ${var} */
          var = eval_str(var.c_str(), eval);	/* evaluate things like: ${...${var}...} */

          const char *new_var;
          const char *read_var;
          if(var[0] == '-')
            /* issue no warnings when a variable is undefined */
	    read_var = var.substr(1).c_str();
          else read_var = var.c_str();

          DM_DBG(DM_N(4), "read_var=|%s|\n", read_var);
          if((new_var = ReadVariable(read_var)) != NULL) {
            DM_DBG(DM_N(4), "var %s=|%s|\n", var.c_str(), new_var);
            s.append(new_var);
          } else {
            /* the variable is not defined, output its name */
            if(var[0] != '-') {
              DM_WARN(ERR_WARN, _("variable `%s' is unset\n"), var.c_str());
            }
            s.append("${" + std::string(read_var) + "}");
          }
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sEvar:{
        if(c == '}' && !brackets) {
          /* we have a complete environment variable */
          var = eval_str(var.c_str(), eval);	/* evaluate things like: $ENV{...${var}...} */

          const char *env_var;
          env_var = getenv(var.c_str());
          
          if(env_var) {
            DM_DBG(DM_N(4), "env var %s=|%s|\n", var.c_str(), env_var);
            s.append(env_var);
          } else {
            /* the environment variable is not defined, output its name */
            DM_WARN(ERR_WARN, _("environment variable `%s' is unset\n"), var.c_str());
            s.append("$ENV{" + var + "}");
          }
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sCounter:{
        if(c == '}' && !brackets) {
          /* we have a reference to a repeat loop counter */
          var = eval_str(var.c_str(), eval);	/* evaluate things like: $I{...${var}...} */

          const char *word;
          word = var.c_str();
          char *endptr;
          int16_t i16;

          i16 = get_int16(word, &endptr, FALSE);
          if(endptr != word) {
            int64_t i64 = 0;
            Node *ptr_node = this;
            while(ptr_node && i16 >= 0) {
              if(ptr_node->REPEAT.type == S2_REPEAT_OR || 
                 ptr_node->REPEAT.type == S2_REPEAT_AND)
              {
                DM_DBG(DM_N(4), "found OR or AND repeat; i16=%d\n", i16);
                if(!i16) {
                  i64 = ptr_node->REPEAT.I;
                  DM_DBG(DM_N(4), "$I{%d}=%"PRIi64"\n", i16, i64);
                  break;
                } 
                i16--;
              }
              ptr_node = ptr_node->parent;
            }
            if(ptr_node == NULL)
              DM_WARN(ERR_WARN, _(FBRANCH"couldn't find repeat loop for repeat counter $I{%s}!\n"), row, executed, evaluated, word);

            DM_DBG(DM_N(4), "writing repeat counter value: %"PRIi64"\n", i64);
            s.append(i2str(i64));
          } else {
            DM_WARN(ERR_WARN, _("cannot evaluate loop nesting `%s': %s\n"), word, _(strerror(errno)));
            s.append("$I{" + var + "}");
          }
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sExpr:{
        if(c == '}' && !brackets) {
          /* we have a complete expression to evaluate */
          var = eval_str(var.c_str(), eval);	/* evaluate things like: $EXPR{...${var}...} */

          int64_t e = 0;
          DM_DBG(DM_N(4), "expr=|%s|\n", var.c_str());
          if(str_expr2i(var.c_str(), &e)) {
            DM_ERR(ERR_ERR, _(FBRANCH"couldn't evaluate expression `%s'\n"), row, executed, evaluated, var.c_str());
          } else s.append(i2str(e));
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sRnd:{
        if(c == '}' && !brackets) {
          /* we have a maximum+1 random number */
          var = eval_str(var.c_str(), eval);	/* evaluate things like: $RND{...${var}...} */

          int64_t e = 0;
          if(str_expr2i(var.c_str(), &e)) {
            DM_ERR(ERR_ERR, _(FBRANCH"couldn't evaluate expression `%s'\n"), row, executed, evaluated, var.c_str());
          } else {
            /* srandom() is done elsewhere */
            s.append(i2str(random() % e));
          }
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sDate:{
        if(c == '}' && !brackets) {
          struct timeval timestamp;
          struct tm *now;
          std::string date;
        
          gettimeofday(&timestamp, NULL);
          now = localtime(&(timestamp.tv_sec));

          if(var == "") {
            /* use the default format */
            date = ssprintf("%04d-%02d-%02d@%02d:%02d:%02d.%06lu",
                     now->tm_year+1900, now->tm_mon+1, now->tm_mday,
                     now->tm_hour, now->tm_min, now->tm_sec, timestamp.tv_usec);
          } else {
            int j, esc = 0, len = var.length();
            for(j = 0; j < len; j++) {
              if(!esc) {
                if(var[j] == '%') esc = 1;
                else s.push_back(var[j]);
                continue;
              }

              /* escape */
              esc = 0;
              switch(var[j]) {
                case 'C': s.append(ssprintf("%02d", (now->tm_year+1900)/100)); break;
                case 'D': s.append(ssprintf("%02d/%02d/%02d", now->tm_mon+1, now->tm_mday, now->tm_year % 100)); break;
                case 'd': s.append(ssprintf("%02d", now->tm_mday)); break;
                case 'e': s.append(ssprintf("%2d", now->tm_mday)); break;
                case 'F': s.append(ssprintf("%04d-%02d-%02d", now->tm_year+1900, now->tm_mon+1, now->tm_mday)); break;
                case 'H': s.append(ssprintf("%02d", now->tm_hour)); break;
                case 'k': s.append(ssprintf("%2d", now->tm_hour)); break;
                case 'I': s.append(ssprintf("%02d", (now->tm_hour % 12) ? (now->tm_hour % 12) : 12)); break;
                case 'l': s.append(ssprintf("%2d", (now->tm_hour % 12) ? (now->tm_hour % 12) : 12)); break;
                case 'j': s.append(ssprintf("%02d", now->tm_yday+1)); break;
                case 'M': s.append(ssprintf("%02d", now->tm_min)); break;
                case 'm': s.append(ssprintf("%02d", now->tm_mon+1)); break;
                case 'n': s.push_back('\n'); break;
                case 'N': s.append(ssprintf("%06d", timestamp.tv_usec)); break; /* cut to 6 digits */
                case 'R': s.append(ssprintf("%02d:%02d", now->tm_hour, now->tm_min)); break;
                case 's': s.append(ssprintf("%d", timestamp.tv_sec)); break;
                case 'S': s.append(ssprintf("%02d", now->tm_sec)); break;
                case 't': s.push_back('\t'); break;
                case 'u': s.append(ssprintf("%d", now->tm_wday)); break;
                case 'V': s.append(ssprintf("%d", week(now))); break;
                case 'T': s.append(ssprintf("%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec)); break;
                case 'Y': s.append(ssprintf("%04d", now->tm_year+1900)); break;
                case 'y': s.append(ssprintf("%02d", now->tm_year%100)); break;
                case 'w': s.append(ssprintf("%d", now->tm_wday-1)); break;
                default:
                  s.push_back(var[j]);
              }
            }
            if(esc) s.push_back('%');
          }
          s.append(date);
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sPrintf:{
        if(c == '}' && !brackets) {
          char **argv;
          int spaces = 0;
          int j, plen;
          const char *var_cstr = NULL;

          var = eval_str(var.c_str(), eval);	/* evaluate things like: $PRINTF{...${var}...} */
          var_cstr = var.c_str();
          plen = var.length();
          for(j = 0; j < plen; j++) {
            /* count the number of spaces to guess the maximum number of $PRINTF parameters (-2) */
            if(IS_WHITE(var_cstr[j])) spaces++;
          }
          if((argv = (char **)malloc(sizeof(char **) * (spaces + 2))) == (char **)NULL) {
            DM_ERR(ERR_SYSTEM, _("malloc failed\n"));
            return s;
          }
          std::string target;
          int l = 0;
          for(j = 0;; j++) {
            int chars;
            chars = get_dq_param(target, var_cstr + l);
            if(chars == 0) break;
            if((argv[j] = (char *)strdup(target.c_str())) == (char *)NULL) {
              DM_ERR(ERR_SYSTEM, _("strdup failed\n"));
              return s;
            }
            l += chars;
          }
          argv[j] = 0;

          s.append(ssprintf_chk(argv));
          for(;j >= 0; j--) {
            FREE(argv[j]);
          }
          FREE(argv);
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sMd5:{
        if(c == '}' && !brackets) {
          /* we have a maximum+1 random number */
          var = eval_str(var.c_str(), eval);	/* evaluate things like: $MD5{...${var}...} */
          char md5str[33];
          gen_md5(md5str, var.c_str());
          s.append(md5str);
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

    }

    /* no TAGs found, simply copy the characters */
    s.push_back(c);

    if(c == '\\') {
      if(bslash) {
        /* two backslashes => no quoting */
        bslash = FALSE;
      } else {
        bslash = TRUE;
      }
    }
  }

  /* sanity checks */
  if(state == sDollar) {
    s.push_back('$');
  }
  
  if(state != sInit) {
    DM_WARN(ERR_WARN, _("unterminated $%s{\n"), state_name[state]);
    s.append("$" + std::string(state_name[state]) + "{" + var);
  }
  
  return s;
} /* eval_str */

std::string
Node::eval_str(const std::string *s, BOOL eval)
{
  if(s == NULL) {
    DM_ERR_ASSERT(_("s == NULL\n"));
    return std::string(S2_NULL_STR);
  }
  return eval_str(s->c_str(), eval);
} /* eval_str */

#define _EVAL2INT(u,s)\
u##int##s##_t \
Node::eval2##u##int##s(const std::string *str)\
{\
  std::string str_val;\
  char *endptr;\
\
  if(str == NULL) {\
    DM_DBG(DM_N(0), _("returning default value 0"));\
    return 0;\
  }\
  str_val = eval_str(str, TRUE);\
\
  return get_##u##int##s(str_val.c_str(), &endptr, TRUE);\
}
_EVAL2INT(,32);
_EVAL2INT(,64);
_EVAL2INT(u,32);
_EVAL2INT(u,64);
#undef _EVAL2INT

#define _EVAL2PINT(u,s)\
p##u##int##s##_t \
Node::eval2p##u##int##s(const std::string *str)\
{\
  std::string str_val;\
  char *endptr;\
  p##u##int##s##_t r;\
\
  if(str == NULL) goto ret_null;\
  str_val = eval_str(str, TRUE);\
\
  r.v = get_##u##int##s(str_val.c_str(), &endptr, TRUE);\
  if(str_val.c_str() == endptr)\
    /* couldn't convert to int */\
    goto ret_null;\
\
  r.p = &r.v;\
  return r;\
\
ret_null:\
  r.p = NULL;\
  return r;\
}
_EVAL2PINT(,32);
_EVAL2PINT(,64);
_EVAL2PINT(u,32);
_EVAL2PINT(u,64);
#undef _EVAL2PINT

/*
 * Evaluate std::vector <std::string *>.
 */
std::vector <std::string *>
Node::eval_vec_str(const std::vector <std::string *> &v, BOOL eval)
{
  std::vector <std::string *> ev;

  for(uint i = 0; i < v.size(); i++) {
    if(v[i]) ev.push_back(new std::string(eval_str(v[i], eval).c_str()));
    else ev.push_back((std::string *)NULL);
  }

  return ev;
} /* eval_vec_str */

/*
 * Evaluate std::vector <std::string *>.
 */
#define _EVAL_VEC_INT(u,s,fp)\
std::vector <u##int##s##_t> \
Node::eval_vec_##u##int##s(const std::vector <std::string *> &v, BOOL eval)\
{\
  std::vector <u##int##s##_t> ev;\
\
  for(uint c = 0; c < v.size(); c++) {\
    if(v[c]) {\
      u##int##s##_t i = eval2##u##int##s(v[c]);\
      DM_DBG(DM_N(3), "evaluating[%u] to %"fp"\n", c, i);\
      ev.push_back(i);\
    } else {\
      DM_DBG(DM_N(3), "default evaluation[%u] to 0\n", c);\
      ev.push_back(0);\
    }\
  }\
\
  return ev;\
}
_EVAL_VEC_INT(,32,"ld");
_EVAL_VEC_INT(,64,"lld");
_EVAL_VEC_INT(u,32,"lu");
_EVAL_VEC_INT(u,64,"llu");
#undef _EVAL_VEC_INT

/*
 * Evaluate std::vector <std::string *> to a std::vector of pointers to integers.
 */
#define _EVAL_VEC_PINT(u,s,fp)\
std::vector <u##int##s##_t *> \
Node::eval_vec_p##u##int##s(const std::vector <std::string *> &v, BOOL eval)\
{\
  std::vector <u##int##s##_t *> ev;\
\
  for(uint c = 0; c < v.size(); c++) {\
    u##int##s##_t *i = NULL;\
    if(v[c]) {\
      p##u##int##s##_t p = eval2p##u##int##s(v[c]);\
      if(p.p) {\
        i = (u##int##s##_t *)malloc(sizeof(u##int##s##_t));\
        if(!i) {\
          DM_ERR(ERR_SYSTEM, _("malloc failed\n"));\
          RETURN(ev);\
        }\
        *i = *p.p;\
      }\
    }\
    if(i) DM_DBG(DM_N(3), "evaluating[%u] to %"fp"\n", c, *i);\
    else DM_DBG(DM_N(3), "evaluating[%u] to NULL\n", c);\
    ev.push_back(i);\
  }\
\
  return ev;\
}
_EVAL_VEC_PINT(,32,"ld");
_EVAL_VEC_PINT(,64,"lld");
_EVAL_VEC_PINT(u,32,"lu");
_EVAL_VEC_PINT(u,64,"llu");
#undef _EVAL_VEC_PINT
