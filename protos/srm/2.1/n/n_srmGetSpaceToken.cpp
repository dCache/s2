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
 * srmGetSpaceToken request constuctor
 */
srmGetSpaceToken::srmGetSpaceToken()
{
  init();
}

/*
 * Initialise srmGetSpaceToken request
 */
void
srmGetSpaceToken::init()
{
  /* request (parser/API) */
  userSpaceTokenDescription = NULL;

  /* response (parser) */
  possibleSpaceTokens = NULL;
}

/*
 * srmGetSpaceToken request copy constuctor
 */
srmGetSpaceToken::srmGetSpaceToken(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmGetSpaceToken request destructor
 */
srmGetSpaceToken::~srmGetSpaceToken()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(userSpaceTokenDescription);

  /* response (parser) */
  DELETE(possibleSpaceTokens);

  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmGetSpaceToken::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(GetSpaceToken);
}

int
srmGetSpaceToken::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

#ifdef SRM2_CALL
  NEW_SRM_RET(GetSpaceToken);

  GetSpaceToken(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(userSpaceTokenDescription),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmGetSpaceTokenResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  struct srm__ArrayOfTSpaceToken *ArrayOfTSpaceToken = resp->srmGetSpaceTokenResponse->arrayOfPossibleSpaceTokens;
  if(!ArrayOfTSpaceToken) {
    /* no returned possible space tokens */
    DM_LOG(DM_N(1), "no returned possible space tokens\n");
    RETURN(ERR_ERR);
  }

  for(uint i = 0; i < ArrayOfTSpaceToken->tokenArray.size(); i++) {
    match = proc->e_match(possibleSpaceTokens,
                          ArrayOfTSpaceToken->tokenArray[i]->value.c_str());
    if(match) {
      DM_LOG(DM_N(1), "we have a match\n");
      break;
    }
  }
  if(match == FALSE) {
    /* no matching space tokens */
    DM_LOG(DM_N(1), "no matching space tokens\n");
    RETURN(ERR_ERR);
  }
  
  RETURN(matchReturnStatus(resp->srmGetSpaceTokenResponse->returnStatus, proc));
}

std::string
srmGetSpaceToken::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(GetSpaceToken);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmGetSpaceToken");
  SS_P_DQ(userID);
  SS_P_DQ(userSpaceTokenDescription);

  /* response (parser) */
  SS_P_DQ(possibleSpaceTokens);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);
  
  /* response (API) */
  if(!resp || !resp->srmGetSpaceTokenResponse) RETURN(ss.str());

  struct srm__ArrayOfTSpaceToken *ArrayOfTSpaceToken = resp->srmGetSpaceTokenResponse->arrayOfPossibleSpaceTokens;
  if(!ArrayOfTSpaceToken) {
    /* no possible space tokens returned */
    DM_LOG(DM_N(1), "no possible space tokens returned\n");
    RETURN(ss.str());
  }

  for(uint i = 0; i < ArrayOfTSpaceToken->tokenArray.size(); i++) {
    ss << " tokenArray" << i << "=" << ArrayOfTSpaceToken->tokenArray[i]->value;
  }

  SS_P_SRM_RETSTAT(resp->srmGetSpaceTokenResponse);

  RETURN(ss.str());
}
