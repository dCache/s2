/**
 * \file ReassignToUser.cpp
 *
 * Implements the SRM2 ReassignToUser method.  SRM2 spec p.14.
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
 * srmReassignToUser method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param assignedUser
 * \param lifeTimeOfThisAssignment
 * \param SURLOrStFN
 * \param storageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
ReassignToUser(struct soap *soap,
               const char *srm_endpoint,
               const char *userID,
               const char *assignedUser,
               const int64_t *lifeTimeOfThisAssignment,
               const char *SURLOrStFN,
               const char *storageSystemInfo,
               struct srm__srmReassignToUserResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmReassignToUserRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  NEW_STR_VAL(userID,TUserID);
  NEW_STR_VAL(assignedUser,TUserID);
  NEW_INT64_VAL(lifeTimeOfThisAssignment,TLifeTimeInSeconds);

  NOT_NULL(req.path = soap_new_srm__TSURLInfo(soap, -1));
  NEW_STR_VAL_OPT(req.path->SURLOrStFN, SURLOrStFN, TSURL);
  NEW_STR_VAL_OPT(req.path->storageSystemInfo, storageSystemInfo, TStorageSystemInfo);

  /* To send the request ... */
  SOAP_CALL_SRM(ReassignToUser);
  
  RETURN(EXIT_SUCCESS);
}
