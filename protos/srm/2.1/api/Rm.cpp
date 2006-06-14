/**
 * \file Rm.cpp
 *
 * Implements the SRM2 Rm method.  SRM2 spec p.15.
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
 * srmRm method.
 *
 * \param srm_endpoint
 * \param userID
 * \param path
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
Rm(const char *srm_endpoint,
   const char *userID,
   const tSurlInfoArray path,
   struct srm__srmRmResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmRmRequest req;
  struct soap soap;
  soap_init(&soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);

  NOT_NULL(req.arrayOfFilePaths = soap_new_srm__ArrayOfTSURLInfo(&soap, -1));
  ARRAY_OF_TSURL_INFO(arrayOfFilePaths);

  /* To send the request ... */
  SOAP_CALL_SRM(Rm); 

  RETURN(EXIT_SUCCESS);
}
