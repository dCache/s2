/**
 * \file SetPermission.cpp
 *
 * Implements the SRM2 SetPermission method.
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
 * srmSetPermission method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param SURL
 * \param permissionType
 * \param ownerPermission
 * \param userPermissions
 * \param groupPermissions
 * \param otherPermission
 * \param storageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
SetPermission(struct soap *soap,
              const char *srm_endpoint,
              const char *authorizationID,
              const char *SURL,
              const long int permissionType,
              const long int *ownerPermission,
              tPermissionArray userPermissions,
              tPermissionArray groupPermissions,
              const long int *otherPermission,
              tStorageSystemInfo storageSystemInfo,
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

  MV_CSTR2PSTR(req.authorizationID,authorizationID);
  MV_CSTR2STR(req.SURL,SURL);
  MV_SOAP(PermissionType,req.permissionType,permissionType);
  MV_PSOAP(PermissionMode,req.ownerPermission,ownerPermission);

#define SET_PERMS(ug,UG)\
  NOT_0(ug ## Permissions.mode,req.arrayOf ## UG ## Permissions,soap_new_srm__ArrayOfT ## UG ## Permission(soap, -1));\
  DM_LOG(DM_N(2), "arrayOf" # UG "Permissions.mode.size() == %d\n", ug ## Permissions.mode.size());\
  for (uint u = 0; u < ug ## Permissions.mode.size(); u++) {\
    srm__T ## UG ## Permission *myPermission;\
\
    NOT_NULL(myPermission = soap_new_srm__T ## UG ## Permission(soap, -1));\
    if(NOT_NULL_VEC(ug ## Permissions,ID)) {\
      myPermission->ug ## ID.assign(ug ## Permissions.ID[u]->c_str());\
      DM_LOG(DM_N(2), "" # ug "ID[%u] == `%s'\n", u, myPermission->ug ## ID.c_str());\
    } else {\
      DM_LOG(DM_N(2), "" # ug "ID[%u] == NUL\n", u);\
    }\
    myPermission->mode = (srm__TPermissionMode)ug ## Permissions.mode[u];\
    DM_LOG(DM_N(2), "mode[%u] == `%s'\n", u, getTPermissionMode(myPermission->mode).c_str());\
\
    req.arrayOf ## UG ## Permissions->ug ## PermissionArray.push_back(myPermission);\
  }\

  SET_PERMS(user,User);
  SET_PERMS(group,Group);
#undef SET_PERMS

  MV_PSOAP(PermissionMode,req.otherPermission,otherPermission);

  /* Storage system info */
  MV_STORAGE_SYSTEM_INFO(req.storageSystemInfo,storageSystemInfo);

  /* To send the request ... */
  SOAP_CALL_SRM(SetPermission);
  
  RETURN(EXIT_SUCCESS);
}
