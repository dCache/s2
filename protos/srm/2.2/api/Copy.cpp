/**
 * \file Copy.cpp
 *
 * Implements the SRM2 Copy method.  SRM2 spec p.17.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
     const long retentionPolicy,
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
  NOT_NULL(req.arrayOfFileRequests = soap_new_srm__ArrayOfTCopyFileRequest(soap, -1));
  unsigned SURLs = sourceSURL.size();
  unsigned sourceSSI_size = sourceStorageSystemInfo.key.size();
  unsigned targetSSI_size = targetStorageSystemInfo.key.size();
  for (uint u = 0; u < SURLs; u++) {
    DM_LOG(DM_N(2), "sourceSURL[%u]\n", u);
    srm__TCopyFileRequest *fileRequest;
    NOT_NULL(fileRequest = soap_new_srm__TCopyFileRequest(soap, -1));
    NOT_NULL(fileRequest->dirOption = soap_new_srm__TDirOption(soap, -1));

    /* source */
    MV_CSTR2STR(fileRequest->sourceSURL,CSTR(sourceSURL[u]));

    /* target */
    MV_CSTR2STR(fileRequest->targetSURL,CSTR(targetSURL[u]));

    /* dirOption */
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
    req.arrayOfFileRequests->requestArray.push_back(fileRequest);
  }

  MV_CSTR2PSTR(req.userRequestDescription,userRequestDescription);
  MV_PSOAP(OverwriteMode,req.overwriteOption,overwriteOption);
  MV_PINT(req.desiredTotalRequestTime,desiredTotalRequestTime);
  MV_PINT(req.desiredTargetSURLLifeTime,desiredTargetSURLLifeTime);
  MV_PSOAP(FileStorageType,req.targetFileStorageType,targetFileStorageType);
  MV_CSTR2PSTR(req.targetSpaceToken,targetSpaceToken);

  /* Retention */
  NOT_NULL(req.targetFileRetentionPolicyInfo = soap_new_srm__TRetentionPolicyInfo(soap, -1));
  MV_SOAP(RetentionPolicy,req.targetFileRetentionPolicyInfo->retentionPolicy,retentionPolicy);
  MV_PSOAP(AccessLatency,req.targetFileRetentionPolicyInfo->accessLatency,accessLatency);

  NOT_NULL(req.sourceStorageSystemInfo = soap_new_srm__ArrayOfTExtraInfo(soap, -1));
  NOT_NULL(req.targetStorageSystemInfo = soap_new_srm__ArrayOfTExtraInfo(soap, -1));

  for(uint j = 0; j < sourceSSI_size; j++) {
    DM_LOG(DM_N(2), "sourceStorageSystemInfo.key[%u]\n", j);
    srm__TExtraInfo *extraInfo;
    NOT_NULL(extraInfo = soap_new_srm__TExtraInfo(soap, -1));
    MV_CSTR2STR(extraInfo->key,CSTR(sourceStorageSystemInfo.key[j]));
    MV_PSTR2PSTR(extraInfo->value,sourceStorageSystemInfo.value[j]);
    req.sourceStorageSystemInfo->extraInfoArray.push_back(extraInfo);
  }

  for(uint j = 0; j < targetSSI_size; j++) {
    DM_LOG(DM_N(2), "targetStorageSystemInfo.key[%u]\n", j);
    srm__TExtraInfo *extraInfo;
    NOT_NULL(extraInfo = soap_new_srm__TExtraInfo(soap, -1));
    MV_CSTR2STR(extraInfo->key,CSTR(targetStorageSystemInfo.key[j]));
    MV_PSTR2PSTR(extraInfo->value,targetStorageSystemInfo.value[j]);
    req.sourceStorageSystemInfo->extraInfoArray.push_back(extraInfo);
  }
  
  /* To send the request ... */
  SOAP_CALL_SRM(Copy); 

  RETURN(EXIT_SUCCESS);
}
