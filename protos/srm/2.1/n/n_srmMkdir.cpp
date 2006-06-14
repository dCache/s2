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
 * srmMkdir request constuctor
 */
srmMkdir::srmMkdir()
{
  init();
}

/*
 * Initialise srmMkdir request
 */
void
srmMkdir::init()
{
  /* request (parser/API) */
  SURLOrStFN = NULL;
  storageSystemInfo = NULL;

  /* response (parser) */

  /* response (API) */
  resp = new srm__srmMkdirResponse_();
  if(resp == NULL) {
    DM_ERR(ERR_SYSTEM, "new failed\n");
  } else {
    memset(resp, 0, sizeof(srm__srmMkdirResponse_));
  }
}

/*
 * srmMkdir request copy constuctor
 */
srmMkdir::srmMkdir(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmMkdir request destructor
 */
srmMkdir::~srmMkdir()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(SURLOrStFN);
  DELETE(storageSystemInfo);

  /* response (parser) */
  
  /* response (API) */
  DELETE(resp);

  DM_DBG_O;
}

int
srmMkdir::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  Mkdir(
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(SURLOrStFN),
    EVAL2CSTR(storageSystemInfo),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmMkdirResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmMkdirResponse->returnStatus, proc));
}

std::string
srmMkdir::toString(Process *proc)
{
  DM_DBG_I;
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmMkdir");
  SS_P_DQ(userID);
  SS_P_DQ(SURLOrStFN);
  SS_P_DQ(storageSystemInfo);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmMkdirResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmMkdirResponse);

  RETURN(ss.str());
}
