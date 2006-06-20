#ifndef _SRM2API_H
#define _SRM2API_H

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#include "soapH.h"              /* srm__*Response_, srm__TStatusCode, srm__TSpaceType__Volatile, ... */

/* type definitions */
typedef struct tArrayOfCopyFileRequests
{ 
  std::vector <int *> allLevelRecursive;
  std::vector <int> isSourceADirectory;
  std::vector <int> numOfLevels;
  std::vector <std::string *> fileStorageType;
  std::vector <std::string *> fromSURLOrStFN;
  std::vector <std::string *> fromStorageSystemInfo;
  std::vector <int64_t *> lifetime;
  std::vector <const long int *> overwriteMode;
  std::vector <std::string *> spaceToken;
  std::vector <std::string *> toSURLOrStFN;
  std::vector <std::string *> toStorageSystemInfo;
} tArrayOfCopyFileRequests;

typedef struct tArrayOfGetFileRequests
{ 
  std::vector <int *> allLevelRecursive;
  std::vector <int> isSourceADirectory;
  std::vector <int> numOfLevels;
  std::vector <std::string *> fileStorageType;
  std::vector <std::string *> SURLOrStFN;
  std::vector <std::string *> storageSystemInfo;
  std::vector <int64_t *> lifetime;
  std::vector <std::string *> spaceToken;
} tArrayOfGetFileRequests;

typedef struct tArrayOfPutFileRequests
{ 
  std::vector <std::string *> fileStorageType;
  std::vector <int64_t *> knownSizeOfThisFile;
  std::vector <int64_t *> lifetime;
  std::vector <std::string *> spaceToken;
  std::vector <std::string *> SURLOrStFN;
  std::vector <std::string *> storageSystemInfo;
} tArrayOfPutFileRequests;

typedef struct tSurlInfoArray
{ 
  std::vector <std::string *> SURLOrStFN;
  std::vector <std::string *> storageSystemInfo;
} tSurlInfoArray;

typedef struct tPermissionArray
{ 
  std::vector <long int> mode;          /* enum */
  std::vector <std::string *> ID;
} tPermissionArray;

/* extern(al) function declarations */
extern int
AbortFiles(struct soap *soap,
           const char *srm_endpoint,
           const char *userID,
           const char *requestToken,
           std::vector <std::string *> arrayOfSiteURL,
           struct srm__srmAbortFilesResponse_ *resp);

extern int
AbortRequest(struct soap *soap,
             const char *srm_endpoint,
             const char *userID,
             const char *requestToken,
             struct srm__srmAbortRequestResponse_ *resp);

extern int
ChangeFileStorageType(struct soap *soap,
                      const char *srm_endpoint,
                      const char *userID,
                      const tSurlInfoArray arrayOfFileRequests,
                      const long int desiredStorageType,
                      struct srm__srmChangeFileStorageTypeResponse_ *resp);

extern int
CheckPermission(struct soap *soap,
                const char *srm_endpoint,
                const char *userID,
                const tSurlInfoArray arrayOfFileRequests,
                bool *checkInLocalCacheOnly, /* yes, no const in gsoap headers */
                struct srm__srmCheckPermissionResponse_ *resp);

extern int
CompactSpace(struct soap *soap,
             const char *srm_endpoint,
             const char *userID,
             const char *spaceToken,
             const char *storageSystemInfo,
             bool *doDynamicCompactFromNowOn, /* yes, no const in gsoap headers */
             struct srm__srmCompactSpaceResponse_ *resp);

extern int
Copy(struct soap *soap,
     const char *srm_endpoint,
     const char *userID,
     const tArrayOfCopyFileRequests arrayOfFileRequests,
     const char *userRequestDescription,
     const long int *overwriteOption,
     bool *removeSourceFiles, /* yes, no const in gsoap headers */
     const char *storageSystemInfo,
     const int64_t *totalRetryTime,
     struct srm__srmCopyResponse_ *resp);

extern int
ExtendFileLifeTime(struct soap *soap,
                   const char *srm_endpoint,
                   const char *userID,
                   const char *requestToken,
                   const char *siteURL,
                   const int64_t *newLifeTime,
                   struct srm__srmExtendFileLifeTimeResponse_ *resp);

extern int
GetRequestID(struct soap *soap,
             const char *srm_endpoint,
             const char *userID,
             const char *userRequestDescription,
             struct srm__srmGetRequestIDResponse_ *resp);

extern int
GetRequestSummary(struct soap *soap,
                  const char *srm_endpoint,
                  const char *userID,
                  std::vector <std::string *> arrayOfRequestToken,
                  struct srm__srmGetRequestSummaryResponse_ *resp);

extern int
GetSpaceMetaData(struct soap *soap,
                 const char *srm_endpoint,
                 const char *userID,
                 std::vector <std::string *> arrayOfSpaceToken,
                 struct srm__srmGetSpaceMetaDataResponse_ *resp);

extern int
GetSpaceToken(struct soap *soap,
              const char *srm_endpoint,
              const char *userID,
              const char *userSpaceTokenDescription,
              struct srm__srmGetSpaceTokenResponse_ *resp);

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
   struct srm__srmLsResponse_ *resp);

extern int
Mkdir(struct soap *soap,
      const char *srm_endpoint,
      const char *userID,
      const char *SURLOrStFN,
      const char *storageSystemInfo,
      struct srm__srmMkdirResponse_ *resp);

