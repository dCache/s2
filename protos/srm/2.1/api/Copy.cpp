/**
 * \file Copy.cpp
 *
 * Implements the SRM2 Copy method.  SRM2 spec p.19.
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
 * \param srm_endpoint
 * \param userID
 * \param arrayOfFileRequests
 * \param userRequestDescription
 * \param overwriteOption
 * \param removeSourceFiles
 * \param storageSystemInfo
 * \param totalRetryTime
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
Copy(const char *srm_endpoint,
     const char *userID,
     const tArrayOfCopyFileRequests arrayOfFileRequests,
     const char *userRequestDescription,
     const long int *overwriteOption,
     bool *removeSourceFiles, /* yes, no const in gsoap headers */
     const char *storageSystemInfo,
     const int64_t *totalRetryTime,
     struct srm__srmCopyResponse_ *resp
     )
{
  DM_DBG_I;
  struct srm__srmCopyRequest req;
  struct soap soap;
  soap_init(&soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);

  NOT_NULL(req.arrayOfFileRequests = soap_new_srm__ArrayOfTCopyFileRequest(&soap, -1));
  /* Create the file request */
  DM_LOG(DM_N(2), "arrayOfFileRequests.fromSURLOrStFN.size() == %d\n", arrayOfFileRequests.fromSURLOrStFN.size());
  for (uint i = 0; i < arrayOfFileRequests.fromSURLOrStFN.size(); i++) {
    srm__TCopyFileRequest *fileRequest;
    NOT_NULL(fileRequest = soap_new_srm__TCopyFileRequest(&soap, -1));

    NOT_NULL(fileRequest->dirOption = soap_new_srm__TDirOption(&soap, -1));
    if(NOT_NULL_VEC(arrayOfFileRequests,allLevelRecursive)) {
      fileRequest->dirOption->allLevelRecursive = (bool *)arrayOfFileRequests.allLevelRecursive[i];
      DM_LOG(DM_N(2), "allLevelRecursive[%u] == %d\n", i, *(fileRequest->dirOption->allLevelRecursive));
    } else {
      fileRequest->dirOption->allLevelRecursive = NULL;
      DM_LOG(DM_N(2), "allLevelRecursive[%u] == NULL\n", i);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,isSourceADirectory)) {
      fileRequest->dirOption->isSourceADirectory = arrayOfFileRequests.isSourceADirectory[i];
      DM_LOG(DM_N(2), "isSourceADirectory[%u] == %d\n", i, fileRequest->dirOption->isSourceADirectory);
    } else {
      fileRequest->dirOption->isSourceADirectory = 0;
      DM_LOG(DM_N(2), "isSourceADirectory == 0\n");
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,numOfLevels)) {
      fileRequest->dirOption->numOfLevels = arrayOfFileRequests.numOfLevels[i];
      DM_LOG(DM_N(2), "numOfLevels[%u] == %d\n", i, fileRequest->dirOption->numOfLevels);
    } else {
      fileRequest->dirOption->numOfLevels = 0;
      DM_LOG(DM_N(2), "numOfLevels == 0\n");
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,fileStorageType)) {
      fileRequest->fileStorageType = (srm__TFileStorageType *)getTFileStorageType(arrayOfFileRequests.fileStorageType[i]->c_str());
      DM_LOG(DM_N(2), "fileStorageType[%u] == `%s'\n", i, getTFileStorageType(*fileRequest->fileStorageType).c_str());
    } else {
      fileRequest->fileStorageType = NULL;
      DM_LOG(DM_N(2), "fileStorageType[%u] == NULL\n", i);
    }
    NOT_NULL(fileRequest->fromSURLInfo = soap_new_srm__TSURLInfo(&soap, -1));
    if(NOT_NULL_VEC(arrayOfFileRequests,fromSURLOrStFN)) {
      NOT_NULL(fileRequest->fromSURLInfo->SURLOrStFN = soap_new_srm__TSURL(&soap, -1));
      fileRequest->fromSURLInfo->SURLOrStFN->value.assign(arrayOfFileRequests.fromSURLOrStFN[i]->c_str());
      DM_LOG(DM_N(2), "fromSURLOrStFN[%u] == `%s'\n", i, fileRequest->fromSURLInfo->SURLOrStFN->value.c_str());
    } else {
      fileRequest->fromSURLInfo->SURLOrStFN = NULL;
      DM_LOG(DM_N(2), "fromSURLOrStFN[%u] == NULL\n", i);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,fromStorageSystemInfo)) {
      NOT_NULL(fileRequest->fromSURLInfo->storageSystemInfo = soap_new_srm__TStorageSystemInfo(&soap, -1));
      fileRequest->fromSURLInfo->storageSystemInfo->value.assign(arrayOfFileRequests.fromStorageSystemInfo[i]->c_str());
      DM_LOG(DM_N(2), "fromStorageSystemInfo[%u] == `%s'\n", i, fileRequest->fromSURLInfo->storageSystemInfo->value.c_str());
    } else {
      fileRequest->fromSURLInfo->storageSystemInfo = NULL;
      DM_LOG(DM_N(2), "fromStorageSystemInfo[%u] == NULL\n", i);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,lifetime)) {
      NOT_NULL(fileRequest->lifetime = soap_new_srm__TLifeTimeInSeconds(&soap, -1));
      fileRequest->lifetime->value = *arrayOfFileRequests.lifetime[i];
      DM_LOG(DM_N(2), "lifetime[%u] == %"PRIi64"\n", i, fileRequest->lifetime->value);
    } else {
      fileRequest->lifetime = NULL;
      DM_LOG(DM_N(2), "lifetime[%u] == NULL\n", i);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,overwriteMode)) {
      fileRequest->overwriteMode = (srm__TOverwriteMode *) arrayOfFileRequests.overwriteMode[i];
      DM_LOG(DM_N(2), "overwriteMode[%u] == `%s'\n", i, getTOverwriteMode(*fileRequest->overwriteMode).c_str());
    } else {
      fileRequest->overwriteMode = NULL;
      DM_LOG(DM_N(2), "overwriteMode[%u] == NULL\n", i);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,spaceToken)) {
      NOT_NULL(fileRequest->spaceToken = soap_new_srm__TSpaceToken(&soap, -1));
      fileRequest->spaceToken->value.assign(arrayOfFileRequests.spaceToken[i]->c_str());
      DM_LOG(DM_N(2), "spaceToken[%u] == `%s'\n", i, fileRequest->spaceToken->value.c_str());
    } else {
      fileRequest->spaceToken = NULL;
      DM_LOG(DM_N(2), "spaceToken[%u] == NULL\n", i);
    }
    NOT_NULL(fileRequest->toSURLInfo = soap_new_srm__TSURLInfo(&soap, -1));
    if(NOT_NULL_VEC(arrayOfFileRequests,toSURLOrStFN)) {
      NOT_NULL(fileRequest->toSURLInfo->SURLOrStFN = soap_new_srm__TSURL(&soap, -1));
      fileRequest->toSURLInfo->SURLOrStFN->value.assign(arrayOfFileRequests.toSURLOrStFN[i]->c_str());
      DM_LOG(DM_N(2), "toSURLOrStFN[%u] == `%s'\n", i, fileRequest->toSURLInfo->SURLOrStFN->value.c_str());
    } else {
      fileRequest->toSURLInfo->SURLOrStFN = NULL;
      DM_LOG(DM_N(2), "toSURLOrStFN[%u] == NULL\n", i);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,toStorageSystemInfo)) {
      NOT_NULL(fileRequest->toSURLInfo->storageSystemInfo = soap_new_srm__TStorageSystemInfo(&soap, -1));
      fileRequest->toSURLInfo->storageSystemInfo->value.assign(arrayOfFileRequests.toStorageSystemInfo[i]->c_str());
      DM_LOG(DM_N(2), "toStorageSystemInfo[%u] == `%s'\n", i, fileRequest->toSURLInfo->storageSystemInfo->value.c_str());
    } else {
      fileRequest->toSURLInfo->storageSystemInfo = NULL;
      DM_LOG(DM_N(2), "toStorageSystemInfo[%u] == NULL\n", i);
    }
    
    req.arrayOfFileRequests->copyRequestArray.push_back(fileRequest);
  }
  
  NEW_STDSTRING(userRequestDescription);
  req.overwriteOption = (srm__TOverwriteMode *) overwriteOption;
  DM_LOG(DM_N(2), "overwriteOption == `%s'\n", getTOverwriteMode(*req.overwriteOption).c_str());
  PBOOL_VAL(removeSourceFiles);
  NEW_STR_VAL(storageSystemInfo,TStorageSystemInfo);
  NEW_INT64_VAL(totalRetryTime,TLifeTimeInSeconds);

  /* To send the request ... */
  SOAP_CALL_SRM(Copy); 

  RETURN(EXIT_SUCCESS);
}
