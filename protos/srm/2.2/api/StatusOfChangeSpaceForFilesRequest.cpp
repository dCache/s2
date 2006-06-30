/**
 * \file StatusOfChangeSpaceForFilesRequest.cpp
 *
 * Implements the SRM2 StatusOfChangeSpaceForFilesRequest method.  SRM2 spec p.12.
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
 * srmStatusOfChangeSpaceForFilesRequest method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param spaceToken
 * \param storageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
StatusOfChangeSpaceForFilesRequest(struct soap *soap,
                                   const char *srm_endpoint,
                                   const char *authorizationID,
                                   const char *requestToken,
                                   struct srm__srmStatusOfChangeSpaceForFilesRequestResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmStatusOfChangeSpaceForFilesRequestRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);
  MV_CSTR2STR(req.requestToken,requestToken);

  /* To send the request ... */
  SOAP_CALL_SRM(StatusOfChangeSpaceForFilesRequest);
  
  RETURN(EXIT_SUCCESS);
}
