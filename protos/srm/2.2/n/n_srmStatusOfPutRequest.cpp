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
 * srmStatusOfPutRequest request constuctor
 */
srmStatusOfPutRequest::srmStatusOfPutRequest()
{
  init();
}

/*
 * Initialise srmStatusOfPutRequest request
 */
void
srmStatusOfPutRequest::init()
{
  /* request (parser/API) */
  requestToken = NULL;

  /* response (parser) */
  fileStatuses = NULL;
  remainingTotalRequestTime = NULL;
}

/*
 * srmStatusOfPutRequest request copy constuctor
 */
srmStatusOfPutRequest::srmStatusOfPutRequest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmStatusOfPutRequest request destructor
 */
srmStatusOfPutRequest::~srmStatusOfPutRequest()
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
srmStatusOfPutRequest::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(StatusOfPutRequest);
}

int
srmStatusOfPutRequest::exec(Process *proc)
{
  DM_DBG_I;

  std::vector <std::string *> SURL = proc->eval_vec_str(srmStatusOfPutRequest::SURL);

#ifdef SRM2_CALL
  NEW_SRM_RET(StatusOfPutRequest);

  StatusOfPutRequest(
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
  if(!resp || !resp->srmStatusOfPutRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  EAT_MATCH(fileStatuses, arrayOfStatusOfPutRequestResponseToString(proc, FALSE, FALSE).c_str());

  /* remainingTotalRequestTime */
  EAT_MATCH_C(resp->srmStatusOfPutRequestResponse->remainingTotalRequestTime,
              remainingTotalRequestTime,
              PI2CSTR(resp->srmStatusOfPutRequestResponse->remainingTotalRequestTime));

  RETURN(matchReturnStatus(resp->srmStatusOfPutRequestResponse->returnStatus, proc));
}

std::string
srmStatusOfPutRequest::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfPutRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> SURL =
    proc? proc->eval_vec_str(srmStatusOfPutRequest::SURL):
          srmStatusOfPutRequest::SURL;
  
  /* request */  
  SS_SRM("srmStatusOfPutRequest");
  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);
  SS_VEC_DEL(SURL);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(remainingTotalRequestTime);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmStatusOfPutRequestResponse) RETURN(ss.str());

  ss << arrayOfStatusOfPutRequestResponseToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmStatusOfPutRequestResponse);

  RETURN(ss.str());
}

std::string
srmStatusOfPutRequest::arrayOfStatusOfPutRequestResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfPutRequest);
  std::stringstream ss;

  if(!resp || !resp->srmStatusOfPutRequestResponse) RETURN(ss.str());
  if(resp->srmStatusOfPutRequestResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TPutRequestFileStatus *> v = resp->srmStatusOfPutRequestResponse->arrayOfFileStatuses->statusArray;

    /* exactly the same code as in srmPrepareToPut */
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
