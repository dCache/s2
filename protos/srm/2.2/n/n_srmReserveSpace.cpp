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
#include "str.h"		/* i2str() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * srmReserveSpace request constuctor
 */
srmReserveSpace::srmReserveSpace()
{
  init();
}

/*
 * Initialise srmReserveSpace request
 */
void
srmReserveSpace::init()
{
  /* request (parser/API) */
  userSpaceTokenDescription = NULL;
  retentionPolicy = NULL;
  accessLatency = NULL;
  desiredSizeOfTotalSpace = NULL;
  desiredSizeOfGuaranteedSpace = NULL;
  desiredLifetimeOfReservedSpace = NULL;
  accessPattern = NULL;
  connectionType = NULL;

  /* response (parser) */
  requestToken = NULL;
  estimatedProcessingTime = NULL;
  respRetentionPolicy = NULL;
  respAccessLatency = NULL;
  sizeOfTotalReservedSpace = NULL;
  sizeOfGuaranteedReservedSpace = NULL;
  lifetimeOfReservedSpace = NULL;
  spaceToken = NULL;
}

/*
 * srmReserveSpace request copy constuctor
 */
srmReserveSpace::srmReserveSpace(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmReserveSpace request destructor
 */
srmReserveSpace::~srmReserveSpace()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(userSpaceTokenDescription);
  DELETE(retentionPolicy);
  DELETE(accessLatency);
  DELETE(desiredSizeOfTotalSpace);
  DELETE(desiredSizeOfGuaranteedSpace);
  DELETE(desiredLifetimeOfReservedSpace);
  DELETE_VEC(expectedFileSizes);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);
  DELETE(accessPattern);
  DELETE(connectionType);
  DELETE_VEC(clientNetworks);
  DELETE_VEC(transferProtocols);

  /* response (parser) */
  DELETE(requestToken);
  DELETE(estimatedProcessingTime);
  DELETE(respRetentionPolicy);
  DELETE(respAccessLatency);
  DELETE(sizeOfTotalReservedSpace);
  DELETE(sizeOfGuaranteedReservedSpace);
  DELETE(lifetimeOfReservedSpace);
  DELETE(spaceToken);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmReserveSpace::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(ReserveSpace);
}

int
srmReserveSpace::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;
#define EVAL_VEC_UINT64_RS(vec) vec = proc->eval_vec_uint64(srmReserveSpace::vec)
#define EVAL_VEC_STR_RS(vec) vec = proc->eval_vec_str(srmReserveSpace::vec)
  
  std::vector <uint64_t> expectedFileSizes;
  tStorageSystemInfo storageSystemInfo;

  EVAL_VEC_UINT64_RS(expectedFileSizes);
  EVAL_VEC_STR_RS(storageSystemInfo.key);
  EVAL_VEC_STR_RS(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(ReserveSpace);

  ReserveSpace(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(userSpaceTokenDescription),
    getTRetentionPolicy(EVAL2CSTR(retentionPolicy),TRUE),	/* one-parameter getT* returns pointer to NULL in 2.2 */
    getTAccessLatency(EVAL2CSTR(accessLatency)),
    proc->eval2puint64(desiredSizeOfTotalSpace).p,
    proc->eval2uint64(desiredSizeOfGuaranteedSpace),
    proc->eval2pint(desiredLifetimeOfReservedSpace).p,
    expectedFileSizes,
    storageSystemInfo,
    getTAccessPattern(EVAL2CSTR(accessPattern)),
    getTConnectionType(EVAL2CSTR(connectionType)),
    resp
  );
#endif

  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);
  
  /* matching */
  if(!resp || !resp->srmReserveSpaceResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* requestToken */
  EAT_MATCH_C(resp->srmReserveSpaceResponse->requestToken,
              requestToken,
              CSTR(resp->srmReserveSpaceResponse->requestToken));

  /* estimatedProcessingTime */
  EAT_MATCH_C(resp->srmReserveSpaceResponse->estimatedProcessingTime,
              estimatedProcessingTime,
              PI2CSTR(resp->srmReserveSpaceResponse->estimatedProcessingTime));

  /* retentionPolicyInfo */
  EAT_MATCH_C(resp->srmReserveSpaceResponse->retentionPolicyInfo,
              respRetentionPolicy,
              getTRetentionPolicy(resp->srmReserveSpaceResponse->retentionPolicyInfo->retentionPolicy).c_str());

  /* accessLatency */
  EAT_MATCH_C(resp->srmReserveSpaceResponse->retentionPolicyInfo && resp->srmReserveSpaceResponse->retentionPolicyInfo->accessLatency,
              respAccessLatency,
              getTAccessLatency(*resp->srmReserveSpaceResponse->retentionPolicyInfo->accessLatency).c_str());

  /* sizeOfTotalReservedSpace */
  EAT_MATCH_C(resp->srmReserveSpaceResponse->sizeOfTotalReservedSpace,
              sizeOfTotalReservedSpace,
              PI2CSTR(resp->srmReserveSpaceResponse->sizeOfTotalReservedSpace));
  
  /* sizeOfGuaranteedReservedSpace */
  EAT_MATCH_C(resp->srmReserveSpaceResponse->sizeOfGuaranteedReservedSpace,
              sizeOfGuaranteedReservedSpace,
              PI2CSTR(resp->srmReserveSpaceResponse->sizeOfGuaranteedReservedSpace));

  /* lifetimeOfReservedSpace */
  EAT_MATCH_C(resp->srmReserveSpaceResponse->lifetimeOfReservedSpace,
              lifetimeOfReservedSpace,
              PI2CSTR(resp->srmReserveSpaceResponse->lifetimeOfReservedSpace));
  
  /* spaceToken */
  EAT_MATCH_C(resp->srmReserveSpaceResponse->spaceToken,
              spaceToken,
              CSTR(resp->srmReserveSpaceResponse->spaceToken));

  RETURN(matchReturnStatus(resp->srmReserveSpaceResponse->returnStatus, proc));

#undef EVAL_VEC_UINT64_RS
#undef EVAL_VEC_STR_RS
}

