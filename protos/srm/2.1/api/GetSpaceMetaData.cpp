/**
 * \file GetSpaceMetaData.cpp
 *
 * Implements the SRM2 GetSpaceMetaData method.  SRM2 spec p.12.
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

#include "srm_macros.h"

/**
 * srmGetSpaceMetaData method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param arrayOfSpaceToken
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
GetSpaceMetaData(struct soap *soap,
                 const char *srm_endpoint,
                 const char *userID,
                 std::vector <std::string *> arrayOfSpaceToken,
                 struct srm__srmGetSpaceMetaDataResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmGetSpaceMetaDataRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);
  NEW_ARRAY_OF_STR_VAL(arrayOfSpaceTokens,arrayOfSpaceToken,tokenArray,TSpaceToken);
  
  /* To send the request ... */
  SOAP_CALL_SRM(GetSpaceMetaData);

  RETURN(EXIT_SUCCESS);
}
