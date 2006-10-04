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
  directoryPath = NULL;

  /* response (parser) */
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
  DELETE(directoryPath);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* response (parser) */
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmMkdir::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(Mkdir);
}

int
srmMkdir::exec(Process *proc)
{
#define EVAL_VEC_STR_MK(vec) vec = proc->eval_vec_str(srmMkdir::vec)
  DM_DBG_I;

  tStorageSystemInfo storageSystemInfo;

  EVAL_VEC_STR_MK(storageSystemInfo.key);
  EVAL_VEC_STR_MK(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(Mkdir);

  Mkdir(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(directoryPath),
    storageSystemInfo,
    resp
  );
#endif

  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);
  
  /* matching */
  if(!resp || !resp->srmMkdirResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmMkdirResponse->returnStatus, proc));
#undef EVAL_VEC_STR_MK
}

std::string
srmMkdir::toString(Process *proc)
{
#define EVAL_VEC_STR_MK(vec) EVAL_VEC_STR(srmMkdir,vec)
  DM_DBG_I;

  GET_SRM_RESP(Mkdir);
  BOOL quote = TRUE;
  std::stringstream ss;

  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_MK(storageSystemInfo.key);
  EVAL_VEC_STR_MK(storageSystemInfo.value);

  /* request */  
  SS_SRM("srmMkdir");
  SS_P_DQ(authorizationID);
  SS_P_DQ(directoryPath);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmMkdirResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmMkdirResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_MK
}
