/**
 * \file UpdateSpace.cpp
 *
 * Implements the SRM2 UpdateSpace method.  SRM2 spec p.11.
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
 * srmUpdateSpace method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param spaceToken
 * \param newSizeOfTotalSpaceDesired
 * \param newSizeOfGuaranteedSpaceDesired
 * \param newLifeTimeFromCallingTime
 * \param storageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
UpdateSpace(struct soap *soap,
            const char *srm_endpoint,
            const char *userID,
            const char *spaceToken,
            const int64_t *newSizeOfTotalSpaceDesired,
            const int64_t *newSizeOfGuaranteedSpaceDesired,
            const int64_t *newLifeTimeFromCallingTime,
            const char *storageSystemInfo,
            struct srm__srmUpdateSpaceResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmUpdateSpaceRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);
  NEW_STR_VAL(spaceToken,TSpaceToken);
  NEW_STR_VAL(storageSystemInfo,TStorageSystemInfo);
  NEW_INT64_VAL(newSizeOfTotalSpaceDesired,TSizeInBytes);
  NEW_INT64_VAL(newSizeOfGuaranteedSpaceDesired,TSizeInBytes);
  NEW_INT64_VAL(newLifeTimeFromCallingTime,TLifeTimeInSeconds);

  /* To send the request ... */
  SOAP_CALL_SRM(UpdateSpace);
  
  RETURN(EXIT_SUCCESS);
}
