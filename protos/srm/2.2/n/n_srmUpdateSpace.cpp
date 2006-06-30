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
#include "str.h"		/* i2str() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * srmUpdateSpace request constuctor
 */
srmUpdateSpace::srmUpdateSpace()
{
  init();
}

/*
 * Initialise srmUpdateSpace request
 */
void
srmUpdateSpace::init()
{
  /* request (parser/API) */
  spaceToken = NULL;
  newSizeOfTotalSpaceDesired = NULL;
  newSizeOfGuaranteedSpaceDesired = NULL;
  newLifeTime = NULL;

  /* response (parser) */
  requestToken = NULL;
  sizeOfTotalSpace = NULL;
  sizeOfGuaranteedSpace = NULL;
  lifetimeGranted = NULL;
}

/*
 * srmUpdateSpace request copy constuctor
 */
srmUpdateSpace::srmUpdateSpace(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmUpdateSpace request destructor
 */
srmUpdateSpace::~srmUpdateSpace()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(spaceToken);
  DELETE(newSizeOfTotalSpaceDesired);
  DELETE(newSizeOfGuaranteedSpaceDesired);
  DELETE(newLifeTime);
  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* response (parser) */
  DELETE(requestToken);
  DELETE(sizeOfTotalSpace);
  DELETE(sizeOfGuaranteedSpace);
  DELETE(lifetimeGranted);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmUpdateSpace::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(UpdateSpace);
}

int
srmUpdateSpace::exec(Process *proc)
{
#define EVAL_VEC_STR_US(vec) vec = proc->eval_vec_str(srmUpdateSpace::vec)
  DM_DBG_I;

  tStorageSystemInfo storageSystemInfo;

  EVAL_VEC_STR_US(storageSystemInfo.key);
  EVAL_VEC_STR_US(storageSystemInfo.value);

#ifdef SRM2_CALL
  NEW_SRM_RET(UpdateSpace);

  UpdateSpace(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    EVAL2CSTR(spaceToken),
    proc->eval2puint64(newSizeOfTotalSpaceDesired).p,
    proc->eval2puint64(newSizeOfGuaranteedSpaceDesired).p,
    proc->eval2pint(newLifeTime).p,
    storageSystemInfo,
    resp
  );
#endif

  DELETE_VEC(storageSystemInfo.key);
  DELETE_VEC(storageSystemInfo.value);

  /* matching */
  if(!resp || !resp->srmUpdateSpaceResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* requestToken */
  EAT_MATCH_C(resp->srmUpdateSpaceResponse->requestToken,
              requestToken,
              CSTR(resp->srmUpdateSpaceResponse->requestToken));

  /* sizeOfTotalSpace */
  EAT_MATCH_C(resp->srmUpdateSpaceResponse->sizeOfTotalSpace,
              sizeOfTotalSpace,
              PI2CSTR(resp->srmUpdateSpaceResponse->sizeOfTotalSpace));

  /* sizeOfGuaranteedSpace */
  EAT_MATCH_C(resp->srmUpdateSpaceResponse->sizeOfGuaranteedSpace,
              sizeOfGuaranteedSpace,
              PI2CSTR(resp->srmUpdateSpaceResponse->sizeOfGuaranteedSpace));

  /* lifetimeGranted */
  EAT_MATCH_C(resp->srmUpdateSpaceResponse,
              lifetimeGranted,
              PI2CSTR(resp->srmUpdateSpaceResponse->lifetimeGranted));

  RETURN(matchReturnStatus(resp->srmUpdateSpaceResponse->returnStatus, proc));

#undef EVAL_VEC_STR_US
}

std::string
srmUpdateSpace::toString(Process *proc)
{
#define EVAL_VEC_STR_US(vec) EVAL_VEC_STR(srmUpdateSpace,vec)
  DM_DBG_I;

  GET_SRM_RESP(UpdateSpace);
  BOOL quote = TRUE;
  std::stringstream ss;

  tStorageSystemInfo_ storageSystemInfo;

  EVAL_VEC_STR_US(storageSystemInfo.key);
  EVAL_VEC_STR_US(storageSystemInfo.value);

  /* request */  
  SS_SRM("srmUpdateSpace");

  SS_P_DQ(authorizationID);
  SS_P_DQ(spaceToken);
  SS_P_DQ(newSizeOfTotalSpaceDesired);
  SS_P_DQ(newSizeOfGuaranteedSpaceDesired);
  SS_P_DQ(newLifeTime);
  SS_VEC_DEL(storageSystemInfo.key);
  SS_VEC_DEL(storageSystemInfo.value);

  /* response (parser) */
  SS_P_DQ(requestToken);
  SS_P_DQ(sizeOfTotalSpace);
  SS_P_DQ(sizeOfGuaranteedSpace);
  SS_P_DQ(lifetimeGranted);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmUpdateSpaceResponse) RETURN(ss.str());

  /* requestToken */
  SS_P_DQ_C(resp->srmUpdateSpaceResponse->requestToken,
            requestToken,
            CSTR(resp->srmUpdateSpaceResponse->requestToken));

  /* sizeOfTotalSpace */
  SS_P_DQ_C(resp->srmUpdateSpaceResponse->sizeOfTotalSpace,
            sizeOfTotalSpace,
            PI2CSTR(resp->srmUpdateSpaceResponse->sizeOfTotalSpace));

  /* sizeOfGuaranteedSpace */
  SS_P_DQ_C(resp->srmUpdateSpaceResponse->sizeOfGuaranteedSpace,
            sizeOfGuaranteedSpace,
            PI2CSTR(resp->srmUpdateSpaceResponse->sizeOfGuaranteedSpace));

  /* lifetimeGranted */
  SS_P_DQ_C(resp->srmUpdateSpaceResponse,
            lifetimeGranted,
            PI2CSTR(resp->srmUpdateSpaceResponse->lifetimeGranted));

  SS_P_SRM_RETSTAT(resp->srmUpdateSpaceResponse);

  RETURN(ss.str());

#undef EVAL_VEC_STR_US
}
