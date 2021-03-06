/**
 * \file Mkdir.cpp
 *
 * Implements the SRM2 Mkdir method.  SRM2 spec p.15.
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
 * srmMkdir method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param SURLOrStFN
 * \param storageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
Mkdir(struct soap *soap,
      const char *srm_endpoint,
      const char *userID,
      const char *SURLOrStFN,
      const char *storageSystemInfo,
      struct srm__srmMkdirResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmMkdirRequest req;

  DO_SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  NEW_STR_VAL(userID,TUserID);

  NOT_NULL(req.directoryPath = soap_new_srm__TSURLInfo(soap, -1));
  NEW_STR_VAL_OPT(req.directoryPath->SURLOrStFN, SURLOrStFN, TSURL);
  NEW_STR_VAL_OPT(req.directoryPath->storageSystemInfo, storageSystemInfo, TStorageSystemInfo);

  /* To send the request ... */
  SOAP_CALL_SRM(Mkdir);
  
  RETURN(EXIT_SUCCESS);
}
