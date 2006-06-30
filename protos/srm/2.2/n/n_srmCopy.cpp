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
  desiredTotalRequestTime = NULL;
  desiredTargetSURLLifeTime = NULL;
  targetFileStorageType = NULL;
  targetSpaceToken = NULL;
  targetFileRetentionPolicyInfo = NULL;
  retentionPolicy = NULL;
  accessLatency = NULL;

  /* response (parser) */
  requestToken = NULL;
  fileStatuses = NULL;
  remainingTotalRequestTime = NULL;
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
  DELETE_VEC(sourceSURL);
  DELETE_VEC(targetSURL);
  DELETE_VEC(isSourceADirectory);
  DELETE_VEC(allLevelRecursive);
  DELETE_VEC(numOfLevels);
  DELETE(userRequestDescription);
  DELETE(overwriteOption);
  DELETE(desiredTotalRequestTime);
  DELETE(desiredTargetSURLLifeTime);
  DELETE(targetFileStorageType);
  DELETE(targetSpaceToken);
  DELETE(targetFileRetentionPolicyInfo);
  DELETE(retentionPolicy);
  DELETE(accessLatency);
  DELETE_VEC(sourceStorageSystemInfo.key);
  DELETE_VEC(sourceStorageSystemInfo.value);
  DELETE_VEC(targetStorageSystemInfo.key);
  DELETE_VEC(targetStorageSystemInfo.value);

  /* response (parser) */
  DELETE(requestToken);
  DELETE(fileStatuses);
  DELETE(remainingTotalRequestTime);
  
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
#define EVAL_VEC_STR_PTP(vec) vec = proc->eval_vec_str(srmCopy::vec)
#define EVAL_VEC_INT_PTP(vec) vec = proc->eval_vec_int(srmCopy::vec)
#define EVAL_VEC_PINT_PTP(vec) vec = proc->eval_vec_pint(srmCopy::vec)
  DM_DBG_I;

  std::vector <std::string *> sourceSURL;
  std::vector <std::string *> targetSURL;
  std::vector <int> isSourceADirectory;
  std::vector <int *> allLevelRecursive;
  std::vector <int *> numOfLevels;
  tStorageSystemInfo sourceStorageSystemInfo;
  tStorageSystemInfo targetStorageSystemInfo;

  EVAL_VEC_STR_PTP(sourceSURL);
  EVAL_VEC_STR_PTP(targetSURL);

  EVAL_VEC_INT_PTP(isSourceADirectory);
  EVAL_VEC_PINT_PTP(allLevelRecursive);
  EVAL_VEC_PINT_PTP(numOfLevels);

  EVAL_VEC_STR_PTP(sourceStorageSystemInfo.key);
  EVAL_VEC_STR_PTP(sourceStorageSystemInfo.value);

  EVAL_VEC_STR_PTP(targetStorageSystemInfo.key);
  EVAL_VEC_STR_PTP(targetStorageSystemInfo.value);


#ifdef SRM2_CALL
  NEW_SRM_RET(Copy);

  Copy(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    sourceSURL,
    targetSURL,
    isSourceADirectory,
    allLevelRecursive,
    numOfLevels,
    EVAL2CSTR(userRequestDescription),
    getTOverwriteMode(EVAL2CSTR(overwriteOption)),
    proc->eval2pint(desiredTotalRequestTime).p,
    proc->eval2pint(desiredTargetSURLLifeTime).p,
    getTFileStorageType(EVAL2CSTR(targetFileStorageType)),
    EVAL2CSTR(targetSpaceToken),
    getTRetentionPolicy(EVAL2CSTR(retentionPolicy),TRUE),	/* one-parameter getT* returns pointer to NULL in 2.2 */
    getTAccessLatency(EVAL2CSTR(accessLatency)),
    sourceStorageSystemInfo,
    targetStorageSystemInfo,
    resp
  );
