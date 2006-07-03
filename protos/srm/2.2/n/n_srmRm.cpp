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
  DELETE_VEC(SURL);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

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

  std::vector <std::string *> SURL;
  tStorageSystemInfo storageSystemInfo;

  EVAL_VEC_STR_RM(SURL);
  EVAL_VEC_STR_RM(storageSystemInfo.key);
  EVAL_VEC_STR_RM(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(Rm);

  Rm(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    SURL,
    storageSystemInfo,
    resp
  );
#endif

  DELETE_VEC(SURL);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* matching */
  if(!resp || !resp->srmRmResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfPathDetails */
  EAT_MATCH(fileStatuses, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());

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

  std::vector <std::string *> SURL;
  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_RM(SURL);
  EVAL_VEC_STR_RM(storageSystemInfo.key);
  EVAL_VEC_STR_RM(storageSystemInfo.value);

  /* request */  
  SS_SRM("srmRm");
  SS_P_DQ(authorizationID);
  SS_VEC_DEL(SURL);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);

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
    std::vector<srm__TSURLReturnStatus *> v = resp->srmRmResponse->arrayOfFileStatuses->statusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(surl);
      SS_P_VEC_SRM_RETSTAT(status);
    }
  }
  
  RETURN(ss.str());
}
