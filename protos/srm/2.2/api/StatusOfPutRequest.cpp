/**
 * \file StatusOfPutRequest.cpp
 *
 * Implements the SRM2 StatusOfPutRequest method.
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
#include "srm_soap27.h"

/**
 * srmStatusOfPutRequest method.
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
StatusOfPutRequest(struct soap *soap,
                   const char *srm_endpoint,
                   const char *authorizationID,
                   const char *requestToken,
                   std::vector <std::string *> SURL,
                   struct srm__srmStatusOfPutRequestResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmStatusOfPutRequestRequest req;

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
  MV_CSTR2STR(req.requestToken,requestToken);
  
  MV_ARRAY_OF_STR_VAL(req.arrayOfTargetSURLs,SURL,urlArray,AnyURI);

  /* To send the request ... */
  SOAP_CALL_SRM(StatusOfPutRequest);

  RETURN(EXIT_SUCCESS);
}
