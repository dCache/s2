/**
 * \file GetPermission.cpp
 *
 * Implements the SRM2 GetPermission method.
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
 * srmLs method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param SURL
 * \param storageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
GetPermission(struct soap *soap,
              const char *srm_endpoint,
              const char *authorizationID,
              std::vector <std::string *> SURL,
              tStorageSystemInfo storageSystemInfo,
              struct srm__srmGetPermissionResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmGetPermissionRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);
  
  /* SURL */
  MV_ARRAY_OF_STR_VAL(req.arrayOfSURLs,SURL,urlArray,AnyURI);

  /* Storage system info */
  MV_STORAGE_SYSTEM_INFO(req.storageSystemInfo,storageSystemInfo);
  
  /* To send the request ... */
  SOAP_CALL_SRM(GetPermission); 

  RETURN(EXIT_SUCCESS);
}
