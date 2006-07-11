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
 * srmStatusOfLsRequest request constuctor
 */
srmStatusOfLsRequest::srmStatusOfLsRequest()
{
  init();
}

/*
 * Initialise srmStatusOfLsRequest request
 */
void
srmStatusOfLsRequest::init()
{
  /* request (parser/API) */
  requestToken = NULL;
  offset = NULL;
  count = NULL;

  /* response (parser) */
  pathDetails = NULL;
}

/*
 * srmStatusOfLsRequest request Ls constuctor
 */
srmStatusOfLsRequest::srmStatusOfLsRequest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmStatusOfLsRequest request destructor
 */
srmStatusOfLsRequest::~srmStatusOfLsRequest()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);
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
srmStatusOfLsRequest::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(StatusOfLsRequest);
}

int
srmStatusOfLsRequest::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RET(StatusOfLsRequest);

  StatusOfLsRequest(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(requestToken),
    proc->eval2pint(offset).p,
    proc->eval2pint(count).p,
    resp
  );
#endif
  
  /* matching */
  if(!resp || !resp->srmStatusOfLsRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfPathDetails */
  EAT_MATCH(pathDetails, arrayOfFileStatusToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmStatusOfLsRequestResponse->returnStatus, proc));
}

std::string
srmStatusOfLsRequest::toString(Process *proc)
{
  DM_DBG_I;
  
  GET_SRM_RESP(StatusOfLsRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmStatusOfLsRequest");
  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);
  SS_P_DQ(offset);
  SS_P_DQ(count);

  /* response (parser) */
  SS_P_DQ(pathDetails);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmStatusOfLsRequestResponse) RETURN(ss.str());

  ss << arrayOfFileStatusToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmStatusOfLsRequestResponse);

  RETURN(ss.str());
}

std::string
srmStatusOfLsRequest::arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfLsRequest);
  std::stringstream ss;

  if(!resp || !resp->srmStatusOfLsRequestResponse) RETURN(ss.str());

  if(resp->srmStatusOfLsRequestResponse->details) {
    std::vector<srm__TMetaDataPathDetail *> v = resp->srmStatusOfLsRequestResponse->details->pathDetailArray;
    ss << arrayOfFileStatusToString(proc, space, quote, v);
  }
  
  RETURN(ss.str());
}

std::string
srmStatusOfLsRequest::arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote, std::vector<srm__TMetaDataPathDetail *> v) const
{
  DM_DBG_I;
  
  std::stringstream ss;
  
  /* the response is exactly the same as srmLs */
  BOOL print_space = space;
  for(uint u = 0; u < v.size(); u++) {
    SS_P_VEC_PAR(surl);
    SS_P_VEC_SRM_RETSTAT(status);
    SS_P_VEC_DPAR(size);
    
    SS_P_VEC_DPAR(createdAtTime);
    SS_P_VEC_DPAR(lastModificationTime);
    SS_P_VEC_DPAR_SOAP(FileStorageType,fileStorageType);
    SS_P_VEC_SRM_RETENTION_POLICY(retentionPolicyInfo);
    SS_P_VEC_DPAR_SOAP(FileLocality,fileLocality);

    if(v[u] && v[u]->arrayOfSpaceTokens) {
      for(uint j = 0; j < v[u]->arrayOfSpaceTokens->stringArray.size(); j++) {
        SS_VEC_SPACE; 
        ss << "spaceToken" << u << ":" << j << "=" << v[u]->arrayOfSpaceTokens->stringArray[j];
      }
    }

    SS_P_VEC_DPAR_SOAP(FileType,type);
    
    SS_P_VEC_DPAR(lifetimeAssigned);
    SS_P_VEC_DPAR(lifetimeLeft);
    if(v[u] && v[u]->ownerPermission) {
      SS_VEC_SPACE; 
      ss << "userID" << u << "=" << v[u]->ownerPermission->userID;
      ss << "mode"   << u << "=" << getTPermissionMode(v[u]->ownerPermission->mode);
    }
    if(v[u] && v[u]->groupPermission) {
      SS_VEC_SPACE; 
      ss << "groupID" << u << "=" << v[u]->groupPermission->groupID;
      ss << "mode"   << u << "=" << getTPermissionMode(v[u]->groupPermission->mode);
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
