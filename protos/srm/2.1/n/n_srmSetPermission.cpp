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
  SURLOrStFN = NULL;
  storageSystemInfo = NULL;
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
  DELETE(SURLOrStFN);
  DELETE(storageSystemInfo);
  DELETE(permissionType);
  DELETE(ownerPermission);
  DELETE_VEC(userPermissionArray.mode);
  DELETE_VEC(userPermissionArray.ID);
  DELETE_VEC(groupPermissionArray.mode);
  DELETE_VEC(groupPermissionArray.ID);
  DELETE(otherPermission);

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
#define EVAL_VEC_PERMISSION_MODE_SP(vec) vec = eval_vec_permission_mode(srmSetPermission::vec, proc)

  DM_DBG_I;

  tPermissionArray userPermissionArray;
  tPermissionArray groupPermissionArray;
  
  EVAL_VEC_PERMISSION_MODE_SP(userPermissionArray.mode);
  EVAL_VEC_PERMISSION_MODE_SP(groupPermissionArray.mode);
  EVAL_VEC_STR_SP(userPermissionArray.ID);
  EVAL_VEC_STR_SP(groupPermissionArray.ID);

#ifdef SRM2_CALL
  NEW_SRM_RET(SetPermission);

  SetPermission(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(SURLOrStFN),
    EVAL2CSTR(storageSystemInfo),
    *getTPermissionType(EVAL2CSTR(permissionType)), /* getTPermissionType() never returns NULL */
    getTPermissionMode(EVAL2CSTR(ownerPermission)),
    userPermissionArray,
    groupPermissionArray,
    getTPermissionMode(EVAL2CSTR(otherPermission)),
    resp
  );
#endif

  DELETE_VEC(userPermissionArray.ID);
  DELETE_VEC(groupPermissionArray.ID);
  
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
  
  tPermissionArray_ userPermissionArray;
  tPermissionArray_ groupPermissionArray;
  
  EVAL_VEC_STR_SP(userPermissionArray.mode);
  EVAL_VEC_STR_SP(userPermissionArray.ID);
  EVAL_VEC_STR_SP(groupPermissionArray.mode);
  EVAL_VEC_STR_SP(groupPermissionArray.ID);

  /* request */  
  SS_SRM("srmSetPermission");
  SS_P_DQ(userID);
  SS_P_DQ(SURLOrStFN);
  SS_P_DQ(storageSystemInfo);
  SS_P_DQ(permissionType);
  SS_P_DQ(ownerPermission);
  SS_VEC_DEL(userPermissionArray.mode);
  SS_VEC_DEL(userPermissionArray.ID);
  SS_VEC_DEL(groupPermissionArray.mode);
  SS_VEC_DEL(groupPermissionArray.ID);
  SS_P_DQ(otherPermission);

  /* response (parser) */
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmSetPermissionResponse) RETURN(ss.str());

  SS_P_SRM_RETSTAT(resp->srmSetPermissionResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_SP
}
