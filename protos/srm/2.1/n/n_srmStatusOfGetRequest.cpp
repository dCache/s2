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
  DELETE_VEC(surlArray);

  /* response (parser) */
  DELETE(fileStatuses);
  
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
  BOOL match = FALSE;

  std::vector <std::string *> surlArray = proc->eval_vec_str(srmStatusOfGetRequest::surlArray);

#ifdef SRM2_CALL
  NEW_SRM_RET(StatusOfGetRequest);

  StatusOfGetRequest(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(requestToken),
    surlArray,
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmStatusOfGetRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  match = proc->e_match(fileStatuses, arrayOfStatusOfGetRequestResponseToString(proc, FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmStatusOfGetRequestResponse->returnStatus, proc));
}

std::string
srmStatusOfGetRequest::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfGetRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> surlArray =
    proc? proc->eval_vec_str(srmStatusOfGetRequest::surlArray):
          srmStatusOfGetRequest::surlArray;
  
  /* request */  
  SS_SRM("srmStatusOfGetRequest");
  SS_P_DQ(userID);
  SS_P_DQ(requestToken);
  SS_VEC(surlArray); if(proc) DELETE_VEC(surlArray);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmStatusOfGetRequestResponse) RETURN(ss.str());

  ss << arrayOfStatusOfGetRequestResponseToString(proc, TRUE, quote);

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
    std::vector<srm__TGetRequestFileStatus *> v = resp->srmStatusOfGetRequestResponse->arrayOfFileStatuses->getStatusArray;
    for(uint i = 0; i < v.size(); i++) {
      SS_P_VEC_PAR_VAL(estimatedProcessingTime);
      SS_P_VEC_PAR_VAL(estimatedWaitTimeOnQueue);
      SS_P_VEC_PAR_VAL(fileSize);
      SS_P_VEC_PAR_VAL(fromSURLInfo);
      SS_P_VEC_PAR_VAL(remainingPinTime);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_PAR_VAL(transferURL);
    }
  }
  RETURN(ss.str());
}
