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
             const char *authorizationID,
             const tArrayOfPutFileRequests fileRequests,
             const char *userRequestDescription,
             const long *overwriteOption,
             tStorageSystemInfo storageSystemInfo,
             int *desiredTotalRequestTime,
             int *desiredPinLifeTime,
             int *desiredFileLifeTime,
             const long *desiredFileStorageType,
             const char *targetSpaceToken,
             const long retentionPolicy,
             const long *accessLatency,
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
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);

  /* Create the file request */
  NOT_NULL(req.arrayOfFileRequests = soap_new_srm__ArrayOfTPutFileRequest(soap, -1));
  for (uint u = 0; u < fileRequests.targetSURL.size(); u++) {
    DM_LOG(DM_N(2), "fileRequests.targetSURL[%u]\n", u);
    srm__TPutFileRequest *fileRequest;
    NOT_NULL(fileRequest = soap_new_srm__TPutFileRequest(soap, -1));

    MV_PSTR2PSTR(fileRequest->targetSURL,fileRequests.targetSURL[u]);
    if(NOT_NULL_VEC(fileRequests,expectedFileSize)) {
      fileRequest->expectedFileSize = fileRequests.expectedFileSize[u];
      DM_LOG(DM_N(2), "expectedFileSize[%u] = %"PRIi64"\n", u, *(fileRequest->expectedFileSize));
    } else {
      fileRequest->expectedFileSize = NULL;
      DM_LOG(DM_N(2), "expectedFileSize[%u] == NULL\n", u);
    }
    
    req.arrayOfFileRequests->requestArray.push_back(fileRequest);
  }

  MV_CSTR2PSTR(req.userRequestDescription,userRequestDescription);
  MV_PSOAP(OverwriteMode,req.overwriteOption,overwriteOption);

  /* Storage system info */
  NOT_NULL(req.storageSystemInfo = soap_new_srm__ArrayOfTExtraInfo(soap, -1));
  for (uint u = 0; u < storageSystemInfo.key.size(); u++) {
    DM_LOG(DM_N(2), "storageSystemInfo.key[%u]\n", u);
    srm__TExtraInfo *extraInfo;

    NOT_NULL(extraInfo = soap_new_srm__TExtraInfo(soap, -1));
    MV_CSTR2STR(extraInfo->key,CSTR(storageSystemInfo.key[u]));
    MV_PSTR2PSTR(extraInfo->value,storageSystemInfo.value[u]);
    req.storageSystemInfo->extraInfoArray.push_back(extraInfo);
  }

  MV_PINT(req.desiredTotalRequestTime,desiredTotalRequestTime);
  MV_PINT(req.desiredPinLifeTime,desiredPinLifeTime);
  MV_PINT(req.desiredFileLifeTime,desiredFileLifeTime);
  MV_PSOAP(FileStorageType,req.desiredFileStorageType,desiredFileStorageType);
  MV_CSTR2PSTR(req.targetSpaceToken,targetSpaceToken);
  
  /* Retention */
  NOT_NULL(req.targetFileRetentionPolicyInfo = soap_new_srm__TRetentionPolicyInfo(soap, -1));
  MV_SOAP(RetentionPolicy,req.targetFileRetentionPolicyInfo->retentionPolicy,retentionPolicy);
  MV_PSOAP(AccessLatency,req.targetFileRetentionPolicyInfo->accessLatency,accessLatency);
  
  /* Transfer parameters */
  NOT_NULL(req.transferParameters = soap_new_srm__TTransferParameters(soap, -1));
  MV_PSOAP(AccessPattern,req.transferParameters->accessPattern,accessPattern);
  MV_PSOAP(ConnectionType,req.transferParameters->connectionType,connectionType);
  NOT_NULL(req.transferParameters->arrayOfClientNetworks = soap_new_srm__ArrayOfString(soap, -1));
  NOT_NULL(req.transferParameters->arrayOfTransferProtocols = soap_new_srm__ArrayOfString(soap, -1));

  /* Fill in client networks */
  for(uint u = 0; u < clientNetworks.size(); u++) {
    DM_LOG(DM_N(2), "clientNetworks[%u]\n", u);
    if(clientNetworks[u]) {
      req.transferParameters->arrayOfClientNetworks->stringArray.push_back(CSTR(clientNetworks[u]));
      DM_LOG(DM_N(2), "clientNetworks[%u] == `%s'\n", u, req.transferParameters->arrayOfClientNetworks->stringArray.back().c_str());
    } else {
      DM_LOG(DM_N(2), "clientNetworks[%u] == NULL\n", u);
    }
  }
  /* Fill in transfer protocols */
  for(uint u = 0; u < transferProtocols.size(); u++) {
    DM_LOG(DM_N(2), "transferProtocols[%u]\n", u);
    if(transferProtocols[u]) {
      req.transferParameters->arrayOfTransferProtocols->stringArray.push_back(CSTR(transferProtocols[u]));
      DM_LOG(DM_N(2), "transferProtocols[%u] == `%s'\n", u, req.transferParameters->arrayOfTransferProtocols->stringArray.back().c_str());
    } else {
      DM_LOG(DM_N(2), "transferProtocols[%u] == NULL\n", u);
    }
  }
  
  /* To send the request ... */
  SOAP_CALL_SRM(PrepareToPut);

  RETURN(EXIT_SUCCESS);
}
