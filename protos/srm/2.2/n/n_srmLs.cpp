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
  requestToken = NULL;
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
  DELETE_VEC(SURL);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);
  DELETE(fileStorageType);
  DELETE(fullDetailedList);
  DELETE(allLevelRecursive);
  DELETE(numOfLevels);
  DELETE(offset);
  DELETE(count);

  /* response (parser) */
  DELETE(pathDetails);
  DELETE(requestToken);
  
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

  std::vector <std::string *> SURL;
  tStorageSystemInfo storageSystemInfo;

  EVAL_VEC_STR_LS(SURL);
  EVAL_VEC_STR_LS(storageSystemInfo.key);
  EVAL_VEC_STR_LS(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(Ls);

  pint_t* fullDetailedListInt = proc->eval2pint(fullDetailedList);
  pint_t* allLevelRecursiveInt = proc->eval2pint(allLevelRecursive);
  pint_t* numOfLevelsInt = proc->eval2pint(numOfLevels);
  pint_t* offsetInt = proc->eval2pint(offset);
  pint_t* countInt = proc->eval2pint(count);

  Ls(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    SURL,
    storageSystemInfo,
    getTFileStorageType(EVAL2CSTR(fileStorageType)),
    (bool *)fullDetailedListInt->p,
    (bool *)allLevelRecursiveInt->p,
    numOfLevelsInt->p,
    offsetInt->p,
    countInt->p,
    resp
  );

  free(fullDetailedListInt);
  free(allLevelRecursiveInt);
  free(numOfLevelsInt);
  free(offsetInt);
  free(countInt);
#endif

  DELETE_VEC(SURL);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* matching */
  if(!resp || !resp->srmLsResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* requestToken */
  EAT_MATCH_C(resp->srmLsResponse->requestToken,
              requestToken,
              CSTR(resp->srmLsResponse->requestToken));

  /* arrayOfPathDetails */
  EAT_MATCH(pathDetails, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());

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

  std::vector <std::string *> SURL;
  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_LS(SURL);
  EVAL_VEC_STR_LS(storageSystemInfo.key);
  EVAL_VEC_STR_LS(storageSystemInfo.value);

  /* request */  
  SS_SRM("srmLs");
  SS_P_DQ(authorizationID);
  SS_VEC_DEL(SURL);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);
  SS_P_DQ(fileStorageType);
  SS_P_DQ(fullDetailedList);
  SS_P_DQ(allLevelRecursive);
  SS_P_DQ(numOfLevels);
  SS_P_DQ(offset);
  SS_P_DQ(count);

  /* response (parser) */
  SS_P_DQ(requestToken);
  SS_P_DQ(pathDetails);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmLsResponse) RETURN(ss.str());

  /* requestToken */
  SS_P_DQ_C(resp->srmLsResponse->requestToken,
            requestToken,
            CSTR(resp->srmLsResponse->requestToken));

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
  
  /* the response is exactly the same as in srmStatusOfLsRequest */
  BOOL print_space = space;
  for(uint u = 0; u < v.size(); u++) {
    SS_P_VEC_PAR(path);
    SS_P_VEC_SRM_RETSTAT(status);
    SS_P_VEC_DPAR(size);
    
    SS_P_VEC_DPAR(createdAtTime);
    SS_P_VEC_DPAR(lastModificationTime);
    SS_P_VEC_DPAR_SOAP(FileStorageType,fileStorageType);
    SS_P_VEC_SRM_RETENTION_POLICY(retentionPolicyInfo);
    SS_P_VEC_DPAR_SOAP(FileLocality,fileLocality);

    if(v[u] && v[u]->arrayOfSpaceTokens) {
      for(uint j = 0; j < v[u]->arrayOfSpaceTokens->stringArray.size(); j++) {
        SS_VEC_SPACE; ss << "spaceToken" << u << ":" << j << "=" << v[u]->arrayOfSpaceTokens->stringArray[j];
      }
    }

    SS_P_VEC_DPAR_SOAP(FileType,type);
    
    SS_P_VEC_DPAR(lifetimeAssigned);
    SS_P_VEC_DPAR(lifetimeLeft);
    if(v[u] && v[u]->ownerPermission) {
      SS_VEC_SPACE; ss << "userID" << u << "=" << v[u]->ownerPermission->userID;
      SS_VEC_SPACE; ss << "mode"   << u << "=" << getTPermissionMode(v[u]->ownerPermission->mode);
    }
    if(v[u] && v[u]->groupPermission) {
      SS_VEC_SPACE; ss << "groupID" << u << "=" << v[u]->groupPermission->groupID;
      SS_VEC_SPACE; ss << "mode"   << u << "=" << getTPermissionMode(v[u]->groupPermission->mode);
    }
    SS_P_VEC_DPAR_SOAP(PermissionMode,otherPermission);
    
    SS_P_VEC_DPAR(checkSumType);
    SS_P_VEC_DPAR(checkSumValue);

    if(v[u]->arrayOfSubPaths) {
      std::vector<srm__TMetaDataPathDetail *> sub_v = v[u]->arrayOfSubPaths->pathDetailArray;
      ss << arrayOfFileStatusToString(proc, print_space, quote, sub_v);
    }
  }
  
  RETURN(ss.str());
}
