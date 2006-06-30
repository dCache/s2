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
 * srmStatusOfUpdateSpaceRequest request constuctor
 */
srmStatusOfUpdateSpaceRequest::srmStatusOfUpdateSpaceRequest()
{
  init();
}

/*
 * Initialise srmStatusOfUpdateSpaceRequest request
 */
void
srmStatusOfUpdateSpaceRequest::init()
{
  /* request (parser/API) */
  requestToken = NULL;

  /* response (parser) */
  sizeOfTotalSpace = NULL;
  sizeOfGuaranteedSpace = NULL;
  lifetimeGranted = NULL;
}

/*
 * srmStatusOfUpdateSpaceRequest request copy constuctor
 */
srmStatusOfUpdateSpaceRequest::srmStatusOfUpdateSpaceRequest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmStatusOfUpdateSpaceRequest request destructor
 */
srmStatusOfUpdateSpaceRequest::~srmStatusOfUpdateSpaceRequest()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);

  /* response (parser) */
  DELETE(sizeOfTotalSpace);
  DELETE(sizeOfGuaranteedSpace);
  DELETE(lifetimeGranted);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmStatusOfUpdateSpaceRequest::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(StatusOfUpdateSpaceRequest);
}

int
srmStatusOfUpdateSpaceRequest::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RET(StatusOfUpdateSpaceRequest);

  StatusOfUpdateSpaceRequest(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(requestToken),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmStatusOfUpdateSpaceRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* sizeOfTotalSpace */
  EAT_MATCH_C(resp->srmStatusOfUpdateSpaceRequestResponse->sizeOfTotalSpace,
              sizeOfTotalSpace,
              PI2CSTR(resp->srmStatusOfUpdateSpaceRequestResponse->sizeOfTotalSpace));

  /* sizeOfGuaranteedSpace */
  EAT_MATCH_C(resp->srmStatusOfUpdateSpaceRequestResponse->sizeOfGuaranteedSpace,
              sizeOfGuaranteedSpace,
              PI2CSTR(resp->srmStatusOfUpdateSpaceRequestResponse->sizeOfGuaranteedSpace));

  /* lifetimeGranted */
  EAT_MATCH_C(resp->srmStatusOfUpdateSpaceRequestResponse,
              lifetimeGranted,
              PI2CSTR(resp->srmStatusOfUpdateSpaceRequestResponse->lifetimeGranted));

  RETURN(matchReturnStatus(resp->srmStatusOfUpdateSpaceRequestResponse->returnStatus, proc));
}

std::string
srmStatusOfUpdateSpaceRequest::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfUpdateSpaceRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmStatusOfUpdateSpaceRequest");
  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);

  /* response (parser) */
  SS_P_DQ(sizeOfTotalSpace);
  SS_P_DQ(sizeOfGuaranteedSpace);
  SS_P_DQ(lifetimeGranted);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmStatusOfUpdateSpaceRequestResponse) RETURN(ss.str());

  /* sizeOfTotalSpace */
  SS_P_DQ_C(resp->srmStatusOfUpdateSpaceRequestResponse->sizeOfTotalSpace,
            sizeOfTotalSpace,
            PI2CSTR(resp->srmStatusOfUpdateSpaceRequestResponse->sizeOfTotalSpace));

  /* sizeOfGuaranteedSpace */
  SS_P_DQ_C(resp->srmStatusOfUpdateSpaceRequestResponse->sizeOfGuaranteedSpace,
            sizeOfGuaranteedSpace,
            PI2CSTR(resp->srmStatusOfUpdateSpaceRequestResponse->sizeOfGuaranteedSpace));

  /* lifetimeGranted */
  SS_P_DQ_C(resp->srmStatusOfUpdateSpaceRequestResponse,
            lifetimeGranted,
            PI2CSTR(resp->srmStatusOfUpdateSpaceRequestResponse->lifetimeGranted));

  SS_P_SRM_RETSTAT(resp->srmStatusOfUpdateSpaceRequestResponse);

  RETURN(ss.str());

#undef EVAL_VEC_STR_US
}
