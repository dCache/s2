/**
 * \file PrepareToPut.cpp
 *
 * Implements the SRM2 PrepareToPut method.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

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
 * \param fileRequests
 * \param userRequestDescription
 * \param overwriteOption
 * \param storageSystemInfo
 * \param desiredTotalRequestTime
 * \param desiredPinLifeTime
 * \param desiredFileLifeTime
 * \param desiredFileStorageType
 * \param targetSpaceToken
 * \param retentionPolicy
 * \param accessLatency
 * \param accessPattern
 * \param connectionType
 * \param clientNetworks
 * \param transferProtocols
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
             const long *retentionPolicy,
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
  NOT_0(fileRequests.SURL, req.arrayOfFileRequests, soap_new_srm__ArrayOfTPutFileRequest(soap, -1));
  for (uint u = 0; u < fileRequests.SURL.size(); u++) {
    DM_LOG(DM_N(2), "fileRequests.SURL[%u]\n", u);
    srm__TPutFileRequest *fileRequest;
    NOT_NULL(fileRequest = soap_new_srm__TPutFileRequest(soap, -1));

    MV_PSTR2PSTR(fileRequest->targetSURL,fileRequests.SURL[u]);
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
  MV_STORAGE_SYSTEM_INFO(req.storageSystemInfo,storageSystemInfo);

  MV_PINT(req.desiredTotalRequestTime,desiredTotalRequestTime);
  MV_PINT(req.desiredPinLifeTime,desiredPinLifeTime);
  MV_PINT(req.desiredFileLifeTime,desiredFileLifeTime);
  MV_PSOAP(FileStorageType,req.desiredFileStorageType,desiredFileStorageType);
  MV_CSTR2PSTR(req.targetSpaceToken,targetSpaceToken);
  
  /* Retention */
  MV_RETENTION_POLICY(req.targetFileRetentionPolicyInfo,retentionPolicy,accessLatency);
  
  /* Transfer parameters */
  MV_TRANSFER_PARAMETERS(req.transferParameters);
  
  /* To send the request ... */
  SOAP_CALL_SRM(PrepareToPut);

  RETURN(EXIT_SUCCESS);
}
