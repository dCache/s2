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
 * srmCopy request constuctor
 */
srmCopy::srmCopy()
{
  init();
}

/*
 * Initialise srmCopy request
 */
void
srmCopy::init()
{
  /* request (parser/API) */
  userRequestDescription = NULL;
  overwriteOption = NULL;
  removeSourceFiles = NULL;
  storageSystemInfo = NULL;
  totalRetryTime = NULL;

  /* response (parser) */
  requestToken = NULL;
  fileStatuses = NULL;
}

/*
 * srmCopy request copy constuctor
 */
srmCopy::srmCopy(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmCopy request destructor
 */
srmCopy::~srmCopy()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE_VEC(arrayOfFileRequests.allLevelRecursive);
  DELETE_VEC(arrayOfFileRequests.isSourceADirectory);
  DELETE_VEC(arrayOfFileRequests.numOfLevels);
  DELETE_VEC(arrayOfFileRequests.fileStorageType);
  DELETE_VEC(arrayOfFileRequests.fromSURLOrStFN);
  DELETE_VEC(arrayOfFileRequests.fromStorageSystemInfo);
  DELETE_VEC(arrayOfFileRequests.lifetime);
  DELETE_VEC(arrayOfFileRequests.overwriteMode);
  DELETE_VEC(arrayOfFileRequests.spaceToken);
  DELETE_VEC(arrayOfFileRequests.toSURLOrStFN);
  DELETE_VEC(arrayOfFileRequests.toStorageSystemInfo);
  DELETE(userRequestDescription);
  DELETE(overwriteOption);
  DELETE(removeSourceFiles);
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
srmCopy::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(Copy);
}

int
srmCopy::exec(Process *proc)
{
#define EVAL_VEC_STR_CPY(vec) vec = proc->eval_vec_str(srmCopy::vec)
#define EVAL_VEC_INT32_CPY(vec) vec = proc->eval_vec_int32(srmCopy::vec)
#define EVAL_VEC_PINT32_CPY(vec) vec = proc->eval_vec_pint32(srmCopy::vec)
#define EVAL_VEC_PINT64_CPY(vec) vec = proc->eval_vec_pint64(srmCopy::vec)
#define EVAL_VEC_OVERWRITE_MODE_CPY(vec) vec = eval_vec_overwrite_mode(srmCopy::vec, proc)
  DM_DBG_I;
  BOOL match = FALSE;

  tArrayOfCopyFileRequests arrayOfFileRequests;

  EVAL_VEC_PINT32_CPY(arrayOfFileRequests.allLevelRecursive);
  EVAL_VEC_INT32_CPY(arrayOfFileRequests.isSourceADirectory);
  EVAL_VEC_INT32_CPY(arrayOfFileRequests.numOfLevels);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.fileStorageType);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.fromSURLOrStFN);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.fromStorageSystemInfo);
  EVAL_VEC_PINT64_CPY(arrayOfFileRequests.lifetime);
  EVAL_VEC_OVERWRITE_MODE_CPY(arrayOfFileRequests.overwriteMode);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.spaceToken);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.toSURLOrStFN);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.toStorageSystemInfo);

#ifdef SRM2_CALL
  NEW_SRM_RET(Copy);

  Copy(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    arrayOfFileRequests,
    EVAL2CSTR(userRequestDescription),
    getTOverwriteMode(EVAL2CSTR(overwriteOption)),
    (bool *)proc->eval2pint32(removeSourceFiles).p,
    EVAL2CSTR(storageSystemInfo),
    proc->eval2pint64(totalRetryTime).p,
    resp
  );
