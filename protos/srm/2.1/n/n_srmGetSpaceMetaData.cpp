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
  DELETE_VEC(arrayOfSpaceToken);

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

  std::vector <std::string *> arrayOfSpaceToken = proc->eval_vec_str(srmGetSpaceMetaData::arrayOfSpaceToken);

#ifdef SRM2_CALL
  NEW_SRM_RET(GetSpaceMetaData);

  GetSpaceMetaData(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(userID),
    arrayOfSpaceToken,
    resp
  );
#endif

  DELETE_VEC(arrayOfSpaceToken);

  /* matching */
  if(!resp || !resp->srmGetSpaceMetaDataResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfSpaceDetails */
  match = proc->e_match(spaceDetails, arrayOfSpaceDetailsToString(proc, FALSE, FALSE).c_str());
  if(!match) {
    DM_LOG(DM_N(1), "no match\n");
    RETURN(ERR_ERR);
  }

  RETURN(matchReturnStatus(resp->srmGetSpaceMetaDataResponse->returnStatus, proc));
}

std::string
srmGetSpaceMetaData::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(GetSpaceMetaData);
  BOOL quote = TRUE;
  std::stringstream ss;

  std::vector <std::string *> arrayOfSpaceToken =
    proc? proc->eval_vec_str(srmGetSpaceMetaData::arrayOfSpaceToken):
          srmGetSpaceMetaData::arrayOfSpaceToken;

  if(srm_endpoint == NULL) {
    DM_ERR_ASSERT(_("srm_endpoint == NULL\n"));
    RETURN(ss.str());
  }

  /* request */  
  SS_SRM("srmGetSpaceMetaData");
  SS_P_DQ(userID);
  SS_VEC_DEL(arrayOfSpaceToken);

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
    std::vector<srm__TMetaDataSpace*> v = resp->srmGetSpaceMetaDataResponse->arrayOfSpaceDetails->spaceDetailArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR_SPACETYPE(type);
      SS_P_VEC_PAR_VAL(spaceToken);
      SS_P_VEC_PAR(isValid);
      SS_P_VEC_PAR_VAL(owner);
      SS_P_VEC_PAR_VAL(totalSize);
      SS_P_VEC_PAR_VAL(guaranteedSize);
      SS_P_VEC_PAR_VAL(unusedSize);
      SS_P_VEC_PAR_VAL(lifetimeAssigned);
      SS_P_VEC_PAR_VAL(lifetimeLeft);
    }
  }
  
  RETURN(ss.str());
}
