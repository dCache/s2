/**
 * \file StatusOfGetRequest.cpp
 *
 * Implements the SRM2 StatusOfGetRequest method.  SRM2 spec p.21.
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
 * srmStatusOfGetRequest method.
 *
 * \param srm_endpoint
 * \param userID
 * \param requestToken
 * \param arrayOfFromSURLs
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
StatusOfGetRequest(const char *srm_endpoint,
                   const char *userID,
                   const char *requestToken,
                   std::vector <std::string *> arrayOfFromSURLs,
                   struct srm__srmStatusOfGetRequestResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmStatusOfGetRequestRequest req;
  struct soap soap;
  soap_init(&soap);
  
#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);
  NEW_STR_VAL(requestToken,TRequestToken);
  NEW_ARRAY_OF_STR_VAL(arrayOfFromSURLs,arrayOfFromSURLs,surlArray,TSURL);

  /* To send the request ... */
  SOAP_CALL_SRM(StatusOfGetRequest); 

  RETURN(EXIT_SUCCESS);
}
