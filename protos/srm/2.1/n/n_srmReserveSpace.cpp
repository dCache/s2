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
 * srmReserveSpace request constuctor
 */
srmReserveSpace::srmReserveSpace()
{
  init();
}

/*
 * Initialise srmReserveSpace request
 */
void
srmReserveSpace::init()
{
  /* request (parser/API) */
  typeOfSpace = NULL;
  userSpaceTokenDescription = NULL;
  sizeOfTotalSpaceDesired = NULL;
  sizeOfGuaranteedSpaceDesired = NULL;
  lifetimeOfSpaceToReserve = NULL;
  storageSystemInfo = NULL;

  /* response (parser) */
  typeOfReservedSpace = NULL;
  sizeOfTotalReservedSpace = NULL;
  sizeOfGuaranteedReservedSpace = NULL;
  lifetimeOfReservedSpace = NULL;
  referenceHandleOfReservedSpace = NULL;
}

/*
 * srmReserveSpace request copy constuctor
 */
srmReserveSpace::srmReserveSpace(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmReserveSpace request destructor
 */
srmReserveSpace::~srmReserveSpace()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(typeOfSpace);
  DELETE(userSpaceTokenDescription);
  DELETE(sizeOfTotalSpaceDesired);
  DELETE(sizeOfGuaranteedSpaceDesired);
  DELETE(lifetimeOfSpaceToReserve);
  DELETE(storageSystemInfo);

  /* response (parser) */
  DELETE(typeOfReservedSpace);
  DELETE(sizeOfTotalReservedSpace);
  DELETE(sizeOfGuaranteedReservedSpace);
  DELETE(lifetimeOfReservedSpace);
  DELETE(referenceHandleOfReservedSpace);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmReserveSpace::finish(Process *proc)
{
  DM_DBG_I;
  srm__srmReserveSpaceResponse_ *resp = (srm__srmReserveSpaceResponse_ *)proc->resp;
  
  DELETE(resp);
}

int
srmReserveSpace::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

#ifdef SRM2_CALL
  NEW_SRM_RESP(ReserveSpace);

  ReserveSpace(
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    *getTSpaceType(EVAL2CSTR(typeOfSpace)), /* getTSpaceType never returns pointer to NULL */
    EVAL2CSTR(userSpaceTokenDescription),
    proc->eval2pint64(sizeOfTotalSpaceDesired).p,
    proc->eval2pint64(sizeOfGuaranteedSpaceDesired).p,
    proc->eval2pint64(lifetimeOfSpaceToReserve).p,
    EVAL2CSTR(storageSystemInfo),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmReserveSpaceResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* typeOfReservedSpace */
  EAT_MATCH(resp->srmReserveSpaceResponse,
            typeOfReservedSpace,
            getTSpaceType(*(resp->srmReserveSpaceResponse->typeOfReservedSpace)).c_str());

  /* sizeOfTotalReservedSpace */
  EAT_MATCH(resp->srmReserveSpaceResponse,
            sizeOfTotalReservedSpace,
            i2str(resp->srmReserveSpaceResponse->sizeOfTotalReservedSpace->value).c_str());

  /* sizeOfGuaranteedReservedSpace */
  EAT_MATCH(resp->srmReserveSpaceResponse,
            sizeOfGuaranteedReservedSpace,
            i2str(resp->srmReserveSpaceResponse->sizeOfGuaranteedReservedSpace->value).c_str());

  /* lifetimeOfReservedSpace */
  EAT_MATCH(resp->srmReserveSpaceResponse,
            lifetimeOfReservedSpace,
            i2str(resp->srmReserveSpaceResponse->lifetimeOfReservedSpace->value).c_str());

  /* referenceHandleOfReservedSpace */
  EAT_MATCH(resp->srmReserveSpaceResponse,
            referenceHandleOfReservedSpace,
            resp->srmReserveSpaceResponse->referenceHandleOfReservedSpace->value.c_str());

  RETURN(matchReturnStatus(resp->srmReserveSpaceResponse->returnStatus, proc));
}

std::string
srmReserveSpace::toString(Process *proc)
{
  DM_DBG_I;

  srm__srmReserveSpaceResponse_ *resp = proc? (srm__srmReserveSpaceResponse_ *)proc->resp : NULL;
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmReserveSpace");
  SS_P_DQ(userID);
  SS_P_DQ(typeOfSpace);
  SS_P_DQ(userSpaceTokenDescription);
  SS_P_DQ(sizeOfTotalSpaceDesired);
  SS_P_DQ(sizeOfGuaranteedSpaceDesired);
  SS_P_DQ(lifetimeOfSpaceToReserve);
  SS_P_DQ(storageSystemInfo);

  /* response (parser) */
  SS_P_DQ(typeOfReservedSpace);
  SS_P_DQ(sizeOfTotalReservedSpace);
  SS_P_DQ(sizeOfGuaranteedReservedSpace);
  SS_P_DQ(lifetimeOfReservedSpace);
  SS_P_DQ(referenceHandleOfReservedSpace);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmReserveSpaceResponse) RETURN(ss.str());

  SS_P_SPACETYPE(resp->srmReserveSpaceResponse, typeOfReservedSpace);
  SS_P_VALUE(resp->srmReserveSpaceResponse, sizeOfTotalReservedSpace);
  SS_P_VALUE(resp->srmReserveSpaceResponse, sizeOfGuaranteedReservedSpace);
  SS_P_VALUE(resp->srmReserveSpaceResponse, lifetimeOfReservedSpace);
  SS_P_VALUE(resp->srmReserveSpaceResponse, referenceHandleOfReservedSpace);

  SS_P_SRM_RETSTAT(resp->srmReserveSpaceResponse);

  RETURN(ss.str());
}
