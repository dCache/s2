/**
 * \file GetSpaceTokens.cpp
 *
 * Implements the SRM2 GetSpaceTokens method.  SRM2 spec p.13.
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

#include "srm_macros.h"

/**
 * srmGetSpaceTokens method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param userSpaceTokenDescription
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
GetSpaceTokens(struct soap *soap,
               const char *srm_endpoint,
               const char *authorizationID,
               const char *userSpaceTokenDescription,
               struct srm__srmGetSpaceTokensResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmGetSpaceTokensRequest req;

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

  /* To send the request ... */
  SOAP_CALL_SRM(GetSpaceTokens);

  RETURN(EXIT_SUCCESS);
}
