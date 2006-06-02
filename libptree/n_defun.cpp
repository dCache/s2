#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"                  /* Node, nDefun, ... */

#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "free.h"               /* FREE(), DELETE() */
#include "io.h"                 /* file_ropen(), ... */
#include "max.h"		/* UPDATE_MAX() */

#include <errno.h>              /* errno */
#include <time.h>               /* nanosleep() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * nDefun constructor
 */
nDefun::nDefun()
{
  /* initialisation */
  init();
}

/*
 * Initialise nDefun request
 */
void
nDefun::init()
{
  name = NULL;
}

/*
 * nDefun copy constuctor
 */
nDefun::nDefun(Node &node)
{
  init();
  Node::init(node);
}

/*
 * nDefun destructor
 */
nDefun::~nDefun()
{
  DELETE(name);
  DELETE_VEC(params);
  DELETE_VEC(params_ref);
}

int
nDefun::exec(Process *proc)
{
  DM_DBG_I;

  RETURN(ERR_OK);
}

std::string
nDefun::toString(Process *proc)
{
  BOOL quote = TRUE;
  uint params_size = params.size();
  uint params_ref_size = params_ref.size();
  std::stringstream ss;

  ss << "DEFUN";

  if(name == NULL) {
    DM_ERR_ASSERT(_("name == NULL\n"));
    return ss.str();
  }
  
  SS_DQ(" ", name);	/* function name */

  /* function definition parameters */
  for(uint u = 0; u < params_size; u++) {
    SS_DQ(" ", params[u]);
  }

  if(params_ref_size) ss << " :";

  /* vector of values returned to parent's scope */
  for(uint u = 0; u < params_ref_size; u++) {
    SS_DQ(" ", params_ref[u]);
  }

  return ss.str();
}
