#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"                  /* Node, nSetenv, ... */

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
 * nSetenv constructor
 */
nSetenv::nSetenv()
{
  /* initialisation */
  init();
}

/*
 * Initialise nSetenv request
 */
void
nSetenv::init()
{
  overwrite = NULL;
}

/*
 * nSetenv copy constuctor
 */
nSetenv::nSetenv(Node &node)
{
  init();
  Node::init(node);
}

/*
 * nSetenv destructor
 */
nSetenv::~nSetenv()
{
  DELETE(overwrite);
  DELETE_VEC(var);
  DELETE_VEC(val);
}

int
nSetenv::exec(Process *proc)
{
  DM_DBG_I;
  uint var_size = var.size();
  uint val_size = val.size();

  if(var_size == 0 || val_size == 0) {
    /* ASSIGN ; (no operands, NOP) */
    RETURN(ERR_OK);
  }

  if(var_size != val_size) {
    DM_ERR_ASSERT(_("var.size(%u) != val.size(%u)\n"), var_size, val_size);
    RETURN(ERR_ASSERT);
  }

  for(uint u = 0; u < var_size; u++) {
    std::string s_var, s_val;
    s_var = Process::eval_str(var[u], proc);
    s_val = Process::eval_str(val[u], proc);

    BOOL write = overwrite == NULL ||			/* overwrite by default */
                 proc->eval2int32(overwrite);		/* evaluate the overwrite parameter */

    DM_DBG(DM_N(3), "SETENV write=%u |%s| |%s|\n", write, s_var.c_str(), s_val.c_str());

    if(setenv(s_var.c_str(), s_val.c_str(), write)) {
      DM_ERR(ERR_SYSTEM, _("setenv failed: %s\n"), _(strerror(errno)));
      RETURN(ERR_SYSTEM);
    }
  }

  RETURN(ERR_OK);
}

std::string
nSetenv::toString(Process *proc)
{
  BOOL quote = TRUE;
  uint var_size = var.size();
  std::stringstream ss;

  ss << "SETENV";
  SS_P_DQ(overwrite);
  for(uint u = 0; u < var_size; u++) {
    SS_DQ(" ", var[u]);
    SS_DQ(" ", val[u]);
  }

  return ss.str();
}
