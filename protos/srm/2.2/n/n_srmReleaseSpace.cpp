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
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);
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

  FREE_SRM_RET(ReleaseSpace);
}

int
srmReleaseSpace::exec(Process *proc)
{
#define EVAL_VEC_STR_RS(vec) vec = proc->eval_vec_str(srmReleaseSpace::vec)
  DM_DBG_I;

  tStorageSystemInfo storageSystemInfo;
  
  EVAL_VEC_STR_RS(storageSystemInfo.key);
  EVAL_VEC_STR_RS(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(ReleaseSpace);

  ReleaseSpace(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(spaceToken),
    storageSystemInfo,
    (bool *)proc->eval2pint32(forceFileRelease).p,
    resp
  );
#endif

  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* matching */
  if(!resp || !resp->srmReleaseSpaceResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmReleaseSpaceResponse->returnStatus, proc));

#undef EVAL_VEC_STR_RS
}

std::string
srmReleaseSpace::toString(Process *proc)
{
#define EVAL_VEC_STR_RS(vec) EVAL_VEC_STR(srmBringOnline,vec)
  DM_DBG_I;

  GET_SRM_RESP(ReleaseSpace);
  BOOL quote = TRUE;
  std::stringstream ss;
  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_RS(storageSystemInfo.key);
  EVAL_VEC_STR_RS(storageSystemInfo.value);

  /* request */  
  SS_SRM("srmReleaseSpace");

  SS_P_DQ(authorizationID);
  SS_P_DQ(spaceToken);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);
  SS_P_DQ(forceFileRelease);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmReleaseSpaceResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmReleaseSpaceResponse);

  RETURN(ss.str());

#undef EVAL_VEC_STR_RS
}
