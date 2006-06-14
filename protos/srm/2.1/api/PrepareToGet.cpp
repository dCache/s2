/**
 * \file PrepareToGet.cpp
 *
 * Implements the SRM2 PrepareToGet method.  SRM2 spec p.17.
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
 * srmPrepareToGet method.
 *
 * \param srm_endpoint
 * \param userID
 * \param arrayOfFileRequests
 * \param arrayOfTransferProtocols
 * \param userRequestDescription
 * \param storageSystemInfo
 * \param totalRetryTime
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
PrepareToGet(const char *srm_endpoint,
             const char *userID,
             const tArrayOfGetFileRequests arrayOfFileRequests,
             std::vector <std::string *> arrayOfTransferProtocols,
             const char *userRequestDescription,
             const char *storageSystemInfo,
             const int64_t *totalRetryTime,
             struct srm__srmPrepareToGetResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmPrepareToGetRequest req;
  struct soap soap;
  soap_init(&soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

  NEW_STR_VAL(userID,TUserID);

  NOT_NULL(req.arrayOfFileRequests = soap_new_srm__ArrayOfTGetFileRequest(&soap, -1));
  /* Create the file request */
  DM_LOG(DM_N(2), "arrayOfFileRequests.SURLOrStFN.size() == %d\n", arrayOfFileRequests.SURLOrStFN.size());
  for (uint i = 0; i < arrayOfFileRequests.SURLOrStFN.size(); i++) {
    srm__TGetFileRequest *fileRequest;
    NOT_NULL(fileRequest = soap_new_srm__TGetFileRequest(&soap, -1));
    
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
      DM_LOG(DM_N(2), "isSourceADirectory[%u] == 0\n", i);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,numOfLevels)) {
      fileRequest->dirOption->numOfLevels = arrayOfFileRequests.numOfLevels[i];
      DM_LOG(DM_N(2), "numOfLevels[%u] == %d\n", i, fileRequest->dirOption->numOfLevels);
    } else {
      fileRequest->dirOption->numOfLevels = 0;
      DM_LOG(DM_N(2), "numOfLevels[%u] == 0\n", i);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,fileStorageType)) {
      fileRequest->fileStorageType = (srm__TFileStorageType *)getTFileStorageType(arrayOfFileRequests.fileStorageType[i]->c_str());
      DM_LOG(DM_N(2), "fileStorageType[%u] == `%s'\n", i, getTFileStorageType(*fileRequest->fileStorageType).c_str());
    } else {
      fileRequest->fileStorageType = NULL;
      DM_LOG(DM_N(2), "fileStorageType[%u] == NULL\n", i);
    }
    NOT_NULL(fileRequest->fromSURLInfo = soap_new_srm__TSURLInfo(&soap, -1));
    if(NOT_NULL_VEC(arrayOfFileRequests,SURLOrStFN)) {
      NOT_NULL(fileRequest->fromSURLInfo->SURLOrStFN = soap_new_srm__TSURL(&soap, -1));
      fileRequest->fromSURLInfo->SURLOrStFN->value.assign(arrayOfFileRequests.SURLOrStFN[i]->c_str());
      DM_LOG(DM_N(2), "SURLOrStFN[%u] == `%s'\n", i, fileRequest->fromSURLInfo->SURLOrStFN->value.c_str());
    } else {
      fileRequest->fromSURLInfo->SURLOrStFN = NULL;
      DM_LOG(DM_N(2), "SURLOrStFN[%u] == NULL\n", i);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,storageSystemInfo)) {
      NOT_NULL(fileRequest->fromSURLInfo->storageSystemInfo = soap_new_srm__TStorageSystemInfo(&soap, -1));
      fileRequest->fromSURLInfo->storageSystemInfo->value.assign(arrayOfFileRequests.storageSystemInfo[i]->c_str());
      DM_LOG(DM_N(2), "storageSystemInfo[%u] == `%s'\n", i, fileRequest->fromSURLInfo->storageSystemInfo->value.c_str());
    } else {
      fileRequest->fromSURLInfo->storageSystemInfo = NULL;
      DM_LOG(DM_N(2), "storageSystemInfo[%u] == NULL\n", i);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,lifetime)) {
      NOT_NULL(fileRequest->lifetime = soap_new_srm__TLifeTimeInSeconds(&soap, -1));
      fileRequest->lifetime->value = *arrayOfFileRequests.lifetime[i];
      DM_LOG(DM_N(2), "lifetime[%u] == %"PRIi64"\n", i, fileRequest->lifetime->value);
    } else {
      fileRequest->lifetime = NULL;
      DM_LOG(DM_N(2), "lifetime[%u] == NULL\n", i);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,spaceToken)) {
      NOT_NULL(fileRequest->spaceToken = soap_new_srm__TSpaceToken(&soap, -1));
      fileRequest->spaceToken->value.assign(arrayOfFileRequests.spaceToken[i]->c_str());
      DM_LOG(DM_N(2), "spaceToken[%u] == `%s'\n", i, fileRequest->spaceToken->value.c_str());
    } else {
      fileRequest->spaceToken = NULL;
      DM_LOG(DM_N(2), "spaceToken[%u] == NULL\n", i);
    }
    
    req.arrayOfFileRequests->getRequestArray.push_back(fileRequest);
  }
  
  req.arrayOfTransferProtocols = soap_new_srm__ArrayOf_USCORExsd_USCOREstring(&soap, -1);
  /* Fill in transfer protocols */
  DM_LOG(DM_N(2), "arrayOfTransferProtocols.size() == %d\n", arrayOfTransferProtocols.size());
  for(uint i = 0; i < arrayOfTransferProtocols.size(); i++) {
    if(arrayOfTransferProtocols[i]) {
      req.arrayOfTransferProtocols->stringArray.push_back(arrayOfTransferProtocols[i]->c_str());
      DM_LOG(DM_N(2), "arrayOfTransferProtocols[%i] == `%s'\n", i, arrayOfTransferProtocols[i]->c_str());
    } else {
      DM_LOG(DM_N(2), "arrayOfTransferProtocols[%i] == NULL\n", i);
    }
  }
  
  NEW_STDSTRING(userRequestDescription);
  NEW_STR_VAL(storageSystemInfo,TStorageSystemInfo);
  NEW_INT64_VAL(totalRetryTime,TLifeTimeInSeconds);
  
  /* To send the request ... */
  SOAP_CALL_SRM(PrepareToGet); 

  RETURN(EXIT_SUCCESS);
}
