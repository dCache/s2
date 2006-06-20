/**
 * \file GetSpaceToken.cpp
 *
 * Implements the SRM2 GetSpaceToken method.  SRM2 spec p.13.
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
 * srmGetSpaceToken method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param userSpaceTokenDescription
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
GetSpaceToken(struct soap *soap,
              const char *srm_endpoint,
              const char *userID,
              const char *userSpaceTokenDescription,
              struct srm__srmGetSpaceTokenResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmGetSpaceTokenRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);

  /* inconsitency with ReserveSpace! */
  if(userSpaceTokenDescription) {
    req.userSpaceTokenDescription.assign(userSpaceTokenDescription);
    DM_LOG(DM_N(2), "userSpaceTokenDescription == `%s'\n", req.userSpaceTokenDescription.c_str());\
  } else {
    /* initialise userSpaceTokenDescription to an empty string (required element) */
    req.userSpaceTokenDescription = "";
    DM_LOG(DM_N(2), "userSpaceTokenDescription == `'\n");
  }

  /* To send the request ... */
  SOAP_CALL_SRM(GetSpaceToken);

  RETURN(EXIT_SUCCESS);
}
