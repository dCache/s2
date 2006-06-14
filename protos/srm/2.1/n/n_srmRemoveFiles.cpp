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

  /* response (API) */
  resp = new srm__srmRemoveFilesResponse_();
  if(resp == NULL) {
    DM_ERR(ERR_SYSTEM, "new failed\n");
  } else {
    memset(resp, 0, sizeof(srm__srmRemoveFilesResponse_));
  }
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
  
  /* response (API) */
  DELETE(resp);

  DM_DBG_O;
}

int
srmRemoveFiles::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

  std::vector <std::string *> surlArray = proc->eval_vec_str(srmRemoveFiles::surlArray);

#ifdef SRM2_CALL
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

  /* arrayOfRequestDetails */
  match = proc->e_match(fileStatuses, arrayOfRemoveFilesResponseToString(FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmRemoveFilesResponse->returnStatus, proc));
}

std::string
srmRemoveFiles::toString(Process *proc)
{
  DM_DBG_I;
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

  ss << arrayOfRemoveFilesResponseToString(TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmRemoveFilesResponse);

  RETURN(ss.str());
}

std::string
srmRemoveFiles::arrayOfRemoveFilesResponseToString(BOOL space, BOOL quote) const
{
  DM_DBG_I;
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
