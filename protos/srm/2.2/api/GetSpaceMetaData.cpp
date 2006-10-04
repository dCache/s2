/**
 * \file GetSpaceMetaData.cpp
 *
 * Implements the SRM2 GetSpaceMetaData method.
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

#include "srm_macros.h"

/**
 * srmGetSpaceMetaData method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param spaceTokens
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
GetSpaceMetaData(struct soap *soap,
                 const char *srm_endpoint,
                 const char *authorizationID,
                 std::vector <std::string *> spaceTokens,
                 struct srm__srmGetSpaceMetaDataResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmGetSpaceMetaDataRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);
  MV_ARRAY_OF_STR_VAL(req.arrayOfSpaceTokens,spaceTokens,stringArray,String);
  
  /* To send the request ... */
  SOAP_CALL_SRM(GetSpaceMetaData);

  RETURN(EXIT_SUCCESS);
}
