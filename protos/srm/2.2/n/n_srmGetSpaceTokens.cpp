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
 * srmGetSpaceTokens request constuctor
 */
srmGetSpaceTokens::srmGetSpaceTokens()
{
  init();
}

/*
 * Initialise srmGetSpaceTokens request
 */
void
srmGetSpaceTokens::init()
{
  /* request (parser/API) */
  userSpaceTokenDescription = NULL;

  /* response (parser) */
  spaceTokens = NULL;
}

/*
 * srmGetSpaceTokens request copy constuctor
 */
srmGetSpaceTokens::srmGetSpaceTokens(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmGetSpaceTokens request destructor
 */
srmGetSpaceTokens::~srmGetSpaceTokens()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(userSpaceTokenDescription);

  /* response (parser) */
  DELETE(spaceTokens);

  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmGetSpaceTokens::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(GetSpaceTokens);
}

int
srmGetSpaceTokens::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

#ifdef SRM2_CALL
  NEW_SRM_RET(GetSpaceTokens);

  GetSpaceTokens(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(userSpaceTokenDescription),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmGetSpaceTokensResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfSpaceTokens */
  EAT_MATCH(spaceTokens, arrayOfSpaceTokensToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmGetSpaceTokensResponse->returnStatus, proc));
}

std::string
srmGetSpaceTokens::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(GetSpaceTokens);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmGetSpaceTokens");
  SS_P_DQ(authorizationID);
  SS_P_DQ(userSpaceTokenDescription);

  /* response (parser) */
  SS_P_DQ(spaceTokens);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);
  
  /* response (API) */
  if(!resp || !resp->srmGetSpaceTokensResponse) RETURN(ss.str());

  ss << arrayOfSpaceTokensToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmGetSpaceTokensResponse);

  RETURN(ss.str());
}

std::string
srmGetSpaceTokens::arrayOfSpaceTokensToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(GetSpaceTokens);
  std::stringstream ss;

  if(!resp || !resp->srmGetSpaceTokensResponse) RETURN(ss.str());

  if(resp->srmGetSpaceTokensResponse->arrayOfSpaceTokens) {
    BOOL print_space = FALSE;
    std::vector<std::string> v = resp->srmGetSpaceTokensResponse->arrayOfSpaceTokens->stringArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_VEC_SPACE;
      ss << " spaceToken" << u << "=" << dq_param(v[u], quote);
    }
  }

  RETURN(ss.str());
}
