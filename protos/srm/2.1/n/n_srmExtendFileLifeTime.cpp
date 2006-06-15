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

  /* response (API) */
  resp = new srm__srmExtendFileLifeTimeResponse_();
  if(resp == NULL) {
    DM_ERR(ERR_SYSTEM, "new failed\n");
  } else {
    memset(resp, 0, sizeof(srm__srmExtendFileLifeTimeResponse_));
  }
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
  
  /* response (API) */
  DELETE(resp);

  DM_DBG_O;
}

int
srmExtendFileLifeTime::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

#ifdef SRM2_CALL
  ExtendFileLifeTime(
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
  EAT_MATCH(resp->srmExtendFileLifeTimeResponse,
            newTimeExtended,
            i2str(resp->srmExtendFileLifeTimeResponse->newTimeExtended->value).c_str());

  RETURN(matchReturnStatus(resp->srmExtendFileLifeTimeResponse->returnStatus, proc));
}

std::string
srmExtendFileLifeTime::toString(Process *proc)
{
  DM_DBG_I;
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