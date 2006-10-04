#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

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
 * srmStatusOfReserveSpaceRequest request constuctor
 */
srmStatusOfReserveSpaceRequest::srmStatusOfReserveSpaceRequest()
{
  init();
}

/*
 * Initialise srmStatusOfReserveSpaceRequest request
 */
void
srmStatusOfReserveSpaceRequest::init()
{
  /* request (parser/API) */
  requestToken = NULL;

  /* response (parser) */
  estimatedProcessingTime = NULL;
  respRetentionPolicy = NULL;
  respAccessLatency = NULL;
  sizeOfTotalReservedSpace = NULL;
  sizeOfGuaranteedReservedSpace = NULL;
  lifetimeOfReservedSpace = NULL;
  spaceToken = NULL;
}

/*
 * srmStatusOfReserveSpaceRequest request copy constuctor
 */
srmStatusOfReserveSpaceRequest::srmStatusOfReserveSpaceRequest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmStatusOfReserveSpaceRequest request destructor
 */
srmStatusOfReserveSpaceRequest::~srmStatusOfReserveSpaceRequest()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);

  /* response (parser) */
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
srmStatusOfReserveSpaceRequest::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(StatusOfReserveSpaceRequest);
}

int
srmStatusOfReserveSpaceRequest::exec(Process *proc)
{
  DM_DBG_I;
  
#ifdef SRM2_CALL
  NEW_SRM_RET(StatusOfReserveSpaceRequest);

  StatusOfReserveSpaceRequest(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(requestToken),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmStatusOfReserveSpaceRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* estimatedProcessingTime */
  EAT_MATCH_C(resp->srmStatusOfReserveSpaceRequestResponse->estimatedProcessingTime,
              estimatedProcessingTime,
              PI2CSTR(resp->srmStatusOfReserveSpaceRequestResponse->estimatedProcessingTime));

  /* retentionPolicyInfo */
  EAT_MATCH_C(resp->srmStatusOfReserveSpaceRequestResponse->retentionPolicyInfo,
              respRetentionPolicy,
              getTRetentionPolicy(resp->srmStatusOfReserveSpaceRequestResponse->retentionPolicyInfo->retentionPolicy).c_str());

  /* accessLatency */
  EAT_MATCH_C(resp->srmStatusOfReserveSpaceRequestResponse->retentionPolicyInfo && resp->srmStatusOfReserveSpaceRequestResponse->retentionPolicyInfo->accessLatency,
              respAccessLatency,
              getTAccessLatency(*resp->srmStatusOfReserveSpaceRequestResponse->retentionPolicyInfo->accessLatency).c_str());

  /* sizeOfTotalReservedSpace */
  EAT_MATCH_C(resp->srmStatusOfReserveSpaceRequestResponse->sizeOfTotalReservedSpace,
              sizeOfTotalReservedSpace,
              PI2CSTR(resp->srmStatusOfReserveSpaceRequestResponse->sizeOfTotalReservedSpace));
  
  /* sizeOfGuaranteedReservedSpace */
  EAT_MATCH_C(resp->srmStatusOfReserveSpaceRequestResponse->sizeOfGuaranteedReservedSpace,
              sizeOfGuaranteedReservedSpace,
              PI2CSTR(resp->srmStatusOfReserveSpaceRequestResponse->sizeOfGuaranteedReservedSpace));

  /* lifetimeOfReservedSpace */
  EAT_MATCH_C(resp->srmStatusOfReserveSpaceRequestResponse->lifetimeOfReservedSpace,
              lifetimeOfReservedSpace,
              PI2CSTR(resp->srmStatusOfReserveSpaceRequestResponse->lifetimeOfReservedSpace));
  
  /* spaceToken */
  EAT_MATCH_C(resp->srmStatusOfReserveSpaceRequestResponse->spaceToken,
              spaceToken,
              CSTR(resp->srmStatusOfReserveSpaceRequestResponse->spaceToken));

  RETURN(matchReturnStatus(resp->srmStatusOfReserveSpaceRequestResponse->returnStatus, proc));
}

std::string
srmStatusOfReserveSpaceRequest::toString(Process *proc)
{
  DM_DBG_I;
  GET_SRM_RESP(StatusOfReserveSpaceRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmStatusOfReserveSpaceRequest");
  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);

  /* response (parser) */
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
  if(!resp || !resp->srmStatusOfReserveSpaceRequestResponse) RETURN(ss.str());

  /* estimatedProcessingTime */
  SS_P_DQ_C(resp->srmStatusOfReserveSpaceRequestResponse->estimatedProcessingTime,
            estimatedProcessingTime,
            PI2CSTR(resp->srmStatusOfReserveSpaceRequestResponse->estimatedProcessingTime));

  /* retentionPolicyInfo */
  SS_P_DQ_C(resp->srmStatusOfReserveSpaceRequestResponse->retentionPolicyInfo,
            respRetentionPolicy,
            getTRetentionPolicy(resp->srmStatusOfReserveSpaceRequestResponse->retentionPolicyInfo->retentionPolicy).c_str());

  /* accessLatency */
  SS_P_DQ_C(resp->srmStatusOfReserveSpaceRequestResponse->retentionPolicyInfo && resp->srmStatusOfReserveSpaceRequestResponse->retentionPolicyInfo->accessLatency,
            respAccessLatency,
            getTAccessLatency(*resp->srmStatusOfReserveSpaceRequestResponse->retentionPolicyInfo->accessLatency).c_str());

  /* sizeOfTotalReservedSpace */
  SS_P_DQ_C(resp->srmStatusOfReserveSpaceRequestResponse->sizeOfTotalReservedSpace,
            sizeOfTotalReservedSpace,
            PI2CSTR(resp->srmStatusOfReserveSpaceRequestResponse->sizeOfTotalReservedSpace));
  
  /* sizeOfGuaranteedReservedSpace */
  SS_P_DQ_C(resp->srmStatusOfReserveSpaceRequestResponse->sizeOfGuaranteedReservedSpace,
            sizeOfGuaranteedReservedSpace,
            PI2CSTR(resp->srmStatusOfReserveSpaceRequestResponse->sizeOfGuaranteedReservedSpace));

  /* lifetimeOfReservedSpace */
  SS_P_DQ_C(resp->srmStatusOfReserveSpaceRequestResponse->lifetimeOfReservedSpace,
            lifetimeOfReservedSpace,
            PI2CSTR(resp->srmStatusOfReserveSpaceRequestResponse->lifetimeOfReservedSpace));
  
  /* spaceToken */
  SS_P_DQ_C(resp->srmStatusOfReserveSpaceRequestResponse->spaceToken,
            spaceToken,
            CSTR(resp->srmStatusOfReserveSpaceRequestResponse->spaceToken));

  SS_P_SRM_RETSTAT(resp->srmStatusOfReserveSpaceRequestResponse);

  RETURN(ss.str());
}