#endif

  FREE_VEC(arrayOfFileRequests.allLevelRecursive);
  DELETE_VEC(arrayOfFileRequests.fileStorageType);
  DELETE_VEC(arrayOfFileRequests.fromSURLOrStFN);
  DELETE_VEC(arrayOfFileRequests.fromStorageSystemInfo);
  FREE_VEC(arrayOfFileRequests.lifetime);
  DELETE_VEC(arrayOfFileRequests.spaceToken);
  DELETE_VEC(arrayOfFileRequests.toSURLOrStFN);
  DELETE_VEC(arrayOfFileRequests.toStorageSystemInfo);

  /* matching */
  if(!resp || !resp->srmCopyResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* requestToken */
  EAT_MATCH_3(resp->srmCopyResponse,
              requestToken,
              resp->srmCopyResponse->requestToken->value.c_str());

  /* arrayOfFileStatus */
  match = proc->e_match(fileStatuses, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmCopyResponse->returnStatus, proc));
#undef EVAL_VEC_STR_CPY
#undef EVAL_VEC_INT32_CPY
#undef EVAL_VEC_PINT32_CPY
#undef EVAL_VEC_PINT64_CPY
#undef EVAL_VEC_OVERWRITE_MODE_CPY
}

std::string
srmCopy::toString(Process *proc)
{
#define EVAL_VEC_STR_CPY(vec) EVAL_VEC_STR(srmCopy,vec)
  DM_DBG_I;
  
  GET_SRM_RESP(Copy);
  BOOL quote = TRUE;
  std::stringstream ss;

  tArrayOfCopyFileRequests_ arrayOfFileRequests;

  EVAL_VEC_STR_CPY(arrayOfFileRequests.allLevelRecursive);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.isSourceADirectory);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.numOfLevels);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.fileStorageType);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.fromSURLOrStFN);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.fromStorageSystemInfo);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.lifetime);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.overwriteMode);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.spaceToken);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.toSURLOrStFN);
  EVAL_VEC_STR_CPY(arrayOfFileRequests.toStorageSystemInfo);

  /* request */  
  SS_SRM("srmCopy");
  SS_P_DQ(userID);
  SS_VEC_DEL(arrayOfFileRequests.allLevelRecursive);
  SS_VEC_DEL(arrayOfFileRequests.isSourceADirectory);
  SS_VEC_DEL(arrayOfFileRequests.numOfLevels);
  SS_VEC_DEL(arrayOfFileRequests.fileStorageType);
  SS_VEC_DEL(arrayOfFileRequests.fromSURLOrStFN);
  SS_VEC_DEL(arrayOfFileRequests.fromStorageSystemInfo);
  SS_VEC_DEL(arrayOfFileRequests.lifetime);
  SS_VEC_DEL(arrayOfFileRequests.overwriteMode);
  SS_VEC_DEL(arrayOfFileRequests.spaceToken);
  SS_VEC_DEL(arrayOfFileRequests.toSURLOrStFN);
  SS_VEC_DEL(arrayOfFileRequests.toStorageSystemInfo);
  SS_P_DQ(userRequestDescription);
  SS_P_DQ(overwriteOption);
  SS_P_DQ(removeSourceFiles);
  SS_P_DQ(storageSystemInfo);
  SS_P_DQ(totalRetryTime);

  /* response (parser) */
  SS_P_DQ(requestToken);
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmCopyResponse) RETURN(ss.str());

  if(!resp->srmCopyResponse->requestToken) {
    /* no request tokens returned */
    DM_LOG(DM_N(1), "no request tokens returned\n");
  } else ss << " requestToken=" << dq_param(resp->srmCopyResponse->requestToken->value, quote);
    
  ss << arrayOfFileStatusToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmCopyResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_CPY
}

std::string
srmCopy::arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(Copy);
  std::stringstream ss;
  
  if(!resp || !resp->srmCopyResponse) RETURN(ss.str());

  if(resp->srmCopyResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TCopyRequestFileStatus *> v = resp->srmCopyResponse->arrayOfFileStatuses->copyStatusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR_VAL(estimatedProcessingTime);
      SS_P_VEC_PAR_VAL(estimatedWaitTimeOnQueue);
      SS_P_VEC_PAR_VAL(fileSize);
      SS_P_VEC_PAR_VAL(fromSURL);
      SS_P_VEC_PAR_VAL(remainingPinTime);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_PAR_VAL(toSURL);
    }
  }
  
  RETURN(ss.str());
}
