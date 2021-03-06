/**
 * \file Ls.cpp
 *
 * Implements the SRM2 Ls method.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#ifdef HAVE_INTTYPES
#include "inttypes.h"
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
 * srmLs method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param SURL
 * \param storageSystemInfo
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
   const char *authorizationID,
   std::vector <std::string *> SURL,
   tStorageSystemInfo storageSystemInfo,
   const long *fileStorageType,
   bool *fullDetailedList,
   bool *allLevelRecursive,
   int *numOfLevels,
   int *offset,
   int *count,
   struct srm__srmLsResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmLsRequest req;

  DO_SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);
  
  /* SURL */
  MV_ARRAY_OF_STR_VAL(req.arrayOfSURLs,SURL,urlArray,AnyURI);

  /* Storage system info */
  MV_STORAGE_SYSTEM_INFO(req.storageSystemInfo,storageSystemInfo);
  
  MV_PSOAP(FileStorageType,req.fileStorageType,fileStorageType);
  MV_PBOOL(req.fullDetailedList,fullDetailedList);
  MV_PBOOL(req.allLevelRecursive,allLevelRecursive);
  MV_PINT(req.numOfLevels,numOfLevels);
  MV_PINT(req.offset,offset);
  MV_PINT(req.count,count);

  /* To send the request ... */
  SOAP_CALL_SRM(Ls); 

  RETURN(EXIT_SUCCESS);
}
