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
 * srmChangeSpaceForFiles request constuctor
 */
srmChangeSpaceForFiles::srmChangeSpaceForFiles()
{
  init();
}

/*
 * Initialise srmChangeSpaceForFiles request
 */
void
srmChangeSpaceForFiles::init()
{
  /* request (parser/API) */
  spaceToken = NULL;

  /* response (parser) */
  requestToken = NULL;
  estimatedProcessingTime = NULL;
  fileStatuses = NULL;
}

/*
 * srmChangeSpaceForFiles request copy constuctor
 */
srmChangeSpaceForFiles::srmChangeSpaceForFiles(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmChangeSpaceForFiles request destructor
 */
srmChangeSpaceForFiles::~srmChangeSpaceForFiles()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(spaceToken);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* response (parser) */
  DELETE(requestToken);
  DELETE(estimatedProcessingTime);
  DELETE(fileStatuses);

  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmChangeSpaceForFiles::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(ChangeSpaceForFiles);
}

int
srmChangeSpaceForFiles::exec(Process *proc)
{
#define EVAL_VEC_STR_CSFF(vec) vec = proc->eval_vec_str(srmChangeSpaceForFiles::vec)
  DM_DBG_I;

  tStorageSystemInfo storageSystemInfo;
  std::vector <std::string *> urlArray;
  
  EVAL_VEC_STR_CSFF(storageSystemInfo.key);
  EVAL_VEC_STR_CSFF(storageSystemInfo.value);
  EVAL_VEC_STR_CSFF(urlArray);

#ifdef SRM2_CALL
  NEW_SRM_RET(ChangeSpaceForFiles);

  ChangeSpaceForFiles(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(spaceToken),
    storageSystemInfo,
    resp
  );
#endif

  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);
  DELETE_VEC(urlArray);

  /* matching */
  if(!resp || !resp->srmChangeSpaceForFilesResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* requestToken */
  EAT_MATCH_C(resp->srmChangeSpaceForFilesResponse->requestToken,
              requestToken,
              CSTR(resp->srmChangeSpaceForFilesResponse->requestToken));

  /* estimatedProcessingTime */
  EAT_MATCH_C(resp->srmChangeSpaceForFilesResponse->estimatedProcessingTime,
              estimatedProcessingTime,
              PI2CSTR(resp->srmChangeSpaceForFilesResponse->estimatedProcessingTime));

  /* fileStatuses */
  EAT_MATCH(fileStatuses, arrayOfChangeSpaceForFilesResponseToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmChangeSpaceForFilesResponse->returnStatus, proc));

#undef EVAL_VEC_STR_CSFF
}

std::string
srmChangeSpaceForFiles::toString(Process *proc)
{
#define EVAL_VEC_STR_CSFF(vec) EVAL_VEC_STR(srmChangeSpaceForFiles,vec)
  DM_DBG_I;

  GET_SRM_RESP(ChangeSpaceForFiles);
  BOOL quote = TRUE;
  std::stringstream ss;

  tStorageSystemInfo_ storageSystemInfo;
  std::vector <std::string *> urlArray;

  EVAL_VEC_STR_CSFF(storageSystemInfo.key);
  EVAL_VEC_STR_CSFF(storageSystemInfo.value);
  EVAL_VEC_STR_CSFF(urlArray);

  /* request */  
  SS_SRM("srmChangeSpaceForFiles");

  SS_P_DQ(authorizationID);
  SS_P_DQ(spaceToken);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);
  SS_VEC_DEL(urlArray);

  /* response (parser) */
  SS_P_DQ(requestToken);
  SS_P_DQ(estimatedProcessingTime);
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmChangeSpaceForFilesResponse) RETURN(ss.str());

  /* requestToken */
  SS_P_DQ_C(resp->srmChangeSpaceForFilesResponse->requestToken,
            requestToken,
            CSTR(resp->srmChangeSpaceForFilesResponse->requestToken));

  /* estimatedProcessingTime */
  SS_P_DQ_C(resp->srmChangeSpaceForFilesResponse->estimatedProcessingTime,
            estimatedProcessingTime,
            PI2CSTR(resp->srmChangeSpaceForFilesResponse->estimatedProcessingTime));

  /* fileStatuses */
  ss << arrayOfChangeSpaceForFilesResponseToString(proc, TRUE, quote);
  
  SS_P_SRM_RETSTAT(resp->srmChangeSpaceForFilesResponse);

  RETURN(ss.str());

#undef EVAL_VEC_STR_CSFF
}

std::string
srmChangeSpaceForFiles::arrayOfChangeSpaceForFilesResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(ChangeSpaceForFiles);
  std::stringstream ss;

  if(!resp || !resp->srmChangeSpaceForFilesResponse) RETURN(ss.str());

  if(resp->srmChangeSpaceForFilesResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TSURLReturnStatus *> v = resp->srmChangeSpaceForFilesResponse->arrayOfFileStatuses->statusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(surl);
      SS_P_VEC_SRM_RETSTAT(status);
    }
  }

  RETURN(ss.str());
}
