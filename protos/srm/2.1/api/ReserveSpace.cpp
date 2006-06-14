/**
 * \file ReserveSpace.cpp
 *
 * Implements the SRM2 ReserveSpace method.  SRM2 spec p.10.
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
#include "srm_soap27.h"

/**
 * srmReserveSpace method.
 *
 * \param srm_endpoint
 * \param userID
 * \param typeOfSpace
 * \param userSpaceTokenDescription
 * \param sizeOfTotalSpaceDesired
 * \param sizeOfGuaranteedSpaceDesired
 * \param lifetimeOfSpaceToReserve
 * \param storageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
ReserveSpace(const char *srm_endpoint,
             const char *userID,
             const long int typeOfSpace,
             const char *userSpaceTokenDescription,
             const int64_t *sizeOfTotalSpaceDesired,
             const int64_t *sizeOfGuaranteedSpaceDesired,
             const int64_t *lifetimeOfSpaceToReserve,
             const char *storageSystemInfo,
             struct srm__srmReserveSpaceResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmReserveSpaceRequest req;
  struct soap soap;
  soap_init(&soap);
  
#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);

  req.typeOfSpace = (srm__TSpaceType)typeOfSpace;
  DM_LOG(DM_N(2), "typeOfSpace == `%s'\n", getTSpaceType(req.typeOfSpace).c_str());

  NEW_STDSTRING(userSpaceTokenDescription);             /* inconsistency with GetSpaceToken */
  NEW_INT64_VAL(sizeOfTotalSpaceDesired,TSizeInBytes);
  NEW_INT64_VAL(sizeOfGuaranteedSpaceDesired,TSizeInBytes);
  NEW_INT64_VAL(lifetimeOfSpaceToReserve,TLifeTimeInSeconds);
  NEW_STR_VAL(storageSystemInfo,TStorageSystemInfo);

  /* To send the request ... */
  SOAP_CALL_SRM(ReserveSpace); 

  RETURN(EXIT_SUCCESS);
}
