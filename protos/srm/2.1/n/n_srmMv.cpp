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
 * srmMv request constuctor
 */
srmMv::srmMv()
{
  init();
}

/*
 * Initialise srmMv request
 */
void
srmMv::init()
{
  /* request (parser/API) */
  fromSURLOrStFN = NULL;
  fromStorageSystemInfo = NULL;
  toSURLOrStFN = NULL;
  toStorageSystemInfo = NULL;

  /* response (parser) */
}

/*
 * srmMv request copy constuctor
 */
srmMv::srmMv(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmMv request destructor
 */
srmMv::~srmMv()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(fromSURLOrStFN);
  DELETE(fromStorageSystemInfo);
  DELETE(toSURLOrStFN);
  DELETE(toStorageSystemInfo);

  /* response (parser) */
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmMv::finish(Process *proc)
{
  DM_DBG_I;
  srm__srmMvResponse_ *resp = (srm__srmMvResponse_ *)proc->resp;
  
  DELETE(resp);
}

int
srmMv::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RESP(Mv);

  Mv(
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(fromSURLOrStFN),
    EVAL2CSTR(fromStorageSystemInfo),
    EVAL2CSTR(toSURLOrStFN),
    EVAL2CSTR(toStorageSystemInfo),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmMvResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmMvResponse->returnStatus, proc));
}

std::string
srmMv::toString(Process *proc)
{
  DM_DBG_I;

  srm__srmMvResponse_ *resp = proc? (srm__srmMvResponse_ *)proc->resp : NULL;
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmMv");
  SS_P_DQ(userID);
  SS_P_DQ(fromSURLOrStFN);
  SS_P_DQ(fromStorageSystemInfo);
  SS_P_DQ(toSURLOrStFN);
  SS_P_DQ(toStorageSystemInfo);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmMvResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmMvResponse);

  RETURN(ss.str());
}
