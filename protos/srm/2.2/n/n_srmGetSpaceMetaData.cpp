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
 * srmGetSpaceMetaData request constuctor
 */
srmGetSpaceMetaData::srmGetSpaceMetaData()
{
  init();
}

/*
 * Initialise srmGetSpaceMetaData request
 */
void
srmGetSpaceMetaData::init()
{
  /* request (parser/API) */

  /* response (parser) */
  spaceDetails = NULL;
}

/*
 * srmGetSpaceMetaData request copy constuctor
 */
srmGetSpaceMetaData::srmGetSpaceMetaData(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmGetSpaceMetaData request destructor
 */
srmGetSpaceMetaData::~srmGetSpaceMetaData()
{
  DM_DBG_I;

  /* request (parser/API) */
  DELETE_VEC(spaceTokens);

  /* response (parser) */
  DELETE(spaceDetails);

  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmGetSpaceMetaData::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(GetSpaceMetaData);
}

int
srmGetSpaceMetaData::exec(Process *proc)
{
  DM_DBG_I;
  BOOL match = FALSE;

  std::vector <std::string *> spaceTokens = proc->eval_vec_str(srmGetSpaceMetaData::spaceTokens);

#ifdef SRM2_CALL
  NEW_SRM_RET(GetSpaceMetaData);

  GetSpaceMetaData(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    spaceTokens,
    resp
  );
#endif

  DELETE_VEC(spaceTokens);

  /* matching */
  if(!resp || !resp->srmGetSpaceMetaDataResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfSpaceDetails */
  EAT_MATCH(spaceDetails, arrayOfSpaceDetailsToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmGetSpaceMetaDataResponse->returnStatus, proc));
}

std::string
srmGetSpaceMetaData::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(GetSpaceMetaData);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> spaceTokens =
    proc? proc->eval_vec_str(srmGetSpaceMetaData::spaceTokens):
          srmGetSpaceMetaData::spaceTokens;

  if(srm_endpoint == NULL) {
    DM_ERR_ASSERT(_("srm_endpoint == NULL\n"));
    RETURN(ss.str());
  }

  /* request */  
  SS_SRM("srmGetSpaceMetaData");
  SS_P_DQ(authorizationID);
  SS_VEC_DEL(spaceTokens);

  /* response (parser) */
  SS_P_DQ(spaceDetails);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmGetSpaceMetaDataResponse) RETURN(ss.str());

  ss << arrayOfSpaceDetailsToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmGetSpaceMetaDataResponse);
  
  RETURN(ss.str());
}

std::string
srmGetSpaceMetaData::arrayOfSpaceDetailsToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(GetSpaceMetaData);
  std::stringstream ss;

  if(!resp || !resp->srmGetSpaceMetaDataResponse) RETURN(ss.str());

  if(resp->srmGetSpaceMetaDataResponse->arrayOfSpaceDetails) {
    BOOL print_space = FALSE;
    std::vector<srm__TMetaDataSpace *> v = resp->srmGetSpaceMetaDataResponse->arrayOfSpaceDetails->spaceDataArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(spaceToken);
      SS_P_VEC_SRM_RETSTAT(status);
      SS_P_VEC_SRM_RETENTION_POLICY(retentionPolicyInfo);
      SS_P_VEC_DPAR(owner);
      SS_P_VEC_DPAR(totalSize);
      SS_P_VEC_DPAR(guaranteedSize);
      SS_P_VEC_DPAR(unusedSize);
      SS_P_VEC_DPAR(lifetimeAssigned);
      SS_P_VEC_DPAR(lifetimeLeft);
    }
  }
  
  RETURN(ss.str());
}
