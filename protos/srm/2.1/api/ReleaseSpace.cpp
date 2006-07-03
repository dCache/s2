/**
 * \file ReleaseSpace.cpp
 *
 * Implements the SRM2 ReleaseSpace method.  SRM2 spec p.12.
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
 * srmReleaseSpace method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param spaceToken
 * \param storageSystemInfo
 * \param forceFileRelease
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
ReleaseSpace(struct soap *soap,
             const char *srm_endpoint,
             const char *userID,
             const char *spaceToken,
             const char *storageSystemInfo,
             bool *forceFileRelease,
             struct srm__srmReleaseSpaceResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmReleaseSpaceRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  NEW_STR_VAL(userID,TUserID);
  NEW_STR_VAL(spaceToken,TSpaceToken);
  NEW_STR_VAL(storageSystemInfo,TStorageSystemInfo);
  PBOOL_VAL(forceFileRelease);

  /* To send the request ... */
  SOAP_CALL_SRM(ReleaseSpace);
  
  RETURN(EXIT_SUCCESS);
}
