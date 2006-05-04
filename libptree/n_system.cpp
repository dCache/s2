#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"                  /* Node, nSystem, ... */

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
 * nSystem constructor
 */
nSystem::nSystem()
{
  /* initialisation */
  init();
}

/*
 * Initialise nSystem request
 */
void
nSystem::init()
{
  out = NULL;
  cmd = NULL;
}

/*
 * nSystem copy constuctor
 */
nSystem::nSystem(Node &node)
{
  Node::init(node);
  init();
}

/*
 * nSystem destructor
 */
nSystem::~nSystem()
{
  DELETE(out);
  DELETE(cmd);
}

int
nSystem::exec(Process *proc)
{
  DM_DBG_I;

  BOOL match;
  int rval;
  FILE *fpin;
  std::string recv;
  std::string s_cmd;
  char c;

  s_cmd = Process::eval_str(cmd, proc);

  if((fpin = popen(s_cmd.c_str(), "r")) == NULL) {
    DM_ERR(ERR_SYSTEM, _("popen failed: %s\n"), _(strerror(errno)));
    RETURN(ERR_SYSTEM);
  }

  /* read the captured output from the pipe to s */
  while((c = getc (fpin)) != EOF) {
    recv += c;
  }

  if((rval = pclose(fpin)) == -1) {
    DM_ERR(ERR_SYSTEM, _("pclose failed: %s\n"), _(strerror(errno)));
    RETURN(ERR_SYSTEM);
  }

  if(out == NULL)
    /* don't capture stdout, show it */
    cout << recv;

  if(rval)
    /* shortened evaluation */
    RETURN(WEXITSTATUS(rval));

  /* matching */
  if(out) {
    match = proc->e_match(out, recv.c_str());
    if(!match) RETURN(ERR_ERR);
  }

  RETURN(ERR_OK);
}

std::string
nSystem::toString(Process *proc)
{
  BOOL quote = TRUE;
  std::stringstream ss;

  ss << "SYSTEM";
  SS_P_DQ(out);
  quote = FALSE;        /* don't quote the system command */
  SS_DQ(" ", cmd);

  return ss.str();
}
