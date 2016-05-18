/**
 * \file ReleaseSpace.cpp
 *
 * Implements the SRM2 ReleaseSpace method.
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

/**
 * srmReleaseSpace method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param spaceToken
 * \param storageSystemInfo
 * \param forceFileRelease
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
ReleaseSpace(struct soap *soap,
             const char *srm_endpoint,
             const char *authorizationID,
             const char *spaceToken,
             tStorageSystemInfo storageSystemInfo,
             bool *forceFileRelease,
             struct srm__srmReleaseSpaceResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmReleaseSpaceRequest req;

  DO_SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);
  MV_CSTR2STR(req.spaceToken,spaceToken);

  /* Storage system info */
  MV_STORAGE_SYSTEM_INFO(req.storageSystemInfo,storageSystemInfo);

  MV_PBOOL(req.forceFileRelease,forceFileRelease);

  /* To send the request ... */
  SOAP_CALL_SRM(ReleaseSpace);
  
  RETURN(EXIT_SUCCESS);
}
