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
 * srmGetPermission request constuctor
 */
srmGetPermission::srmGetPermission()
{
  init();
}

/*
 * Initialise srmGetPermission request
 */
void
srmGetPermission::init()
{
  /* request (parser/API) */

  /* response (parser) */
  permissionArray = NULL;
}

/*
 * srmGetPermission request copy constuctor
 */
srmGetPermission::srmGetPermission(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmGetPermission request destructor
 */
srmGetPermission::~srmGetPermission()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE_VEC(SURL);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* response (parser) */
  DELETE(permissionArray);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmGetPermission::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(GetPermission);
}

int
srmGetPermission::exec(Process *proc)
{
#define EVAL_VEC_STR_GP(vec) vec = proc->eval_vec_str(srmGetPermission::vec)
  DM_DBG_I;

  std::vector <std::string *> SURL;
  tStorageSystemInfo storageSystemInfo;

  EVAL_VEC_STR_GP(SURL);
  EVAL_VEC_STR_GP(storageSystemInfo.key);
  EVAL_VEC_STR_GP(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(GetPermission);

  GetPermission(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    SURL,
    storageSystemInfo,
    resp
  );
#endif

  DELETE_VEC(SURL);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* matching */
  if(!resp || !resp->srmGetPermissionResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfPathDetails */
  EAT_MATCH(permissionArray, arrayOfFilePermissionToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmGetPermissionResponse->returnStatus, proc));
#undef EVAL_VEC_STR_GP
}

std::string
srmGetPermission::toString(Process *proc)
{
#define EVAL_VEC_STR_GP(vec) EVAL_VEC_STR(srmGetPermission,vec)
  DM_DBG_I;
  
  GET_SRM_RESP(GetPermission);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> SURL;
  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_GP(SURL);
  EVAL_VEC_STR_GP(storageSystemInfo.key);
  EVAL_VEC_STR_GP(storageSystemInfo.value);

  /* request */  
  SS_SRM("srmGetPermission");
  SS_P_DQ(authorizationID);
  SS_VEC_DEL(SURL);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);

  /* response (parser) */
  SS_P_DQ(permissionArray);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmGetPermissionResponse) RETURN(ss.str());

  ss << arrayOfFilePermissionToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmGetPermissionResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_GP
}

std::string
srmGetPermission::arrayOfFilePermissionToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(GetPermission);
  std::stringstream ss;

  if(!resp || !resp->srmGetPermissionResponse) RETURN(ss.str());

  if(resp->srmGetPermissionResponse->arrayOfPermissionReturns) {
    std::vector<srm__TPermissionReturn *> v = resp->srmGetPermissionResponse->arrayOfPermissionReturns->permissionArray;
    BOOL print_space = FALSE;
    DM_DBG(DM_N(3), "v.size() = %u\n", v.size());
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(surl);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_DPAR(owner);
      SS_P_VEC_DPAR_SOAP(PermissionMode,ownerPermission);

      DM_DBG(DM_N(3), "v[u]->arrayOfUserPermissions=%p\n", v[u]->arrayOfUserPermissions);
      if(v[u] && v[u]->arrayOfUserPermissions) {
        for(uint j = 0; j < v[u]->arrayOfUserPermissions->userPermissionArray.size(); j++) {
          if(!v[u]->arrayOfUserPermissions->userPermissionArray[j]) continue;
          SS_VEC_SPACE; ss << "userID" << u << ":" << j << "=" << v[u]->arrayOfUserPermissions->userPermissionArray[j]->userID;
          SS_VEC_SPACE; ss << "mode"   << u << ":" << j << "=" << getTPermissionMode(v[u]->arrayOfUserPermissions->userPermissionArray[j]->mode);
        }
      }
      DM_DBG(DM_N(3), "v[u]->arrayOfGroupPermissions=%p\n", v[u]->arrayOfGroupPermissions);
      DM_DBG(DM_N(3), "v[u]->arrayOfGroupPermissions->groupPermissionArray.size()=%u\n", v[u]->arrayOfGroupPermissions->groupPermissionArray.size());

      if(v[u] && v[u]->arrayOfGroupPermissions) {
        DM_DBG(DM_N(3), "v[u]->arrayOfGroupPermissions->groupPermissionArray.size()=%u\n", v[u]->arrayOfGroupPermissions->groupPermissionArray.size());
        for(uint j = 0; j < v[u]->arrayOfGroupPermissions->groupPermissionArray.size(); j++) {
          if(!(v[u]->arrayOfGroupPermissions->groupPermissionArray[j])) continue;
          DM_DBG(DM_N(3), "v[u]->arrayOfGroupPermissions->groupPermissionArray[j]=%p\n", (v[u]->arrayOfGroupPermissions->groupPermissionArray[j]));
          SS_VEC_SPACE; ss << "groupID" << u << ":" << j << "=" << v[u]->arrayOfGroupPermissions->groupPermissionArray[j]->groupID;
          SS_VEC_SPACE; ss << "mode"   << u << ":" << j << "=" << getTPermissionMode(v[u]->arrayOfGroupPermissions->groupPermissionArray[j]->mode);
        }
      }

      SS_P_VEC_DPAR_SOAP(PermissionMode,otherPermission);
    }
  }
  
  RETURN(ss.str());
}
