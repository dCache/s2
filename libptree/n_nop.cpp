#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"                  /* Node, nNop, ... */

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
 * nNop constructor
 */
nNop::nNop()
{
  /* initialisation */
  init();
}

/*
 * Initialise nNop request
 */
void
nNop::init()
{
  val = NULL;
}

/*
 * nNop copy constuctor
 */
nNop::nNop(Node &node)
{
  Node::init(node);
  init();
}

/*
 * nNop destructor
 */
nNop::~nNop()
{
  DELETE(val);
}

int
nNop::exec(Process *proc)
{
  DM_DBG_I;

  int rval;
  std::string s_val;
  const char *word;
  char *endptr;

  s_val = Process::eval_str(val, proc);
  word = s_val.c_str();

  DM_DBG(DM_N(3), "NOP val=|%s|\n", word);
  rval = get_int32(s_val.c_str(), &endptr, FALSE);
  if(endptr == word) {
    DM_ERR(ERR_ERR, _("cannot evaluate NOP value `%s': %s\n"), word, _(strerror(errno)));
    RETURN(ERR_ERR);
  }

  RETURN(rval);
}

std::string
nNop::toString(Process *proc)
{
  BOOL quote = TRUE;
  std::stringstream ss;

  ss << "NOP";
  SS_DQ(" ", val);

  return ss.str();
}
