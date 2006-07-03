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
 * srmPurgeFromSpace request constuctor
 */
srmPurgeFromSpace::srmPurgeFromSpace()
{
  init();
}

/*
 * Initialise srmPurgeFromSpace request
 */
void
srmPurgeFromSpace::init()
{
  /* request (parser/API) */
  spaceToken = NULL;

  /* response (parser) */
  fileStatuses = NULL;
}

/*
 * srmPurgeFromSpace request copy constuctor
 */
srmPurgeFromSpace::srmPurgeFromSpace(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmPurgeFromSpace request destructor
 */
srmPurgeFromSpace::~srmPurgeFromSpace()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE_VEC(SURL);
  DELETE(spaceToken);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* response (parser) */
  DELETE(fileStatuses);

  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmPurgeFromSpace::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(PurgeFromSpace);
}

int
srmPurgeFromSpace::exec(Process *proc)
{
#define EVAL_VEC_STR_PFS(vec) vec = proc->eval_vec_str(srmPurgeFromSpace::vec)
  DM_DBG_I;

  std::vector <std::string *> SURL;
  tStorageSystemInfo storageSystemInfo;
  
  EVAL_VEC_STR_PFS(SURL);
  EVAL_VEC_STR_PFS(storageSystemInfo.key);
  EVAL_VEC_STR_PFS(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(PurgeFromSpace);

  PurgeFromSpace(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    SURL,
    EVAL2CSTR(spaceToken),
    storageSystemInfo,
    resp
  );
#endif

  DELETE_VEC(SURL);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* matching */
  if(!resp || !resp->srmPurgeFromSpaceResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* fileStatuses */
  EAT_MATCH(fileStatuses, arrayOfPurgeFromSpaceResponseToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmPurgeFromSpaceResponse->returnStatus, proc));

#undef EVAL_VEC_STR_PFS
}

std::string
srmPurgeFromSpace::toString(Process *proc)
{
#define EVAL_VEC_STR_PFS(vec) EVAL_VEC_STR(srmPurgeFromSpace,vec)
  DM_DBG_I;

  GET_SRM_RESP(PurgeFromSpace);
  BOOL quote = TRUE;
  std::stringstream ss;
  std::vector <std::string *> SURL;
  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_PFS(SURL);
  EVAL_VEC_STR_PFS(storageSystemInfo.key);
  EVAL_VEC_STR_PFS(storageSystemInfo.value);

  /* request */  
  SS_SRM("srmPurgeFromSpace");

  SS_P_DQ(authorizationID);
  SS_VEC_DEL(SURL);
  SS_P_DQ(spaceToken);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmPurgeFromSpaceResponse) RETURN(ss.str());

  /* fileStatuses */
  ss << arrayOfPurgeFromSpaceResponseToString(proc, TRUE, quote);
  
  SS_P_SRM_RETSTAT(resp->srmPurgeFromSpaceResponse);

  RETURN(ss.str());

#undef EVAL_VEC_STR_PFS
}

std::string
srmPurgeFromSpace::arrayOfPurgeFromSpaceResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(PurgeFromSpace);
  std::stringstream ss;

  if(!resp || !resp->srmPurgeFromSpaceResponse) RETURN(ss.str());

  if(resp->srmPurgeFromSpaceResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TSURLReturnStatus *> v = resp->srmPurgeFromSpaceResponse->arrayOfFileStatuses->statusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(surl);
      SS_P_VEC_SRM_RETSTAT(status);
    }
  }
  RETURN(ss.str());
}
