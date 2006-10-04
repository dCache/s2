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
  newLifeTimeFromCallingTime = NULL;
  storageSystemInfo = NULL;

  /* response (parser) */
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
  DELETE(newLifeTimeFromCallingTime);
  DELETE(storageSystemInfo);

  /* response (parser) */
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
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RET(UpdateSpace);

  UpdateSpace(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(spaceToken),
    proc->eval2pint64(newSizeOfTotalSpaceDesired).p,
    proc->eval2pint64(newSizeOfGuaranteedSpaceDesired).p,
    proc->eval2pint64(newLifeTimeFromCallingTime).p,
    EVAL2CSTR(storageSystemInfo),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmUpdateSpaceResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* sizeOfTotalSpace */
  EAT_MATCH_C(resp->srmUpdateSpaceResponse->sizeOfTotalSpace,
              sizeOfTotalSpace,
              i2str(resp->srmUpdateSpaceResponse->sizeOfTotalSpace->value).c_str());

  /* sizeOfGuaranteedSpace */
  EAT_MATCH_C(resp->srmUpdateSpaceResponse->sizeOfGuaranteedSpace,
              sizeOfGuaranteedSpace,
              i2str(resp->srmUpdateSpaceResponse->sizeOfGuaranteedSpace->value).c_str());

  /* lifetimeGranted */
  EAT_MATCH_C(resp->srmUpdateSpaceResponse->lifetimeGranted,
              lifetimeGranted,
              i2str(resp->srmUpdateSpaceResponse->lifetimeGranted->value).c_str());

  RETURN(matchReturnStatus(resp->srmUpdateSpaceResponse->returnStatus, proc));
}

std::string
srmUpdateSpace::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(UpdateSpace);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmUpdateSpace");

  SS_P_DQ(userID);
  SS_P_DQ(spaceToken);
  SS_P_DQ(newSizeOfTotalSpaceDesired);
  SS_P_DQ(newSizeOfGuaranteedSpaceDesired);
  SS_P_DQ(newLifeTimeFromCallingTime);
  SS_P_DQ(storageSystemInfo);

  /* response (parser) */
  SS_P_DQ(sizeOfTotalSpace);
  SS_P_DQ(sizeOfGuaranteedSpace);
  SS_P_DQ(lifetimeGranted);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmUpdateSpaceResponse) RETURN(ss.str());

  SS_P_VALUE(resp->srmUpdateSpaceResponse, sizeOfTotalSpace);
  SS_P_VALUE(resp->srmUpdateSpaceResponse, sizeOfGuaranteedSpace);
  SS_P_VALUE(resp->srmUpdateSpaceResponse, lifetimeGranted);

  SS_P_SRM_RETSTAT(resp->srmUpdateSpaceResponse);

  RETURN(ss.str());
}
