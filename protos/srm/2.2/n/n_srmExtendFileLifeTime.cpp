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
#include "str.h"		/* i2str() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * srmExtendFileLifeTime request constuctor
 */
srmExtendFileLifeTime::srmExtendFileLifeTime()
{
  init();
}

/*
 * Initialise srmExtendFileLifeTime request
 */
void
srmExtendFileLifeTime::init()
{
  /* request (parser/API) */
  requestToken = NULL;
  newFileLifeTime = NULL;
  newPinLifeTime = NULL;

  /* response (parser) */
  fileStatuses = NULL;
}

/*
 * srmExtendFileLifeTime request copy constuctor
 */
srmExtendFileLifeTime::srmExtendFileLifeTime(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmExtendFileLifeTime request destructor
 */
srmExtendFileLifeTime::~srmExtendFileLifeTime()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(requestToken);
  DELETE_VEC(SURL);
  DELETE(newFileLifeTime);
  DELETE(newPinLifeTime);

  /* response (parser) */
  DELETE(fileStatuses);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmExtendFileLifeTime::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(ExtendFileLifeTime);
}

int
srmExtendFileLifeTime::exec(Process *proc)
{
  DM_DBG_I;

  std::vector <std::string *> SURL = proc->eval_vec_str(srmExtendFileLifeTime::SURL);
  
#ifdef SRM2_CALL
  NEW_SRM_RET(ExtendFileLifeTime);

  ExtendFileLifeTime(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(requestToken),
    SURL,
    proc->eval2pint(newFileLifeTime).p,
    proc->eval2pint(newPinLifeTime).p,
    resp
  );
#endif

  DELETE_VEC(SURL);
  
  /* matching */
  if(!resp || !resp->srmExtendFileLifeTimeResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* fileStatuses */
  EAT_MATCH(fileStatuses, arrayOfExtendFileLifeTimeResponseToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmExtendFileLifeTimeResponse->returnStatus, proc));
}

std::string
srmExtendFileLifeTime::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(ExtendFileLifeTime);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> SURL =
    proc? proc->eval_vec_str(srmExtendFileLifeTime::SURL):
          srmExtendFileLifeTime::SURL;

  /* request */  
  SS_SRM("srmExtendFileLifeTime");
  SS_P_DQ(authorizationID);
  SS_P_DQ(requestToken);
  SS_VEC_DEL(SURL);
  SS_P_DQ(newFileLifeTime);
  SS_P_DQ(newPinLifeTime);

  /* response (parser) */
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmExtendFileLifeTimeResponse) RETURN(ss.str());

  ss << arrayOfExtendFileLifeTimeResponseToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmExtendFileLifeTimeResponse);

  RETURN(ss.str());
}

std::string
srmExtendFileLifeTime::arrayOfExtendFileLifeTimeResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(ExtendFileLifeTime);
  std::stringstream ss;

  if(!resp || !resp->srmExtendFileLifeTimeResponse) RETURN(ss.str());

  if(resp->srmExtendFileLifeTimeResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TSURLLifetimeReturnStatus *> v = resp->srmExtendFileLifeTimeResponse->arrayOfFileStatuses->statusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(surl);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_DPAR(fileLifetime);
      SS_P_VEC_DPAR(pinLifetime);
    }
  }

  RETURN(ss.str());
}
