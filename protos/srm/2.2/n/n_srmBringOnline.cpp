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
 * srmBringOnline request constuctor
 */
srmBringOnline::srmBringOnline()
{
  init();
}

/*
 * Initialise srmBringOnline request
 */
void
srmBringOnline::init()
{
  /* request (parser/API) */
  userRequestDescription = NULL;
  desiredFileStorageType = NULL;
  desiredTotalRequestTime = NULL;
  desiredLifeTime = NULL;
  targetSpaceToken = NULL;
  retentionPolicy = NULL;
  accessLatency = NULL;
  accessPattern = NULL;
  connectionType = NULL;

  /* response (parser) */
  requestToken = NULL;
  fileStatuses = NULL;
  remainingTotalRequestTime = NULL;
  remainingDeferredStartTime = NULL;
}

/*
 * srmBringOnline request copy constuctor
 */
srmBringOnline::srmBringOnline(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmBringOnline request destructor
 */
srmBringOnline::~srmBringOnline()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE_VEC(fileRequests.sourceSURL);
  DELETE_VEC(fileRequests.isSourceADirectory);
  DELETE_VEC(fileRequests.numOfLevels);
  DELETE_VEC(fileRequests.allLevelRecursive);
  DELETE(userRequestDescription);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);
  DELETE(desiredFileStorageType);
  DELETE(desiredTotalRequestTime);
  DELETE(desiredLifeTime);
  DELETE(targetSpaceToken);
  DELETE(retentionPolicy);
  DELETE(accessLatency);
  DELETE(accessPattern);
  DELETE(connectionType);
  DELETE_VEC(clientNetworks);
  DELETE_VEC(transferProtocols);

  /* response (parser) */
  DELETE(requestToken);
  DELETE(fileStatuses);
  DELETE(remainingTotalRequestTime);
  DELETE(remainingDeferredStartTime);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmBringOnline::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(BringOnline);
}

