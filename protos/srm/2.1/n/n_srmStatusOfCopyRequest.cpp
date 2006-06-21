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
 * srmStatusOfCopyRequest request constuctor
 */
srmStatusOfCopyRequest::srmStatusOfCopyRequest()
{
  init();
}

/*
 * Initialise srmStatusOfCopyRequest request
 */
void
srmStatusOfCopyRequest::init()
{
  /* request (parser/API) */
  requestToken = NULL;

  /* response (parser) */
  fileStatuses = NULL;
}

/*
 * srmStatusOfCopyRequest request copy constuctor
 */
srmStatusOfCopyRequest::srmStatusOfCopyRequest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmStatusOfCopyRequest request destructor
 */
srmStatusOfCopyRequest::~srmStatusOfCopyRequest()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);
  DELETE_VEC(fromSurlArray);
  DELETE_VEC(toSurlArray);

  /* response (parser) */
  DELETE(fileStatuses);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmStatusOfCopyRequest::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(StatusOfCopyRequest);
}

int
srmStatusOfCopyRequest::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

  std::vector <std::string *> fromSurlArray = proc->eval_vec_str(srmStatusOfCopyRequest::fromSurlArray);
  std::vector <std::string *> toSurlArray = proc->eval_vec_str(srmStatusOfCopyRequest::toSurlArray);

#ifdef SRM2_CALL
  NEW_SRM_RET(StatusOfCopyRequest);

  StatusOfCopyRequest(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(requestToken),
    fromSurlArray,
    toSurlArray,
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmStatusOfCopyRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  match = proc->e_match(fileStatuses, arrayOfStatusOfCopyRequestResponseToString(proc, FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmStatusOfCopyRequestResponse->returnStatus, proc));
}

std::string
srmStatusOfCopyRequest::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfCopyRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> fromSurlArray =
    proc? proc->eval_vec_str(srmStatusOfCopyRequest::fromSurlArray):
          srmStatusOfCopyRequest::fromSurlArray;
  std::vector <std::string *> toSurlArray =
    proc? proc->eval_vec_str(srmStatusOfCopyRequest::toSurlArray):
          srmStatusOfCopyRequest::toSurlArray;
  
  /* request */  
  SS_SRM("srmStatusOfCopyRequest");
  SS_P_DQ(userID);
  SS_P_DQ(requestToken);
  SS_VEC(fromSurlArray); if(proc) DELETE_VEC(fromSurlArray);
  SS_VEC(toSurlArray); if(proc) DELETE_VEC(toSurlArray);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmStatusOfCopyRequestResponse) RETURN(ss.str());

  ss << arrayOfStatusOfCopyRequestResponseToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmStatusOfCopyRequestResponse);

  RETURN(ss.str());
}

std::string
srmStatusOfCopyRequest::arrayOfStatusOfCopyRequestResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfCopyRequest);
  std::stringstream ss;

  if(!resp || !resp->srmStatusOfCopyRequestResponse) RETURN(ss.str());
  if(resp->srmStatusOfCopyRequestResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TCopyRequestFileStatus *> v = resp->srmStatusOfCopyRequestResponse->arrayOfFileStatuses->copyStatusArray;
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
