/**
 * \file StatusOfCopyRequest.cpp
 *
 * Implements the SRM2 StatusOfCopyRequest method.
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
 * srmStatusOfCopyRequest method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param requestToken
 * \param sourceSURL
 * \param targetSURL
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
StatusOfCopyRequest(struct soap *soap,
                    const char *srm_endpoint,
                    const char *authorizationID,
                    const char *requestToken,
                    std::vector <std::string *> sourceSURL,
                    std::vector <std::string *> targetSURL,
                    struct srm__srmStatusOfCopyRequestResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmStatusOfCopyRequestRequest req;

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
  
  MV_ARRAY_OF_STR_VAL(req.arrayOfSourceSURLs,sourceSURL,urlArray,AnyURI);
  MV_ARRAY_OF_STR_VAL(req.arrayOfTargetSURLs,targetSURL,urlArray,AnyURI);

  /* To send the request ... */
  SOAP_CALL_SRM(StatusOfCopyRequest);

  RETURN(EXIT_SUCCESS);
}
