#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"                  /* Node, nTest, ... */

#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "free.h"               /* FREE(), DELETE() */
#include "io.h"                 /* file_ropen(), ... */
#include "expr.h"		/* Expr */
#include "str.h"		/* dq_param() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * nTest constructor
 */
nTest::nTest()
{
  /* initialisation */
  init();
}

/*
 * Initialise nTest request
 */
void
nTest::init()
{
  expr = NULL;
}

/*
 * nTest copy constuctor
 */
nTest::nTest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * nTest destructor
 */
nTest::~nTest()
{
  DELETE(expr);
}

int
nTest::exec(Process *proc)
{
  DM_DBG_I;

  Expr e = Expr(expr->c_str(), proc);
  std::string s = e.parse().toString();

  DM_DBG(DM_N(3), "|%s|\n", s.c_str());

  if(s.length() == 0 || (s.length() == 1 && s[0] == '0')) return ERR_ERR;

  RETURN(ERR_OK);
}

std::string
nTest::toString(Process *proc)
{
  BOOL quote = TRUE;
  std::stringstream ss;

  ss << "TEST";
  if(!expr)
    /* shouldn't ever happen, don't bother with ASSERT */
    return ss.str();
  
  if(!(expr->length() == 0 || (expr->length() == 1) && (*expr)[0] == '0')) {
    const char *target_cstr = expr->c_str();
    int l = 0;
    /* process and double-quote TEST arguments separately (e.g.: TEST $empty == "") */
    while(1) {
      BOOL ws_only;
      int chars;
      std::string arg;
      chars = get_dq_param(arg, target_cstr + l, ws_only);
      arg = Process::eval_str(arg.c_str(), proc);
      DM_DBG(DM_N(5), "arg%d=|%s|\n", l, arg.c_str());
      if(ws_only) break;
      ss << " " << dq_param(arg, quote);
      l += chars;
    }
  }

  return ss.str();
}
