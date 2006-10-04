/**
 * \file Ls.cpp
 *
 * Implements the SRM2 Ls method.  SRM2 spec p.16.
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
 * srmLs method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param path
 * \param fileStorageType
 * \param fullDetailedList
 * \param allLevelRecursive
 * \param numOfLevels
 * \param offset
 * \param count
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
Ls(struct soap *soap,
   const char *srm_endpoint,
   const char *userID,
   const tSurlInfoArray path,
   const long int *fileStorageType,
   bool *fullDetailedList,      /* yes, no const in gsoap headers */
   bool *allLevelRecursive,     /* yes, no const in gsoap headers */
   int *numOfLevels,            /* yes, no const in gsoap headers */
   int *offset,                 /* yes, no const in gsoap headers */
   int *count,                  /* yes, no const in gsoap headers */
   struct srm__srmLsResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmLsRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  NEW_STR_VAL(userID,TUserID);

  NOT_NULL(req.paths = soap_new_srm__ArrayOfTSURLInfo(soap, -1));
  ARRAY_OF_TSURL_INFO(paths);
  
  req.fileStorageType = (srm__TFileStorageType *)fileStorageType;
  DM_LOG(DM_N(2), "fileStorageType == `%s'\n", getTFileStorageType(*req.fileStorageType).c_str());

  PBOOL_VAL(fullDetailedList);
  PBOOL_VAL(allLevelRecursive);
  PINT_VAL(numOfLevels);
  PINT_VAL(offset);
  PINT_VAL(count);
  
  /* To send the request ... */
  SOAP_CALL_SRM(Ls); 

  RETURN(EXIT_SUCCESS);
}
