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
 * srmGetRequestTokens request constuctor
 */
srmGetRequestTokens::srmGetRequestTokens()
{
  init();
}

/*
 * Initialise srmGetRequestTokens request
 */
void
srmGetRequestTokens::init()
{
  /* request (parser/API) */
  userRequestDescription = NULL;

  /* response (parser) */
  requestTokens = NULL;
}

/*
 * srmGetRequestTokens request copy constuctor
 */
srmGetRequestTokens::srmGetRequestTokens(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmGetRequestTokens request destructor
 */
srmGetRequestTokens::~srmGetRequestTokens()
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
srmGetRequestTokens::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(GetRequestTokens);
}

int
srmGetRequestTokens::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RET(GetRequestTokens);

  GetRequestTokens(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(userRequestDescription),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmGetRequestTokensResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  EAT_MATCH(requestTokens, arrayOfRequestDetailsToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmGetRequestTokensResponse->returnStatus, proc));
}

std::string
srmGetRequestTokens::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(GetRequestTokens);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmGetRequestTokens");
  SS_P_DQ(authorizationID);
  SS_P_DQ(userRequestDescription);

  /* response (parser) */
  SS_P_DQ(requestTokens);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmGetRequestTokensResponse) RETURN(ss.str());

  ss << arrayOfRequestDetailsToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmGetRequestTokensResponse);
  
  RETURN(ss.str());
}

std::string
srmGetRequestTokens::arrayOfRequestDetailsToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(GetRequestTokens);
  std::stringstream ss;

  if(!resp || !resp->srmGetRequestTokensResponse) RETURN(ss.str());

  if(resp->srmGetRequestTokensResponse->arrayOfRequestTokens) {
    BOOL print_space = FALSE;
    std::vector<srm__TRequestTokenReturn *> v = resp->srmGetRequestTokensResponse->arrayOfRequestTokens->tokenArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(requestToken);
      SS_P_VEC_DPAR(createdAtTime);
    }
  }
  
  RETURN(ss.str());
}
