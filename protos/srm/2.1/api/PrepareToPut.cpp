/**
 * \file PrepareToPut.cpp
 *
 * Implements the SRM2 PrepareToPut method.  SRM2 spec p.18.
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
#include "srm_soap27.h"

/**
 * srmPrepareToPut method.
 *
 * \param soap
 * \param srm_endpoint
 * \param userID
 * \param arrayOfFileRequests
 * \param arrayOfTransferProtocols
 * \param userRequestDescription
 * \param overwriteOption
 * \param storageSystemInfo
 * \param totalRetryTime
 * \param resp request response
 *
 * \returns request exit status (EXIT_SUCCESS/EXIT_FAILURE)
 */
extern int
PrepareToPut(struct soap *soap,
             const char *srm_endpoint,
             const char *userID,
             const tArrayOfPutFileRequests arrayOfFileRequests,
             std::vector <std::string *> arrayOfTransferProtocols,
             const char *userRequestDescription,
             const long int *overwriteOption,
             const char *storageSystemInfo,
             const int64_t *totalRetryTime,
             struct srm__srmPrepareToPutResponse_ *resp)
{
  DM_DBG_I;
  struct srm__srmPrepareToPutRequest req;

  SOAP_INIT(soap);

#ifdef HAVE_CGSI_PLUGIN
  int flags;
  flags = CGSI_OPT_DISABLE_NAME_CHECK|CGSI_OPT_DELEG_FLAG;
  soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#else
#warning "Compiling without CGSI plugin support, i.e. no security"
#endif

  NEW_STR_VAL(userID,TUserID);

  NOT_NULL(req.arrayOfFileRequests = soap_new_srm__ArrayOfTPutFileRequest(soap, -1));
  /* Create the file request */

  DM_LOG(DM_N(2), "arrayOfFileRequests.SURLOrStFN.size() == %d\n", arrayOfFileRequests.SURLOrStFN.size());
  for (uint u = 0; u < arrayOfFileRequests.SURLOrStFN.size(); u++) {
    srm__TPutFileRequest *fileRequest;
    NOT_NULL(fileRequest = soap_new_srm__TPutFileRequest(soap, -1));
    
    if(NOT_NULL_VEC(arrayOfFileRequests,fileStorageType)) {
      fileRequest->fileStorageType = (srm__TFileStorageType *)getTFileStorageType(arrayOfFileRequests.fileStorageType[u]->c_str());
      DM_LOG(DM_N(2), "fileStorageType[%u] == `%s'\n", u, getTFileStorageType(*fileRequest->fileStorageType).c_str());
    } else {
      fileRequest->fileStorageType = NULL;
      DM_LOG(DM_N(2), "fileStorageType[%u] == NULL\n", u);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,knownSizeOfThisFile)) {
      NOT_NULL(fileRequest->knownSizeOfThisFile = soap_new_srm__TSizeInBytes(soap, -1));
      fileRequest->knownSizeOfThisFile->value = *(arrayOfFileRequests.knownSizeOfThisFile[u]);
      DM_LOG(DM_N(2), "knownSizeOfThisFile[%u] = %"PRIi64"\n", u, fileRequest->knownSizeOfThisFile->value);
    } else {
      fileRequest->knownSizeOfThisFile = NULL;
      DM_LOG(DM_N(2), "knownSizeOfThisFile[%u] == NULL\n", u);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,lifetime)) {
      NOT_NULL(fileRequest->lifetime = soap_new_srm__TLifeTimeInSeconds(soap, -1));
      fileRequest->lifetime->value = *arrayOfFileRequests.lifetime[u];
      DM_LOG(DM_N(2), "lifetime[%u] == %"PRIi64"\n", u, fileRequest->lifetime->value);
    } else {
      fileRequest->lifetime = NULL;
      DM_LOG(DM_N(2), "lifetime[%u] == NULL\n", u);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,spaceToken)) {
      NOT_NULL(fileRequest->spaceToken = soap_new_srm__TSpaceToken(soap, -1));
      fileRequest->spaceToken->value.assign(arrayOfFileRequests.spaceToken[u]->c_str());
      DM_LOG(DM_N(2), "spaceToken[%u] == `%s'\n", u, fileRequest->spaceToken->value.c_str());
    } else {
      fileRequest->spaceToken = NULL;
      DM_LOG(DM_N(2), "spaceToken[%u] == NULL\n", u);
    }
    NOT_NULL(fileRequest->toSURLInfo = soap_new_srm__TSURLInfo(soap, -1));
    if(NOT_NULL_VEC(arrayOfFileRequests,SURLOrStFN)) {
      NOT_NULL(fileRequest->toSURLInfo->SURLOrStFN = soap_new_srm__TSURL(soap, -1));
      fileRequest->toSURLInfo->SURLOrStFN->value.assign(arrayOfFileRequests.SURLOrStFN[u]->c_str());
      DM_LOG(DM_N(2), "SURLOrStFN[%u] == `%s'\n", u, fileRequest->toSURLInfo->SURLOrStFN->value.c_str());
    } else {
      fileRequest->toSURLInfo->SURLOrStFN = NULL;
      DM_LOG(DM_N(2), "SURLOrStFN[%u] == NULL\n", u);
    }
    if(NOT_NULL_VEC(arrayOfFileRequests,storageSystemInfo)) {
      NOT_NULL(fileRequest->toSURLInfo->storageSystemInfo = soap_new_srm__TStorageSystemInfo(soap, -1));
      fileRequest->toSURLInfo->storageSystemInfo->value.assign(arrayOfFileRequests.storageSystemInfo[u]->c_str());
      DM_LOG(DM_N(2), "storageSystemInfo[%u] == `%s'\n", u, fileRequest->toSURLInfo->storageSystemInfo->value.c_str());
    } else {
      fileRequest->toSURLInfo->storageSystemInfo = NULL;
      DM_LOG(DM_N(2), "storageSystemInfo[%u] == NULL\n", u);
    }
    
    req.arrayOfFileRequests->putRequestArray.push_back(fileRequest);
  }
  
  req.arrayOfTransferProtocols = soap_new_srm__ArrayOf_USCORExsd_USCOREstring(soap, -1);
  /* Fill in transfer protocols */
  DM_LOG(DM_N(2), "arrayOfTransferProtocols.size() == %d\n", arrayOfTransferProtocols.size());
  for(uint u = 0; u < arrayOfTransferProtocols.size(); u++) {
    if(arrayOfTransferProtocols[u]) {
      req.arrayOfTransferProtocols->stringArray.push_back(arrayOfTransferProtocols[u]->c_str());
      DM_LOG(DM_N(2), "arrayOfTransferProtocols[%i] == `%s'\n", u, arrayOfTransferProtocols[u]->c_str());
    } else {
      DM_LOG(DM_N(2), "arrayOfTransferProtocols[%i] == NULL\n", u);
    }
  }
  
  NEW_STDSTRING(userRequestDescription);
  req.overwriteOption = (srm__TOverwriteMode *) overwriteOption;
  DM_LOG(DM_N(2), "overwriteOption == `%s'\n", getTOverwriteMode(*req.overwriteOption).c_str());
  NEW_STR_VAL(storageSystemInfo,TStorageSystemInfo);
  NEW_INT64_VAL(totalRetryTime,TLifeTimeInSeconds);
  
  /* To send the request ... */
  SOAP_CALL_SRM(PrepareToPut); 

  RETURN(EXIT_SUCCESS);
}
