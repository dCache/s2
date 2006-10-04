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
 * srmPutDone request constuctor
 */
srmPutDone::srmPutDone()
{
  init();
}

/*
 * Initialise srmPutDone request
 */
void
srmPutDone::init()
{
  /* request (parser/API) */
  requestToken = NULL;

  /* response (parser) */
  fileStatuses = NULL;
}

/*
 * srmPutDone request copy constuctor
 */
srmPutDone::srmPutDone(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmPutDone request destructor
 */
srmPutDone::~srmPutDone()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);
  DELETE_VEC(SURL);

  /* response (parser) */
  DELETE(fileStatuses);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmPutDone::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(PutDone);
}

int
srmPutDone::exec(Process *proc)
{
  DM_DBG_I;

  std::vector <std::string *> SURL = proc->eval_vec_str(srmPutDone::SURL);

#ifdef SRM2_CALL
  NEW_SRM_RET(PutDone);

  PutDone(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(requestToken),
    SURL,
    resp
  );
#endif

  DELETE_VEC(SURL);

  /* matching */
  if(!resp || !resp->srmPutDoneResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  EAT_MATCH(fileStatuses, arrayOfPutDoneResponseToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmPutDoneResponse->returnStatus, proc));
}

std::string
srmPutDone::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(PutDone);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> SURL =
    proc? proc->eval_vec_str(srmPutDone::SURL):
          srmPutDone::SURL;
  
  /* request */  
  SS_SRM("srmPutDone");
  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);
  SS_VEC_DEL(SURL);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmPutDoneResponse) RETURN(ss.str());

  ss << arrayOfPutDoneResponseToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmPutDoneResponse);

  RETURN(ss.str());
}

std::string
srmPutDone::arrayOfPutDoneResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(PutDone);
  std::stringstream ss;

  if(!resp || !resp->srmPutDoneResponse) RETURN(ss.str());

  if(resp->srmPutDoneResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TSURLReturnStatus *> v = resp->srmPutDoneResponse->arrayOfFileStatuses->statusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(surl);
      SS_P_VEC_SRM_RETSTAT(status);
    }
  }
  RETURN(ss.str());
}
