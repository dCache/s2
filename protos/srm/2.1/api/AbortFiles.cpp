/**
 * \file AbortFiles.cpp
 *
 * Implements the SRM2 AbortFiles method.  SRM2 spec p.20.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h> /* Port to SL4 */

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
 * srmAbortFiles method.
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
AbortFiles(struct soap *soap,
           const char *srm_endpoint,
           const char *userID,
           const char *requestToken,
           std::vector <std::string *> arrayOfSiteURL,
           struct srm__srmAbortFilesResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmAbortFilesRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  NEW_STR_VAL(userID,TUserID);
  NEW_STR_VAL(requestToken,TRequestToken);
  NEW_ARRAY_OF_STR_VAL(arrayOfSiteURLs,arrayOfSiteURL,surlArray,TSURL);

  /* To send the request ... */
  SOAP_CALL_SRM(AbortFiles);
  
  RETURN(EXIT_SUCCESS);
}
