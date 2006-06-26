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
  remainingTotalRequestTime = NULL;
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
  DELETE_VEC(sourceUrlArray);
  DELETE_VEC(targetUrlArray);

  /* response (parser) */
  DELETE(fileStatuses);
  DELETE(remainingTotalRequestTime);
  
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

  std::vector <std::string *> sourceUrlArray = proc->eval_vec_str(srmStatusOfCopyRequest::sourceUrlArray);
  std::vector <std::string *> targetUrlArray = proc->eval_vec_str(srmStatusOfCopyRequest::targetUrlArray);

#ifdef SRM2_CALL
  NEW_SRM_RET(StatusOfCopyRequest);

  StatusOfCopyRequest(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(requestToken),
    sourceUrlArray,
    targetUrlArray,
    resp
  );
#endif
  
  DELETE_VEC(sourceUrlArray);
  DELETE_VEC(targetUrlArray);

  /* matching */
  if(!resp || !resp->srmStatusOfCopyRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  EAT_MATCH(fileStatuses, arrayOfStatusOfCopyRequestResponseToString(proc, FALSE, FALSE).c_str());

  /* remainingTotalRequestTime */
  EAT_MATCH_3(resp->srmStatusOfCopyRequestResponse, remainingTotalRequestTime, PI2CSTR(resp->srmStatusOfCopyRequestResponse->remainingTotalRequestTime));

  RETURN(matchReturnStatus(resp->srmStatusOfCopyRequestResponse->returnStatus, proc));
}

std::string
srmStatusOfCopyRequest::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfCopyRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> sourceUrlArray =
    proc? proc->eval_vec_str(srmStatusOfCopyRequest::sourceUrlArray):
          srmStatusOfCopyRequest::sourceUrlArray;
  
  std::vector <std::string *> targetUrlArray =
    proc? proc->eval_vec_str(srmStatusOfCopyRequest::targetUrlArray):
          srmStatusOfCopyRequest::targetUrlArray;
  
  /* request */  
  SS_SRM("srmStatusOfCopyRequest");
  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);
  SS_VEC(sourceUrlArray); if(proc) DELETE_VEC(sourceUrlArray);
  SS_VEC(targetUrlArray); if(proc) DELETE_VEC(targetUrlArray);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(remainingTotalRequestTime);
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
    std::vector<srm__TCopyRequestFileStatus *> v = resp->srmStatusOfCopyRequestResponse->arrayOfFileStatuses->statusArray;

    /* exactly the same code as in srmCopy */
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
