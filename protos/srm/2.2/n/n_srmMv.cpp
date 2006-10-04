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
  fromSURL = NULL;
  toSURL = NULL;

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
  DELETE(fromSURL);
  DELETE(toSURL);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

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

  FREE_SRM_RET(Mv);
}

int
srmMv::exec(Process *proc)
{
#define EVAL_VEC_STR_MV(vec) vec = proc->eval_vec_str(srmMv::vec)
  DM_DBG_I;

  tStorageSystemInfo storageSystemInfo;

  EVAL_VEC_STR_MV(storageSystemInfo.key);
  EVAL_VEC_STR_MV(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(Mv);

  Mv(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(fromSURL),
    EVAL2CSTR(toSURL),
    storageSystemInfo,
    resp
  );
#endif

  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);
  
  /* matching */
  if(!resp || !resp->srmMvResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmMvResponse->returnStatus, proc));
#undef EVAL_VEC_STR_MV
}

std::string
srmMv::toString(Process *proc)
{
#define EVAL_VEC_STR_MV(vec) EVAL_VEC_STR(srmMv,vec)
  DM_DBG_I;

  GET_SRM_RESP(Mv);
  BOOL quote = TRUE;
  std::stringstream ss;

  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_MV(storageSystemInfo.key);
  EVAL_VEC_STR_MV(storageSystemInfo.value);

  /* request */  
  SS_SRM("srmMv");
  SS_P_DQ(authorizationID);
  SS_P_DQ(fromSURL);
  SS_P_DQ(toSURL);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmMvResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmMvResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_MV
}
