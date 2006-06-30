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
 * srmExtendFileLifeTimeInSpace request constuctor
 */
srmExtendFileLifeTimeInSpace::srmExtendFileLifeTimeInSpace()
{
  init();
}

/*
 * Initialise srmExtendFileLifeTimeInSpace request
 */
void
srmExtendFileLifeTimeInSpace::init()
{
  /* request (parser/API) */
  spaceToken = NULL;
  newLifeTime = NULL;

  /* response (parser) */
  newTimeExtended = NULL;
  fileStatuses = NULL;
}

/*
 * srmExtendFileLifeTimeInSpace request copy constuctor
 */
srmExtendFileLifeTimeInSpace::srmExtendFileLifeTimeInSpace(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmExtendFileLifeTimeInSpace request destructor
 */
srmExtendFileLifeTimeInSpace::~srmExtendFileLifeTimeInSpace()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(spaceToken);
  DELETE_VEC(SURL);
  DELETE(newLifeTime);

  /* response (parser) */
  DELETE(newTimeExtended);
  DELETE(fileStatuses);

  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmExtendFileLifeTimeInSpace::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(ExtendFileLifeTimeInSpace);
}

int
srmExtendFileLifeTimeInSpace::exec(Process *proc)
{
#define EVAL_VEC_STR_EF(vec) vec = proc->eval_vec_str(srmExtendFileLifeTimeInSpace::vec)
  DM_DBG_I;

  tStorageSystemInfo storageSystemInfo;
  std::vector <std::string *> SURL;
  
  EVAL_VEC_STR_EF(SURL);

#ifdef SRM2_CALL
  NEW_SRM_RET(ExtendFileLifeTimeInSpace);

  ExtendFileLifeTimeInSpace(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(spaceToken),
    SURL,
    resp
  );
#endif

  DELETE_VEC(SURL);

  /* matching */
  if(!resp || !resp->srmExtendFileLifeTimeInSpaceResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* newTimeExtended */
  EAT_MATCH_C(resp->srmExtendFileLifeTimeInSpaceResponse->newTimeExtended,
              newTimeExtended,
              PI2CSTR(resp->srmExtendFileLifeTimeInSpaceResponse->newTimeExtended));

  /* fileStatuses */
  EAT_MATCH(fileStatuses, arrayOfExtendFileLifeTimeInSpaceResponseToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmExtendFileLifeTimeInSpaceResponse->returnStatus, proc));

#undef EVAL_VEC_STR_EF
}

std::string
srmExtendFileLifeTimeInSpace::toString(Process *proc)
{
#define EVAL_VEC_STR_EF(vec) EVAL_VEC_STR(srmExtendFileLifeTimeInSpace,vec)
  DM_DBG_I;

  GET_SRM_RESP(ExtendFileLifeTimeInSpace);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> SURL;

  EVAL_VEC_STR_EF(SURL);

  /* request */  
  SS_SRM("srmExtendFileLifeTimeInSpace");

  SS_P_DQ(authorizationID);
  SS_P_DQ(spaceToken);
  SS_VEC_DEL(SURL);
  SS_P_DQ(newLifeTime);

  /* response (parser) */
  SS_P_DQ(newTimeExtended);
  SS_P_DQ(fileStatuses);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmExtendFileLifeTimeInSpaceResponse) RETURN(ss.str());

  /* newTimeExtended */
  SS_P_DQ_C(resp->srmExtendFileLifeTimeInSpaceResponse->newTimeExtended,
            newTimeExtended,
            PI2CSTR(resp->srmExtendFileLifeTimeInSpaceResponse->newTimeExtended));

  /* fileStatuses */
  ss << arrayOfExtendFileLifeTimeInSpaceResponseToString(proc, TRUE, quote);
  
  SS_P_SRM_RETSTAT(resp->srmExtendFileLifeTimeInSpaceResponse);

  RETURN(ss.str());

#undef EVAL_VEC_STR_EF
}

std::string
srmExtendFileLifeTimeInSpace::arrayOfExtendFileLifeTimeInSpaceResponseToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(ExtendFileLifeTimeInSpace);
  std::stringstream ss;

  if(!resp || !resp->srmExtendFileLifeTimeInSpaceResponse) RETURN(ss.str());

  if(resp->srmExtendFileLifeTimeInSpaceResponse->arrayOfFileStatuses) {
    BOOL print_space = FALSE;
    std::vector<srm__TSURLLifetimeReturnStatus *> v = resp->srmExtendFileLifeTimeInSpaceResponse->arrayOfFileStatuses->statusArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(surl);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_DPAR(fileLifetime);
      SS_P_VEC_DPAR(pinLifetime);
    }
  }

  RETURN(ss.str());
}
