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
#include "sysdep.h"		/* BOOL, STD_BUF, ... */

#include "free.h"               /* FREE(), DELETE() */
#include "str.h"		/* i2str() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * srmCompactSpace request constuctor
 */
srmCompactSpace::srmCompactSpace()
{
  init();
}

/*
 * Initialise srmCompactSpace request
 */
void
srmCompactSpace::init()
{
  /* request (parser/API) */
  spaceToken = NULL;
  storageSystemInfo = NULL;
  doDynamicCompactFromNowOn = NULL;

  /* response (parser) */
  newSizeOfThisSpace = NULL;
}

/*
 * srmCompactSpace request copy constuctor
 */
srmCompactSpace::srmCompactSpace(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmCompactSpace request destructor
 */
srmCompactSpace::~srmCompactSpace()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE(spaceToken);
  DELETE(storageSystemInfo);
  DELETE(doDynamicCompactFromNowOn);

  /* response (parser) */
  DELETE(newSizeOfThisSpace);
  
  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmCompactSpace::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(CompactSpace);
}

int
srmCompactSpace::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RET(CompactSpace);

  CompactSpace(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    EVAL2CSTR(spaceToken),
    EVAL2CSTR(storageSystemInfo),
    (bool *)proc->eval2pint32(doDynamicCompactFromNowOn).p,
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmCompactSpaceResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* newSizeOfThisSpace */
  EAT_MATCH_C(resp->srmCompactSpaceResponse->newSizeOfThisSpace,
              newSizeOfThisSpace,
              i2str(resp->srmCompactSpaceResponse->newSizeOfThisSpace->value).c_str());

  RETURN(matchReturnStatus(resp->srmCompactSpaceResponse->returnStatus, proc));
}

std::string
srmCompactSpace::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(CompactSpace);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmCompactSpace");

  SS_P_DQ(userID);
  SS_P_DQ(spaceToken);
  SS_P_DQ(storageSystemInfo);
  SS_P_DQ(doDynamicCompactFromNowOn);

  /* response (parser) */
  SS_P_DQ(newSizeOfThisSpace);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmCompactSpaceResponse) RETURN(ss.str());

  SS_P_VALUE(resp->srmCompactSpaceResponse, newSizeOfThisSpace);

  SS_P_SRM_RETSTAT(resp->srmCompactSpaceResponse);

  RETURN(ss.str());
}
