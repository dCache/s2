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
 * srmPing request constuctor
 */
srmPing::srmPing()
{
  init();
}

/*
 * Initialise srmPing request
 */
void
srmPing::init()
{
  /* request (parser/API) */

  /* response (parser) */
  versionInfo = NULL;
  otherInfo = NULL;
}

/*
 * srmPing request Ping constuctor
 */
srmPing::srmPing(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmPing request destructor
 */
srmPing::~srmPing()
{
  DM_DBG_I;

  /* request (parser/API) */

  /* response (parser) */
  DELETE(versionInfo);
  DELETE(otherInfo);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmPing::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(Ping);
}

int
srmPing::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RET(Ping);

  Ping(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmPingResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* versionInfo */
  EAT_MATCH(versionInfo, resp->srmPingResponse->versionInfo.c_str());

  /* arrayOfFileStatus */
  EAT_MATCH(otherInfo, arrayOfOtherInfoToString(proc, FALSE, FALSE).c_str());
  
  return ERR_OK;
}

std::string
srmPing::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(Ping);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmPing");
  SS_P_DQ(authorizationID);

  /* response (parser) */
  SS_P_DQ(versionInfo);
  SS_P_DQ(otherInfo);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmPingResponse) RETURN(ss.str());

  ss << " versionInfo=" << resp->srmPingResponse->versionInfo;
  ss << arrayOfOtherInfoToString(proc, TRUE, quote);
  
  RETURN(ss.str());
}

std::string
srmPing::arrayOfOtherInfoToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(Ping);
  std::stringstream ss;

  if(!resp || !resp->srmPingResponse) RETURN(ss.str());

  if(resp->srmPingResponse->otherInfo) {
    BOOL print_space = FALSE;
    std::vector<srm__TExtraInfo *> extraInfoArray = resp->srmPingResponse->otherInfo->extraInfoArray;
    SS_P_VEC_SRM_EXTRA_INFO(extraInfoArray);
  }

  RETURN(ss.str());
}
