/**
 * \file PutDone.cpp
 *
 * Implements the SRM2 PutDone method.  SRM2 spec p.20.
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
 * srmPutDone method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param requestToken
 * \param arrayOfSiteURL
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
PutDone(struct soap *soap,
        const char *srm_endpoint,
        const char *authorizationID,
        const char *requestToken,
        std::vector <std::string *> SURL,
        struct srm__srmPutDoneResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmPutDoneRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);

  /* Create the file request */
  MV_CSTR2STR(req.requestToken,requestToken);

  MV_ARRAY_OF_STR_VAL(req.arrayOfSURLs,SURL,urlArray,AnyURI);

  /* To send the request ... */
  SOAP_CALL_SRM(PutDone);
  
  RETURN(EXIT_SUCCESS);
}
