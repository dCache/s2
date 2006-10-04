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
 * srmGetTransferProtocols request constuctor
 */
srmGetTransferProtocols::srmGetTransferProtocols()
{
  init();
}

/*
 * Initialise srmGetTransferProtocols request
 */
void
srmGetTransferProtocols::init()
{
  /* request (parser/API) */

  /* response (parser) */
  transferProtocols = NULL;
}

/*
 * srmGetTransferProtocols request copy constuctor
 */
srmGetTransferProtocols::srmGetTransferProtocols(Node &node)
{
  init();
  Node::init(node);
}

/*
 * srmGetTransferProtocols request destructor
 */
srmGetTransferProtocols::~srmGetTransferProtocols()
{
  DM_DBG_I;

  /* request (parser/API) */

  /* response (parser) */
  DELETE(transferProtocols);

  DM_DBG_O;
}

/*
 * Free process-related structures.
 */
void
srmGetTransferProtocols::finish(Process *proc)
{
  DM_DBG_I;

  FREE_SRM_RET(GetTransferProtocols);
}

int
srmGetTransferProtocols::exec(Process *proc)
{
  DM_DBG_I;

#ifdef SRM2_CALL
  NEW_SRM_RET(GetTransferProtocols);

  GetTransferProtocols(
    soap,
    EVAL2CSTR(srm_endpoint),
    EVAL2CSTR(authorizationID),
    resp
  );
#endif

  /* matching */
  if(!resp || !resp->srmGetTransferProtocolsResponse) {
    DM_LOG(DM_N(1), "no SRM response\n");
    RETURN(ERR_ERR);
  }

  /* arrayOfRequestDetails */
  EAT_MATCH(transferProtocols, arrayOfRequestDetailsToString(proc, FALSE, FALSE).c_str());

  RETURN(matchReturnStatus(resp->srmGetTransferProtocolsResponse->returnStatus, proc));
}

std::string
srmGetTransferProtocols::toString(Process *proc)
{
  DM_DBG_I;

  GET_SRM_RESP(GetTransferProtocols);
  BOOL quote = TRUE;
  std::stringstream ss;

  /* request */  
  SS_SRM("srmGetTransferProtocols");
  SS_P_DQ(authorizationID);

  /* response (parser) */
  SS_P_DQ(transferProtocols);
  SS_P_DQ(returnStatus.explanation);
  SS_P_DQ(returnStatus.statusCode);

  /* response (API) */
  if(!resp || !resp->srmGetTransferProtocolsResponse) RETURN(ss.str());

  ss << arrayOfRequestDetailsToString(proc, TRUE, quote);

  SS_P_SRM_RETSTAT(resp->srmGetTransferProtocolsResponse);
  
  RETURN(ss.str());
}

std::string
srmGetTransferProtocols::arrayOfRequestDetailsToString(Process *proc, BOOL space, BOOL quote) const
{
  DM_DBG_I;

  GET_SRM_RESP(GetTransferProtocols);
  std::stringstream ss;

  if(!resp || !resp->srmGetTransferProtocolsResponse) RETURN(ss.str());

  if(resp->srmGetTransferProtocolsResponse->protocolInfo) {
    BOOL print_space = FALSE;
    std::vector<srm__TSupportedTransferProtocol *> v = resp->srmGetTransferProtocolsResponse->protocolInfo->protocolArray;
    for(uint u = 0; u < v.size(); u++) {
      SS_P_VEC_PAR(transferProtocol);

      /* attributes */
      if(v[u] && v[u]->attributes) {
        std::vector<srm__TExtraInfo *> extraInfoArray = v[u]->attributes->extraInfoArray;
        SS_P_VEC_SRM_EXTRA_INFOu(extraInfoArray);
      }
    }
  }
  
  RETURN(ss.str());
}
