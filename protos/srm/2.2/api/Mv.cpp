/**
 * \file Mv.cpp
 *
 * Implements the SRM2 Mv method.
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
 * srmMv method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param fromSURL
 * \param toSURL
 * \param storageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
Mv(struct soap *soap,
   const char *srm_endpoint,
   const char *authorizationID,
   const char *fromSURL,
   const char *toSURL,
   const tStorageSystemInfo storageSystemInfo,
   struct srm__srmMvResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmMvRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);

  /* SURLs */
  MV_CSTR2STR(req.fromSURL,fromSURL);
  MV_CSTR2STR(req.toSURL,toSURL);

  /* Storage system info */
  MV_STORAGE_SYSTEM_INFO(req.storageSystemInfo,storageSystemInfo);

  /* To send the request ... */
  SOAP_CALL_SRM(Mv);
  
  RETURN(EXIT_SUCCESS);
}
