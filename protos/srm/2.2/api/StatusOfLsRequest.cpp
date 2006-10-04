/**
 * \file StatusOfLsRequest.cpp
 *
 * Implements the SRM2 StatusOfLsRequest method.
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
 * srmStatusOfLsRequest method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param requestToken
 * \param offset
 * \param count
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
StatusOfLsRequest(struct soap *soap,
                  const char *srm_endpoint,
                  const char *authorizationID,
                  const char *requestToken,
                  int *offset,
                  int *count,
                  struct srm__srmStatusOfLsRequestResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmStatusOfLsRequestRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);

  MV_CSTR2STR(req.requestToken,requestToken);
  MV_PINT(req.offset,offset);
  MV_PINT(req.count,count);

  /* To send the request ... */
  SOAP_CALL_SRM(StatusOfLsRequest); 

  RETURN(EXIT_SUCCESS);
}
