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
 * srmPrepareToPut request constuctor
 */
srmPrepareToPut::srmPrepareToPut()
{
  init();
}

/*
 * Initialise srmPrepareToPut request
 */
void
srmPrepareToPut::init()
{
  /* request (parser/API) */
  userRequestDescription = NULL;
  overwriteOption = NULL;
  storageSystemInfo = NULL;
  totalRetryTime = NULL;

  /* response (parser) */
  requestToken = NULL;
  fileStatuses = NULL;
}

/*
 * srmPrepareToPut request copy constuctor
 */
srmPrepareToPut::srmPrepareToPut(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmPrepareToPut request destructor
 */
srmPrepareToPut::~srmPrepareToPut()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE_VEC(arrayOfFileRequests.fileStorageType);
  DELETE_VEC(arrayOfFileRequests.knownSizeOfThisFile);
  DELETE_VEC(arrayOfFileRequests.lifetime);
  DELETE_VEC(arrayOfFileRequests.spaceToken);
  DELETE_VEC(arrayOfFileRequests.SURLOrStFN);
  DELETE_VEC(arrayOfFileRequests.storageSystemInfo);
  DELETE_VEC(arrayOfTransferProtocols);
  DELETE(userRequestDescription);
  DELETE(overwriteOption);
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
srmPrepareToPut::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(PrepareToPut);
}

int
srmPrepareToPut::exec(Process *proc)
{
#define EVAL_VEC_STR_PTP(vec) vec = proc->eval_vec_str(srmPrepareToPut::vec)
#define EVAL_VEC_PINT64_PTP(vec) vec = proc->eval_vec_pint64(srmPrepareToPut::vec)
  DM_DBG_I;
  BOOL match = FALSE;

  tArrayOfPutFileRequests arrayOfFileRequests;
  std::vector <std::string *> arrayOfTransferProtocols;

  EVAL_VEC_STR_PTP(arrayOfFileRequests.fileStorageType);
  EVAL_VEC_PINT64_PTP(arrayOfFileRequests.knownSizeOfThisFile);
  EVAL_VEC_PINT64_PTP(arrayOfFileRequests.lifetime);
  EVAL_VEC_STR_PTP(arrayOfFileRequests.spaceToken);
  EVAL_VEC_STR_PTP(arrayOfFileRequests.SURLOrStFN);
  EVAL_VEC_STR_PTP(arrayOfFileRequests.storageSystemInfo);
  EVAL_VEC_STR_PTP(arrayOfTransferProtocols);

#ifdef SRM2_CALL
  NEW_SRM_RET(PrepareToPut);

  PrepareToPut(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    arrayOfFileRequests,
    arrayOfTransferProtocols,
    EVAL2CSTR(userRequestDescription),
    getTOverwriteMode(EVAL2CSTR(overwriteOption)),
    EVAL2CSTR(storageSystemInfo),
    proc->eval2pint64(totalRetryTime).p,
    resp
  );
#endif

  DELETE_VEC(arrayOfFileRequests.fileStorageType);
  FREE_VEC(arrayOfFileRequests.knownSizeOfThisFile);
  FREE_VEC(arrayOfFileRequests.lifetime);
  DELETE_VEC(arrayOfFileRequests.spaceToken);
  DELETE_VEC(arrayOfFileRequests.SURLOrStFN);
  DELETE_VEC(arrayOfFileRequests.storageSystemInfo);
  DELETE_VEC(arrayOfTransferProtocols);

  /* matching */
  if(!resp || !resp->srmPrepareToPutResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* requestToken */
  EAT_MATCH_C(resp->srmPrepareToPutResponse->requestToken,
              requestToken,
              resp->srmPrepareToPutResponse->requestToken->value.c_str());

  /* arrayOfFileStatus */
  EAT_MATCH(fileStatuses, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmPrepareToPutResponse->returnStatus, proc));
#undef EVAL_VEC_STR_PTP
#undef EVAL_VEC_PINT64_PTP
}

std::string
srmPrepareToPut::toString(Process *proc)
{
#define EVAL_VEC_STR_PTP(vec) EVAL_VEC_STR(srmPrepareToPut,vec)
  DM_DBG_I;

  GET_SRM_RESP(PrepareToPut);
  BOOL quote = TRUE;
  std::stringstream ss;

  tArrayOfPutFileRequests_ arrayOfFileRequests;
  std::vector <std::string *> arrayOfTransferProtocols;

  EVAL_VEC_STR_PTP(arrayOfFileRequests.fileStorageType);
  EVAL_VEC_STR_PTP(arrayOfFileRequests.knownSizeOfThisFile);
  EVAL_VEC_STR_PTP(arrayOfFileRequests.lifetime);
  EVAL_VEC_STR_PTP(arrayOfFileRequests.spaceToken);
  EVAL_VEC_STR_PTP(arrayOfFileRequests.SURLOrStFN);
  EVAL_VEC_STR_PTP(arrayOfFileRequests.storageSystemInfo);
  EVAL_VEC_STR_PTP(arrayOfTransferProtocols);

  /* request */  
  SS_SRM("srmPrepareToPut");
  SS_P_DQ(userID);
  SS_VEC_DEL(arrayOfFileRequests.fileStorageType);
  SS_VEC_DEL(arrayOfFileRequests.knownSizeOfThisFile);
  SS_VEC_DEL(arrayOfFileRequests.lifetime);
  SS_VEC_DEL(arrayOfFileRequests.spaceToken);
  SS_VEC_DEL(arrayOfFileRequests.SURLOrStFN);
  SS_VEC_DEL(arrayOfFileRequests.storageSystemInfo);
  SS_VEC_DEL(arrayOfTransferProtocols);
  SS_P_DQ(userRequestDescription);
  SS_P_DQ(overwriteOption);
  SS_P_DQ(storageSystemInfo);
  SS_P_DQ(totalRetryTime);

  /* response (parser) */
  SS_P_DQ(requestToken);
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmPrepareToPutResponse) RETURN(ss.str());

  if(!resp->srmPrepareToPutResponse->requestToken) {
    /* no request tokens returned */
    DM_LOG(DM_N(1), "no request tokens returned\n");
  } else ss << " requestToken=" << dq_param(resp->srmPrepareToPutResponse->requestToken->value, quote);
    
  ss << arrayOfFileStatusToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmPrepareToPutResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_PTP
}

std::string
srmPrepareToPut::arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(PrepareToPut);
  std::stringstream ss;
  
  if(!resp || !resp->srmPrepareToPutResponse) RETURN(ss.str());

  if(resp->srmPrepareToPutResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TPutRequestFileStatus *> v = resp->srmPrepareToPutResponse->arrayOfFileStatuses->putStatusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR_VAL(estimatedProcessingTime);
      SS_P_VEC_PAR_VAL(estimatedWaitTimeOnQueue);
      SS_P_VEC_PAR_VAL(fileSize);
      SS_P_VEC_PAR_VAL(remainingPinTime);
      SS_P_VEC_PAR_VAL(siteURL);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_PAR_VAL(transferURL);
    }
  }
  
  RETURN(ss.str());
}
