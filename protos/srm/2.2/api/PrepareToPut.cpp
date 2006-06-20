/**
 * \file PrepareToPut.cpp
 *
 * Implements the SRM2 PrepareToPut method.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "gsoap/soapH.h"

#ifdef HAVE_CGSI_PLUGIN
#include "cgsi_plugin.h"
#endif

#include "srm2api.h"
#include "srm_macros.h"
#include "srm_soap27.h"

/**
 * srmPrepareToPut method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 *
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
PrepareToPut(struct soap *soap,
             const char *srm_endpoint,
             std::string *authorizationID,
             const tArrayOfPutFileRequests putFileRequests,
             tStorageSystemInfo storageSystemInfo,
             std::string *desiredTotalRequestTime,
             std::string *desiredPinLifeTime,
             std::string *desiredFileLifeTime,
             std::string *desiredFileStorageType,
             std::string *targetSpaceToken,
             std::string *retentionPolicy,
             std::string *accessLatency,
             const long *accessPattern,
             const long *connectionType,
             std::vector <std::string *> clientNetworks,
             std::vector <std::string *> transferProtocols,
             struct srm__srmPrepareToPutResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmPrepareToPutRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#endif

  req.authorizationID = authorizationID;

  /* Create the file request */
  NOT_NULL(req.arrayOfFileRequests = soap_new_srm__ArrayOfTPutFileRequest(soap, -1));
//           = soap_new_srm__ArrayOfAnyURI(soap, -1));
  DM_LOG(DM_N(2), "targetSURL.size() == %d\n", putFileRequests.targetSURL.size());
  for (uint i = 0; i < putFileRequests.targetSURL.size(); i++) {
    srm__TPutFileRequest *fileRequest;
    NOT_NULL(fileRequest = soap_new_srm__TPutFileRequest(soap, -1));
    fileRequest->targetSURL = putFileRequests.targetSURL[i];

    DM_LOG(DM_N(2), "targetSURL.size() == %d\n", putFileRequests.targetSURL.size());

    if(NOT_NULL_VEC(putFileRequests,expectedFileSize)) {
      fileRequest->expectedFileSize = putFileRequests.expectedFileSize[i];
      DM_LOG(DM_N(2), "expectedFileSize[%u] = %"PRIi64"\n", i, *(fileRequest->expectedFileSize));
    } else {
      fileRequest->expectedFileSize = NULL;
      DM_LOG(DM_N(2), "expectedFileSize[%u] == NULL\n", i);
    }
    
    req.arrayOfFileRequests->requestArray.push_back(fileRequest);
//             
  }
  /* TODO... */

  /* Transfer parameters */
  NOT_NULL(req.transferParameters = soap_new_srm__TTransferParameters(soap, -1));
  req.transferParameters->accessPattern = (srm__TAccessPattern *)accessPattern;
  req.transferParameters->connectionType = (srm__TConnectionType *)connectionType;
  NOT_NULL(req.transferParameters->arrayOfClientNetworks = soap_new_srm__ArrayOfString(soap, -1));
  NOT_NULL(req.transferParameters->arrayOfTransferProtocols = soap_new_srm__ArrayOfString(soap, -1));

  /* Fill in client networks */
  DM_LOG(DM_N(2), "clientNetworks.size() == %d\n", clientNetworks.size());
  for(uint i = 0; i < clientNetworks.size(); i++) {
    if(clientNetworks[i]) {
      req.transferParameters->arrayOfClientNetworks->stringArray.push_back(clientNetworks[i]->c_str());
      DM_LOG(DM_N(2), "clientNetworks[%i] == `%s'\n", i, clientNetworks[i]->c_str());
    } else {
      DM_LOG(DM_N(2), "clientNetworks[%i] == NULL\n", i);
    }
  }
  /* Fill in transfer protocols */
  DM_LOG(DM_N(2), "transferProtocols.size() == %d\n", transferProtocols.size());
  for(uint i = 0; i < transferProtocols.size(); i++) {
    if(transferProtocols[i]) {
      req.transferParameters->arrayOfTransferProtocols->stringArray.push_back(transferProtocols[i]->c_str());
      DM_LOG(DM_N(2), "transferProtocols[%i] == `%s'\n", i, transferProtocols[i]->c_str());
    } else {
      DM_LOG(DM_N(2), "transferProtocols[%i] == NULL\n", i);
    }
  }
  
  /* To send the request ... */
  SOAP_CALL_SRM(PrepareToPut);

  RETURN(EXIT_SUCCESS);
}
