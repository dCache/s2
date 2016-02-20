/**
 * \file ChangeFileStorageType.cpp
 *
 * Implements the SRM2 ChangeFileStorageType method.  SRM2 spec p.12.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h> /* Port to SL4 */

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
 * srmChangeFileStorageType method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param path
 * \param desiredStorageType
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
ChangeFileStorageType(struct soap *soap,
                      const char *srm_endpoint,
                      const char *userID,
                      const tSurlInfoArray path,
                      const long int desiredStorageType,
                      struct srm__srmChangeFileStorageTypeResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmChangeFileStorageTypeRequest req;

  DO_SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  NEW_STR_VAL(userID,TUserID);

  NOT_NULL(req.arrayOfPaths = soap_new_srm__ArrayOfTSURLInfo(soap, -1));
  ARRAY_OF_TSURL_INFO(arrayOfPaths);

  req.desiredStorageType = (srm__TFileStorageType)desiredStorageType;
  DM_LOG(DM_N(2), "desiredStorageType == `%s'\n", getTFileStorageType(req.desiredStorageType).c_str());
  
  /* To send the request ... */
  SOAP_CALL_SRM(ChangeFileStorageType); 

  RETURN(EXIT_SUCCESS);
}
