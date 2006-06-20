/**
 * \file GetRequestSummary.cpp
 *
 * Implements the SRM2 GetRequestSummary method.  SRM2 spec p.22.
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
 * srmGetRequestSummary method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param arrayOfRequestToken
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
GetRequestSummary(struct soap *soap,
                  const char *srm_endpoint,
                  const char *userID,
                  std::vector <std::string *> arrayOfRequestToken,
                  struct srm__srmGetRequestSummaryResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmGetRequestSummaryRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);
  NEW_ARRAY_OF_STR_VAL(arrayOfRequestTokens,arrayOfRequestToken,requestTokenArray,TRequestToken);
 
  /* To send the request ... */
  SOAP_CALL_SRM(GetRequestSummary);

  RETURN(EXIT_SUCCESS);
}
