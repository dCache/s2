/**
 * \file Copy.cpp
 *
 * Implements the SRM2 Copy method.
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
 * srmCopy method.
 *
 * \param soap
 * \param srm_endpoint
 * \param authorizationID
 * \param sourceSURL
 * \param targetSURL
 * \param isSourceADirectory
 * \param allLevelRecursive
 * \param numOfLevels
 * \param userRequestDescription
 * \param overwriteOption
 * \param desiredTotalRequestTime
 * \param desiredTargetSURLLifeTime
 * \param targetFileStorageType
 * \param targetSpaceToken
 * \param retentionPolicy
 * \param accessLatency
 * \param sourceStorageSystemInfo
 * \param targetStorageSystemInfo
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
Copy(struct soap *soap,
     const char *srm_endpoint,
     const char *authorizationID,
     std::vector <std::string *> sourceSURL,
     std::vector <std::string *> targetSURL,
     std::vector <int> isSourceADirectory,
     std::vector <int *> allLevelRecursive,
     std::vector <int *> numOfLevels,
     const char *userRequestDescription,
     const long *overwriteOption,
     int *desiredTotalRequestTime,
     int *desiredTargetSURLLifeTime,
     const long *targetFileStorageType,
     const char *targetSpaceToken,
     const long *retentionPolicy,
     const long *accessLatency,
     tStorageSystemInfo sourceStorageSystemInfo,
     tStorageSystemInfo targetStorageSystemInfo,
     struct srm__srmCopyResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmCopyRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  MV_CSTR2PSTR(req.authorizationID,authorizationID);

  /* Create the file request */
  unsigned SURLs = sourceSURL.size();
  NOT_0(sourceSURL,req.arrayOfFileRequests,soap_new_srm__ArrayOfTCopyFileRequest(soap, -1));
  for (uint u = 0; u < SURLs; u++) {
    DM_LOG(DM_N(2), "sourceSURL[%u]\n", u);
    srm__TCopyFileRequest *fileRequest;
    NOT_NULL(fileRequest = soap_new_srm__TCopyFileRequest(soap, -1));

    /* source */
    MV_CSTR2STR(fileRequest->sourceSURL,CSTR(sourceSURL[u]));

    /* target */
    if(NOT_NULL_VEC1(targetSURL)) {
      MV_CSTR2STR(fileRequest->targetSURL, CSTR(targetSURL[u]));
    } else {
      /* NULL --> "" */
      MV_CSTR2STR(fileRequest->targetSURL, "");
    }

    /* dirOption */
    if(isSourceADirectory.size() != 0
       || allLevelRecursive.size() != 0
       || numOfLevels.size() != 0) {
      NOT_NULL(fileRequest->dirOption = soap_new_srm__TDirOption(soap, -1));
      if(NOT_NULL_VEC1(isSourceADirectory)) {
        fileRequest->dirOption->isSourceADirectory = isSourceADirectory[u];
        DM_LOG(DM_N(2), "isSourceADirectory[%u] = %d\n", u, fileRequest->dirOption->isSourceADirectory);
      } else {
        fileRequest->dirOption->isSourceADirectory = 0;
        DM_LOG(DM_N(2), "isSourceADirectory[%u] == 0\n", u);
      }
      if(NOT_NULL_VEC1(allLevelRecursive)) {
        fileRequest->dirOption->allLevelRecursive = (bool *)allLevelRecursive[u];
        DM_LOG(DM_N(2), "allLevelRecursive[%u] = %d\n", u, *(fileRequest->dirOption->allLevelRecursive));
      } else {
        fileRequest->dirOption->allLevelRecursive = NULL;
        DM_LOG(DM_N(2), "allLevelRecursive[%u] == NULL\n", u);
      }
      if(NOT_NULL_VEC1(numOfLevels)) {
        fileRequest->dirOption->numOfLevels = numOfLevels[u];
        DM_LOG(DM_N(2), "numOfLevels[%u] = %d\n", u, *(fileRequest->dirOption->numOfLevels));
      } else {
        fileRequest->dirOption->numOfLevels = NULL;
        DM_LOG(DM_N(2), "numOfLevels[%u] == NULL\n", u);
      }
    } else {
      DM_DBG(DM_N(3), "dirOption = NULL\n");
      fileRequest->dirOption = NULL;
    }
    req.arrayOfFileRequests->requestArray.push_back(fileRequest);
  }

  MV_CSTR2PSTR(req.userRequestDescription,userRequestDescription);
  MV_PSOAP(OverwriteMode,req.overwriteOption,overwriteOption);
  MV_PINT(req.desiredTotalRequestTime,desiredTotalRequestTime);
  MV_PINT(req.desiredTargetSURLLifeTime,desiredTargetSURLLifeTime);
  MV_PSOAP(FileStorageType,req.targetFileStorageType,targetFileStorageType);
  MV_CSTR2PSTR(req.targetSpaceToken,targetSpaceToken);

  /* Retention */
  MV_RETENTION_POLICY(req.targetFileRetentionPolicyInfo,retentionPolicy,accessLatency);

  /* Storage System Info */
  MV_STORAGE_SYSTEM_INFO(req.sourceStorageSystemInfo,sourceStorageSystemInfo);
  MV_STORAGE_SYSTEM_INFO(req.targetStorageSystemInfo,targetStorageSystemInfo);

  /* To send the request ... */
  SOAP_CALL_SRM(Copy); 

  RETURN(EXIT_SUCCESS);
}
