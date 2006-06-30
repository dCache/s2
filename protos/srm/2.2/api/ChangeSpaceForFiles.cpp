/**
 * \file ChangeSpaceForFiles.cpp
 *
 * Implements the SRM2 ChangeSpaceForFiles method.  SRM2 spec p.12.
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
 * srmChangeSpaceForFiles method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param spaceToken
 * \param storageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
ChangeSpaceForFiles(struct soap *soap,
                    const char *srm_endpoint,
                    const char *authorizationID,
                    const char *spaceToken,
                    tStorageSystemInfo storageSystemInfo,
                    std::vector <std::string *> SURL,
                    struct srm__srmChangeSpaceForFilesResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmChangeSpaceForFilesRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);
  MV_CSTR2STR(req.targetSpaceToken,spaceToken);

  MV_ARRAY_OF_STR_VAL(req.arrayOfSURLs,SURL,urlArray,AnyURI);

  /* Storage system info */
  MV_STORAGE_SYSTEM_INFO(req.storageSystemInfo,storageSystemInfo);

  /* To send the request ... */
  SOAP_CALL_SRM(ChangeSpaceForFiles);
  
  RETURN(EXIT_SUCCESS);
}
