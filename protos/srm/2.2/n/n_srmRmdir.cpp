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
  SURL = NULL;
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
  DELETE(SURL);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);
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

  FREE_SRM_RET(Rmdir);
}

int
srmRmdir::exec(Process *proc)
{
#define EVAL_VEC_STR_MK(vec) vec = proc->eval_vec_str(srmRmdir::vec)
  DM_DBG_I;

  tStorageSystemInfo storageSystemInfo;

  EVAL_VEC_STR_MK(storageSystemInfo.key);
  EVAL_VEC_STR_MK(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(Rmdir);

  Rmdir(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(SURL),
    storageSystemInfo,
    (bool *)proc->eval2pint(recursive).p,
    resp
  );
#endif

  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);
  
  /* matching */
  if(!resp || !resp->srmRmdirResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmRmdirResponse->returnStatus, proc));
#undef EVAL_VEC_STR_MK
}

std::string
srmRmdir::toString(Process *proc)
{
#define EVAL_VEC_STR_MK(vec) EVAL_VEC_STR(srmRmdir,vec)
  DM_DBG_I;

  GET_SRM_RESP(Rmdir);
  BOOL quote = TRUE;
  std::stringstream ss;

  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_MK(storageSystemInfo.key);
  EVAL_VEC_STR_MK(storageSystemInfo.value);

  /* request */  
  SS_SRM("srmRmdir");
  SS_P_DQ(authorizationID);
  SS_P_DQ(SURL);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);
  SS_P_DQ(recursive);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmRmdirResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmRmdirResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_MK
}
