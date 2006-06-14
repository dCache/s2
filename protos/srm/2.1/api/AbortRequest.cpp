/**
 * \file AbortRequest.cpp
 *
 * Implements the SRM2 AbortRequest method.  SRM2 spec p.21.
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
 * srmAbortRequest method.
 *
 * \param srm_endpoint
 * \param userID
 * \param requestToken
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
AbortRequest(const char *srm_endpoint,
             const char *userID,
             const char *requestToken,
             struct srm__srmAbortRequestResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmAbortRequestRequest req;
  struct soap soap;
  soap_init(&soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);
  NEW_STR_VAL(requestToken,TRequestToken);

  /* To send the request ... */
  SOAP_CALL_SRM(AbortRequest);
  
  RETURN(EXIT_SUCCESS);
}