#endif

  DELETE_VEC(sourceSURL);
  DELETE_VEC(targetSURL);

  FREE_VEC(allLevelRecursive);
  FREE_VEC(numOfLevels);

  DELETE_VEC(sourceStorageSystemInfo.key);
  DELETE_VEC(sourceStorageSystemInfo.value);

  DELETE_VEC(targetStorageSystemInfo.key);
  DELETE_VEC(targetStorageSystemInfo.value);

  /* matching */
  if(!resp || !resp->srmCopyResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* requestToken */
  EAT_MATCH_C(resp->srmCopyResponse->requestToken,
              requestToken,
              CSTR(resp->srmCopyResponse->requestToken));

  /* arrayOfFileStatus */
  EAT_MATCH(fileStatuses, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());

  /* remainingTotalRequestTime */
  EAT_MATCH_C(resp->srmCopyResponse->remainingTotalRequestTime,
              remainingTotalRequestTime,
              PI2CSTR(resp->srmCopyResponse->remainingTotalRequestTime));

  RETURN(matchReturnStatus(resp->srmCopyResponse->returnStatus, proc));
#undef EVAL_VEC_STR_PTP
#undef EVAL_VEC_INT_PTP
#undef EVAL_VEC_PINT_PTP
}

std::string
srmCopy::toString(Process *proc)
{
#define EVAL_VEC_STR_PTP(vec) EVAL_VEC_STR(srmCopy,vec)
  DM_DBG_I;

  GET_SRM_RESP(Copy);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> sourceSURL;
  std::vector <std::string *> targetSURL;
  std::vector <std::string *> isSourceADirectory;
  std::vector <std::string *> allLevelRecursive;
  std::vector <std::string *> numOfLevels;
  tStorageSystemInfo_ sourceStorageSystemInfo;
  tStorageSystemInfo_ targetStorageSystemInfo;

  EVAL_VEC_STR_PTP(sourceSURL);
  EVAL_VEC_STR_PTP(targetSURL);

  EVAL_VEC_STR_PTP(isSourceADirectory);
  EVAL_VEC_STR_PTP(allLevelRecursive);
  EVAL_VEC_STR_PTP(numOfLevels);

  EVAL_VEC_STR_PTP(sourceStorageSystemInfo.key);
  EVAL_VEC_STR_PTP(sourceStorageSystemInfo.value);

  EVAL_VEC_STR_PTP(targetStorageSystemInfo.key);
  EVAL_VEC_STR_PTP(targetStorageSystemInfo.value);

  /* request */  
  SS_SRM("srmCopy");
  SS_P_DQ(authorizationID);
  SS_VEC_DEL(sourceSURL);
  SS_VEC_DEL(targetSURL);
  SS_VEC_DEL(isSourceADirectory);
  SS_VEC_DEL(allLevelRecursive);
  SS_VEC_DEL(numOfLevels);
  SS_P_DQ(userRequestDescription);
  SS_P_DQ(overwriteOption);
  SS_P_DQ(desiredTotalRequestTime);
  SS_P_DQ(desiredTargetSURLLifeTime);
  SS_P_DQ(targetFileStorageType);
  SS_P_DQ(targetSpaceToken);
  SS_P_DQ(targetFileRetentionPolicyInfo);
  SS_P_DQ(retentionPolicy);
  SS_P_DQ(accessLatency);
  SS_VEC_DEL(sourceStorageSystemInfo.key);
  SS_VEC_DEL(sourceStorageSystemInfo.value);
  SS_VEC_DEL(targetStorageSystemInfo.key);
  SS_VEC_DEL(targetStorageSystemInfo.value);
  
  /* response (parser) */
  SS_P_DQ(requestToken);
  SS_P_DQ(fileStatuses);
  SS_P_DQ(remainingTotalRequestTime);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmCopyResponse) RETURN(ss.str());

  /* requestToken */
  SS_P_DQ_C(resp->srmCopyResponse->requestToken,
            requestToken,
            CSTR(resp->srmCopyResponse->requestToken));

  ss << arrayOfFileStatusToString(proc, TRUE, quote);
  
  /* remainingTotalRequestTime */
  SS_P_DQ_C(resp->srmCopyResponse->remainingTotalRequestTime,
            remainingTotalRequestTime,
            PI2CSTR(resp->srmCopyResponse->remainingTotalRequestTime));
  
  SS_P_SRM_RETSTAT(resp->srmCopyResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_PTP
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
    std::vector<srm__TCopyRequestFileStatus *> v = resp->srmCopyResponse->arrayOfFileStatuses->statusArray;

    /* exactly the same code as in srmStatusOfCopyRequest */
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(sourceSURL);
      SS_P_VEC_PAR(targetSURL);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_DPAR(fileSize);
      SS_P_VEC_DPAR(estimatedWaitTime);
      SS_P_VEC_DPAR(remainingFileLifetime);
    }
  }
  
  RETURN(ss.str());
}
