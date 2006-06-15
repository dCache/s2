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

  /* response (API) */
  resp = new srm__srmStatusOfPutRequestResponse_();
  if(resp == NULL) {
    DM_ERR(ERR_SYSTEM, "new failed\n");
  } else {
    memset(resp, 0, sizeof(srm__srmStatusOfPutRequestResponse_));
  }
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
  DELETE_VEC(surlArray);

  /* response (parser) */
  DELETE(fileStatuses);
  
  /* response (API) */
  DELETE(resp);

  DM_DBG_O;
}

int
srmStatusOfPutRequest::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

  std::vector <std::string *> surlArray = proc->eval_vec_str(srmStatusOfPutRequest::surlArray);

#ifdef SRM2_CALL
  StatusOfPutRequest(
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(requestToken),
    surlArray,
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmStatusOfPutRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  match = proc->e_match(fileStatuses, arrayOfStatusOfPutRequestResponseToString(FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmStatusOfPutRequestResponse->returnStatus, proc));
}

std::string
srmStatusOfPutRequest::toString(Process *proc)
{
  DM_DBG_I;
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> surlArray =
    proc? proc->eval_vec_str(srmStatusOfPutRequest::surlArray):
          srmStatusOfPutRequest::surlArray;
  
  /* request */  
  SS_SRM("srmStatusOfPutRequest");
  SS_P_DQ(userID);
  SS_P_DQ(requestToken);
  SS_VEC(surlArray); if(proc) DELETE_VEC(surlArray);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmStatusOfPutRequestResponse) RETURN(ss.str());

  ss << arrayOfStatusOfPutRequestResponseToString(TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmStatusOfPutRequestResponse);

  RETURN(ss.str());
}

std::string
srmStatusOfPutRequest::arrayOfStatusOfPutRequestResponseToString(BOOL space, BOOL quote) const
{
  DM_DBG_I;
  std::stringstream ss;

  if(!resp || !resp->srmStatusOfPutRequestResponse) RETURN(ss.str());
  if(resp->srmStatusOfPutRequestResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TPutRequestFileStatus *> v = resp->srmStatusOfPutRequestResponse->arrayOfFileStatuses->putStatusArray;
    for(uint i = 0; i < v.size(); i++) {
      SS_P_VEC_PAR_VAL(estimatedProcessingTime);
      SS_P_VEC_PAR_VAL(estimatedWaitTimeOnQueue);
      SS_P_VEC_PAR_VAL(fileSize);
      SS_P_VEC_PAR_VAL(remainingPinTime);
      SS_P_VEC_PAR_VAL(siteURL);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_PAR_VAL(transferURL);
    }
  }
  RETURN(ss.str());
}