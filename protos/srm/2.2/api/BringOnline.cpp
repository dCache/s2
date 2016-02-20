/**
 * \file BringOnLine.cpp
 *
 * Implements the SRM2 BringOnLine method.
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
 * srmBringOnLine method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param fileRequests
 * \param userRequestDescription
 * \param storageSystemInfo
 * \param desiredFileStorageType
 * \param desiredTotalRequestTime
 * \param desiredLifeTime
 * \param targetSpaceToken
 * \param retentionPolicy
 * \param accessLatency
 * \param accessPattern
 * \param connectionType
 * \param clientNetworks
 * \param transferProtocols
 * \param deferredStartTime
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
BringOnline(struct soap *soap,
            const char *srm_endpoint,
            const char *authorizationID,
            const tArrayOfGetFileRequests fileRequests,
            const char *userRequestDescription,
            const tStorageSystemInfo storageSystemInfo,
            const long *desiredFileStorageType,
            int *desiredTotalRequestTime,
            int *desiredLifeTime,
            const char *targetSpaceToken,
            const long *retentionPolicy,
            const long *accessLatency,
            const long *accessPattern,
            const long *connectionType,
            std::vector <std::string *> clientNetworks,
            std::vector <std::string *> transferProtocols,
            int *deferredStartTime,
            struct srm__srmBringOnlineResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmBringOnlineRequest req;

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
  MV_PINT(req.desiredLifeTime,desiredLifeTime);
  MV_CSTR2PSTR(req.targetSpaceToken,targetSpaceToken);

  /* Retention */
  MV_RETENTION_POLICY(req.targetFileRetentionPolicyInfo,retentionPolicy,accessLatency);

  /* Transfer parameters */
  MV_TRANSFER_PARAMETERS(req.transferParameters);

  MV_PINT(req.desiredTotalRequestTime,desiredTotalRequestTime);

  /* To send the request ... */
  SOAP_CALL_SRM(BringOnline); 

  RETURN(EXIT_SUCCESS);
}
