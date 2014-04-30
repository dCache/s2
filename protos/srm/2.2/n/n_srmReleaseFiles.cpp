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
 * srmReleaseFiles request constuctor
 */
srmReleaseFiles::srmReleaseFiles()
{
  init();
}

/*
 * Initialise srmReleaseFiles request
 */
void
srmReleaseFiles::init()
{
  /* request (parser/API) */
  requestToken = NULL;
  doRemove = NULL;

  /* response (parser) */
  fileStatuses = NULL;
}

/*
 * srmReleaseFiles request copy constuctor
 */
srmReleaseFiles::srmReleaseFiles(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmReleaseFiles request destructor
 */
srmReleaseFiles::~srmReleaseFiles()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);
  DELETE_VEC(SURL);
  DELETE(doRemove);

  /* response (parser) */
  DELETE(fileStatuses);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmReleaseFiles::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(ReleaseFiles);
}

int
srmReleaseFiles::exec(Process *proc)
{
  DM_DBG_I;

  std::vector <std::string *> SURL = proc->eval_vec_str(srmReleaseFiles::SURL);

#ifdef SRM2_CALL
  NEW_SRM_RET(ReleaseFiles);

  pint_t* doRemoveInt = proc->eval2pint(doRemove);

  ReleaseFiles(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(requestToken),
    SURL,
    (bool *)doRemoveInt->p,
    resp
  );

  free(doRemoveInt);
#endif

  DELETE_VEC(SURL);

  /* matching */
  if(!resp || !resp->srmReleaseFilesResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  EAT_MATCH(fileStatuses, arrayOfReleaseFilesResponseToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmReleaseFilesResponse->returnStatus, proc));
}

std::string
srmReleaseFiles::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(ReleaseFiles);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> SURL =
    proc? proc->eval_vec_str(srmReleaseFiles::SURL):
          srmReleaseFiles::SURL;
  
  /* request */  
  SS_SRM("srmReleaseFiles");
  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);
  SS_VEC_DEL(SURL);
  SS_P_DQ(doRemove);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmReleaseFilesResponse) RETURN(ss.str());

  ss << arrayOfReleaseFilesResponseToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmReleaseFilesResponse);

  RETURN(ss.str());
}

std::string
srmReleaseFiles::arrayOfReleaseFilesResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(ReleaseFiles);
  std::stringstream ss;

  if(!resp || !resp->srmReleaseFilesResponse) RETURN(ss.str());

  if(resp->srmReleaseFilesResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TSURLReturnStatus *> v = resp->srmReleaseFilesResponse->arrayOfFileStatuses->statusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(surl);
      SS_P_VEC_SRM_RETSTAT(status);
    }
  }
  RETURN(ss.str());
}
