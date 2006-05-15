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
  overwrite = NULL;
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
  DELETE(overwrite);
  DELETE_VEC(var);
  DELETE_VEC(val);
}

int
nAssign::exec(Process *proc)
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

    BOOL write = overwrite == NULL ||				/* overwrite by default */
                 proc->eval2int32(overwrite) ||			/* evaluate the overwrite parameter */
                 proc->ReadVariable(s_var.c_str()) == NULL;	/* write if variable not defined */

    DM_DBG(DM_N(3), "ASSIGN write=%u |%s| |%s|\n", write, s_var.c_str(), s_val.c_str());

    if(write)
      proc->WriteVariable(s_var.c_str(), s_val.c_str());
  }

  RETURN(ERR_OK);
}

std::string
nAssign::toString(Process *proc)
{
  BOOL quote = TRUE;
  uint var_size = var.size();
  std::stringstream ss;

  ss << "ASSIGN";
  SS_P_DQ(overwrite);
  for(uint u = 0; u < var_size; u++) {
    SS_DQ(" ", var[u]);
    SS_DQ(" ", val[u]);
  }

  return ss.str();
}
