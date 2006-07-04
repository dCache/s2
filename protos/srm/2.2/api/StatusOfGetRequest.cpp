/**
 * \file StatusOfGetRequest.cpp
 *
 * Implements the SRM2 StatusOfGetRequest method.
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
 * srmStatusOfGetRequest method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param requestToken
 * \param SURL
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
StatusOfGetRequest(struct soap *soap,
                   const char *srm_endpoint,
                   const char *authorizationID,
                   const char *requestToken,
                   std::vector <std::string *> SURL,
                   struct srm__srmStatusOfGetRequestResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmStatusOfGetRequestRequest req;

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
  MV_CSTR2STR(req.requestToken,requestToken);
  
  MV_ARRAY_OF_STR_VAL(req.arrayOfSourceSURLs,SURL,urlArray,AnyURI);

  /* To send the request ... */
  SOAP_CALL_SRM(StatusOfGetRequest);

  RETURN(EXIT_SUCCESS);
}
