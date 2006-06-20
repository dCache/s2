#ifndef _N_SRM_H
#define _N_SRM_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

/* macros common to all SRM methods */
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

typedef struct tSoapCallRet
{ 
  struct soap *soap;	/* gSoap structure */
  void *resp;		/* SRM response */
} tSoapCallRet;

#define NEW_SRM_RET(r)\
  proc->ret = (tSoapCallRet *) malloc(sizeof(tSoapCallRet));\
  if(proc->ret == NULL) {\
    DM_ERR(ERR_SYSTEM, "malloc failed\n");\
    RETURN(ERR_SYSTEM);\
  }\
  srm__srm##r##Response_ *resp = (srm__srm##r##Response_ *)((tSoapCallRet *)proc->ret)->resp = new srm__srm##r##Response_();\
  struct soap *soap = ((tSoapCallRet *)proc->ret)->soap = soap_new();\
  if(resp == NULL) {\
    DM_ERR(ERR_SYSTEM, "new failed\n");\
    RETURN(ERR_SYSTEM);\
  } else {\
    memset(resp, 0, sizeof(srm__srm##r##Response_));\
  }

#define GET_SRM_RESP(r)	srm__srm##r##Response_ *resp = proc && proc->ret? (srm__srm##r##Response_ *)((tSoapCallRet *)proc->ret)->resp : NULL
#define GET_SRM_SOAP	struct soap *soap = proc && proc->ret? (struct soap *)((tSoapCallRet *)proc->ret)->soap : NULL
#define FREE_SRM_RET(r)\
  do {\
    GET_SRM_SOAP;\
    GET_SRM_RESP(r);\
    if(soap) {\
      DM_DBG(DM_N(1), "freeing soap=%p\n", soap);\
      if(resp) {\
        soap_delete_srm__srm##r##Response(soap, resp->srm##r##Response);\
	soap_delete(soap, resp);\
      };\
      soap_destroy(soap);\
      soap_end(soap);\
      FREE(soap);\
    };\
    FREE((tSoapCallRet *)proc->ret);\
  } while(0)

#if 0
  GET_SRM_RESP(r);
  FREE(soap);
  if(resp) soap_delete(soap, resp);
  soap_destroy(soap);
  soap_free(soap);
  soap_done(soap);	/* SEGVs if CGSI plugin is used */
#endif

#include "n.h"			/* Node */
#define SRM2_CALL		/* disable for debug purposes */


/********************************************************************/
#ifdef HAVE_SRM21

/* simple macros */

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

public:
  srmAbortFiles();
  srmAbortFiles(Node &node);
  ~srmAbortFiles();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfAbortFilesResponseToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmAbortRequest();
  srmAbortRequest(Node &node);
  ~srmAbortRequest();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmCompactSpace();
  srmCompactSpace(Node &node);
  ~srmCompactSpace();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmCopy();
  srmCopy(Node &node);
  ~srmCopy();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmExtendFileLifeTime();
  srmExtendFileLifeTime(Node &node);
  ~srmExtendFileLifeTime();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmGetRequestID();
  srmGetRequestID(Node &node);
  ~srmGetRequestID();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfRequestDetailsToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmGetRequestSummary();
  srmGetRequestSummary(Node &node);
  ~srmGetRequestSummary();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfRequestDetailsToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmGetSpaceMetaData();
  srmGetSpaceMetaData(Node &node);
  ~srmGetSpaceMetaData();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfSpaceDetailsToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmGetSpaceToken();
  srmGetSpaceToken(Node &node);
  ~srmGetSpaceToken();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmChangeFileStorageType();
  srmChangeFileStorageType(Node &node);
  ~srmChangeFileStorageType();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmCheckPermission();
  srmCheckPermission(Node &node);
  ~srmCheckPermission();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmLs();
  srmLs(Node &node);
  ~srmLs();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const;
  std::string srmLs::arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote, std::vector<struct srm__TMetaDataPathDetail *> details) const;

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

public:
  srmMkdir();
  srmMkdir(Node &node);
  ~srmMkdir();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmMv();
  srmMv(Node &node);
  ~srmMv();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmPrepareToGet();
  srmPrepareToGet(Node &node);
  ~srmPrepareToGet();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmPrepareToPut();
  srmPrepareToPut(Node &node);
  ~srmPrepareToPut();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmPutDone();
  srmPutDone(Node &node);
  ~srmPutDone();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfPutDoneResponseToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmReassignToUser();
  srmReassignToUser(Node &node);
  ~srmReassignToUser();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmReleaseFiles();
  srmReleaseFiles(Node &node);
  ~srmReleaseFiles();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfReleaseFilesResponseToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmReleaseSpace();
  srmReleaseSpace(Node &node);
  ~srmReleaseSpace();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmRemoveFiles();
  srmRemoveFiles(Node &node);
  ~srmRemoveFiles();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfRemoveFilesResponseToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmReserveSpace();
  srmReserveSpace(Node &node);
  ~srmReserveSpace();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmResumeRequest();
  srmResumeRequest(Node &node);
  ~srmResumeRequest();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmRm();
  srmRm(Node &node);
  ~srmRm();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmRmdir();
  srmRmdir(Node &node);
  ~srmRmdir();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmSetPermission();
  srmSetPermission(Node &node);
  ~srmSetPermission();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmStatusOfCopyRequest();
  srmStatusOfCopyRequest(Node &node);
  ~srmStatusOfCopyRequest();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfStatusOfCopyRequestResponseToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmStatusOfGetRequest();
  srmStatusOfGetRequest(Node &node);
  ~srmStatusOfGetRequest();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfStatusOfGetRequestResponseToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmStatusOfPutRequest();
  srmStatusOfPutRequest(Node &node);
  ~srmStatusOfPutRequest();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfStatusOfPutRequestResponseToString(Process *proc, BOOL space, BOOL quote) const;

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

public:
  srmSuspendRequest();
  srmSuspendRequest(Node &node);
  ~srmSuspendRequest();

  virtual void init();
  virtual void finish(Process *proc);
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

public:
  srmUpdateSpace();
  srmUpdateSpace(Node &node);
  ~srmUpdateSpace();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

#endif	/* HAVE_SRM21 */

/********************************************************************/
struct SRM2 : public Node
{
  std::string *srm_endpoint;
  std::string *authorizationID;

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


#ifdef HAVE_SRM22

/* type definitions */
typedef struct tArrayOfPutFileRequests_
{ 
  std::vector <std::string *> targetSURL;
  std::vector <std::string *> expectedFileSize;
};

typedef struct tStorageSystemInfo_
{ 
  std::vector <std::string *> key;
  std::vector <std::string *> value;
};

/*
 * srmPrepareToPut request
 */
struct srmPrepareToPut : public SRM2
{
  /* request (parser/API) */
  tArrayOfPutFileRequests_ putFileRequests;

  std::string *userRequestDescription;
  std::string *overwriteOption;

  tStorageSystemInfo_ storageSystemInfo;

  std::string *desiredTotalRequestTime;
  std::string *desiredPinLifeTime;
  std::string *desiredFileLifeTime;
  std::string *desiredFileStorageType;
  std::string *targetSpaceToken;
  std::string *retentionPolicy;
  std::string *accessLatency;
  std::string *accessPattern;
  std::string *connectionType;

  std::vector <std::string *> clientNetworks;
  std::vector <std::string *> transferProtocols;
 
  /* response (parser) */
  std::string *requestToken;
  std::string *fileStatuses;
  std::string *remainingTotalRequestTime;

public:
  srmPrepareToPut();
  srmPrepareToPut(Node &node);
  ~srmPrepareToPut();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const;

private:
};

#endif	/* HAVE_SRM22 */

#endif /* _N_SRM_H */