std::string
srmReserveSpace::toString(Process *proc)
{
  DM_DBG_I;
#define EVAL_VEC_STR_RS(vec) EVAL_VEC_STR(srmReserveSpace,vec)

  GET_SRM_RESP(ReserveSpace);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> expectedFileSizes;
  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_RS(expectedFileSizes);
  EVAL_VEC_STR_RS(storageSystemInfo.key);
  EVAL_VEC_STR_RS(storageSystemInfo.value);
  
  /* request */  
  SS_SRM("srmReserveSpace");
  SS_P_DQ(authorizationID);
  SS_P_DQ(retentionPolicy);
  SS_P_DQ(accessLatency);
  SS_P_DQ(desiredSizeOfTotalSpace);
  SS_P_DQ(desiredSizeOfGuaranteedSpace);
  SS_P_DQ(desiredLifetimeOfReservedSpace);
  SS_VEC_DEL(expectedFileSizes);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);
  SS_P_DQ(accessPattern);
  SS_P_DQ(connectionType);

  /* response (parser) */
  SS_P_DQ(requestToken);
  SS_P_DQ(estimatedProcessingTime);
  SS_P_DQ(respRetentionPolicy);
  SS_P_DQ(respAccessLatency);
  SS_P_DQ(sizeOfTotalReservedSpace);
  SS_P_DQ(sizeOfGuaranteedReservedSpace);
  SS_P_DQ(lifetimeOfReservedSpace);
  SS_P_DQ(spaceToken);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmReserveSpaceResponse) RETURN(ss.str());

  /* requestToken */
  SS_P_DQ_C(resp->srmReserveSpaceResponse->requestToken,
            requestToken,
            resp->srmReserveSpaceResponse->requestToken);

  /* estimatedProcessingTime */
  SS_P_DQ_C(resp->srmReserveSpaceResponse->estimatedProcessingTime,
            estimatedProcessingTime,
            PI2CSTR(resp->srmReserveSpaceResponse->estimatedProcessingTime));

  /* retentionPolicyInfo */
  SS_P_DQ_C(resp->srmReserveSpaceResponse->retentionPolicyInfo,
            respRetentionPolicy,
            getTRetentionPolicy(resp->srmReserveSpaceResponse->retentionPolicyInfo->retentionPolicy).c_str());

  /* accessLatency */
  SS_P_DQ_C(resp->srmReserveSpaceResponse->retentionPolicyInfo && resp->srmReserveSpaceResponse->retentionPolicyInfo->accessLatency,
            respAccessLatency,
            getTAccessLatency(*resp->srmReserveSpaceResponse->retentionPolicyInfo->accessLatency).c_str());

  /* sizeOfTotalReservedSpace */
  SS_P_DQ_C(resp->srmReserveSpaceResponse->sizeOfTotalReservedSpace,
            sizeOfTotalReservedSpace,
            PI2CSTR(resp->srmReserveSpaceResponse->sizeOfTotalReservedSpace));
  
  /* sizeOfGuaranteedReservedSpace */
  SS_P_DQ_C(resp->srmReserveSpaceResponse->sizeOfGuaranteedReservedSpace,
            sizeOfGuaranteedReservedSpace,
            PI2CSTR(resp->srmReserveSpaceResponse->sizeOfGuaranteedReservedSpace));

  /* lifetimeOfReservedSpace */
  SS_P_DQ_C(resp->srmReserveSpaceResponse->lifetimeOfReservedSpace,
            lifetimeOfReservedSpace,
            PI2CSTR(resp->srmReserveSpaceResponse->lifetimeOfReservedSpace));
  
  /* spaceToken */
  SS_P_DQ_C(resp->srmReserveSpaceResponse->spaceToken,
            spaceToken,
            CSTR(resp->srmReserveSpaceResponse->spaceToken));

  SS_P_SRM_RETSTAT(resp->srmReserveSpaceResponse);

  RETURN(ss.str());

#undef EVAL_VEC_STR_RS
}
