#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"                  /* Node, nMatch, ... */

#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "free.h"               /* FREE(), DELETE() */
#include "io.h"                 /* file_ropen(), ... */
#include "str.h"

#include <signal.h>             /* signal() */
#include <stdlib.h>             /* exit(), system() */
#include <stdio.h>              /* stderr */
#include <errno.h>              /* errno */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * nMatch constructor
 */
nMatch::nMatch()
{
  /* initialisation */
  init();
}

/*
 * Initialise nMatch request
 */
void
nMatch::init()
{
  expected = NULL;
  received = NULL;
}

/*
 * nMatch copy constuctor
 */
nMatch::nMatch(Node &node)
{
  Node::init(node);
  init();
}

/*
 * nMatch destructor
 */
nMatch::~nMatch()
{
  DELETE(expected);
  DELETE(received);
}

int
nMatch::exec()
{
  DM_DBG_I;

  int rval;

  if(expected == NULL || received == NULL) {
    /* expected/received is never NULL (see parser.cpp) */
    DM_DBG(DM_N(1), "expected == %p || received == %p\n", expected, received);
    RETURN(!(match_opt.pcre & PCRE_NOTEMPTY));
  }

  rval = e_match(EVAL2CSTR(expected), EVAL2CSTR(received))? ERR_OK: ERR_ERR;

  RETURN(rval);
}

std::string
nMatch::toString(BOOL eval)
{
  BOOL quote = TRUE;
  std::stringstream ss;

  ss << "MATCH";
  SS_DQ(" ", expected);
  SS_DQ(" ", received);

  return ss.str();
}
