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
#include "str.h"		/* dq_param() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * srmStatusOfGetRequest request constuctor
 */
srmStatusOfGetRequest::srmStatusOfGetRequest()
{
  init();
}

/*
 * Initialise srmStatusOfGetRequest request
 */
void
srmStatusOfGetRequest::init()
{
  /* request (parser/API) */
  requestToken = NULL;

  /* response (parser) */
  fileStatuses = NULL;
  remainingTotalRequestTime = NULL;
}

/*
 * srmStatusOfGetRequest request copy constuctor
 */
srmStatusOfGetRequest::srmStatusOfGetRequest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmStatusOfGetRequest request destructor
 */
srmStatusOfGetRequest::~srmStatusOfGetRequest()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);
  DELETE_VEC(SURL);

  /* response (parser) */
  DELETE(fileStatuses);
  DELETE(remainingTotalRequestTime);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmStatusOfGetRequest::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(StatusOfGetRequest);
}

int
srmStatusOfGetRequest::exec(Process *proc)
{
  DM_DBG_I;

  std::vector <std::string *> SURL = proc->eval_vec_str(srmStatusOfGetRequest::SURL);

#ifdef SRM2_CALL
  NEW_SRM_RET(StatusOfGetRequest);

  StatusOfGetRequest(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(requestToken),
    SURL,
    resp
  );
#endif

  DELETE_VEC(SURL);

  /* matching */
  if(!resp || !resp->srmStatusOfGetRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  EAT_MATCH(fileStatuses, arrayOfStatusOfGetRequestResponseToString(proc, FALSE, FALSE).c_str());

  /* remainingTotalRequestTime */
  EAT_MATCH_C(resp->srmStatusOfGetRequestResponse->remainingTotalRequestTime,
              remainingTotalRequestTime,
              PI2CSTR(resp->srmStatusOfGetRequestResponse->remainingTotalRequestTime));

  RETURN(matchReturnStatus(resp->srmStatusOfGetRequestResponse->returnStatus, proc));
}

std::string
srmStatusOfGetRequest::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfGetRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> SURL =
    proc? proc->eval_vec_str(srmStatusOfGetRequest::SURL):
          srmStatusOfGetRequest::SURL;
  
  /* request */  
  SS_SRM("srmStatusOfGetRequest");
  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);
  SS_VEC_DEL(SURL);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(remainingTotalRequestTime);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmStatusOfGetRequestResponse) RETURN(ss.str());

  ss << arrayOfStatusOfGetRequestResponseToString(proc, TRUE, quote);

  /* remainingTotalRequestTime */
  SS_P_DQ_C(resp->srmStatusOfGetRequestResponse->remainingTotalRequestTime,
            remainingTotalRequestTime,
            PI2CSTR(resp->srmStatusOfGetRequestResponse->remainingTotalRequestTime));

  SS_P_SRM_RETSTAT(resp->srmStatusOfGetRequestResponse);

  RETURN(ss.str());
}

std::string
srmStatusOfGetRequest::arrayOfStatusOfGetRequestResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfGetRequest);
  std::stringstream ss;

  if(!resp || !resp->srmStatusOfGetRequestResponse) RETURN(ss.str());

  if(resp->srmStatusOfGetRequestResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TGetRequestFileStatus *> v = resp->srmStatusOfGetRequestResponse->arrayOfFileStatuses->statusArray;

    /* exactly the same code as in srmPrepareToGet */
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(sourceSURL);
      SS_P_VEC_DPAR(fileSize);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_DPAR(estimatedWaitTime);
      SS_P_VEC_DPAR(remainingPinTime);
      SS_P_VEC_DPAR(transferURL);

      if(v[u] && v[u]->transferProtocolInfo) {
        std::vector<srm__TExtraInfo *> extraInfoArray = v[u]->transferProtocolInfo->extraInfoArray;
        SS_P_VEC_SRM_EXTRA_INFOu(extraInfoArray);
      }
    }
  }

  RETURN(ss.str());
}
