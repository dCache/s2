/**
 * \file GetTransferProtocols.cpp
 *
 * Implements the SRM2 GetTransferProtocols method.
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
 * srmGetTransferProtocols method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
GetTransferProtocols(struct soap *soap,
                     const char *srm_endpoint,
                     const char *authorizationID,
                     struct srm__srmGetTransferProtocolsResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmGetTransferProtocolsRequest req;

  DO_SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);
 
  /* To send the request ... */
  SOAP_CALL_SRM(GetTransferProtocols);

  RETURN(EXIT_SUCCESS);
}
