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
  keepFiles = NULL;

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
  DELETE_VEC(surlArray);
  DELETE(keepFiles);

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
  BOOL match = FALSE;

  std::vector <std::string *> surlArray = proc->eval_vec_str(srmReleaseFiles::surlArray);

#ifdef SRM2_CALL
  NEW_SRM_RET(ReleaseFiles);

  ReleaseFiles(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(requestToken),
    surlArray,
    (bool *)proc->eval2pint64(keepFiles).p,
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmReleaseFilesResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  match = proc->e_match(fileStatuses, arrayOfReleaseFilesResponseToString(proc, FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmReleaseFilesResponse->returnStatus, proc));
}

std::string
srmReleaseFiles::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(ReleaseFiles);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> surlArray =
    proc? proc->eval_vec_str(srmReleaseFiles::surlArray):
          srmReleaseFiles::surlArray;
  
  /* request */  
  SS_SRM("srmReleaseFiles");
  SS_P_DQ(userID);
  SS_P_DQ(requestToken);
  SS_VEC(surlArray); if(proc) DELETE_VEC(surlArray);
  SS_P_DQ(keepFiles);

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
    std::vector<srm__TSURLReturnStatus *> v = resp->srmReleaseFilesResponse->arrayOfFileStatuses->surlReturnStatusArray;
    for(uint i = 0; i < v.size(); i++) {
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_PAR_VAL(surl);
    }
  }
  RETURN(ss.str());
}
