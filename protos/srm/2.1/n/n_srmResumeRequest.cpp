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
 * srmResumeRequest request constuctor
 */
srmResumeRequest::srmResumeRequest()
{
  init();
}

/*
 * Initialise srmResumeRequest request
 */
void
srmResumeRequest::init()
{
  /* request (parser/API) */
  requestToken = NULL;

  /* response (parser) */

  /* response (API) */
  resp = new srm__srmResumeRequestResponse_();
  if(resp == NULL) {
    DM_ERR(ERR_SYSTEM, "new failed\n");
  } else {
    memset(resp, 0, sizeof(srm__srmResumeRequestResponse_));
  }
}

/*
 * srmResumeRequest request copy constuctor
 */
srmResumeRequest::srmResumeRequest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmResumeRequest request destructor
 */
srmResumeRequest::~srmResumeRequest()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);

  /* response (parser) */
  
  /* response (API) */
  DELETE(resp);

  DM_DBG_O;
}

int
srmResumeRequest::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  ResumeRequest(
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(requestToken),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmResumeRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmResumeRequestResponse->returnStatus, proc));
}

std::string
srmResumeRequest::toString(Process *proc)
{
  DM_DBG_I;
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmResumeRequest");
  SS_P_DQ(userID);
  SS_P_DQ(requestToken);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmResumeRequestResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmResumeRequestResponse);

  RETURN(ss.str());
}
