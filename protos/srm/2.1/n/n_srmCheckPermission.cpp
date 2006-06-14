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
  checkInLocalCacheOnly = NULL;

  /* response (parser) */
  permissions = NULL;

  /* response (API) */
  resp = new srm__srmCheckPermissionResponse_();
  if(resp == NULL) {
    DM_ERR(ERR_SYSTEM, "new failed\n");
  } else {
    memset(resp, 0, sizeof(srm__srmCheckPermissionResponse_));
  }
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
  DELETE_VEC(path.SURLOrStFN);
  DELETE_VEC(path.storageSystemInfo);
  DELETE(checkInLocalCacheOnly);

  /* response (parser) */
  DELETE(permissions);
  
  /* response (API) */
  DELETE(resp);

  DM_DBG_O;
}

int
srmCheckPermission::exec(Process *proc)
{
#define EVAL_VEC_STR_CHP(vec) vec = proc->eval_vec_str(srmCheckPermission::vec)
  DM_DBG_I;
  BOOL match = FALSE;

  tSurlInfoArray path;

  EVAL_VEC_STR_CHP(path.SURLOrStFN);
  EVAL_VEC_STR_CHP(path.storageSystemInfo);

#ifdef SRM2_CALL
  CheckPermission(
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    path,
    (bool *)proc->eval2pint32(checkInLocalCacheOnly).p,
    resp
  );
#endif

  DELETE_VEC(path.SURLOrStFN);
  DELETE_VEC(path.storageSystemInfo);

  /* matching */
  if(!resp || !resp->srmCheckPermissionResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfFileStatus */
  match = proc->e_match(permissions, arrayOfFileStatusToString(FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmCheckPermissionResponse->returnStatus, proc));
#undef EVAL_VEC_STR_CHP
}

std::string
srmCheckPermission::toString(Process *proc)
{
#define EVAL_VEC_STR_CHP(vec) EVAL_VEC_STR(srmCheckPermission,vec)
  DM_DBG_I;
  
  BOOL quote = TRUE;
  std::stringstream ss;

  tArrayOfPutFileRequests_ path;

  EVAL_VEC_STR_CHP(path.SURLOrStFN);
  EVAL_VEC_STR_CHP(path.storageSystemInfo);

  /* request */  
  SS_SRM("srmCheckPermission");
  SS_P_DQ(userID);
  SS_VEC_DEL(path.SURLOrStFN);
  SS_VEC_DEL(path.storageSystemInfo);
  SS_P_DQ(checkInLocalCacheOnly);

  /* response (parser) */
  SS_P_DQ(permissions);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmCheckPermissionResponse) RETURN(ss.str());

  ss << arrayOfFileStatusToString(TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmCheckPermissionResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_CHP
}

std::string
srmCheckPermission::arrayOfFileStatusToString(BOOL space, BOOL quote) const
{
  DM_DBG_I;

  std::stringstream ss;

  if(!resp || !resp->srmCheckPermissionResponse) RETURN(ss.str());

  if(resp->srmCheckPermissionResponse->arrayOfPermissions) {
    BOOL print_space = FALSE;

    std::vector<srm__TSURLPermissionReturn *> v = resp->srmCheckPermissionResponse->arrayOfPermissions->surlPermissionArray;
    for(uint i = 0; i < v.size(); i++) {
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_PAR_VAL(surl);
      SS_P_VEC_PAR_PERMISSIONMODE_PTR(userPermission);
    }
  }
  
  RETURN(ss.str());
}
