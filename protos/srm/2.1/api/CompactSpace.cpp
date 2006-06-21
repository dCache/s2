/**
 * \file CompactSpace.cpp
 *
 * Implements the SRM2 CompactSpace method.  SRM2 spec p.12.
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
 * srmCompactSpace method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param spaceToken
 * \param storageSystemInfo
 * \param doDynamicCompactFromNowOn
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
CompactSpace(struct soap *soap,
             const char *srm_endpoint,
             const char *userID,
             const char *spaceToken,
             const char *storageSystemInfo,
             bool *doDynamicCompactFromNowOn,
             struct srm__srmCompactSpaceResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmCompactSpaceRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  NEW_STR_VAL(userID,TUserID);
  NEW_STR_VAL(spaceToken,TSpaceToken);
  NEW_STR_VAL(storageSystemInfo,TStorageSystemInfo);
  PBOOL_VAL(doDynamicCompactFromNowOn);

  /* To send the request ... */
  SOAP_CALL_SRM(CompactSpace);

  RETURN(EXIT_SUCCESS);
}
