/**
 * \file GetRequestTokens.cpp
 *
 * Implements the SRM2 GetRequestTokens method.
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
 * srmGetRequestTokens method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param userRequestDescription
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
GetRequestTokens(struct soap *soap,
                 const char *srm_endpoint,
                 const char *authorizationID,
                 const char *userRequestDescription,
                 struct srm__srmGetRequestTokensResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmGetRequestTokensRequest req;

  DO_SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);
  MV_CSTR2PSTR(req.userRequestDescription,userRequestDescription);
 
  /* To send the request ... */
  SOAP_CALL_SRM(GetRequestTokens);

  RETURN(EXIT_SUCCESS);
}
