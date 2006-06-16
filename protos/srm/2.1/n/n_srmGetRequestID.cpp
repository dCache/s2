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
 * srmGetRequestID request constuctor
 */
srmGetRequestID::srmGetRequestID()
{
  init();
}

/*
 * Initialise srmGetRequestID request
 */
void
srmGetRequestID::init()
{
  /* request (parser/API) */
  userRequestDescription = NULL;

  /* response (parser) */
  requestTokens = NULL;
}

/*
 * srmGetRequestID request copy constuctor
 */
srmGetRequestID::srmGetRequestID(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmGetRequestID request destructor
 */
srmGetRequestID::~srmGetRequestID()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(userRequestDescription);

  /* response (parser) */
  DELETE(requestTokens);

  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmGetRequestID::finish(Process *proc)
{
  DM_DBG_I;
  srm__srmGetRequestIDResponse_ *resp = (srm__srmGetRequestIDResponse_ *)proc->resp;
  
  DELETE(resp);
}

int
srmGetRequestID::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

#ifdef SRM2_CALL
  NEW_SRM_RESP(GetRequestID);

  GetRequestID(
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(userRequestDescription),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmGetRequestIDResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  match = proc->e_match(requestTokens, arrayOfRequestDetailsToString(proc, FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmGetRequestIDResponse->returnStatus, proc));
}

std::string
srmGetRequestID::toString(Process *proc)
{
  DM_DBG_I;

  srm__srmGetRequestIDResponse_ *resp = proc? (srm__srmGetRequestIDResponse_ *)proc->resp : NULL;
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmGetRequestID");
  SS_P_DQ(userID);
  SS_P_DQ(userRequestDescription);

  /* response (parser) */
  SS_P_DQ(requestTokens);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmGetRequestIDResponse) RETURN(ss.str());

  ss << arrayOfRequestDetailsToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmGetRequestIDResponse);

  RETURN(ss.str());
}

std::string
srmGetRequestID::arrayOfRequestDetailsToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  srm__srmGetRequestIDResponse_ *resp = proc? (srm__srmGetRequestIDResponse_ *)proc->resp : NULL;
  std::stringstream ss;

  if(!resp || !resp->srmGetRequestIDResponse) RETURN(ss.str());

  if(resp->srmGetRequestIDResponse->arrayOfRequestTokens) {
    BOOL print_space = FALSE;
    std::vector<srm__TRequestToken*> v = resp->srmGetRequestIDResponse->arrayOfRequestTokens->requestTokenArray;
    for(uint i = 0; i < v.size(); i++) {
      SS_P_VEC_VAL(requestToken);
    }
  }
  
  RETURN(ss.str());
}
