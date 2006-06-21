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
 * srmChangeFileStorageType request constuctor
 */
srmChangeFileStorageType::srmChangeFileStorageType()
{
  init();
}

/*
 * Initialise srmChangeFileStorageType request
 */
void
srmChangeFileStorageType::init()
{
  /* request (parser/API) */
  desiredStorageType = NULL;

  /* response (parser) */
  fileStatuses = NULL;
}

/*
 * srmChangeFileStorageType request copy constuctor
 */
srmChangeFileStorageType::srmChangeFileStorageType(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmChangeFileStorageType request destructor
 */
srmChangeFileStorageType::~srmChangeFileStorageType()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE_VEC(path.SURLOrStFN);
  DELETE_VEC(path.storageSystemInfo);
  DELETE(desiredStorageType);

  /* response (parser) */
  DELETE(fileStatuses);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmChangeFileStorageType::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(ChangeFileStorageType);
}

int
srmChangeFileStorageType::exec(Process *proc)
{
#define EVAL_VEC_STR_CHT(vec) vec = proc->eval_vec_str(srmChangeFileStorageType::vec)
  DM_DBG_I;
  BOOL match = FALSE;

  tSurlInfoArray path;

  EVAL_VEC_STR_CHT(path.SURLOrStFN);
  EVAL_VEC_STR_CHT(path.storageSystemInfo);

#ifdef SRM2_CALL
  NEW_SRM_RET(ChangeFileStorageType);

  ChangeFileStorageType(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    path,
    *getTFileStorageType(EVAL2CSTR(desiredStorageType)), /* getTFileStorageType never returns NULL */
    resp
  );
#endif

  DELETE_VEC(path.SURLOrStFN);
  DELETE_VEC(path.storageSystemInfo);

  /* matching */
  if(!resp || !resp->srmChangeFileStorageTypeResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfFileStatus */
  match = proc->e_match(fileStatuses, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmChangeFileStorageTypeResponse->returnStatus, proc));
#undef EVAL_VEC_STR_CHT
}

std::string
srmChangeFileStorageType::toString(Process *proc)
{
#define EVAL_VEC_STR_CHT(vec) EVAL_VEC_STR(srmChangeFileStorageType,vec)
  DM_DBG_I;

  GET_SRM_RESP(ChangeFileStorageType);
  BOOL quote = TRUE;
  std::stringstream ss;

  tArrayOfPutFileRequests_ path;

  EVAL_VEC_STR_CHT(path.SURLOrStFN);
  EVAL_VEC_STR_CHT(path.storageSystemInfo);

  /* request */  
  SS_SRM("srmChangeFileStorageType");
  SS_P_DQ(userID);
  SS_VEC_DEL(path.SURLOrStFN);
  SS_VEC_DEL(path.storageSystemInfo);
  SS_P_DQ(desiredStorageType);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmChangeFileStorageTypeResponse) RETURN(ss.str());

  ss << arrayOfFileStatusToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmChangeFileStorageTypeResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_CHT
}

std::string
srmChangeFileStorageType::arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(ChangeFileStorageType);
  std::stringstream ss;

  if(!resp || !resp->srmChangeFileStorageTypeResponse) RETURN(ss.str());

  if(resp->srmChangeFileStorageTypeResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;

    std::vector<srm__TSURLReturnStatus *> v = resp->srmChangeFileStorageTypeResponse->arrayOfFileStatuses->surlReturnStatusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_PAR_VAL(surl);
    }
  }
  
  RETURN(ss.str());
}
