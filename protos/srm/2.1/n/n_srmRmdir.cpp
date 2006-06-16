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
 * srmRmdir request constuctor
 */
srmRmdir::srmRmdir()
{
  init();
}

/*
 * Initialise srmRmdir request
 */
void
srmRmdir::init()
{
  /* request (parser/API) */
  SURLOrStFN = NULL;
  storageSystemInfo = NULL;
  recursive = NULL;

  /* response (parser) */
}

/*
 * srmRmdir request copy constuctor
 */
srmRmdir::srmRmdir(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmRmdir request destructor
 */
srmRmdir::~srmRmdir()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(SURLOrStFN);
  DELETE(storageSystemInfo);
  DELETE(recursive);

  /* response (parser) */
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmRmdir::finish(Process *proc)
{
  DM_DBG_I;
  srm__srmRmdirResponse_ *resp = (srm__srmRmdirResponse_ *)proc->resp;
  
  DELETE(resp);
}

int
srmRmdir::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RESP(Rmdir);

  Rmdir(
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(SURLOrStFN),
    EVAL2CSTR(storageSystemInfo),
    (bool *)proc->eval2pint32(recursive).p,
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmRmdirResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmRmdirResponse->returnStatus, proc));
}

std::string
srmRmdir::toString(Process *proc)
{
  DM_DBG_I;

  srm__srmRmdirResponse_ *resp = proc? (srm__srmRmdirResponse_ *)proc->resp : NULL;
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmRmdir");
  SS_P_DQ(userID);
  SS_P_DQ(SURLOrStFN);
  SS_P_DQ(storageSystemInfo);
  SS_P_DQ(recursive);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmRmdirResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmRmdirResponse);

  RETURN(ss.str());
}
