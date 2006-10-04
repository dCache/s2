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
  BOOL match = FALSE;

  std::vector <std::string *> surlArray = proc->eval_vec_str(srmStatusOfPutRequest::surlArray);

#ifdef SRM2_CALL
  NEW_SRM_RET(StatusOfPutRequest);

  StatusOfPutRequest(
    soap,
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
  match = proc->e_match(fileStatuses, arrayOfStatusOfPutRequestResponseToString(proc, FALSE, FALSE).c_str());
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

  GET_SRM_RESP(StatusOfPutRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> surlArray =
    proc? proc->eval_vec_str(srmStatusOfPutRequest::surlArray):
          srmStatusOfPutRequest::surlArray;
  
  /* request */  
  SS_SRM("srmStatusOfPutRequest");
  SS_P_DQ(userID);
  SS_P_DQ(requestToken);
  SS_VEC_DEL(surlArray);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
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
    std::vector<srm__TPutRequestFileStatus *> v = resp->srmStatusOfPutRequestResponse->arrayOfFileStatuses->putStatusArray;
    for(uint u = 0; u < v.size(); u++) {
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
