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
 * srmSuspendRequest request constuctor
 */
srmSuspendRequest::srmSuspendRequest()
{
  init();
}

/*
 * Initialise srmSuspendRequest request
 */
void
srmSuspendRequest::init()
{
  /* request (parser/API) */
  requestToken = NULL;

  /* response (parser) */
}

/*
 * srmSuspendRequest request copy constuctor
 */
srmSuspendRequest::srmSuspendRequest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmSuspendRequest request destructor
 */
srmSuspendRequest::~srmSuspendRequest()
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
srmSuspendRequest::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(SuspendRequest);
}

int
srmSuspendRequest::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RET(SuspendRequest);

  SuspendRequest(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(requestToken),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmSuspendRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmSuspendRequestResponse->returnStatus, proc));
}

std::string
srmSuspendRequest::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(SuspendRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmSuspendRequest");
  SS_P_DQ(userID);
  SS_P_DQ(requestToken);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmSuspendRequestResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmSuspendRequestResponse);

  RETURN(ss.str());
}
