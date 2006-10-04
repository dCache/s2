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
 * srmCheckPermission request constuctor
 */
srmCheckPermission::srmCheckPermission()
{
  init();
}

/*
 * Initialise srmCheckPermission request
 */
void
srmCheckPermission::init()
{
  /* request (parser/API) */

  /* response (parser) */
  permissionArray = NULL;
}

/*
 * srmCheckPermission request copy constuctor
 */
srmCheckPermission::srmCheckPermission(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmCheckPermission request destructor
 */
srmCheckPermission::~srmCheckPermission()
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
srmCheckPermission::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(CheckPermission);
}

int
srmCheckPermission::exec(Process *proc)
{
#define EVAL_VEC_STR_GP(vec) vec = proc->eval_vec_str(srmCheckPermission::vec)
  DM_DBG_I;

  std::vector <std::string *> SURL;
  tStorageSystemInfo storageSystemInfo;

  EVAL_VEC_STR_GP(SURL);
  EVAL_VEC_STR_GP(storageSystemInfo.key);
  EVAL_VEC_STR_GP(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(CheckPermission);

  CheckPermission(
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
  if(!resp || !resp->srmCheckPermissionResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfPathDetails */
  EAT_MATCH(permissionArray, arrayOfFilePermissionToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmCheckPermissionResponse->returnStatus, proc));
#undef EVAL_VEC_STR_GP
}

std::string
srmCheckPermission::toString(Process *proc)
{
#define EVAL_VEC_STR_GP(vec) EVAL_VEC_STR(srmCheckPermission,vec)
  DM_DBG_I;
  
  GET_SRM_RESP(CheckPermission);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> SURL;
  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_GP(SURL);
  EVAL_VEC_STR_GP(storageSystemInfo.key);
  EVAL_VEC_STR_GP(storageSystemInfo.value);

  /* request */  
  SS_SRM("srmCheckPermission");
  SS_P_DQ(authorizationID);
  SS_VEC_DEL(SURL);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);

  /* response (parser) */
  SS_P_DQ(permissionArray);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmCheckPermissionResponse) RETURN(ss.str());

  ss << arrayOfFilePermissionToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmCheckPermissionResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_GP
}

std::string
srmCheckPermission::arrayOfFilePermissionToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(CheckPermission);
  std::stringstream ss;

  if(!resp || !resp->srmCheckPermissionResponse) RETURN(ss.str());

  if(resp->srmCheckPermissionResponse->arrayOfPermissions) {
    std::vector<srm__TSURLPermissionReturn *> v = resp->srmCheckPermissionResponse->arrayOfPermissions->surlPermissionArray;
    BOOL print_space = FALSE;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(surl);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_DPAR_SOAP(PermissionMode,permission);
    }
  }
  
  RETURN(ss.str());
}