int
srmBringOnline::exec(Process *proc)
{
#define EVAL_VEC_STR_PTG(vec) vec = proc->eval_vec_str(srmBringOnline::vec)
#define EVAL_VEC_INT_PTG(vec) vec = proc->eval_vec_int(srmBringOnline::vec)
#define EVAL_VEC_PINT_PTG(vec) vec = proc->eval_vec_pint(srmBringOnline::vec)
  DM_DBG_I;
  BOOL match = FALSE;

  tArrayOfGetFileRequests fileRequests;
  tStorageSystemInfo storageSystemInfo;
  std::vector <std::string *> clientNetworks;
  std::vector <std::string *> transferProtocols;

  EVAL_VEC_STR_PTG(fileRequests.sourceSURL);
  EVAL_VEC_INT_PTG(fileRequests.isSourceADirectory);
  EVAL_VEC_PINT_PTG(fileRequests.numOfLevels);
  EVAL_VEC_PINT_PTG(fileRequests.allLevelRecursive);

  EVAL_VEC_STR_PTG(storageSystemInfo.key);
  EVAL_VEC_STR_PTG(storageSystemInfo.value);

  EVAL_VEC_STR_PTG(clientNetworks);
  EVAL_VEC_STR_PTG(transferProtocols);

#ifdef SRM2_CALL
  NEW_SRM_RET(BringOnline);

  BringOnline(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    fileRequests,
    EVAL2CSTR(userRequestDescription),
    storageSystemInfo,
    getTFileStorageType(EVAL2CSTR(desiredFileStorageType)),
    proc->eval2pint(desiredTotalRequestTime).p,
    proc->eval2pint(desiredLifeTime).p,
    EVAL2CSTR(targetSpaceToken),
    *getTRetentionPolicy(EVAL2CSTR(retentionPolicy)),	/* getT* never returns pointer to NULL */
    getTAccessLatency(EVAL2CSTR(accessLatency)),
    getTAccessPattern(EVAL2CSTR(accessPattern)),
    getTConnectionType(EVAL2CSTR(connectionType)),
    clientNetworks,
    transferProtocols,
    proc->eval2pint(deferredStartTime).p,
    resp
  );
#endif

  DELETE_VEC(fileRequests.sourceSURL);
  FREE_VEC(fileRequests.numOfLevels);
  FREE_VEC(fileRequests.allLevelRecursive);
  
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  DELETE_VEC(clientNetworks);
  DELETE_VEC(transferProtocols);

  /* matching */
  if(!resp || !resp->srmBringOnlineResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* requestToken */
  EAT_MATCH_C(resp->srmBringOnlineResponse->requestToken,
              requestToken,
              CSTR(resp->srmBringOnlineResponse->requestToken));

  /* arrayOfFileStatus */
  EAT_MATCH(fileStatuses, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());

  /* remainingTotalRequestTime */
  EAT_MATCH_C(resp->srmBringOnlineResponse->remainingTotalRequestTime,
              remainingTotalRequestTime,
              PI2CSTR(resp->srmBringOnlineResponse->remainingTotalRequestTime));
  
  /* remainingTotalRequestTime */
  EAT_MATCH_C(resp->srmBringOnlineResponse->remainingDeferredStartTime,
              remainingDeferredStartTime,
              PI2CSTR(resp->srmBringOnlineResponse->remainingDeferredStartTime));
  
  RETURN(matchReturnStatus(resp->srmBringOnlineResponse->returnStatus, proc));
#undef EVAL_VEC_STR_PTG
#undef EVAL_VEC_INT_PTG
#undef EVAL_VEC_PINT_PTG
}

std::string
srmBringOnline::toString(Process *proc)
{
#define EVAL_VEC_STR_PTG(vec) EVAL_VEC_STR(srmBringOnline,vec)
  DM_DBG_I;

  GET_SRM_RESP(BringOnline);
  BOOL quote = TRUE;
  std::stringstream ss;

  tArrayOfGetFileRequests_ fileRequests;
  tStorageSystemInfo_ storageSystemInfo;
  std::vector <std::string *> clientNetworks;
  std::vector <std::string *> transferProtocols;

  EVAL_VEC_STR_PTG(fileRequests.sourceSURL);
  EVAL_VEC_STR_PTG(fileRequests.isSourceADirectory);
  EVAL_VEC_STR_PTG(fileRequests.numOfLevels);
  EVAL_VEC_STR_PTG(fileRequests.allLevelRecursive);

  EVAL_VEC_STR_PTG(storageSystemInfo.key);
  EVAL_VEC_STR_PTG(storageSystemInfo.value);

  EVAL_VEC_STR_PTG(clientNetworks);
  EVAL_VEC_STR_PTG(transferProtocols);

  /* request */  
  SS_SRM("srmBringOnline");
  SS_P_DQ(authorizationID);
  SS_VEC_DEL(fileRequests.sourceSURL);
  SS_VEC_DEL(fileRequests.isSourceADirectory);
  SS_VEC_DEL(fileRequests.numOfLevels);
  SS_VEC_DEL(fileRequests.allLevelRecursive);
  SS_P_DQ(userRequestDescription);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);
  SS_P_DQ(desiredFileStorageType);
  SS_P_DQ(desiredTotalRequestTime);
  SS_P_DQ(desiredLifeTime);
  SS_P_DQ(targetSpaceToken);
  SS_P_DQ(retentionPolicy);
  SS_P_DQ(accessLatency);
  SS_P_DQ(accessPattern);
  SS_P_DQ(connectionType);
  SS_VEC_DEL(clientNetworks);
  SS_VEC_DEL(transferProtocols);

  /* response (parser) */
  SS_P_DQ(requestToken);
  SS_P_DQ(fileStatuses);
  SS_P_DQ(remainingTotalRequestTime);
  SS_P_DQ(remainingDeferredStartTime);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmBringOnlineResponse) RETURN(ss.str());

  /* requestToken */
  SS_P_DQ_C(resp->srmBringOnlineResponse->requestToken,
            requestToken,
            CSTR(resp->srmBringOnlineResponse->requestToken));

  ss << arrayOfFileStatusToString(proc, TRUE, quote);
  
  /* remainingTotalRequestTime */
  SS_P_DQ_C(resp->srmBringOnlineResponse->remainingTotalRequestTime,
            remainingTotalRequestTime,
            PI2CSTR(resp->srmBringOnlineResponse->remainingTotalRequestTime));
  
  /* remainingTotalRequestTime */
  SS_P_DQ_C(resp->srmBringOnlineResponse->remainingDeferredStartTime,
            remainingDeferredStartTime,
            PI2CSTR(resp->srmBringOnlineResponse->remainingDeferredStartTime));
  
  SS_P_SRM_RETSTAT(resp->srmBringOnlineResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_PTG
}

std::string
srmBringOnline::arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(BringOnline);
  std::stringstream ss;

  if(!resp || !resp->srmBringOnlineResponse) RETURN(ss.str());

  if(resp->srmBringOnlineResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TBringOnlineRequestFileStatus *> v = resp->srmBringOnlineResponse->arrayOfFileStatuses->statusArray;

    /* exactly the same code as in srmStatusOfBringOnlineRequest */
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(sourceSURL);
      SS_P_VEC_SRM_RETSTAT(status);	/* status <--> fileSize for consistency with PrepareToGet */
      SS_P_VEC_DPAR(fileSize);
      SS_P_VEC_DPAR(estimatedWaitTime);
      SS_P_VEC_DPAR(remainingPinTime);
    }
  }

  RETURN(ss.str());
}
