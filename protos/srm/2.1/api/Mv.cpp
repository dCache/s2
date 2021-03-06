/**
 * \file Mv.cpp
 *
 * Implements the SRM2 Mv method.  SRM2 spec p.16.
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

/**
 * srmMv method.
 *
 * \param soap
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
Mv(struct soap *soap,
   const char *srm_endpoint,
   const char *userID,
   const char *fromSURLOrStFN,
   const char *fromStorageSystemInfo,
   const char *toSURLOrStFN,
   const char *toStorageSystemInfo,
   struct srm__srmMvResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmMvRequest req;

  DO_SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  NEW_STR_VAL(userID,TUserID);

  NOT_NULL(req.fromPath = soap_new_srm__TSURLInfo(soap, -1));
  NEW_STR_VAL_OPT(req.fromPath->SURLOrStFN, fromSURLOrStFN, TSURL);
  NEW_STR_VAL_OPT(req.fromPath->storageSystemInfo, fromStorageSystemInfo, TStorageSystemInfo);

  NOT_NULL(req.toPath = soap_new_srm__TSURLInfo(soap, -1));
  NEW_STR_VAL_OPT(req.toPath->SURLOrStFN, toSURLOrStFN, TSURL);
  NEW_STR_VAL_OPT(req.toPath->storageSystemInfo, toStorageSystemInfo, TStorageSystemInfo);

  /* To send the request ... */
  SOAP_CALL_SRM(Mv);
  
  RETURN(EXIT_SUCCESS);
}
