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
 * srmSetPermission request constuctor
 */
srmSetPermission::srmSetPermission()
{
  init();
}

/*
 * Initialise srmSetPermission request
 */
void
srmSetPermission::init()
{
  /* request (parser/API) */
  SURL = NULL;
  permissionType = NULL;
  ownerPermission = NULL;
  otherPermission = NULL;

  /* response (parser) */
}

/*
 * srmSetPermission request copy constuctor
 */
srmSetPermission::srmSetPermission(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmSetPermission request destructor
 */
srmSetPermission::~srmSetPermission()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(SURL);
  DELETE(permissionType);
  DELETE(ownerPermission);
  DELETE_VEC(userPermission.ID);
  DELETE_VEC(userPermission.mode);
  DELETE_VEC(groupPermission.ID);
  DELETE_VEC(groupPermission.mode);
  DELETE(otherPermission);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* response (parser) */
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmSetPermission::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(SetPermission);
}

int
srmSetPermission::exec(Process *proc)
{
#define EVAL_VEC_STR_SP(vec) vec = proc->eval_vec_str(srmSetPermission::vec)
#define EVAL_VEC_PERMISSION_MODE_SP(vec) vec = eval_vec_permission_mode(srmSetPermission::vec,proc)

  DM_DBG_I;

  tPermissionArray userPermission;
  tPermissionArray groupPermission;
  tStorageSystemInfo storageSystemInfo;

  EVAL_VEC_STR_SP(userPermission.ID);
  EVAL_VEC_STR_SP(groupPermission.ID);
  EVAL_VEC_PERMISSION_MODE_SP(userPermission.mode);
  EVAL_VEC_PERMISSION_MODE_SP(groupPermission.mode);
  EVAL_VEC_STR_SP(storageSystemInfo.key);
  EVAL_VEC_STR_SP(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(SetPermission);

  SetPermission(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(SURL),
    getTPermissionType(EVAL2CSTR(permissionType),TRUE),		/* one-parameter getT* returns pointer to NULL in 2.2 */
    getTPermissionMode(EVAL2CSTR(ownerPermission)),
    userPermission,
    groupPermission,
    getTPermissionMode(EVAL2CSTR(otherPermission)),
    storageSystemInfo,
    resp
  );
#endif

  DELETE_VEC(userPermission.ID);
  DELETE_VEC(groupPermission.ID);
  
  /* matching */
  if(!resp || !resp->srmSetPermissionResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmSetPermissionResponse->returnStatus, proc));

#undef EVAL_VEC_STR_SP
#undef EVAL_VEC_PERMISSION_MODE_SP
}

std::string
srmSetPermission::toString(Process *proc)
{
#define EVAL_VEC_STR_SP(vec) EVAL_VEC_STR(srmSetPermission,vec)
  DM_DBG_I;

  GET_SRM_RESP(SetPermission);
  BOOL quote = TRUE;
  std::stringstream ss;
  
  tPermissionArray_ userPermission;
  tPermissionArray_ groupPermission;
  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_SP(userPermission.ID);
  EVAL_VEC_STR_SP(groupPermission.ID);
  EVAL_VEC_STR_SP(userPermission.mode);
  EVAL_VEC_STR_SP(groupPermission.mode);
  EVAL_VEC_STR_SP(storageSystemInfo.key);
  EVAL_VEC_STR_SP(storageSystemInfo.value);

  /* request */  
  SS_SRM("srmSetPermission");
  SS_P_DQ(authorizationID);
  SS_P_DQ(SURL);
  SS_P_DQ(permissionType);
  SS_P_DQ(ownerPermission);
  SS_VEC_DEL(userPermission.ID);
  SS_VEC_DEL(userPermission.mode);
  SS_VEC_DEL(groupPermission.ID);
  SS_VEC_DEL(groupPermission.mode);
  SS_P_DQ(otherPermission);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmSetPermissionResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmSetPermissionResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_SP
}
