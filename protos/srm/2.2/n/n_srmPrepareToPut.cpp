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
  desiredTotalRequestTime = NULL;;
  desiredPinLifeTime = NULL;
  desiredFileLifeTime = NULL;
  desiredFileStorageType = NULL;
  targetSpaceToken = NULL;
  retentionPolicy = NULL;
  accessLatency = NULL;
  accessPattern = NULL;
  connectionType = NULL;

  /* response (parser) */
  requestToken = NULL;
  fileStatuses = NULL;
  remainingTotalRequestTime = NULL;
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
  DELETE_VEC(fileRequests.SURL);
  DELETE_VEC(fileRequests.expectedFileSize);
  DELETE(userRequestDescription);
  DELETE(overwriteOption);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);
  DELETE(desiredTotalRequestTime);
  DELETE(desiredPinLifeTime);
  DELETE(desiredFileLifeTime);
  DELETE(desiredFileStorageType);
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
#define EVAL_VEC_PUINT64_PTP(vec) vec = proc->eval_vec_puint64(srmPrepareToPut::vec)
  DM_DBG_I;

  tArrayOfPutFileRequests fileRequests;
  tStorageSystemInfo storageSystemInfo;
  std::vector <std::string *> clientNetworks;
  std::vector <std::string *> transferProtocols;

  EVAL_VEC_STR_PTP(fileRequests.SURL);
  EVAL_VEC_PUINT64_PTP(fileRequests.expectedFileSize);

  EVAL_VEC_STR_PTP(storageSystemInfo.key);
  EVAL_VEC_STR_PTP(storageSystemInfo.value);

  EVAL_VEC_STR_PTP(clientNetworks);
  EVAL_VEC_STR_PTP(transferProtocols);

#ifdef SRM2_CALL
  NEW_SRM_RET(PrepareToPut);

  PrepareToPut(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    fileRequests,
    EVAL2CSTR(userRequestDescription),
    getTOverwriteMode(EVAL2CSTR(overwriteOption)),
    storageSystemInfo,
    proc->eval2pint(desiredTotalRequestTime).p,
    proc->eval2pint(desiredPinLifeTime).p,
    proc->eval2pint(desiredFileLifeTime).p,
    getTFileStorageType(EVAL2CSTR(desiredFileStorageType)),
    EVAL2CSTR(targetSpaceToken),
    getTRetentionPolicy(EVAL2CSTR(retentionPolicy)),
    getTAccessLatency(EVAL2CSTR(accessLatency)),
    getTAccessPattern(EVAL2CSTR(accessPattern)),
    getTConnectionType(EVAL2CSTR(connectionType)),
    clientNetworks,
    transferProtocols,
    resp
  );
#endif

  DELETE_VEC(fileRequests.SURL);
  FREE_VEC(fileRequests.expectedFileSize);

  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  DELETE_VEC(clientNetworks);
  DELETE_VEC(transferProtocols);

  /* matching */
  if(!resp || !resp->srmPrepareToPutResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* requestToken */
  EAT_MATCH_C(resp->srmPrepareToPutResponse->requestToken,
              requestToken,
              CSTR(resp->srmPrepareToPutResponse->requestToken));

  /* arrayOfFileStatus */
  EAT_MATCH(fileStatuses, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());

  /* remainingTotalRequestTime */
  EAT_MATCH_C(resp->srmPrepareToPutResponse->remainingTotalRequestTime,
              remainingTotalRequestTime,
              PI2CSTR(resp->srmPrepareToPutResponse->remainingTotalRequestTime));

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

  tArrayOfPutFileRequests_ fileRequests;
  tStorageSystemInfo_ storageSystemInfo;
  std::vector <std::string *> clientNetworks;
  std::vector <std::string *> transferProtocols;

  EVAL_VEC_STR_PTP(fileRequests.SURL);
  EVAL_VEC_STR_PTP(fileRequests.expectedFileSize);

  EVAL_VEC_STR_PTP(storageSystemInfo.key);
  EVAL_VEC_STR_PTP(storageSystemInfo.value);

  EVAL_VEC_STR_PTP(clientNetworks);
  EVAL_VEC_STR_PTP(transferProtocols);

  /* request */  
  SS_SRM("srmPrepareToPut");
  SS_P_DQ(authorizationID);
  SS_VEC_DEL(fileRequests.SURL);
  SS_VEC_DEL(fileRequests.expectedFileSize);
  SS_P_DQ(userRequestDescription);
  SS_P_DQ(overwriteOption);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);
  SS_P_DQ(desiredTotalRequestTime);
  SS_P_DQ(desiredPinLifeTime);
  SS_P_DQ(desiredFileLifeTime);
  SS_P_DQ(desiredFileStorageType);
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
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmPrepareToPutResponse) RETURN(ss.str());

  /* requestToken */
  SS_P_DQ_C(resp->srmPrepareToPutResponse->requestToken,
            requestToken,
            CSTR(resp->srmPrepareToPutResponse->requestToken));

  ss << arrayOfFileStatusToString(proc, TRUE, quote);
  
  /* remainingTotalRequestTime */
  SS_P_DQ_C(resp->srmPrepareToPutResponse->remainingTotalRequestTime,
            remainingTotalRequestTime,
            PI2CSTR(resp->srmPrepareToPutResponse->remainingTotalRequestTime));

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
    std::vector<srm__TPutRequestFileStatus *> v = resp->srmPrepareToPutResponse->arrayOfFileStatuses->statusArray;

    /* exactly the same code as in srmStatusOfPutRequest */
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(SURL);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_DPAR(fileSize);
      SS_P_VEC_DPAR(estimatedWaitTime);
      SS_P_VEC_DPAR(remainingPinLifetime);
      SS_P_VEC_DPAR(remainingFileLifetime);
      SS_P_VEC_DPAR(transferURL);

      if(v[u] && v[u]->transferProtocolInfo) {
        std::vector<srm__TExtraInfo *> extraInfoArray = v[u]->transferProtocolInfo->extraInfoArray;
        SS_P_VEC_SRM_EXTRA_INFOu(extraInfoArray);
      }
    }
  }
  
  RETURN(ss.str());
}
