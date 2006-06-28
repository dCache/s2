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
 * srmStatusOfBringOnlineRequest request constuctor
 */
srmStatusOfBringOnlineRequest::srmStatusOfBringOnlineRequest()
{
  init();
}

/*
 * Initialise srmStatusOfBringOnlineRequest request
 */
void
srmStatusOfBringOnlineRequest::init()
{
  /* request (parser/API) */
  requestToken = NULL;

  /* response (parser) */
  fileStatuses = NULL;
  remainingTotalRequestTime = NULL;
  remainingDeferredStartTime = NULL;
}

/*
 * srmStatusOfBringOnlineRequest request copy constuctor
 */
srmStatusOfBringOnlineRequest::srmStatusOfBringOnlineRequest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmStatusOfBringOnlineRequest request destructor
 */
srmStatusOfBringOnlineRequest::~srmStatusOfBringOnlineRequest()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);
  DELETE_VEC(urlArray);

  /* response (parser) */
  DELETE(fileStatuses);
  DELETE(remainingTotalRequestTime);
  DELETE(remainingDeferredStartTime);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmStatusOfBringOnlineRequest::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(StatusOfBringOnlineRequest);
}

int
srmStatusOfBringOnlineRequest::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

  std::vector <std::string *> urlArray = proc->eval_vec_str(srmStatusOfBringOnlineRequest::urlArray);

#ifdef SRM2_CALL
  NEW_SRM_RET(StatusOfBringOnlineRequest);

  StatusOfBringOnlineRequest(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(requestToken),
    urlArray,
    resp
  );
#endif

  DELETE_VEC(urlArray);

  /* matching */
  if(!resp || !resp->srmStatusOfBringOnlineRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  EAT_MATCH(fileStatuses, arrayOfStatusOfBringOnlineRequestResponseToString(proc, FALSE, FALSE).c_str());

  /* remainingTotalRequestTime */
  EAT_MATCH_C(resp->srmStatusOfBringOnlineRequestResponse->remainingTotalRequestTime,
              remainingTotalRequestTime,
              PI2CSTR(resp->srmStatusOfBringOnlineRequestResponse->remainingTotalRequestTime));

  /* remainingDeferredStartTime */
  EAT_MATCH_C(resp->srmStatusOfBringOnlineRequestResponse->remainingDeferredStartTime,
              remainingDeferredStartTime,
              PI2CSTR(resp->srmStatusOfBringOnlineRequestResponse->remainingDeferredStartTime));

  RETURN(matchReturnStatus(resp->srmStatusOfBringOnlineRequestResponse->returnStatus, proc));
}

std::string
srmStatusOfBringOnlineRequest::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfBringOnlineRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> urlArray =
    proc? proc->eval_vec_str(srmStatusOfBringOnlineRequest::urlArray):
          srmStatusOfBringOnlineRequest::urlArray;
  
  /* request */  
  SS_SRM("srmStatusOfBringOnlineRequest");
  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);
  SS_VEC_DEL(urlArray);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(remainingTotalRequestTime);
  SS_P_DQ(remainingDeferredStartTime);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmStatusOfBringOnlineRequestResponse) RETURN(ss.str());

  ss << arrayOfStatusOfBringOnlineRequestResponseToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmStatusOfBringOnlineRequestResponse);

  RETURN(ss.str());
}

std::string
srmStatusOfBringOnlineRequest::arrayOfStatusOfBringOnlineRequestResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfBringOnlineRequest);
  std::stringstream ss;

  if(!resp || !resp->srmStatusOfBringOnlineRequestResponse) RETURN(ss.str());

  if(resp->srmStatusOfBringOnlineRequestResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TBringOnlineRequestFileStatus *> v = resp->srmStatusOfBringOnlineRequestResponse->arrayOfFileStatuses->statusArray;

    /* exactly the same code as in srmBringOnline */
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
