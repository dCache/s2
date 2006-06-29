/**
 * \file PrepareToGet.cpp
 *
 * Implements the SRM2 PrepareToGet method.  SRM2 spec p.17.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES
#include "inttypes.h"
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
 * srmPrepareToGet method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
PrepareToGet(struct soap *soap,
             const char *srm_endpoint,
             const char *authorizationID,
             const tArrayOfGetFileRequests fileRequests,
             const char *userRequestDescription,
             const tStorageSystemInfo storageSystemInfo,
             const long *desiredFileStorageType,
             int *desiredTotalRequestTime,
             int *desiredPinLifeTime,
             const char *targetSpaceToken,
             const long retentionPolicy,
             const long *accessLatency,
             const long *accessPattern,
             const long *connectionType,
             std::vector <std::string *> clientNetworks,
             std::vector <std::string *> transferProtocols,
             struct srm__srmPrepareToGetResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmPrepareToGetRequest req;

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
  NOT_NULL(req.arrayOfFileRequests = soap_new_srm__ArrayOfTGetFileRequest(soap, -1));
  for (uint u = 0; u < fileRequests.sourceSURL.size(); u++) {
    DM_LOG(DM_N(2), "fileRequests.sourceSURL[%u]\n", u);
    srm__TGetFileRequest *fileRequest;
    NOT_NULL(fileRequest = soap_new_srm__TGetFileRequest(soap, -1));
    MV_CSTR2STR(fileRequest->sourceSURL,CSTR(fileRequests.sourceSURL[u]));
    NOT_NULL(fileRequest->dirOption = soap_new_srm__TDirOption(soap, -1));
    
    /* dirOption */
    if(NOT_NULL_VEC(fileRequests,isSourceADirectory)) {
      fileRequest->dirOption->isSourceADirectory = fileRequests.isSourceADirectory[u];
      DM_LOG(DM_N(2), "isSourceADirectory[%u] = %d\n", u, fileRequest->dirOption->isSourceADirectory);
    } else {
      fileRequest->dirOption->isSourceADirectory = 0;
      DM_LOG(DM_N(2), "isSourceADirectory[%u] == 0\n", u);
    }
    if(NOT_NULL_VEC(fileRequests,allLevelRecursive)) {
      fileRequest->dirOption->allLevelRecursive = (bool *)fileRequests.allLevelRecursive[u];
      DM_LOG(DM_N(2), "allLevelRecursive[%u] = %d\n", u, *(fileRequest->dirOption->allLevelRecursive));
    } else {
      fileRequest->dirOption->allLevelRecursive = NULL;
      DM_LOG(DM_N(2), "allLevelRecursive[%u] == NULL\n", u);
    }
    if(NOT_NULL_VEC(fileRequests,numOfLevels)) {
      fileRequest->dirOption->numOfLevels = fileRequests.numOfLevels[u];
      DM_LOG(DM_N(2), "numOfLevels[%u] = %d\n", u, *(fileRequest->dirOption->numOfLevels));
    } else {
      fileRequest->dirOption->numOfLevels = NULL;
      DM_LOG(DM_N(2), "numOfLevels[%u] == NULL\n", u);
    }
    req.arrayOfFileRequests->requestArray.push_back(fileRequest);
  }

  MV_CSTR2PSTR(req.userRequestDescription,userRequestDescription);

  /* Storage system info */
  MV_STORAGE_SYSTEM_INFO(req.storageSystemInfo,storageSystemInfo);

  MV_PSOAP(FileStorageType,req.desiredFileStorageType,desiredFileStorageType);
  MV_PINT(req.desiredTotalRequestTime,desiredTotalRequestTime);
  MV_PINT(req.desiredPinLifeTime,desiredPinLifeTime);
  MV_CSTR2PSTR(req.targetSpaceToken,targetSpaceToken);

  /* Retention */
  NOT_NULL(req.targetFileRetentionPolicyInfo = soap_new_srm__TRetentionPolicyInfo(soap, -1));
  MV_SOAP(RetentionPolicy,req.targetFileRetentionPolicyInfo->retentionPolicy,retentionPolicy);
  MV_PSOAP(AccessLatency,req.targetFileRetentionPolicyInfo->accessLatency,accessLatency);

  /* Transfer parameters */
  MV_TRANSFER_PARAMETERS(req.transferParameters);
  
  /* To send the request ... */
  SOAP_CALL_SRM(PrepareToGet); 

  RETURN(EXIT_SUCCESS);
}