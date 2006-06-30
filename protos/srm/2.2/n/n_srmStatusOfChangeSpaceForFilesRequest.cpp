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
 * srmStatusOfChangeSpaceForFilesRequest request constuctor
 */
srmStatusOfChangeSpaceForFilesRequest::srmStatusOfChangeSpaceForFilesRequest()
{
  init();
}

/*
 * Initialise srmStatusOfChangeSpaceForFilesRequest request
 */
void
srmStatusOfChangeSpaceForFilesRequest::init()
{
  /* request (parser/API) */
  spaceToken = NULL;
  requestToken = NULL;

  /* response (parser) */
  estimatedProcessingTime = NULL;
  fileStatuses = NULL;
}

/*
 * srmStatusOfChangeSpaceForFilesRequest request copy constuctor
 */
srmStatusOfChangeSpaceForFilesRequest::srmStatusOfChangeSpaceForFilesRequest(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmStatusOfChangeSpaceForFilesRequest request destructor
 */
srmStatusOfChangeSpaceForFilesRequest::~srmStatusOfChangeSpaceForFilesRequest()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(spaceToken);
  DELETE(requestToken);

  /* response (parser) */
  DELETE(estimatedProcessingTime);
  DELETE(fileStatuses);

  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmStatusOfChangeSpaceForFilesRequest::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(StatusOfChangeSpaceForFilesRequest);
}

int
srmStatusOfChangeSpaceForFilesRequest::exec(Process *proc)
{
#define EVAL_VEC_STR_CSFF(vec) vec = proc->eval_vec_str(srmStatusOfChangeSpaceForFilesRequest::vec)
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RET(StatusOfChangeSpaceForFilesRequest);

  StatusOfChangeSpaceForFilesRequest(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(requestToken),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmStatusOfChangeSpaceForFilesRequestResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* estimatedProcessingTime */
  EAT_MATCH_C(resp->srmStatusOfChangeSpaceForFilesRequestResponse->estimatedProcessingTime,
              estimatedProcessingTime,
              PI2CSTR(resp->srmStatusOfChangeSpaceForFilesRequestResponse->estimatedProcessingTime));

  /* fileStatuses */
  EAT_MATCH(fileStatuses, arrayOfStatusOfChangeSpaceForFilesRequestResponseToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmStatusOfChangeSpaceForFilesRequestResponse->returnStatus, proc));

#undef EVAL_VEC_STR_CSFF
}

std::string
srmStatusOfChangeSpaceForFilesRequest::toString(Process *proc)
{
#define EVAL_VEC_STR_CSFF(vec) EVAL_VEC_STR(srmStatusOfChangeSpaceForFilesRequest,vec)
  DM_DBG_I;

  GET_SRM_RESP(StatusOfChangeSpaceForFilesRequest);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmStatusOfChangeSpaceForFilesRequest");

  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);

  /* response (parser) */
  SS_P_DQ(estimatedProcessingTime);
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmStatusOfChangeSpaceForFilesRequestResponse) RETURN(ss.str());

  /* estimatedProcessingTime */
  SS_P_DQ_C(resp->srmStatusOfChangeSpaceForFilesRequestResponse->estimatedProcessingTime,
            estimatedProcessingTime,
            PI2CSTR(resp->srmStatusOfChangeSpaceForFilesRequestResponse->estimatedProcessingTime));

  /* fileStatuses */
  ss << arrayOfStatusOfChangeSpaceForFilesRequestResponseToString(proc, TRUE, quote);
  
  SS_P_SRM_RETSTAT(resp->srmStatusOfChangeSpaceForFilesRequestResponse);

  RETURN(ss.str());

#undef EVAL_VEC_STR_CSFF
}

std::string
srmStatusOfChangeSpaceForFilesRequest::arrayOfStatusOfChangeSpaceForFilesRequestResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(StatusOfChangeSpaceForFilesRequest);
  std::stringstream ss;

  if(!resp || !resp->srmStatusOfChangeSpaceForFilesRequestResponse) RETURN(ss.str());

  if(resp->srmStatusOfChangeSpaceForFilesRequestResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TSURLReturnStatus *> v = resp->srmStatusOfChangeSpaceForFilesRequestResponse->arrayOfFileStatuses->statusArray;
    /* same code as srmChangeSpaceForFiles */
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(surl);
      SS_P_VEC_SRM_RETSTAT(status);
    }
  }

  RETURN(ss.str());
}
