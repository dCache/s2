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
 * srmGetRequestSummary request constuctor
 */
srmGetRequestSummary::srmGetRequestSummary()
{
  init();
}

/*
 * Initialise srmGetRequestSummary request
 */
void
srmGetRequestSummary::init()
{
  /* request (parser/API) */

  /* response (parser) */
  requestSummary = NULL;
}

/*
 * srmGetRequestSummary request copy constuctor
 */
srmGetRequestSummary::srmGetRequestSummary(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmGetRequestSummary request destructor
 */
srmGetRequestSummary::~srmGetRequestSummary()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE_VEC(arrayOfRequestToken);

  /* response (parser) */
  DELETE(requestSummary);

  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmGetRequestSummary::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(GetRequestSummary);
}

int
srmGetRequestSummary::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

  std::vector <std::string *> arrayOfRequestToken = proc->eval_vec_str(srmGetRequestSummary::arrayOfRequestToken);

#ifdef SRM2_CALL
  NEW_SRM_RET(GetRequestSummary);

  GetRequestSummary(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    arrayOfRequestToken,
    resp
  );
#endif

  DELETE_VEC(arrayOfRequestToken);

  /* matching */
  if(!resp || !resp->srmGetRequestSummaryResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  match = proc->e_match(requestSummary, arrayOfRequestDetailsToString(proc, FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmGetRequestSummaryResponse->returnStatus, proc));
}

std::string
srmGetRequestSummary::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(GetRequestSummary);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> arrayOfRequestToken =
    proc? proc->eval_vec_str(srmGetRequestSummary::arrayOfRequestToken):
          srmGetRequestSummary::arrayOfRequestToken;

  /* request */  
  SS_SRM("srmGetRequestSummary");
  SS_P_DQ(userID);
  SS_VEC_DEL(arrayOfRequestToken);

  /* response (parser) */
  SS_P_DQ(requestSummary);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmGetRequestSummaryResponse) RETURN(ss.str());

  ss << arrayOfRequestDetailsToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmGetRequestSummaryResponse);
  
  RETURN(ss.str());
}

std::string
srmGetRequestSummary::arrayOfRequestDetailsToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(GetRequestSummary);
  std::stringstream ss;

  if(!resp || !resp->srmGetRequestSummaryResponse) RETURN(ss.str());

  if(resp->srmGetRequestSummaryResponse->arrayOfRequestSummaries) {
    BOOL print_space = FALSE;
    std::vector<srm__TRequestSummary*> v = resp->srmGetRequestSummaryResponse->arrayOfRequestSummaries->summaryArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(isSuspended);
      SS_P_VEC_PAR(numOfFinishedRequests);
      SS_P_VEC_PAR(numOfProgressingRequests);
      SS_P_VEC_PAR(numOfQueuedRequests);
      SS_P_VEC_PAR_VAL(requestToken);
      SS_P_VEC_PAR_REQUESTTYPE(requestType);
//      if(v[i] && v[i]->requestType) {SS_VEC_SPACE; ss << "requestType" << i << "=" << *(v[i]->requestType);}
      SS_P_VEC_PAR(totalFilesInThisRequest);
    }
  }
  
  RETURN(ss.str());
}
