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
 * srmRemoveFiles request constuctor
 */
srmRemoveFiles::srmRemoveFiles()
{
  init();
}

/*
 * Initialise srmRemoveFiles request
 */
void
srmRemoveFiles::init()
{
  /* request (parser/API) */
  requestToken = NULL;

  /* response (parser) */
  fileStatuses = NULL;
}

/*
 * srmRemoveFiles request copy constuctor
 */
srmRemoveFiles::srmRemoveFiles(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmRemoveFiles request destructor
 */
srmRemoveFiles::~srmRemoveFiles()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);
  DELETE_VEC(surlArray);

  /* response (parser) */
  DELETE(fileStatuses);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmRemoveFiles::finish(Process *proc)
{
  DM_DBG_I;
  srm__srmRemoveFilesResponse_ *resp = (srm__srmRemoveFilesResponse_ *)proc->resp;
  
  DELETE(resp);
}

int
srmRemoveFiles::exec(Process *proc)
{
  DM_DBG_I;

  std::vector <std::string *> surlArray = proc->eval_vec_str(srmRemoveFiles::surlArray);

#ifdef SRM2_CALL
  NEW_SRM_RESP(RemoveFiles);

  RemoveFiles(
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(requestToken),
    surlArray,
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmRemoveFilesResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmRemoveFilesResponse->returnStatus, proc));
}

std::string
srmRemoveFiles::toString(Process *proc)
{
  DM_DBG_I;

  srm__srmRemoveFilesResponse_ *resp = proc? (srm__srmRemoveFilesResponse_ *)proc->resp : NULL;
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> surlArray =
    proc? proc->eval_vec_str(srmRemoveFiles::surlArray):
          srmRemoveFiles::surlArray;
  
  /* request */  
  SS_SRM("srmRemoveFiles");
  SS_P_DQ(userID);
  SS_P_DQ(requestToken);
  SS_VEC(surlArray); if(proc) DELETE_VEC(surlArray);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmRemoveFilesResponse) RETURN(ss.str());

  ss << arrayOfRemoveFilesResponseToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmRemoveFilesResponse);

  RETURN(ss.str());
}

std::string
srmRemoveFiles::arrayOfRemoveFilesResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  srm__srmRemoveFilesResponse_ *resp = proc? (srm__srmRemoveFilesResponse_ *)proc->resp : NULL;
  std::stringstream ss;

  if(!resp || !resp->srmRemoveFilesResponse) RETURN(ss.str());

  if(resp->srmRemoveFilesResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TSURLReturnStatus *> v = resp->srmRemoveFilesResponse->arrayOfFileStatuses->surlReturnStatusArray;
    for(uint i = 0; i < v.size(); i++) {
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_PAR_VAL(surl);
    }
  }
  RETURN(ss.str());
}
