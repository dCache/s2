#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n_srm.h"
#include "srm2api.h"
#include "srm_soap27.h"

#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "free.h"               /* FREE(), DELETE() */
#include "str.h"		/* dq_param() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * srmRm request constuctor
 */
srmRm::srmRm()
{
  init();
}

/*
 * Initialise srmRm request
 */
void
srmRm::init()
{
  /* request (parser/API) */

  /* response (parser) */
  fileStatuses = NULL;
}

/*
 * srmRm request copy constuctor
 */
srmRm::srmRm(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmRm request destructor
 */
srmRm::~srmRm()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE_VEC(path.SURLOrStFN);
  DELETE_VEC(path.storageSystemInfo);

  /* response (parser) */
  DELETE(fileStatuses);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmRm::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(Rm);
}

int
srmRm::exec(Process *proc)
{
#define EVAL_VEC_STR_RM(vec) vec = proc->eval_vec_str(srmRm::vec)
  DM_DBG_I;
  BOOL match = FALSE;

  tSurlInfoArray path;

  EVAL_VEC_STR_RM(path.SURLOrStFN);
  EVAL_VEC_STR_RM(path.storageSystemInfo);

#ifdef SRM2_CALL
  NEW_SRM_RET(Rm);

  Rm(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    path,
    resp
  );
#endif

  DELETE_VEC(path.SURLOrStFN);
  DELETE_VEC(path.storageSystemInfo);

  /* matching */
  if(!resp || !resp->srmRmResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfFileStatus */
  match = proc->e_match(fileStatuses, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmRmResponse->returnStatus, proc));
#undef EVAL_VEC_STR_RM
}

std::string
srmRm::toString(Process *proc)
{
#define EVAL_VEC_STR_RM(vec) EVAL_VEC_STR(srmRm,vec)
  DM_DBG_I;
  
  GET_SRM_RESP(Rm);
  BOOL quote = TRUE;
  std::stringstream ss;

  tArrayOfPutFileRequests_ path;

  EVAL_VEC_STR_RM(path.SURLOrStFN);
  EVAL_VEC_STR_RM(path.storageSystemInfo);

  /* request */  
  SS_SRM("srmRm");
  SS_P_DQ(userID);
  SS_VEC_DEL(path.SURLOrStFN);
  SS_VEC_DEL(path.storageSystemInfo);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmRmResponse) RETURN(ss.str());

  ss << arrayOfFileStatusToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmRmResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_RM
}

std::string
srmRm::arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(Rm);
  std::stringstream ss;

  if(!resp || !resp->srmRmResponse) RETURN(ss.str());

  if(resp->srmRmResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;

    std::vector<srm__TSURLReturnStatus *> v = resp->srmRmResponse->arrayOfFileStatuses->surlReturnStatusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_PAR_VAL(surl);
    }
  }
  
  RETURN(ss.str());
}
