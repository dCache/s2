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
 * srmAbortRequest request constuctor
 */
srmAbortRequest::srmAbortRequest()
{
  init();
}

/*
 * Initialise srmAbortRequest request
 */
void
srmAbortRequest::init()
{
  /* request (parser/API) */
  requestToken = NULL;

  /* response (parser) */

}

/*
 * srmAbortRequest request copy constuctor
 */
srmAbortRequest::srmAbortRequest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmAbortRequest request destructor
 */
srmAbortRequest::~srmAbortRequest()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);

  /* response (parser) */
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmAbortRequest::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(AbortRequest);
}

int
srmAbortRequest::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RET(AbortRequest);

  AbortRequest(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(requestToken),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmAbortRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmAbortRequestResponse->returnStatus, proc));
}

std::string
srmAbortRequest::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(AbortRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmAbortRequest");
  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmAbortRequestResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmAbortRequestResponse);

  RETURN(ss.str());
}
