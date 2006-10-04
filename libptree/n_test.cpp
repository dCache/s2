#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

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

  std::string s = Expr::eval2s(expr->c_str(), proc);

  DM_DBG(DM_N(3), "|%s|\n", s.c_str());

  if(s.length() == 0 || (s.length() == 1 && s[0] == '0')) return ERR_ERR;

  RETURN(ERR_OK);
}

std::string
nTest::toString(Process *proc)
{
  std::stringstream ss;

  ss << "TEST";
  if(!expr)
    /* shouldn't ever happen, don't bother with ASSERT */
    return ss.str();
  
  if(!(expr->length() == 0 || (expr->length() == 1) && (*expr)[0] == '0')) {
    ss << " " << Lex::eval(expr->c_str(), proc);
  }

  return ss.str();
}
