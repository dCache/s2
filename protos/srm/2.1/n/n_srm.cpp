#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n_srm.h"
#include "srm2api.h"
#include "srm_soap27.h"

#include "match.h"              /* PCRE_ANCHORED, ... */
#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "free.h"               /* FREE(), DELETE() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * SRM2 constructor
 */
SRM2::SRM2()
{
  /* initialisation */
  srm_endpoint = NULL;
  userID = NULL;

  returnStatus.explanation = NULL;
  returnStatus.statusCode = NULL;

  /* PCRE options */
  match_opt.px = MATCH_PX;
  match_opt.pcre = MATCH_PCRE;
}

/*
 * SRM2 destructor
 */
SRM2::~SRM2()
{
  DELETE(srm_endpoint);
  DELETE(userID);

  DELETE(returnStatus.explanation);
  DELETE(returnStatus.statusCode);
}

/*
 * Use PCRE to match return status.
 */
int
SRM2::matchReturnStatus(srm__TReturnStatus *returnStatus, Process *proc)
{
  DM_DBG_I;
  BOOL match;

  /* returnStatus.explanation */
  if(returnStatus && returnStatus->explanation) {
    match = proc->e_match(SRM2::returnStatus.explanation,
                          returnStatus->explanation->c_str());
    if(!match) RETURN(ERR_ERR);
  }

  /* returnStatus.statusCode */
  if(returnStatus) {
    match = proc->e_match(SRM2::returnStatus.statusCode,
                          getTStatusCode(returnStatus->statusCode).c_str());
    if(!match) RETURN(ERR_ERR);
  }

  RETURN(ERR_OK);
}

/*
 * Evaluate std::vector <std::string *>.
 */
std::vector <const long int *>
SRM2::eval_vec_overwrite_mode(const std::vector <std::string *> &v, Process *proc)
{
  std::vector <const long int *> ev;

  for(uint c = 0; c < v.size(); c++) {
    if(v[c]) {
      const long int *i = getTOverwriteMode(proc->eval_str(v[c], proc).c_str());
      DM_DBG(DM_N(3), "evaluating[%u] to %ld\n", c, *i);
      ev.push_back(i);
    } else {
      DM_DBG(DM_N(3), "default evaluation[%u] to 0\n", c);
      ev.push_back(0);
    }
  }

  return ev;
} /* eval_vec_overwrite_mode */

/*
 * Evaluate std::vector <std::string *>.
 */
std::vector <long int>
SRM2::eval_vec_permission_mode(const std::vector <std::string *> &v, Process *proc)
{
  std::vector <long int> ev;

  for(uint c = 0; c < v.size(); c++) {
    if(v[c]) {
      long int i = *getTPermissionMode(proc->eval_str(v[c], proc).c_str());
      DM_DBG(DM_N(3), "evaluating[%u] to %ld\n", c, i);
      ev.push_back(i);
    } else {
      DM_DBG(DM_N(3), "default evaluation[%u] to 0\n", c);
      ev.push_back(0);
    }
  }

  return ev;
} /* eval_vec_permission_mode */
