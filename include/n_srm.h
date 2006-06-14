#ifndef _N_SRM_H
#define _N_SRM_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#if defined(HAVE_SRM21)

#include "n.h"			/* Node */

#define SRM2_CALL		/* disable for debug purposes */

/* simple macros */
#define SS_SRM(method)\
  if(srm_endpoint == NULL) {\
    DM_ERR_ASSERT(_("srm_endpoint == NULL\n"));\
    return ss.str();\
  }\
  ss << method << " " << Process::eval_str(srm_endpoint,proc)

#define SS_P_SPACETYPE(r,param)\
  if(r->param) ss << " "#param << "=" << getTSpaceType(*(r->param))

#define SS_P_SRM_RETSTAT(r)\
  if(r->returnStatus && r->returnStatus->explanation)\
    ss << " returnStatus.explanation=" << dq_param(r->returnStatus->explanation, quote);\
  if(r->returnStatus)\
    ss << " returnStatus.statusCode=" << getTStatusCode(r->returnStatus->statusCode)

#define SS_P_VEC_PAR_PERMISSIONTYPE(param)\
  if(v[i] && v[i]->param) {SS_VEC_SPACE; ss << ""#param << i << "=" << getTPermissionType(*(v[i]->param));}

#define SS_P_VEC_PAR_PERMISSIONMODE(param)\
  if(v[i] && v[i]->param) {SS_VEC_SPACE; ss << ""#param << i << "=" << getTPermissionMode(v[i]->param->mode);}

#define SS_P_VEC_PAR_PERMISSIONMODE_PTR(param)\
  if(v[i] && v[i]->param) {SS_VEC_SPACE; ss << ""#param << i << "=" << getTPermissionMode(*(v[i]->param));}

#define SS_P_VEC_PAR_REQUESTTYPE(param)\
  if(v[i] && v[i]->param) {SS_VEC_SPACE; ss << ""#param << i << "=" << getTRequestType(*(v[i]->param));}

#define SS_P_VEC_PAR_SPACETYPE(param)\
  if(v[i] && v[i]->param) {SS_VEC_SPACE; ss << ""#param << i << "=" << getTSpaceType(*(v[i]->param));}

#define SS_P_VEC_SRM_RETSTAT(param)\
  if(v[i] && v[i]->param && v[i]->param->explanation)\
    {SS_VEC_SPACE; ss << "returnStatus.explanation" << i << "=" << dq_param(v[i]->param->explanation, quote);}\
  if(v[i] && v[i]->param)\
    {SS_VEC_SPACE; ss << "returnStatus.statusCode" << i << "=" << getTStatusCode(v[i]->param->statusCode);}

#define EAT_MATCH(p,q,recv)\
  if(p->q) {\
    match = proc->e_match(q, recv);\
    if(!match) {\
      DM_DBG(DM_N(1), "no match\n");\
      return ERR_ERR;\
    }\
  }

/* type definitions */
typedef struct tArrayOfCopyFileRequests_ /* <std::string *> version of tArrayOfCopyFileRequests */
{ 
  std::vector <std::string *> allLevelRecursive;
  std::vector <std::string *> isSourceADirectory;
  std::vector <std::string *> numOfLevels;
  std::vector <std::string *> fileStorageType;
  std::vector <std::string *> fromSURLOrStFN;
  std::vector <std::string *> fromStorageSystemInfo;
  std::vector <std::string *> lifetime;
  std::vector <std::string *> overwriteMode;
  std::vector <std::string *> spaceToken;
  std::vector <std::string *> toSURLOrStFN;
  std::vector <std::string *> toStorageSystemInfo;
} tArrayOfCopyFileRequests_;

typedef struct tArrayOfGetFileRequests_ /* <std::string *> version of tArrayOfGetFileRequests */
{ 
  std::vector <std::string *> allLevelRecursive;
  std::vector <std::string *> isSourceADirectory;
  std::vector <std::string *> numOfLevels;
  std::vector <std::string *> fileStorageType;
  std::vector <std::string *> SURLOrStFN;
  std::vector <std::string *> storageSystemInfo;
  std::vector <std::string *> lifetime;
  std::vector <std::string *> spaceToken;
} tArrayOfGetFileRequests_;

typedef struct tArrayOfPutFileRequests_ /* <std::string *> version of tArrayOfPutFileRequests */
{ 
  std::vector <std::string *> fileStorageType;
  std::vector <std::string *> knownSizeOfThisFile;
  std::vector <std::string *> lifetime;
  std::vector <std::string *> spaceToken;
  std::vector <std::string *> SURLOrStFN;
  std::vector <std::string *> storageSystemInfo;
} tArrayOfPutFileRequests_;

typedef struct tSurlInfoArray_ /* <std::string *> version of tSurlInfoArray */
{ 
  std::vector <std::string *> SURLOrStFN;
  std::vector <std::string *> storageSystemInfo;
} tSurlInfoArray_;

typedef struct tPermissionArray_ /* <std::string *> version of tPermissionArray_ */
{ 
  std::vector <std::string *> mode;
  std::vector <std::string *> ID;
} tPermissionArray_;

struct SRM2 : public Node
{
  std::string *srm_endpoint;
  std::string *userID;

  struct tReturnStatus
  {
    std::string *explanation;
    std::string *statusCode;
  } returnStatus;

public:
  SRM2();
  ~SRM2();

  int matchReturnStatus(struct srm__TReturnStatus *returnStatus, Process *proc);
  std::vector <const long int *> eval_vec_overwrite_mode(const std::vector <std::string *> &v, Process *proc);
  std::vector <long int> eval_vec_permission_mode(const std::vector <std::string *> &v, Process *proc);

};

/*
 * srmAbortFiles request
 */
struct srmAbortFiles : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *>surlArray;

  /* response (parser) */
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmAbortFilesResponse_ *resp;

public:
  srmAbortFiles();
  srmAbortFiles(Node &node);
  ~srmAbortFiles();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfAbortFilesResponseToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmAbortRequest request
 */
struct srmAbortRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;

  /* response (parser) */

  /* response (API) */
  struct srm__srmAbortRequestResponse_ *resp;

public:
  srmAbortRequest();
  srmAbortRequest(Node &node);
  ~srmAbortRequest();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmCompactSpace request
 */
struct srmCompactSpace : public SRM2
{
  /* request (parser/API) */
  std::string *spaceToken;
  std::string *storageSystemInfo;
  std::string *doDynamicCompactFromNowOn;

  /* response (parser) */
  std::string *newSizeOfThisSpace;

  /* response (API) */
  struct srm__srmCompactSpaceResponse_ *resp;

public:
  srmCompactSpace();
  srmCompactSpace(Node &node);
  ~srmCompactSpace();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmCopy request
 */
struct srmCopy : public SRM2
{
  /* request (parser/API) */
  tArrayOfCopyFileRequests_ arrayOfFileRequests;

  std::string *userRequestDescription;
  std::string *overwriteOption;
  std::string *removeSourceFiles;
  std::string *storageSystemInfo;
  std::string *totalRetryTime;
  
  /* response (parser) */
  std::string *requestToken;
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmCopyResponse_ *resp;

public:
  srmCopy();
  srmCopy(Node &node);
  ~srmCopy();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmExtendFileLifeTime request
 */
struct srmExtendFileLifeTime : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::string *siteURL;
  std::string *newLifeTime;

  /* response (parser) */
  std::string *newTimeExtended;

  /* response (API) */
  struct srm__srmExtendFileLifeTimeResponse_ *resp;

public:
  srmExtendFileLifeTime();
  srmExtendFileLifeTime(Node &node);
  ~srmExtendFileLifeTime();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmGetRequestID request
 */
struct srmGetRequestID : public SRM2
{
  /* request (parser/API) */
  std::string *userRequestDescription;

  /* response (parser) */
  std::string *requestTokens;

  /* response (API) */
  struct srm__srmGetRequestIDResponse_ *resp;

public:
  srmGetRequestID();
  srmGetRequestID(Node &node);
  ~srmGetRequestID();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfRequestDetailsToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmGetRequestSummary request
 */
struct srmGetRequestSummary : public SRM2
{
  /* request (parser/API) */
  std::vector <std::string *> arrayOfRequestToken;

  /* response (parser) */
  std::string *requestSummary;

  /* response (API) */
  struct srm__srmGetRequestSummaryResponse_ *resp;

public:
  srmGetRequestSummary();
  srmGetRequestSummary(Node &node);
  ~srmGetRequestSummary();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfRequestDetailsToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmGetSpaceMetaData request
 */
struct srmGetSpaceMetaData : public SRM2
{
  /* request (parser/API) */
  std::vector <std::string *> arrayOfSpaceToken;

  /* response (parser) */
  std::string *spaceDetails;

  /* response (API) */
  struct srm__srmGetSpaceMetaDataResponse_ *resp;

public:
  srmGetSpaceMetaData();
  srmGetSpaceMetaData(Node &node);
  ~srmGetSpaceMetaData();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfSpaceDetailsToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmGetSpaceToken request
 */
struct srmGetSpaceToken : public SRM2
{
  /* request (parser/API) */
  std::string *userSpaceTokenDescription;

  /* response (parser) */
  std::string *possibleSpaceTokens;

  /* response (API) */
  struct srm__srmGetSpaceTokenResponse_ *resp;

public:
  srmGetSpaceToken();
  srmGetSpaceToken(Node &node);
  ~srmGetSpaceToken();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmChangeFileStorageType request
 */
struct srmChangeFileStorageType : public SRM2
{
  /* request (parser/API) */
  tSurlInfoArray_ path;
  std::string *desiredStorageType;
  
  /* response (parser) */
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmChangeFileStorageTypeResponse_ *resp;

public:
  srmChangeFileStorageType();
  srmChangeFileStorageType(Node &node);
  ~srmChangeFileStorageType();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmCheckPermission request
 */
struct srmCheckPermission : public SRM2
{
  /* request (parser/API) */
  tSurlInfoArray_ path;
  std::string *checkInLocalCacheOnly;
  
  /* response (parser) */
  std::string *permissions;

  /* response (API) */
  struct srm__srmCheckPermissionResponse_ *resp;

public:
  srmCheckPermission();
  srmCheckPermission(Node &node);
  ~srmCheckPermission();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmLs request
 */
struct srmLs : public SRM2
{
  /* request (parser/API) */
  tSurlInfoArray_ path;
  std::string *fileStorageType;
  std::string *fullDetailedList;
  std::string *allLevelRecursive;
  std::string *numOfLevels;
  std::string *offset;
  std::string *count;
  
  /* response (parser) */
  std::string *pathDetails;

  /* response (API) */
  struct srm__srmLsResponse_ *resp;

public:
  srmLs();
  srmLs(Node &node);
  ~srmLs();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(BOOL space, BOOL quote) const;
  std::string srmLs::arrayOfFileStatusToString(BOOL space, BOOL quote, std::vector<struct srm__TMetaDataPathDetail *> details) const;

private:
};

/*
 * srmMkdir request
 */
struct srmMkdir : public SRM2
{
  /* request (parser/API) */
  std::string *SURLOrStFN;
  std::string *storageSystemInfo;

  /* response (parser) */

  /* response (API) */
  struct srm__srmMkdirResponse_ *resp;

public:
  srmMkdir();
  srmMkdir(Node &node);
  ~srmMkdir();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmMv request
 */
struct srmMv : public SRM2
{
  /* request (parser/API) */
  std::string *fromSURLOrStFN;
  std::string *fromStorageSystemInfo;
  std::string *toSURLOrStFN;
  std::string *toStorageSystemInfo;

  /* response (parser) */

  /* response (API) */
  struct srm__srmMvResponse_ *resp;

public:
  srmMv();
  srmMv(Node &node);
  ~srmMv();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmPrepareToGet request
 */
struct srmPrepareToGet : public SRM2
{
  /* request (parser/API) */
  tArrayOfGetFileRequests_ arrayOfFileRequests;

  std::vector <std::string *> arrayOfTransferProtocols;
  std::string *userRequestDescription;
  std::string *storageSystemInfo;
  std::string *totalRetryTime;
  
  /* response (parser) */
  std::string *requestToken;
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmPrepareToGetResponse_ *resp;

public:
  srmPrepareToGet();
  srmPrepareToGet(Node &node);
  ~srmPrepareToGet();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmPrepareToPut request
 */
struct srmPrepareToPut : public SRM2
{
  /* request (parser/API) */
  tArrayOfPutFileRequests_ arrayOfFileRequests;

  std::vector <std::string *> arrayOfTransferProtocols;
  std::string *userRequestDescription;
  std::string *overwriteOption;
  std::string *storageSystemInfo;
  std::string *totalRetryTime;
  
  /* response (parser) */
  std::string *requestToken;
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmPrepareToPutResponse_ *resp;

public:
  srmPrepareToPut();
  srmPrepareToPut(Node &node);
  ~srmPrepareToPut();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmPutDone request
 */
struct srmPutDone : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *>surlArray;

  /* response (parser) */
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmPutDoneResponse_ *resp;

public:
  srmPutDone();
  srmPutDone(Node &node);
  ~srmPutDone();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfPutDoneResponseToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmReassignToUser request
 */
struct srmReassignToUser : public SRM2
{
  /* request (parser/API) */
  std::string *assignedUser;
  std::string *lifeTimeOfThisAssignment;
  std::string *SURLOrStFN;
  std::string *storageSystemInfo;

  /* response (parser) */

  /* response (API) */
  struct srm__srmReassignToUserResponse_ *resp;

public:
  srmReassignToUser();
  srmReassignToUser(Node &node);
  ~srmReassignToUser();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmReleaseFiles request
 */
struct srmReleaseFiles : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *>surlArray;
  std::string *keepFiles;

  /* response (parser) */
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmReleaseFilesResponse_ *resp;

public:
  srmReleaseFiles();
  srmReleaseFiles(Node &node);
  ~srmReleaseFiles();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfReleaseFilesResponseToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmReleaseSpace request
 */
struct srmReleaseSpace : public SRM2
{
  /* request (parser/API) */
  std::string *spaceToken;
  std::string *storageSystemInfo;
  std::string *forceFileRelease;

  /* response (parser) */

  /* response (API) */
  struct srm__srmReleaseSpaceResponse_ *resp;

public:
  srmReleaseSpace();
  srmReleaseSpace(Node &node);
  ~srmReleaseSpace();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmRemoveFiles request
 */
struct srmRemoveFiles : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *>surlArray;

  /* response (parser) */
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmRemoveFilesResponse_ *resp;

public:
  srmRemoveFiles();
  srmRemoveFiles(Node &node);
  ~srmRemoveFiles();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfRemoveFilesResponseToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmReserveSpace request
 */
struct srmReserveSpace : public SRM2
{
  /* request (parser/API) */
  std::string *typeOfSpace;
  std::string *userSpaceTokenDescription;
  std::string *sizeOfTotalSpaceDesired;
  std::string *sizeOfGuaranteedSpaceDesired;
  std::string *lifetimeOfSpaceToReserve;
  std::string *storageSystemInfo;

  /* response (parser) */
  std::string *typeOfReservedSpace;
  std::string *sizeOfTotalReservedSpace;
  std::string *sizeOfGuaranteedReservedSpace;
  std::string *lifetimeOfReservedSpace;
  std::string *referenceHandleOfReservedSpace;

  /* response (API) */
  struct srm__srmReserveSpaceResponse_ *resp;

public:
  srmReserveSpace();
  srmReserveSpace(Node &node);
  ~srmReserveSpace();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmResumeRequest request
 */
struct srmResumeRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;

  /* response (parser) */

  /* response (API) */
  struct srm__srmResumeRequestResponse_ *resp;

public:
  srmResumeRequest();
  srmResumeRequest(Node &node);
  ~srmResumeRequest();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmRm request
 */
struct srmRm : public SRM2
{
  /* request (parser/API) */
  tSurlInfoArray_ path;
  
  /* response (parser) */
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmRmResponse_ *resp;

public:
  srmRm();
  srmRm(Node &node);
  ~srmRm();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmRmdir request
 */
struct srmRmdir : public SRM2
{
  /* request (parser/API) */
  std::string *SURLOrStFN;
  std::string *storageSystemInfo;
  std::string *recursive;

  /* response (parser) */

  /* response (API) */
  struct srm__srmRmdirResponse_ *resp;

public:
  srmRmdir();
  srmRmdir(Node &node);
  ~srmRmdir();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmSetPermission request
 */
struct srmSetPermission : public SRM2
{
  /* request (parser/API) */
  std::string *SURLOrStFN;
  std::string *storageSystemInfo;
  std::string *permissionType;
  std::string *ownerPermission;
  tPermissionArray_ userPermissionArray;
  tPermissionArray_ groupPermissionArray;
  std::string *otherPermission;

  /* response (parser) */

  /* response (API) */
  struct srm__srmSetPermissionResponse_ *resp;

public:
  srmSetPermission();
  srmSetPermission(Node &node);
  ~srmSetPermission();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmStatusOfCopyRequest request
 */
struct srmStatusOfCopyRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *>fromSurlArray;
  std::vector <std::string *>toSurlArray;

  /* response (parser) */
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmStatusOfCopyRequestResponse_ *resp;

public:
  srmStatusOfCopyRequest();
  srmStatusOfCopyRequest(Node &node);
  ~srmStatusOfCopyRequest();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfStatusOfCopyRequestResponseToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmStatusOfGetRequest request
 */
struct srmStatusOfGetRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *>surlArray;

  /* response (parser) */
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmStatusOfGetRequestResponse_ *resp;

public:
  srmStatusOfGetRequest();
  srmStatusOfGetRequest(Node &node);
  ~srmStatusOfGetRequest();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfStatusOfGetRequestResponseToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmStatusOfPutRequest request
 */
struct srmStatusOfPutRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *>surlArray;

  /* response (parser) */
  std::string *fileStatuses;

  /* response (API) */
  struct srm__srmStatusOfPutRequestResponse_ *resp;

public:
  srmStatusOfPutRequest();
  srmStatusOfPutRequest(Node &node);
  ~srmStatusOfPutRequest();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfStatusOfPutRequestResponseToString(BOOL space, BOOL quote) const;

private:
};

/*
 * srmSuspendRequest request
 */
struct srmSuspendRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;

  /* response (parser) */

  /* response (API) */
  struct srm__srmSuspendRequestResponse_ *resp;

public:
  srmSuspendRequest();
  srmSuspendRequest(Node &node);
  ~srmSuspendRequest();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmUpdateSpace request
 */
struct srmUpdateSpace : public SRM2
{
  /* request (parser/API) */
  std::string *spaceToken;
  std::string *newSizeOfTotalSpaceDesired;
  std::string *newSizeOfGuaranteedSpaceDesired;
  std::string *newLifeTimeFromCallingTime;
  std::string *storageSystemInfo;

  /* response (parser) */
  std::string *sizeOfTotalSpace;
  std::string *sizeOfGuaranteedSpace;
  std::string *lifetimeGranted;

  /* response (API) */
  struct srm__srmUpdateSpaceResponse_ *resp;

public:
  srmUpdateSpace();
  srmUpdateSpace(Node &node);
  ~srmUpdateSpace();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

#endif	/* HAVE_SRM21 */


#endif /* _N_SRM_H */
