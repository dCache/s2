#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"                  /* Node, nFun, ... */
#include "parse.h"		/* TFunctions */

#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "free.h"               /* FREE(), DELETE() */
#include "io.h"                 /* file_ropen(), ... */
#include "max.h"		/* UPDATE_MAX() */
#include "str.h"		/* dq_param() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * nFun constructor
 */
nFun::nFun()
{
  /* initialisation */
  init();
}

/*
 * Initialise nFun request
 */
void
nFun::init()
{
  name = NULL;
  nDefunNode = NULL;	/* DON'T free this, reference to DEFUN */
}

/*
 * nFun copy constuctor
 */
nFun::nFun(Node &node)
{
  init();
  Node::init(node);
}

/*
 * nFun destructor
 */
nFun::~nFun()
{
  DELETE(name);
  DELETE_VEC(args);
  DELETE_VEC(args_ref);
}

int
nFun::exec(Process *proc)
{
  DM_DBG_I;

  RETURN(ERR_OK);
}

int
nFun::exec(Process *proc, Process &proc_fun)
{
  DM_DBG_I;
  TFunctions::iterator iter;		/* name/function object pair */

  if(!nDefunNode) {
    if((iter = gl_fun_tab.find(name->c_str())) == gl_fun_tab.end()) {
      /* Function `name' is not defined. */
      DM_ERR(ERR_ERR, _("function `%s' not defined\n"), name->c_str());
      RETURN(ERR_ERR);
    }
  
    nDefunNode = iter->second;
  }

  /* check number of parameters */
  uint params_size = nDefunNode->params.size();
  uint params_ref_size = nDefunNode->params_ref.size();
  uint args_size = args.size();
  uint args_ref_size = args_ref.size();
  
  if(args_size != params_size)
  {
    DM_ERR(ERR_ERR, _("number of function `%s' parameters (%u) != number of its arguments (%u) passed by value\n"), name->c_str(), params_size, args_size);
    RETURN(ERR_ERR);
  }
  
  if(params_ref_size != args_ref_size)
  {
    DM_ERR(ERR_ERR, _("number of function `%s' parameters (%u) != number of its arguments (%u) passed by reference\n"), name->c_str(), params_ref_size, args_ref_size);
    RETURN(ERR_ERR);
  }

  proc_fun = Process(nDefunNode, proc, NULL);
  proc_fun.fun = TRUE;			/* this is a function call */

  /* check variable table */  
  if(proc_fun.var_tab != NULL) {
    DM_ERR_ASSERT(_("proc_fun.var_tab != NULL\n"));
    RETURN(ERR_ASSERT);
  }
  proc_fun.var_tab = new Vars_t();
  DM_DBG(DM_N(2), "created local variable table %p for function `%s'\n", proc_fun.var_tab, nDefunNode->name->c_str());

  DM_DBG(DM_N(3), "gl_var_tab=%p, proc->var_tab=%p, proc_fun.var_tab=%p\n", &gl_var_tab, proc->var_tab, proc_fun.var_tab);
  
  /* pass arguments to the function by value (evaluate the arguments) */
  for(uint u = 0; u < args_size; u++) {
    DM_DBG(DM_N(3), "FUN arg[%u]=|%s|\n", u, Process::eval_str(args[u], proc).c_str());
    proc_fun.WriteVariable(nDefunNode->params[u]->c_str(),
                           Process::eval_str(args[u], proc).c_str(), FALSE);
  }

  /* pass arguments to the function "by reference" (do not evaluate the arguments) */
  for(uint u = 0; u < args_ref_size; u++) {
    const char *v = proc->ReadVariable(args_ref[u]->c_str());
    if(v) {
      /* args_ref[u] is set, write it to params_ref[u] */
      proc_fun.WriteVariable(nDefunNode->params_ref[u]->c_str(), v, FALSE);
    }
  }

  proc_fun.FUN_OFFSET = nDefunNode->OFFSET - proc->n->OFFSET;

  RETURN(ERR_OK);
}

void
nFun::exec_finish(Process *proc, Process &proc_fun)
{
  uint params_ref_size = nDefunNode->params_ref.size();

  /* simulate "passed by reference", i.e.: write into parent's scope */
  for(uint u = 0; u < params_ref_size; u++) {
    const char *v = proc_fun.ReadVariable(nDefunNode->params_ref[u]->c_str());
    if(v) {
      /* params/args_ref[u] is set */
      proc->WriteVariable(args_ref[u]->c_str(), v, FALSE);
    }
  }
}

std::string
nFun::toString(Process *proc)
{
  BOOL quote = TRUE;
  uint args_size = args.size();
  uint args_ref_size = args_ref.size();
  std::stringstream ss;

  ss << "FUN";

  if(name == NULL) {
    DM_ERR_ASSERT(_("name == NULL\n"));
    return ss.str();
  }
  
  SS_DQ(" ", name);	/* function name */

  /* function call arguments */
  for(uint u = 0; u < args_size; u++) {
    SS_DQ(" ", args[u]);
  }

  if(args_ref_size) ss << " :";

  /* vector of values returned to parent's scope */
  for(uint u = 0; u < args_ref_size; u++) {
    SS_DQ(" ", args_ref[u]);
  }

  return ss.str();
}

std::string
nFun::getByRefVals(Process *proc)
{
  BOOL quote = TRUE;
  uint args_ref_size = args_ref.size();
  std::stringstream ss;

  /* vector of values returned to parent's scope */
  for(uint u = 0; u < args_ref_size; u++) {
    const char *var = proc->ReadVariable(args_ref[u]->c_str());
//    std::string s_var = var? std::string(var) : "";

    if(!var) var = S2_NULL_STR;

    SS_DQ(" ", var);
  }

  return ss.str();
}
