#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"                  /* Node, nAssign, ... */

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
 * nAssign constructor
 */
nAssign::nAssign()
{
  /* initialisation */
  init();
}

/*
 * Initialise nAssign request
 */
void
nAssign::init()
{
  var = NULL;
  val = NULL;
}

/*
 * nAssign copy constuctor
 */
nAssign::nAssign(Node &node)
{
  Node::init(node);
  init();
}

/*
 * nAssign destructor
 */
nAssign::~nAssign()
{
  DELETE(var);
  DELETE(val);
}

int
nAssign::exec()
{
  DM_DBG_I;

  if(var == NULL || val == NULL) {
    /* var/val is never NULL (see parser.cpp) */
    DM_ERR_ASSERT(_("var == NULL || val == NULL\n"));
    RETURN(ERR_ASSERT);
  }

  WriteVariable(EVAL2CSTR(var), EVAL2CSTR(val));

  RETURN(ERR_OK);
}

std::string
nAssign::toString(BOOL eval)
{
  BOOL quote = TRUE;
  std::stringstream ss;

  ss << "ASSIGN";
  SS_DQ(" ", var);
  SS_DQ(" ", val);

  return ss.str();
}
