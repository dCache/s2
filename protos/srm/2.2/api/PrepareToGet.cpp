/**
 * \file PrepareToGet.cpp
 *
 * Implements the SRM2 PrepareToGet method.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

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
 * \param fileRequests
 * \param userRequestDescription
 * \param storageSystemInfo
 * \param desiredFileStorageType
 * \param desiredTotalRequestTime
 * \param desiredPinLifeTime
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
             const long *retentionPolicy,
             const long *accessLatency,
             const long *accessPattern,
             const long *connectionType,
             std::vector <std::string *> clientNetworks,
             std::vector <std::string *> transferProtocols,
             struct srm__srmPrepareToGetResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmPrepareToGetRequest req;

  DO_SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);

  /* Create the file request */
  NOT_0(fileRequests.SURL,req.arrayOfFileRequests,soap_new_srm__ArrayOfTGetFileRequest(soap, -1));
  for (uint u = 0; u < fileRequests.SURL.size(); u++) {
    DM_LOG(DM_N(2), "fileRequests.SURL[%u]\n", u);
    srm__TGetFileRequest *fileRequest;
    NOT_NULL(fileRequest = soap_new_srm__TGetFileRequest(soap, -1));
    MV_CSTR2STR(fileRequest->sourceSURL,CSTR(fileRequests.SURL[u]));
    MV_DIR_OPTION_VEC(fileRequest);
    req.arrayOfFileRequests->requestArray.push_back(fileRequest);
  } /* for */

  MV_CSTR2PSTR(req.userRequestDescription,userRequestDescription);

  /* Storage system info */
  MV_STORAGE_SYSTEM_INFO(req.storageSystemInfo,storageSystemInfo);

  MV_PSOAP(FileStorageType,req.desiredFileStorageType,desiredFileStorageType);
  MV_PINT(req.desiredTotalRequestTime,desiredTotalRequestTime);
  MV_PINT(req.desiredPinLifeTime,desiredPinLifeTime);
  MV_CSTR2PSTR(req.targetSpaceToken,targetSpaceToken);

  /* Retention */
  MV_RETENTION_POLICY(req.targetFileRetentionPolicyInfo,retentionPolicy,accessLatency);

  /* Transfer parameters */
  MV_TRANSFER_PARAMETERS(req.transferParameters);
  
  /* To send the request ... */
  SOAP_CALL_SRM(PrepareToGet); 

  RETURN(EXIT_SUCCESS);
}
