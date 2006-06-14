/**
 * \file Mv.cpp
 *
 * Implements the SRM2 Mv method.  SRM2 spec p.16.
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
 * srmMv method.
 *
 * \param srm_endpoint
 * \param userID
 * \param fromSURLOrStFN
 * \param fromStorageSystemInfo
 * \param toSURLOrStFN
 * \param toStorageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
Mv(const char *srm_endpoint,
   const char *userID,
   const char *fromSURLOrStFN,
   const char *fromStorageSystemInfo,
   const char *toSURLOrStFN,
   const char *toStorageSystemInfo,
   struct srm__srmMvResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmMvRequest req;
  struct soap soap;
  soap_init(&soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);

  NOT_NULL(req.fromPath = soap_new_srm__TSURLInfo(&soap, -1));
  NEW_STR_VAL_OPT(req.fromPath->SURLOrStFN, fromSURLOrStFN, TSURL);
  NEW_STR_VAL_OPT(req.fromPath->storageSystemInfo, fromStorageSystemInfo, TStorageSystemInfo);

  NOT_NULL(req.toPath = soap_new_srm__TSURLInfo(&soap, -1));
  NEW_STR_VAL_OPT(req.toPath->SURLOrStFN, toSURLOrStFN, TSURL);
  NEW_STR_VAL_OPT(req.toPath->storageSystemInfo, toStorageSystemInfo, TStorageSystemInfo);

  /* To send the request ... */
  SOAP_CALL_SRM(Mv);
  
  RETURN(EXIT_SUCCESS);
}
