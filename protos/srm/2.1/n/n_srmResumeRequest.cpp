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
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmResumeRequest::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(ResumeRequest);
}

int
srmResumeRequest::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RET(ResumeRequest);

  ResumeRequest(
    soap,
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

  GET_SRM_RESP(ResumeRequest);
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
