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
 * srmPrepareToGet request constuctor
 */
srmPrepareToGet::srmPrepareToGet()
{
  init();
}

/*
 * Initialise srmPrepareToGet request
 */
void
srmPrepareToGet::init()
{
  /* request (parser/API) */
  userRequestDescription = NULL;
  storageSystemInfo = NULL;
  totalRetryTime = NULL;

  /* response (parser) */
  requestToken = NULL;
  fileStatuses = NULL;
}

/*
 * srmPrepareToGet request copy constuctor
 */
srmPrepareToGet::srmPrepareToGet(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmPrepareToGet request destructor
 */
srmPrepareToGet::~srmPrepareToGet()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE_VEC(arrayOfFileRequests.allLevelRecursive);
  DELETE_VEC(arrayOfFileRequests.isSourceADirectory);
  DELETE_VEC(arrayOfFileRequests.numOfLevels);
  DELETE_VEC(arrayOfFileRequests.fileStorageType);
  DELETE_VEC(arrayOfFileRequests.SURLOrStFN);
  DELETE_VEC(arrayOfFileRequests.storageSystemInfo);
  DELETE_VEC(arrayOfFileRequests.lifetime);
  DELETE_VEC(arrayOfFileRequests.spaceToken);
  DELETE_VEC(arrayOfTransferProtocols);
  DELETE(userRequestDescription);
  DELETE(storageSystemInfo);
  DELETE(totalRetryTime);

  /* response (parser) */
  DELETE(requestToken);
  DELETE(fileStatuses);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmPrepareToGet::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(PrepareToGet);
}

int
srmPrepareToGet::exec(Process *proc)
{
#define EVAL_VEC_STR_PTG(vec) vec = proc->eval_vec_str(srmPrepareToGet::vec)
#define EVAL_VEC_INT32_PTG(vec) vec = proc->eval_vec_int32(srmPrepareToGet::vec)
#define EVAL_VEC_PINT32_PTG(vec) vec = proc->eval_vec_pint32(srmPrepareToGet::vec)
#define EVAL_VEC_PINT64_PTG(vec) vec = proc->eval_vec_pint64(srmPrepareToGet::vec)
  DM_DBG_I;
  BOOL match = FALSE;

  tArrayOfGetFileRequests arrayOfFileRequests;
  std::vector <std::string *> arrayOfTransferProtocols;

  EVAL_VEC_PINT32_PTG(arrayOfFileRequests.allLevelRecursive);
  EVAL_VEC_INT32_PTG(arrayOfFileRequests.isSourceADirectory);
  EVAL_VEC_INT32_PTG(arrayOfFileRequests.numOfLevels);
  EVAL_VEC_STR_PTG(arrayOfFileRequests.fileStorageType);
  EVAL_VEC_STR_PTG(arrayOfFileRequests.SURLOrStFN);
  EVAL_VEC_STR_PTG(arrayOfFileRequests.storageSystemInfo);
  EVAL_VEC_PINT64_PTG(arrayOfFileRequests.lifetime);
  EVAL_VEC_STR_PTG(arrayOfFileRequests.spaceToken);
  EVAL_VEC_STR_PTG(arrayOfTransferProtocols);

#ifdef SRM2_CALL
  NEW_SRM_RET(PrepareToGet);

  PrepareToGet(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    arrayOfFileRequests,
    arrayOfTransferProtocols,
    EVAL2CSTR(userRequestDescription),
    EVAL2CSTR(storageSystemInfo),
    proc->eval2pint64(totalRetryTime).p,
    resp
  );
#endif

  FREE_VEC(arrayOfFileRequests.allLevelRecursive);
  DELETE_VEC(arrayOfFileRequests.fileStorageType);
  DELETE_VEC(arrayOfFileRequests.SURLOrStFN);
  DELETE_VEC(arrayOfFileRequests.storageSystemInfo);
  FREE_VEC(arrayOfFileRequests.lifetime);
  DELETE_VEC(arrayOfFileRequests.spaceToken);
  DELETE_VEC(arrayOfTransferProtocols);

  /* matching */
  if(!resp || !resp->srmPrepareToGetResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* requestToken */
  EAT_MATCH_3(resp->srmPrepareToGetResponse,
              requestToken,
              resp->srmPrepareToGetResponse->requestToken->value.c_str());

  /* arrayOfFileStatus */
  EAT_MATCH(fileStatuses, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmPrepareToGetResponse->returnStatus, proc));
#undef EVAL_VEC_STR_PTG
#undef EVAL_VEC_INT32_PTG
#undef EVAL_VEC_PINT32_PTG
#undef EVAL_VEC_PINT64_PTG
}

