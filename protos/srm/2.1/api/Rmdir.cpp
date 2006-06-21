/**
 * \file Rmdir.cpp
 *
 * Implements the SRM2 Rmdir method.  SRM2 spec p.16.
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
 * srmRmdir method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param SURLOrStFN
 * \param storageSystemInfo
 * \param recursive
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
Rmdir(struct soap *soap,
      const char *srm_endpoint,
      const char *userID,
      const char *SURLOrStFN,
      const char *storageSystemInfo,
      bool *recursive, /* yes, no const in gsoap headers */
      struct srm__srmRmdirResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmRmdirRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  NEW_STR_VAL(userID,TUserID);

  NOT_NULL(req.directoryPath = soap_new_srm__TSURLInfo(soap, -1));
  NEW_STR_VAL_OPT(req.directoryPath->SURLOrStFN, SURLOrStFN, TSURL);
  NEW_STR_VAL_OPT(req.directoryPath->storageSystemInfo, storageSystemInfo, TStorageSystemInfo);
  PBOOL_VAL(recursive);

  /* To send the request ... */
  SOAP_CALL_SRM(Rmdir);
  
  RETURN(EXIT_SUCCESS);
}
