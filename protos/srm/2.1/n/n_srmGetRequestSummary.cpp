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

  /* response (API) */
  resp = new srm__srmGetRequestSummaryResponse_();
  if(resp == NULL) {
    DM_ERR(ERR_SYSTEM, "new failed\n");
  } else {
    memset(resp, 0, sizeof(srm__srmGetRequestSummaryResponse_));
  }
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

  /* response (API) */
  DELETE(resp);

  DM_DBG_O;
}

int
srmGetRequestSummary::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

  std::vector <std::string *> arrayOfRequestToken = proc->eval_vec_str(srmGetRequestSummary::arrayOfRequestToken);

#ifdef SRM2_CALL
  GetRequestSummary(
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
  match = proc->e_match(requestSummary, arrayOfRequestDetailsToString(FALSE, FALSE).c_str());
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
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> arrayOfRequestToken =
    proc? proc->eval_vec_str(srmGetRequestSummary::arrayOfRequestToken):
          srmGetRequestSummary::arrayOfRequestToken;

  /* request */  
  SS_SRM("srmGetRequestSummary");
  SS_P_DQ(userID);
  SS_VEC(arrayOfRequestToken); if(proc) DELETE_VEC(arrayOfRequestToken);

  /* response (parser) */
  SS_P_DQ(requestSummary);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmGetRequestSummaryResponse) RETURN(ss.str());

  ss << arrayOfRequestDetailsToString(TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmGetRequestSummaryResponse);
  
  RETURN(ss.str());
}

std::string
srmGetRequestSummary::arrayOfRequestDetailsToString(BOOL space, BOOL quote) const
{
  DM_DBG_I;
  std::stringstream ss;

  if(!resp || !resp->srmGetRequestSummaryResponse) RETURN(ss.str());

  if(resp->srmGetRequestSummaryResponse->arrayOfRequestSummaries) {
    BOOL print_space = FALSE;
    std::vector<srm__TRequestSummary*> v = resp->srmGetRequestSummaryResponse->arrayOfRequestSummaries->summaryArray;
    for(uint i = 0; i < v.size(); i++) {
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
