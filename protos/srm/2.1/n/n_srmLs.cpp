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
 * srmLs request constuctor
 */
srmLs::srmLs()
{
  init();
}

/*
 * Initialise srmLs request
 */
void
srmLs::init()
{
  /* request (parser/API) */
  fileStorageType = NULL;
  fullDetailedList = NULL;
  allLevelRecursive = NULL;
  numOfLevels = NULL;
  offset = NULL;
  count = NULL;

  /* response (parser) */
  pathDetails = NULL;
}

/*
 * srmLs request copy constuctor
 */
srmLs::srmLs(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmLs request destructor
 */
srmLs::~srmLs()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE_VEC(path.SURLOrStFN);
  DELETE_VEC(path.storageSystemInfo);
  DELETE(fileStorageType);
  DELETE(fullDetailedList);
  DELETE(allLevelRecursive);
  DELETE(numOfLevels);
  DELETE(offset);
  DELETE(count);

  /* response (parser) */
  DELETE(pathDetails);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmLs::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(Ls);
}

int
srmLs::exec(Process *proc)
{
#define EVAL_VEC_STR_LS(vec) vec = proc->eval_vec_str(srmLs::vec)
  DM_DBG_I;
  BOOL match = FALSE;

  tSurlInfoArray path;

  EVAL_VEC_STR_LS(path.SURLOrStFN);
  EVAL_VEC_STR_LS(path.storageSystemInfo);

#ifdef SRM2_CALL
  NEW_SRM_RET(Ls);

  Ls(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    path,
    getTFileStorageType(EVAL2CSTR(fileStorageType)),
    (bool *)proc->eval2pint32(fullDetailedList).p,
    (bool *)proc->eval2pint32(allLevelRecursive).p,
    proc->eval2pint32(numOfLevels).p,
    proc->eval2pint32(offset).p,
    proc->eval2pint32(count).p,
    resp
  );
#endif

  DELETE_VEC(path.SURLOrStFN);
  DELETE_VEC(path.storageSystemInfo);

  /* matching */
  if(!resp || !resp->srmLsResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfPathDetails */
  match = proc->e_match(pathDetails, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmLsResponse->returnStatus, proc));
#undef EVAL_VEC_STR_LS
}

std::string
srmLs::toString(Process *proc)
{
#define EVAL_VEC_STR_LS(vec) EVAL_VEC_STR(srmLs,vec)
  DM_DBG_I;
  
  GET_SRM_RESP(Ls);
  BOOL quote = TRUE;
  std::stringstream ss;

  tArrayOfPutFileRequests_ path;

  EVAL_VEC_STR_LS(path.SURLOrStFN);
  EVAL_VEC_STR_LS(path.storageSystemInfo);

  /* request */  
  SS_SRM("srmLs");
  SS_P_DQ(userID);
  SS_VEC_DEL(path.SURLOrStFN);
  SS_VEC_DEL(path.storageSystemInfo);
  SS_P_DQ(fileStorageType);
  SS_P_DQ(fullDetailedList);
  SS_P_DQ(allLevelRecursive);
  SS_P_DQ(numOfLevels);
  SS_P_DQ(offset);
  SS_P_DQ(count);

  /* response (parser) */
  SS_P_DQ(pathDetails);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmLsResponse) RETURN(ss.str());

  ss << arrayOfFileStatusToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmLsResponse);

  RETURN(ss.str());
#undef EVAL_VEC_STR_LS
}

std::string
srmLs::arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(Ls);
  std::stringstream ss;

  if(!resp || !resp->srmLsResponse) RETURN(ss.str());

  if(resp->srmLsResponse->details) {
    std::vector<srm__TMetaDataPathDetail *> v = resp->srmLsResponse->details->pathDetailArray;
    ss << arrayOfFileStatusToString(proc, space, quote, v);
  }
  
  RETURN(ss.str());
}

std::string
srmLs::arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote, std::vector<srm__TMetaDataPathDetail *> v) const
{
  DM_DBG_I;
  
  std::stringstream ss;
  
  BOOL print_space = space;
  for(uint u = 0; u < v.size(); u++) {
    SS_P_VEC_PAR(path);
    SS_P_VEC_SRM_RETSTAT(status);
    SS_P_VEC_PAR_VAL(size);
    
    SS_P_VEC_PAR_PERMISSIONMODE(ownerPermission);
    if(v[u]->userPermissions) {
      std::vector<srm__TUserPermission*> perms = v[u]->userPermissions->userPermissionArray;
      for(uint j = 0; j < perms.size(); j++) {
        SS_VEC_SPACE;
        ss << "user[" << j << "]=" << perms[j]->userID->value << " perm[" << j << "]=" << perms[j]->mode;
      }
    }
    if(v[u]->groupPermissions) {
      std::vector<srm__TGroupPermission*> perms = v[u]->groupPermissions->groupPermissionArray;
      for(uint j = 0; j < perms.size(); j++) {
        SS_VEC_SPACE;
        ss << "group[" << j << "]=" << perms[j]->groupID->value << " perm[" << j << "]=" << perms[j]->mode;
      }
    }
    SS_P_VEC_PAR_PERMISSIONMODE(otherPermission);

    SS_P_VEC_PAR_VAL(createdAtTime);
    SS_P_VEC_PAR_VAL(lastModificationTime);
    SS_P_VEC_PAR_VAL(owner);
    if(v[u]->fileStorageType) {
      SS_VEC_SPACE;
      ss << "fileStorageType=" << getTFileStorageType(*v[u]->fileStorageType);
    }
    if(v[u]->type) {
      SS_VEC_SPACE;
      ss << "type=" << *(v[u]->type);
    }
    SS_P_VEC_PAR_VAL(lifetimeAssigned);
    SS_P_VEC_PAR_VAL(lifetimeLeft);
    SS_P_VEC_PAR_VAL(checkSumType);
    SS_P_VEC_PAR_VAL(checkSumValue);
    SS_P_VEC_PAR_VAL(originalSURL);
    
    if(v[u]->subPaths) {
      std::vector<srm__TMetaDataPathDetail *> sub_v = v[u]->subPaths->pathDetailArray;
      ss << arrayOfFileStatusToString(proc, print_space, quote, sub_v);
    }
  }
  
  RETURN(ss.str());
}
