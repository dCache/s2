#ifndef _SRM2API_H
#define _SRM2API_H

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#include "soapH.h"              /* srm__*Response_, srm__TStatusCode, srm__TSpaceType__Volatile, ... */

/* type definitions */
typedef struct tStorageSystemInfo
{ 
  std::vector <std::string *> key;
  std::vector <std::string *> value;
};

typedef struct tArrayOfGetFileRequests
{ 
  std::vector <std::string *> sourceSURL;
  std::vector <int> isSourceADirectory;
  std::vector <int *> allLevelRecursive;
  std::vector <int *> numOfLevels;
};

typedef struct tArrayOfPutFileRequests
{ 
  std::vector <std::string *> targetSURL;
  std::vector <uint64_t *> expectedFileSize;
};

/* extern(al) function declarations */
extern int
BringOnline(struct soap *soap,
            const char *srm_endpoint,
            const char *authorizationID,
            const tArrayOfGetFileRequests fileRequests,
            const char *userRequestDescription,
            const tStorageSystemInfo storageSystemInfo,
            const long *desiredFileStorageType,
            int *desiredTotalRequestTime,
            int *desiredLifeTime,
            const char *targetSpaceToken,
            const long retentionPolicy,
            const long *accessLatency,
            const long *accessPattern,
            const long *connectionType,
            std::vector <std::string *> clientNetworks,
            std::vector <std::string *> transferProtocols,
            int *deferredStartTime,
            struct srm__srmBringOnlineResponse_ *resp);

extern int
Copy(struct soap *soap,
     const char *srm_endpoint,
     const char *authorizationID,
     std::vector <std::string *> source,
     std::vector <std::string *> target,
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
     struct srm__srmCopyResponse_ *resp);

extern int
GetSpaceMetaData(struct soap *soap,
                 const char *srm_endpoint,
                 const char *authorizationID,
                 std::vector <std::string *> spaceTokens,
                 struct srm__srmGetSpaceMetaDataResponse_ *resp);

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
   struct srm__srmLsResponse_ *resp);

extern int
Ping(struct soap *soap,
     const char *srm_endpoint,
     const char *authorizationID,
     struct srm__srmPingResponse_ *resp);

extern int
PrepareToGet(struct soap *soap,
             const char *srm_endpoint,
             const char *authorizationID,
             const tArrayOfGetFileRequests fileRequests,
             const char *userRequestDescription,
             const tStorageSystemInfo storageSystemInfo,
             const long *desiredFileStorageType,
             int *desiredTotalRequestTime,
             int *desiredPinLifeTime,
             const char *targetSpaceToken,
             const long retentionPolicy,
             const long *accessLatency,
             const long *accessPattern,
             const long *connectionType,
             std::vector <std::string *> clientNetworks,
             std::vector <std::string *> transferProtocols,
             struct srm__srmPrepareToGetResponse_ *resp);

extern int
PrepareToPut(struct soap *soap,
             const char *srm_endpoint,
             const char *authorizationID,
             const tArrayOfPutFileRequests putFileRequests,
             const char *userRequestDescription,
             const long *overwriteOption,
             tStorageSystemInfo storageSystemInfo,
             int *desiredTotalRequestTime,
             int *desiredPinLifeTime,
             int *desiredFileLifeTime,
             const long *desiredFileStorageType,
             const char *targetSpaceToken,
             const long retentionPolicy,
             const long *accessLatency,
             const long *accessPattern,
             const long *connectionType,
             std::vector <std::string *> clientNetworks,
             std::vector <std::string *> transferProtocols,
             struct srm__srmPrepareToPutResponse_ *resp);

extern int
PutDone(struct soap *soap,
        const char *srm_endpoint,
        const char *authorizationID,
        const char *requestToken,
        std::vector <std::string *> arrayOfSURLs,
        struct srm__srmPutDoneResponse_ *resp);

extern int
ReleaseFiles(struct soap *soap,
             const char *srm_endpoint,
             const char *authorizationID,
             const char *requestToken,
             std::vector <std::string *> SURLs,
             bool *doRemove,
             struct srm__srmReleaseFilesResponse_ *resp);

extern int
ReleaseSpace(struct soap *soap,
             const char *srm_endpoint,
             const char *authorizationID,
             const char *spaceToken,
             tStorageSystemInfo storageSystemInfo,
             bool *forceFileRelease,
             struct srm__srmReleaseSpaceResponse_ *resp);

extern int
ReserveSpace(struct soap *soap,
             const char *srm_endpoint,
             const char *authorizationID,
             const char *userSpaceTokenDescription,
             const long retentionPolicy,
             const long *accessLatency,
             uint64_t *desiredSizeOfTotalSpace,
             const uint64_t desiredSizeOfGuaranteedSpace,
             int *desiredLifetimeOfReservedSpace,
             std::vector <uint64_t> arrayOfExpectedFileSizes,
             tStorageSystemInfo storageSystemInfo,
             const long *accessPattern,
             const long *connectionType,
             struct srm__srmReserveSpaceResponse_ *resp);

extern int
StatusOfBringOnlineRequest(struct soap *soap,
                           const char *srm_endpoint,
                           const char *authorizationID,
                           const char *requestToken,
                           std::vector <std::string *> sourceSURLs,
                           struct srm__srmStatusOfBringOnlineRequestResponse_ *resp);

extern int
StatusOfCopyRequest(struct soap *soap,
                    const char *srm_endpoint,
                    const char *authorizationID,
                    const char *requestToken,
                    std::vector <std::string *> sourceSURLs,
                    std::vector <std::string *> targetSURLs,
                    struct srm__srmStatusOfCopyRequestResponse_ *resp);

extern int
StatusOfGetRequest(struct soap *soap,
                   const char *srm_endpoint,
                   const char *authorizationID,
                   const char *requestToken,
                   std::vector <std::string *> sourceSURLs,
                   struct srm__srmStatusOfGetRequestResponse_ *resp);

extern int
StatusOfLsRequest(struct soap *soap,
                  const char *srm_endpoint,
                  const char *authorizationID,
                  const char *requestToken,
                  int *offset,
                  int *count,
                  struct srm__srmStatusOfLsRequestResponse_ *resp);

extern int
StatusOfPutRequest(struct soap *soap,
                   const char *srm_endpoint,
                   const char *authorizationID,
                   const char *requestToken,
                   std::vector <std::string *> targetSURLs,
                   struct srm__srmStatusOfPutRequestResponse_ *resp);

extern int
UpdateSpace(struct soap *soap,
            const char *srm_endpoint,
            const char *authorizationID,
            const char *spaceToken,
            uint64_t *newSizeOfTotalSpaceDesired,
            uint64_t *newSizeOfGuaranteedSpaceDesired,
            int *newLifeTime,
            tStorageSystemInfo storageSystemInfo,
            struct srm__srmUpdateSpaceResponse_ *resp);

#endif /* _SRM2API_H */
