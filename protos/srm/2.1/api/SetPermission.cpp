/**
 * \file SetPermission.cpp
 *
 * Implements the SRM2 SetPermission method.  SRM2 spec p.13.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

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
 * srmSetPermission method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param SURLOrStFN
 * \param storageSystemInfo
 * \param permissionType
 * \param ownerPermission
 * \param userPermissionArray
 * \param groupPermissionArray
 * \param otherPermission
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
SetPermission(struct soap *soap,
              const char *srm_endpoint,
              const char *userID,
              const char *SURLOrStFN,
              const char *storageSystemInfo,
              const long int permissionType,
              const long int *ownerPermission,
              tPermissionArray userPermissionArray,
              tPermissionArray groupPermissionArray,
              const long int *otherPermission,
              struct srm__srmSetPermissionResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmSetPermissionRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  NEW_STR_VAL(userID,TUserID);

  NOT_NULL(req.path = soap_new_srm__TSURLInfo(soap, -1));
  NEW_STR_VAL_OPT(req.path->SURLOrStFN, SURLOrStFN, TSURL);
  NEW_STR_VAL_OPT(req.path->storageSystemInfo, storageSystemInfo, TStorageSystemInfo);
  req.permissionType = (srm__TPermissionType)permissionType;
  DM_LOG(DM_N(2), "permissionType == `%s'\n", getTPermissionType(req.permissionType).c_str());
  NEW_PERMISSION(ownerPermission, TOwnerPermission);

#define SET_PERMS(ug,UG)\
  if(ug ## PermissionArray.mode.size() != 0) {\
    NOT_NULL(req.ug ## Permission = soap_new_srm__ArrayOfT ## UG ## Permission(soap, -1));\
    DM_LOG(DM_N(2), "" # ug "PermissionArray.mode.size() == %d\n", ug ## PermissionArray.mode.size());\
    for (uint u = 0; u < ug ## PermissionArray.mode.size(); u++) {\
      srm__T ## UG ## Permission *myPermission;\
\
      NOT_NULL(myPermission = soap_new_srm__T ## UG ## Permission(soap, -1));\
      if(NOT_NULL_VEC(ug ## PermissionArray,ID)) {\
        NOT_NULL(myPermission->ug ## ID = soap_new_srm__T ## UG ## ID(soap, -1));\
        myPermission->ug ## ID->value.assign(ug ## PermissionArray.ID[u]->c_str());\
        DM_LOG(DM_N(2), "" # ug "ID[%u] == `%s'\n", u, myPermission->ug ## ID->value.c_str());\
      } else {\
        myPermission->ug ## ID = NULL;\
        DM_LOG(DM_N(2), "" # ug "ID[%u] == NULL\n", u);\
      }\
      myPermission->mode = (srm__TPermissionMode)ug ## PermissionArray.mode[u];\
      DM_LOG(DM_N(2), "mode[%u] == `%s'\n", u, getTPermissionMode(myPermission->mode).c_str());\
\
      req.ug ## Permission->ug ## PermissionArray.push_back(myPermission);\
    }\
  } else {\
    req.ug ## Permission = NULL;\
  }

  SET_PERMS(user,User);
  SET_PERMS(group,Group);
#undef SET_PERMS

  NEW_PERMISSION(otherPermission, TOtherPermission);

  /* To send the request ... */
  SOAP_CALL_SRM(SetPermission);
  
  RETURN(EXIT_SUCCESS);
}
