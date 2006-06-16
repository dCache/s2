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
 * srmReleaseSpace request constuctor
 */
srmReleaseSpace::srmReleaseSpace()
{
  init();
}

/*
 * Initialise srmReleaseSpace request
 */
void
srmReleaseSpace::init()
{
  /* request (parser/API) */
  spaceToken = NULL;
  storageSystemInfo = NULL;
  forceFileRelease = NULL;

  /* response (parser) */
}

/*
 * srmReleaseSpace request copy constuctor
 */
srmReleaseSpace::srmReleaseSpace(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmReleaseSpace request destructor
 */
srmReleaseSpace::~srmReleaseSpace()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(spaceToken);
  DELETE(storageSystemInfo);
  DELETE(forceFileRelease);

  /* response (parser) */

  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmReleaseSpace::finish(Process *proc)
{
  DM_DBG_I;
  srm__srmReleaseSpaceResponse_ *resp = (srm__srmReleaseSpaceResponse_ *)proc->resp;
  
  DELETE(resp);
}

int
srmReleaseSpace::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RESP(ReleaseSpace);

  ReleaseSpace(
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(spaceToken),
    EVAL2CSTR(storageSystemInfo),
    (bool *)proc->eval2pint32(forceFileRelease).p,
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmReleaseSpaceResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmReleaseSpaceResponse->returnStatus, proc));
}

std::string
srmReleaseSpace::toString(Process *proc)
{
  DM_DBG_I;

  srm__srmReleaseSpaceResponse_ *resp = proc? (srm__srmReleaseSpaceResponse_ *)proc->resp : NULL;
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmReleaseSpace");

  SS_P_DQ(userID);
  SS_P_DQ(spaceToken);
  SS_P_DQ(storageSystemInfo);
  SS_P_DQ(forceFileRelease);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmReleaseSpaceResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmReleaseSpaceResponse);

  RETURN(ss.str());
}