extern int
Mv(struct soap *soap,
   const char *srm_endpoint,
   const char *userID,
   const char *fromSURLOrStFN,
   const char *fromStorageSystemInfo,
   const char *toSURLOrStFN,
   const char *toStorageSystemInfo,
   struct srm__srmMvResponse_ *resp);

extern int
PrepareToGet(struct soap *soap,
             const char *srm_endpoint,
             const char *userID,
             const tArrayOfGetFileRequests arrayOfFileRequests,
             std::vector <std::string *> arrayOfTransferProtocols,
             const char *userRequestDescription,
             const char *storageSystemInfo,
             const int64_t *totalRetryTime,
             struct srm__srmPrepareToGetResponse_ *resp);

extern int
PrepareToPut(struct soap *soap,
             const char *srm_endpoint,
             const char *userID,
             const tArrayOfPutFileRequests arrayOfPutFileRequests,
             std::vector <std::string *> arrayOfTransferProtocols,
             const char *userRequestDescription,
             const long int *overwriteOption,
             const char *storageSystemInfo,
             const int64_t *totalRetryTime,
             struct srm__srmPrepareToPutResponse_ *resp);

extern int
PutDone(struct soap *soap,
        const char *srm_endpoint,
        const char *userID,
        const char *requestToken,
        std::vector <std::string *> arrayOfSiteURL,
        struct srm__srmPutDoneResponse_ *resp);

extern int
ReassignToUser(struct soap *soap,
               const char *srm_endpoint,
               const char *userID,
               const char *assignedUser,
               const int64_t *lifeTimeOfThisAssignment,
               const char *SURLOrStFN,
               const char *storageSystemInfo,
               struct srm__srmReassignToUserResponse_ *resp);

extern int
ReleaseFiles(struct soap *soap,
             const char *srm_endpoint,
             const char *userID,
             const char *requestToken,
             std::vector <std::string *> siteURLs,
             bool *keepSpace, /* yes, no const in gsoap headers */
             struct srm__srmReleaseFilesResponse_ *resp);

extern int
ReleaseSpace(struct soap *soap,
             const char *srm_endpoint,
             const char *userID,
             const char *spaceToken,
             const char *storageSystemInfo,
             bool *forceFileRelease, /* yes, no const in gsoap headers */
             struct srm__srmReleaseSpaceResponse_ *resp);

extern int
RemoveFiles(struct soap *soap,
            const char *srm_endpoint,
            const char *userID,
            const char *requestToken,
            std::vector <std::string *> siteURLs,
            struct srm__srmRemoveFilesResponse_ *resp);

extern int
ReserveSpace(struct soap *soap,
             const char *srm_endpoint,
             const char *userID,
             const long int typeOfSpace,
             const char *userSpaceTokenDescription,
             const int64_t *sizeOfTotalSpaceDesired,
             const int64_t *sizeOfGuaranteedSpaceDesired,
             const int64_t *lifetimeOfSpaceToReserve,
             const char *storageSystemInfo,
             struct srm__srmReserveSpaceResponse_ *resp);

extern int
ResumeRequest(struct soap *soap,
              const char *srm_endpoint,
              const char *userID,
              const char *requestToken,
              struct srm__srmResumeRequestResponse_ *resp);

extern int
Rm(struct soap *soap,
   const char *srm_endpoint,
   const char *userID,
   const tSurlInfoArray arrayOfFileRequests,
   struct srm__srmRmResponse_ *resp);

extern int
Rmdir(struct soap *soap,
      const char *srm_endpoint,
      const char *userID,
      const char *SURLOrStFN,
      const char *storageSystemInfo,
      bool *recursive, /* yes, no const in gsoap headers */
      struct srm__srmRmdirResponse_ *resp);

extern int
SetPermission(struct soap *soap,
              const char *srm_endpoint,
              const char *userID,
              const char *SURLOrStFN,
              const char *storageSystemInfo,
              const long int permissionType,
              const long int *ownerPermission,
              tPermissionArray userPermissionArray,
              tPermissionArray groupPermissionArray,
              const long int *otherPermission,
              struct srm__srmSetPermissionResponse_ *resp);

extern int
StatusOfCopyRequest(struct soap *soap,
                    const char *srm_endpoint,
                    const char *userID,
                    const char *requestToken,
                    std::vector <std::string *> arrayOfFromSURLs,
                    std::vector <std::string *> arrayOfToSURLs,
                    struct srm__srmStatusOfCopyRequestResponse_ *resp);

extern int
StatusOfGetRequest(struct soap *soap,
                   const char *srm_endpoint,
                   const char *userID,
                   const char *requestToken,
                   std::vector <std::string *> arrayOfFromSURLs,
                   struct srm__srmStatusOfGetRequestResponse_ *resp);

extern int
StatusOfPutRequest(struct soap *soap,
                   const char *srm_endpoint,
                   const char *userID,
                   const char *requestToken,
                   std::vector <std::string *> arrayOfToSURLs,
                   struct srm__srmStatusOfPutRequestResponse_ *resp);

extern int
SuspendRequest(struct soap *soap,
               const char *srm_endpoint,
               const char *userID,
               const char *requestToken,
               struct srm__srmSuspendRequestResponse_ *resp);

extern int
UpdateSpace(struct soap *soap,
            const char *srm_endpoint,
            const char *userID,
            const char *spaceToken,
            const int64_t *newSizeOfTotalSpaceDesired,
            const int64_t *newSizeOfGuaranteedSpaceDesired,
            const int64_t *newLifeTimeFromCallingTime,
            const char *storageSystemInfo,
            struct srm__srmUpdateSpaceResponse_ *resp);

#endif /* _SRM2API_H */
