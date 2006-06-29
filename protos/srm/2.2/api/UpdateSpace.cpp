/**
 * \file UpdateSpace.cpp
 *
 * Implements the SRM2 UpdateSpace method.  SRM2 spec p.11.
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

/**
 * srmUpdateSpace method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param spaceToken
 * \param newSizeOfTotalSpaceDesired
 * \param newSizeOfGuaranteedSpaceDesired
 * \param newLifeTime
 * \param storageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
UpdateSpace(struct soap *soap,
            const char *srm_endpoint,
            const char *authorizationID,
            const char *spaceToken,
            uint64_t *newSizeOfTotalSpaceDesired,
            uint64_t *newSizeOfGuaranteedSpaceDesired,
            int *newLifeTime,
            tStorageSystemInfo storageSystemInfo,
            struct srm__srmUpdateSpaceResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmUpdateSpaceRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);
  MV_CSTR2STR(req.spaceToken,spaceToken);
  MV_PUINT64(req.newSizeOfTotalSpaceDesired,newSizeOfTotalSpaceDesired);
  MV_PUINT64(req.newSizeOfGuaranteedSpaceDesired,newSizeOfGuaranteedSpaceDesired);
  MV_PINT(req.newLifeTime,newLifeTime);

  /* Storage system info */
  MV_STORAGE_SYSTEM_INFO(req.storageSystemInfo,storageSystemInfo);

  /* To send the request ... */
  SOAP_CALL_SRM(UpdateSpace);
  
  RETURN(EXIT_SUCCESS);
}
