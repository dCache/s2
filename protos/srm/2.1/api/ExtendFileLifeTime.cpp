/**
 * \file ExtendFileLifeTime.cpp
 *
 * Implements the SRM2 ExtendFileLifeTime method.  SRM2 spec p.23.
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
 * srmExtendFileLifeTime method.
 *
 * \param srm_endpoint
 * \param userID
 * \param requestToken
 * \param siteURL
 * \param newLifeTime
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
ExtendFileLifeTime(const char *srm_endpoint,
                   const char *userID,
                   const char *requestToken,
                   const char *siteURL,
                   const int64_t *newLifeTime,
                   struct srm__srmExtendFileLifeTimeResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmExtendFileLifeTimeRequest req;
  struct soap soap;
  soap_init(&soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);
  NEW_STR_VAL(requestToken,TRequestToken);
  NEW_STR_VAL(siteURL,TSURL);
  NEW_INT64_VAL(newLifeTime,TLifeTimeInSeconds);

  /* To send the request ... */
  SOAP_CALL_SRM(ExtendFileLifeTime);
  
  RETURN(EXIT_SUCCESS);
}
