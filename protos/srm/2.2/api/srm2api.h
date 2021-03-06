#ifndef _SRM2API_H
#define _SRM2API_H

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#include "soapH.h"              /* srm__*Response_, srm__TStatusCode, srm__TSpaceType__Volatile, ... */

/* type definitions */
typedef struct {
  std::vector <std::string *> key;
  std::vector <std::string *> value;
} tStorageSystemInfo;

typedef struct {
  std::vector <std::string *> SURL;
  std::vector <int> isSourceADirectory;
  std::vector <int *> allLevelRecursive;
  std::vector <int *> numOfLevels;
} tArrayOfGetFileRequests;

typedef struct {
  std::vector <std::string *> SURL;
  std::vector <uint64_t *> expectedFileSize;
} tArrayOfPutFileRequests;

typedef struct {
  std::vector <std::string *> ID;
  std::vector <long int> mode;          /* enum */
} tPermissionArray;

/* extern(al) function declarations */
extern int
AbortFiles(struct soap *soap,
           const char *srm_endpoint,
           const char *authorizationID,
           const char *requestToken,
           std::vector <std::string *> SURL,
           struct srm__srmAbortFilesResponse_ *resp);

extern int
AbortRequest(struct soap *soap,
             const char *srm_endpoint,
             const char *authorizationID,
             const char *requestToken,
             struct srm__srmAbortRequestResponse_ *resp);

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
            const long *retentionPolicy,
            const long *accessLatency,
            const long *accessPattern,
            const long *connectionType,
            std::vector <std::string *> clientNetworks,
            std::vector <std::string *> transferProtocols,
            int *deferredStartTime,
            struct srm__srmBringOnlineResponse_ *resp);

extern int
ChangeSpaceForFiles(struct soap *soap,
                    const char *srm_endpoint,
                    const char *authorizationID,
                    const char *spaceToken,
                    tStorageSystemInfo storageSystemInfo,
                    std::vector <std::string *> SURLs,
                    struct srm__srmChangeSpaceForFilesResponse_ *resp);

extern int
CheckPermission(struct soap *soap,
                const char *srm_endpoint,
                const char *authorizationID,
                std::vector <std::string *> SURL,
                tStorageSystemInfo storageSystemInfo,
                struct srm__srmCheckPermissionResponse_ *resp);

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
     const long *retentionPolicy,
     const long *accessLatency,
     tStorageSystemInfo sourceStorageSystemInfo,
     tStorageSystemInfo targetStorageSystemInfo,
     struct srm__srmCopyResponse_ *resp);

extern int
ExtendFileLifeTime(struct soap *soap,
                   const char *srm_endpoint,
                   const char *authorizationID,
                   const char *requestToken,
                   std::vector <std::string *> SURL,
                   int *newFileLifeTime,
                   int *newPinLifeTime,
                   struct srm__srmExtendFileLifeTimeResponse_ *resp);

extern int
ExtendFileLifeTimeInSpace(struct soap *soap,
                          const char *srm_endpoint,
                          const char *authorizationID,
                          const char *spaceToken,
                          std::vector <std::string *> SURL,
                          struct srm__srmExtendFileLifeTimeInSpaceResponse_ *resp);

extern int
GetPermission(struct soap *soap,
              const char *srm_endpoint,
              const char *authorizationID,
              std::vector <std::string *> SURL,
              tStorageSystemInfo storageSystemInfo,
              struct srm__srmGetPermissionResponse_ *resp);

extern int
GetRequestSummary(struct soap *soap,
                  const char *srm_endpoint,
                  const char *authorizationID,
                  std::vector <std::string *> requestTokens,
                  struct srm__srmGetRequestSummaryResponse_ *resp);

extern int
GetRequestTokens(struct soap *soap,
                 const char *srm_endpoint,
                 const char *authorizationID,
                 const char *userRequestDescription,
                 struct srm__srmGetRequestTokensResponse_ *resp);

extern int
GetSpaceMetaData(struct soap *soap,
                 const char *srm_endpoint,
                 const char *authorizationID,
                 std::vector <std::string *> spaceTokens,
                 struct srm__srmGetSpaceMetaDataResponse_ *resp);

extern int
GetSpaceTokens(struct soap *soap,
               const char *srm_endpoint,
               const char *authorizationID,
               const char *userSpaceTokenDescription,
               struct srm__srmGetSpaceTokensResponse_ *resp);

