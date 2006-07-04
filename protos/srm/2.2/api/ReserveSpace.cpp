/**
 * \file ReserveSpace.cpp
 *
 * Implements the SRM2 ReserveSpace method.
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
 * srmReserveSpace method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param userSpaceTokenDescription
 * \param retentionPolicy
 * \param accessLatency
 * \param desiredSizeOfTotalSpace
 * \param desiredSizeOfGuaranteedSpace
 * \param desiredLifetimeOfReservedSpace
 * \param arrayOfExpectedFileSizes
 * \param storageSystemInfo
 * \param accessPattern
 * \param connectionType
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
ReserveSpace(struct soap *soap,
             const char *srm_endpoint,
             const char *authorizationID,
             const char *userSpaceTokenDescription,
             const long retentionPolicy,
             const long *accessLatency,
             uint64_t *desiredSizeOfTotalSpace,
             const uint64_t desiredSizeOfGuaranteedSpace,
             int *desiredLifetimeOfReservedSpace,
             std::vector <uint64_t> arrayOfExpectedFileSizes,
             tStorageSystemInfo storageSystemInfo,
             const long *accessPattern,
             const long *connectionType,
             struct srm__srmReserveSpaceResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmReserveSpaceRequest req;

  SOAP_INIT(soap);
  
#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);
  
  MV_CSTR2PSTR(req.userSpaceTokenDescription,userSpaceTokenDescription);

  /* Retention */
  NOT_NULL(req.retentionPolicyInfo = soap_new_srm__TRetentionPolicyInfo(soap, -1));
  MV_SOAP(RetentionPolicy,req.retentionPolicyInfo->retentionPolicy,retentionPolicy);
  MV_PSOAP(AccessLatency,req.retentionPolicyInfo->accessLatency,accessLatency);
  
  MV_PUINT64(req.desiredSizeOfTotalSpace,desiredSizeOfTotalSpace);
  MV_UINT64(req.desiredSizeOfGuaranteedSpace,desiredSizeOfGuaranteedSpace);
  MV_PINT(req.desiredLifetimeOfReservedSpace,desiredLifetimeOfReservedSpace);

  /* arrayOfExpectedFileSizes */
  NOT_NULL(req.arrayOfExpectedFileSizes = soap_new_srm__ArrayOfUnsignedLong(soap, -1));
  for(uint u; u < arrayOfExpectedFileSizes.size(); u++) {
    req.arrayOfExpectedFileSizes->unsignedLongArray.push_back(arrayOfExpectedFileSizes[u]);
    DM_LOG(DM_N(2), "arrayOfExpectedFileSizes[%"PRIi64"]\n", req.arrayOfExpectedFileSizes->unsignedLongArray.back());
  }
  
  /* Storage system info */
  MV_STORAGE_SYSTEM_INFO(req.storageSystemInfo,storageSystemInfo);
  
  MV_PSOAP(AccessPattern,req.transferParameters->accessPattern,accessPattern);
  MV_PSOAP(ConnectionType,req.transferParameters->connectionType,connectionType);

  /* To send the request ... */
  SOAP_CALL_SRM(ReserveSpace); 

  RETURN(EXIT_SUCCESS);
}
