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
#include "str.h"		/* i2str() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * srmExtendFileLifeTime request constuctor
 */
srmExtendFileLifeTime::srmExtendFileLifeTime()
{
  init();
}

/*
 * Initialise srmExtendFileLifeTime request
 */
void
srmExtendFileLifeTime::init()
{
  /* request (parser/API) */
  requestToken = NULL;
  siteURL = NULL;
  newLifeTime = NULL;

  /* response (parser) */
  newTimeExtended = NULL;
}

/*
 * srmExtendFileLifeTime request copy constuctor
 */
srmExtendFileLifeTime::srmExtendFileLifeTime(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmExtendFileLifeTime request destructor
 */
srmExtendFileLifeTime::~srmExtendFileLifeTime()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);
  DELETE(siteURL);
  DELETE(newLifeTime);

  /* response (parser) */
  DELETE(newTimeExtended);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmExtendFileLifeTime::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(ExtendFileLifeTime);
}

int
srmExtendFileLifeTime::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

#ifdef SRM2_CALL
  NEW_SRM_RET(ExtendFileLifeTime);

  ExtendFileLifeTime(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(requestToken),
    EVAL2CSTR(siteURL),
    proc->eval2pint64(newLifeTime).p,
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmExtendFileLifeTimeResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* newTimeExtended */
  EAT_MATCH_3(resp->srmExtendFileLifeTimeResponse,
              newTimeExtended,
              i2str(resp->srmExtendFileLifeTimeResponse->newTimeExtended->value).c_str());

  RETURN(matchReturnStatus(resp->srmExtendFileLifeTimeResponse->returnStatus, proc));
}

std::string
srmExtendFileLifeTime::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(ExtendFileLifeTime);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmExtendFileLifeTime");

  SS_P_DQ(userID);
  SS_P_DQ(requestToken);
  SS_P_DQ(siteURL);
  SS_P_DQ(newLifeTime);

  /* response (parser) */
  SS_P_DQ(newTimeExtended);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmExtendFileLifeTimeResponse) RETURN(ss.str());

  SS_P_VALUE(resp->srmExtendFileLifeTimeResponse, newTimeExtended);

  SS_P_SRM_RETSTAT(resp->srmExtendFileLifeTimeResponse);

  RETURN(ss.str());
}