std::string
srmPrepareToGet::toString(Process *proc)
{
#define EVAL_VEC_STR_PTG(vec) EVAL_VEC_STR(srmPrepareToGet,vec)
  DM_DBG_I;
  
  GET_SRM_RESP(PrepareToGet);
  BOOL quote = TRUE;
  std::stringstream ss;

  tArrayOfGetFileRequests_ arrayOfFileRequests;
  std::vector <std::string *> arrayOfTransferProtocols;

  EVAL_VEC_STR_PTG(arrayOfFileRequests.allLevelRecursive);
  EVAL_VEC_STR_PTG(arrayOfFileRequests.isSourceADirectory);
  EVAL_VEC_STR_PTG(arrayOfFileRequests.numOfLevels);
  EVAL_VEC_STR_PTG(arrayOfFileRequests.fileStorageType);
  EVAL_VEC_STR_PTG(arrayOfFileRequests.SURLOrStFN);
  EVAL_VEC_STR_PTG(arrayOfFileRequests.storageSystemInfo);
  EVAL_VEC_STR_PTG(arrayOfFileRequests.lifetime);
  EVAL_VEC_STR_PTG(arrayOfFileRequests.spaceToken);
  EVAL_VEC_STR_PTG(arrayOfTransferProtocols);

  /* request */  
  SS_SRM("srmPrepareToGet");
  SS_P_DQ(userID);
  SS_VEC_DEL(arrayOfFileRequests.allLevelRecursive);
  SS_VEC_DEL(arrayOfFileRequests.isSourceADirectory);
  SS_VEC_DEL(arrayOfFileRequests.numOfLevels);
  SS_VEC_DEL(arrayOfFileRequests.fileStorageType);
  SS_VEC_DEL(arrayOfFileRequests.SURLOrStFN);
  SS_VEC_DEL(arrayOfFileRequests.storageSystemInfo);
  SS_VEC_DEL(arrayOfFileRequests.lifetime);
  SS_VEC_DEL(arrayOfFileRequests.spaceToken);
  SS_VEC_DEL(arrayOfTransferProtocols);
  SS_P_DQ(userRequestDescription);
  SS_P_DQ(storageSystemInfo);
  SS_P_DQ(totalRetryTime);

  /* response (parser) */
  SS_P_DQ(requestToken);
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmPrepareToGetResponse) RETURN(ss.str());

  if(!resp->srmPrepareToGetResponse->requestToken) {
    /* no request tokens returned */
    DM_LOG(DM_N(1), "no request tokens returned\n");
  } else ss << " requestToken=" << dq_param(resp->srmPrepareToGetResponse->requestToken->value, quote);
    
  ss << arrayOfFileStatusToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmPrepareToGetResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_PTG
}

std::string
srmPrepareToGet::arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(PrepareToGet);
  std::stringstream ss;
  
  if(!resp || !resp->srmPrepareToGetResponse) RETURN(ss.str());

  if(resp->srmPrepareToGetResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TGetRequestFileStatus *> v = resp->srmPrepareToGetResponse->arrayOfFileStatuses->getStatusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR_VAL(estimatedProcessingTime);
      SS_P_VEC_PAR_VAL(estimatedWaitTimeOnQueue);
      SS_P_VEC_PAR_VAL(fileSize);
      SS_P_VEC_PAR_VAL(fromSURLInfo);
      SS_P_VEC_PAR_VAL(remainingPinTime);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_PAR_VAL(transferURL);
    }
  }
  
  RETURN(ss.str());
}