extern int
GetTransferProtocols(struct soap *soap,
                     const char *srm_endpoint,
                     const char *authorizationID,
                     struct srm__srmGetTransferProtocolsResponse_ *resp);

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
             const long *retentionPolicy,
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
             const long *retentionPolicy,
             const long *accessLatency,
             const long *accessPattern,
             const long *connectionType,
             std::vector <std::string *> clientNetworks,
             std::vector <std::string *> transferProtocols,
             struct srm__srmPrepareToPutResponse_ *resp);

extern int
PurgeFromSpace(struct soap *soap,
               const char *srm_endpoint,
               const char *authorizationID,
               std::vector <std::string *> SURL,
               const char *spaceToken,
               tStorageSystemInfo storageSystemInfo,
               struct srm__srmPurgeFromSpaceResponse_ *resp);

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
             const long *retentionPolicy,
             const long *accessLatency,
             uint64_t *desiredSizeOfTotalSpace,
             const uint64_t desiredSizeOfGuaranteedSpace,
             int *desiredLifetimeOfReservedSpace,
             std::vector <uint64_t> arrayOfExpectedFileSizes,
             tStorageSystemInfo storageSystemInfo,
             const long *accessPattern,
             const long *connectionType,
             std::vector <std::string *> clientNetworks,
             std::vector <std::string *> transferProtocols,
             struct srm__srmReserveSpaceResponse_ *resp);

extern int
ResumeRequest(struct soap *soap,
              const char *srm_endpoint,
              const char *authorizationID,
              const char *requestToken,
              struct srm__srmResumeRequestResponse_ *resp);

extern int
Mkdir(struct soap *soap,
      const char *srm_endpoint,
      const char *authorizationID,
      const char *directoryPath,
      const tStorageSystemInfo storageSystemInfo,
      struct srm__srmMkdirResponse_ *resp);

extern int
Mv(struct soap *soap,
   const char *srm_endpoint,
   const char *authorizationID,
   const char *fromSURL,
   const char *toSURL,
   const tStorageSystemInfo storageSystemInfo,
   struct srm__srmMvResponse_ *resp);

extern int
Rm(struct soap *soap,
   const char *srm_endpoint,
   const char *authorizationID,
   std::vector <std::string *> SURL,
   tStorageSystemInfo storageSystemInfo,
   struct srm__srmRmResponse_ *resp);

extern int
Rmdir(struct soap *soap,
      const char *srm_endpoint,
      const char *authorizationID,
      const char *directoryPath,
      const tStorageSystemInfo storageSystemInfo,
      bool *recursive,
      struct srm__srmRmdirResponse_ *resp);

extern int
SetPermission(struct soap *soap,
              const char *srm_endpoint,
              const char *authorizationID,
              const char *SURL,
              const long int permissionType,
              const long int *ownerPermission,
              tPermissionArray userPermissions,
              tPermissionArray groupPermissions,
              const long int *otherPermission,
              tStorageSystemInfo storageSystemInfo,
              struct srm__srmSetPermissionResponse_ *resp);

extern int
StatusOfBringOnlineRequest(struct soap *soap,
                           const char *srm_endpoint,
                           const char *authorizationID,
                           const char *requestToken,
                           std::vector <std::string *> sourceSURLs,
                           struct srm__srmStatusOfBringOnlineRequestResponse_ *resp);

extern int
StatusOfChangeSpaceForFilesRequest(struct soap *soap,
                                   const char *srm_endpoint,
                                   const char *authorizationID,
                                   const char *requestToken,
                                   struct srm__srmStatusOfChangeSpaceForFilesRequestResponse_ *resp);

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
StatusOfReserveSpaceRequest(struct soap *soap,
                            const char *srm_endpoint,
                            const char *authorizationID,
                            const char *requestToken,
                            struct srm__srmStatusOfReserveSpaceRequestResponse_ *resp);

extern int
StatusOfUpdateSpaceRequest(struct soap *soap,
                           const char *srm_endpoint,
                           const char *authorizationID,
                           const char *requestToken,
                           struct srm__srmStatusOfUpdateSpaceRequestResponse_ *resp);

extern int
SuspendRequest(struct soap *soap,
               const char *srm_endpoint,
               const char *authorizationID,
               const char *requestToken,
               struct srm__srmSuspendRequestResponse_ *resp);

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
