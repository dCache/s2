#ifndef _N_SRM_H
#define _N_SRM_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

/* macros common to all SRM methods */
#define CSTR(s)		(s)? s->c_str(): (const char *)NULL		/* no SEGVs if s is NULL */
#define PI2CSTR(s)	(s)? i2str(*(s)).c_str(): (const char *)NULL	/* no SEGVs if s is NULL */

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
  if(v[u] && v[u]->param) {SS_VEC_SPACE; ss << ""#param << u << "=" << getTPermissionType(*(v[u]->param));}

#define SS_P_VEC_PAR_PERMISSIONMODE(param)\
  if(v[u] && v[u]->param) {SS_VEC_SPACE; ss << ""#param << u << "=" << getTPermissionMode(v[u]->param->mode);}

#define SS_P_VEC_DPAR_PERMISSIONMODE(param)\
  if(v[u] && v[u]->param) {SS_VEC_SPACE; ss << ""#param << u << "=" << getTPermissionMode(*(v[u]->param));}

#define SS_P_VEC_PAR_REQUESTTYPE(param)\
  if(v[u] && v[u]->param) {SS_VEC_SPACE; ss << ""#param << u << "=" << getTRequestType(*(v[u]->param));}

#define SS_P_VEC_PAR_SPACETYPE(param)\
  if(v[u] && v[u]->param) {SS_VEC_SPACE; ss << ""#param << u << "=" << getTSpaceType(*(v[u]->param));}

#define SS_P_VEC_SRM_RETSTAT(param)\
  if(v[u] && v[u]->param && v[u]->param->explanation)\
    {SS_VEC_SPACE; ss << "returnStatus.explanation" << u << "=" << dq_param(v[u]->param->explanation, quote);}\
  if(v[u] && v[u]->param)\
    {SS_VEC_SPACE; ss << "returnStatus.statusCode" << u << "=" << getTStatusCode(v[u]->param->statusCode);}

#define SS_P_VEC_SRM_RETENTION_POLICY(par)\
  if(v[u] && v[u]->par) {\
    SS_VEC_SPACE;\
    ss << "retentionPolicy" << u << "=" << getTRetentionPolicy((v[u]->par->retentionPolicy));\
    if(v[u]->par->accessLatency) {\
      SS_VEC_SPACE;\
      ss << "accessLatency" << u << "=" << getTRetentionPolicy(*(v[u]->par->accessLatency));\
    }\
  }

#define EAT_MATCH(p,r)\
  do {\
    if(!proc->e_match(p,r)) {\
      DM_DBG(DM_N(1), "no match\n");\
      return ERR_ERR;\
    }\
  } while(0)

#define EAT_MATCH_C(c,p,r) if(c) EAT_MATCH(p,r)

/* SRM 2.2 macros */
#define SS_P_VEC_SRM_EXTRA_INFOu(v) \
  for(uint __u = 0; __u < v.size(); __u++) {\
    if(v[__u]->value) {SS_VEC_SPACE; ss << v[__u]->key << u << ":" << __u << "=" << dq_param(Process::eval_str(v[__u]->value,proc), quote);}\
  }

#define SS_P_VEC_SRM_EXTRA_INFO(v) \
  for(uint __u = 0; __u < v.size(); __u++) {\
    if(v[__u]->value) {SS_VEC_SPACE; ss << v[__u]->key << ":" << __u << "=" << dq_param(Process::eval_str(v[__u]->value,proc), quote);}\
  }

#define SS_P_VEC_PAR_SOAP(t,param)\
  if(v[u] && v[u]->param) {SS_VEC_SPACE; ss << ""#param << u << "=" << getT##t((v[u]->param));}

#define SS_P_VEC_DPAR_SOAP(t,param)\
  if(v[u] && v[u]->param) {SS_VEC_SPACE; ss << ""#param << u << "=" << getT##t(*(v[u]->param));}


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
  struct srm__srm##r##Response_ *resp = new srm__srm##r##Response_();\
  ((tSoapCallRet *)proc->ret)->resp = resp;\
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
    FREE(proc->ret);\
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
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote, std::vector<struct srm__TMetaDataPathDetail *> details) const;

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

#ifdef HAVE_SRM22
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

/* type definitions */
typedef struct tStorageSystemInfo_
{ 
  std::vector <std::string *> key;
  std::vector <std::string *> value;
};

typedef struct tArrayOfGetFileRequests_
{ 
  std::vector <std::string *> SURL;
  std::vector <std::string *> isSourceADirectory;
  std::vector <std::string *> allLevelRecursive;
  std::vector <std::string *> numOfLevels;
};

typedef struct tArrayOfPutFileRequests_
{ 
  std::vector <std::string *> SURL;
  std::vector <std::string *> expectedFileSize;
};

/*
 * srmBringOnline request
 */
struct srmBringOnline : public SRM2
{
  /* request (parser/API) */
  tArrayOfGetFileRequests_ fileRequests;

  std::string *userRequestDescription;

  tStorageSystemInfo_ storageSystemInfo;
  std::string *desiredFileStorageType;
  std::string *desiredTotalRequestTime;
  std::string *desiredLifeTime;
  std::string *targetSpaceToken;
  std::string *retentionPolicy;
  std::string *accessLatency;
  std::string *accessPattern;
  std::string *connectionType;
  
  std::vector <std::string *> clientNetworks;
  std::vector <std::string *> transferProtocols;

  std::string *deferredStartTime;
  
  /* response (parser) */
  std::string *requestToken;
  std::string *fileStatuses;
  std::string *remainingTotalRequestTime;
  std::string *remainingDeferredStartTime;

public:
  srmBringOnline();
  srmBringOnline(Node &node);
  ~srmBringOnline();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const;

private:
};

/*
 * srmCopy request
 */
struct srmCopy : public SRM2
{
  /* request (parser/API) */
  std::vector <std::string *> sourceSURL;
  std::vector <std::string *> targetSURL;

  std::vector <std::string *> isSourceADirectory;
  std::vector <std::string *> allLevelRecursive;
  std::vector <std::string *> numOfLevels;

  std::string *userRequestDescription;
  std::string *overwriteOption;
  std::string *desiredTotalRequestTime;
  std::string *desiredTargetSURLLifeTime;
  std::string *targetFileStorageType;
  std::string *targetSpaceToken;
  std::string *targetFileRetentionPolicyInfo;
  std::string *retentionPolicy;
  std::string *accessLatency;

  tStorageSystemInfo_ sourceStorageSystemInfo;
  tStorageSystemInfo_ targetStorageSystemInfo;

  /* response (parser) */
  std::string *requestToken;
  std::string *fileStatuses;
  std::string *remainingTotalRequestTime;

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
 * srmChangeSpaceForFiles request
 */
struct srmChangeSpaceForFiles : public SRM2
{
  /* request (parser/API) */
  std::string *spaceToken;
  tStorageSystemInfo_ storageSystemInfo;
  std::vector <std::string *> SURL;

  /* response (parser) */
  std::string *requestToken;
  std::string *estimatedProcessingTime;
  std::string *fileStatuses;

public:
  srmChangeSpaceForFiles();
  srmChangeSpaceForFiles(Node &node);
  ~srmChangeSpaceForFiles();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string srmChangeSpaceForFiles::arrayOfChangeSpaceForFilesResponseToString(Process *proc, BOOL space, BOOL quote) const;

private:
};

/*
 * srmExtendFileLifeTimeInSpace request
 */
struct srmExtendFileLifeTimeInSpace : public SRM2
{
  /* request (parser/API) */
  std::string *spaceToken;
  std::vector <std::string *> SURL;
  std::string *newLifeTime;

  /* response (parser) */
  std::string *newTimeExtended;
  std::string *fileStatuses;

public:
  srmExtendFileLifeTimeInSpace();
  srmExtendFileLifeTimeInSpace(Node &node);
  ~srmExtendFileLifeTimeInSpace();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string srmExtendFileLifeTimeInSpace::arrayOfExtendFileLifeTimeInSpaceResponseToString(Process *proc, BOOL space, BOOL quote) const;

private:
};

/*
 * srmGetSpaceMetaData request
 */
struct srmGetSpaceMetaData : public SRM2
{
  /* request (parser/API) */
  std::vector <std::string *> spaceTokens;

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
 * srmGetSpaceTokens request
 */
struct srmGetSpaceTokens : public SRM2
{
  /* request (parser/API) */
  std::string *userSpaceTokenDescription;

  /* response (parser) */
  std::string *spaceTokens;

public:
  srmGetSpaceTokens();
  srmGetSpaceTokens(Node &node);
  ~srmGetSpaceTokens();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfSpaceTokensToString(Process *proc, BOOL space, BOOL quote) const;

private:
};

/*
 * srmLs request
 */
struct srmLs : public SRM2
{
  /* request (parser/API) */
  std::vector <std::string *> SURL;
  tStorageSystemInfo_ storageSystemInfo;
  
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
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote, std::vector<struct srm__TMetaDataPathDetail *> details) const;

private:
};

/*
 * srmPing request
 */
struct srmPing : public SRM2
{
  /* request (parser/API) */

  /* response (parser) */
  std::string *versionInfo;
  std::string *otherInfo;

public:
  srmPing();
  srmPing(Node &node);
  ~srmPing();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfOtherInfoToString(Process *proc, BOOL space, BOOL quote) const;

private:
};

/*
 * srmPrepareToGet request
 */
struct srmPrepareToGet : public SRM2
{
  /* request (parser/API) */
  tArrayOfGetFileRequests_ fileRequests;

  std::string *userRequestDescription;

  tStorageSystemInfo_ storageSystemInfo;
  std::string *desiredFileStorageType;
  std::string *desiredTotalRequestTime;
  std::string *desiredPinLifeTime;
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
  tArrayOfPutFileRequests_ fileRequests;

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

/*
 * srmPutDone request
 */
struct srmPutDone : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *> SURL;

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
 * srmReleaseFiles request
 */
struct srmReleaseFiles : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *> SURL;
  std::string *doRemove;

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
  tStorageSystemInfo_ storageSystemInfo;
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
 * srmReserveSpace request
 */
struct srmReserveSpace : public SRM2
{
  /* request (parser/API) */
  std::string *userSpaceTokenDescription;
  std::string *retentionPolicy;
  std::string *accessLatency;
  std::string *desiredSizeOfTotalSpace;
  std::string *desiredSizeOfGuaranteedSpace;
  std::string *desiredLifetimeOfReservedSpace;
  std::vector <std::string *> expectedFileSizes;
  tStorageSystemInfo_ storageSystemInfo;
  std::string *accessPattern;
  std::string *connectionType;
  
  std::vector <std::string *> clientNetworks;
  std::vector <std::string *> transferProtocols;

  /* response (parser) */
  std::string *requestToken;
  std::string *estimatedProcessingTime;
  std::string *respRetentionPolicy;
  std::string *respAccessLatency;
  std::string *sizeOfTotalReservedSpace;
  std::string *sizeOfGuaranteedReservedSpace;
  std::string *lifetimeOfReservedSpace;
  std::string *spaceToken;

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
 * srmStatusOfBringOnlineRequest request
 */
struct srmStatusOfBringOnlineRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *> SURL;

  /* response (parser) */
  std::string *fileStatuses;
  std::string *remainingTotalRequestTime;
  std::string *remainingDeferredStartTime;

public:
  srmStatusOfBringOnlineRequest();
  srmStatusOfBringOnlineRequest(Node &node);
  ~srmStatusOfBringOnlineRequest();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfStatusOfBringOnlineRequestResponseToString(Process *proc, BOOL space, BOOL quote) const;

private:
};

/*
 * srmStatusOfChangeSpaceForFilesRequest request
 */
struct srmStatusOfChangeSpaceForFilesRequest : public SRM2
{
  /* request (parser/API) */
  std::string *spaceToken;
  std::string *requestToken;

  /* response (parser) */
  std::string *estimatedProcessingTime;
  std::string *fileStatuses;

public:
  srmStatusOfChangeSpaceForFilesRequest();
  srmStatusOfChangeSpaceForFilesRequest(Node &node);
  ~srmStatusOfChangeSpaceForFilesRequest();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfStatusOfChangeSpaceForFilesRequestResponseToString(Process *proc, BOOL space, BOOL quote) const;

private:
};

/*
 * srmStatusOfCopyRequest request
 */
struct srmStatusOfCopyRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *> sourceSURL;
  std::vector <std::string *> targetSURL;

  /* response (parser) */
  std::string *fileStatuses;
  std::string *remainingTotalRequestTime;

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
  std::vector <std::string *> SURL;

  /* response (parser) */
  std::string *fileStatuses;
  std::string *remainingTotalRequestTime;

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
 * srmStatusOfLsRequest request
 */
struct srmStatusOfLsRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::string *offset;
  std::string *count;

  /* response (parser) */
  std::string *pathDetails;

public:
  srmStatusOfLsRequest();
  srmStatusOfLsRequest(Node &node);
  ~srmStatusOfLsRequest();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote) const;
  std::string arrayOfFileStatusToString(Process *proc, BOOL space, BOOL quote, std::vector<struct srm__TMetaDataPathDetail *> details) const;

private:
};

/*
 * srmStatusOfPutRequest request
 */
struct srmStatusOfPutRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;
  std::vector <std::string *> SURL;

  /* response (parser) */
  std::string *fileStatuses;
  std::string *remainingTotalRequestTime;

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
 * srmStatusOfReserveSpaceRequest request
 */
struct srmStatusOfReserveSpaceRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;

  /* response (parser) */
  std::string *estimatedProcessingTime;
  std::string *respRetentionPolicy;
  std::string *respAccessLatency;
  std::string *sizeOfTotalReservedSpace;
  std::string *sizeOfGuaranteedReservedSpace;
  std::string *lifetimeOfReservedSpace;
  std::string *spaceToken;

public:
  srmStatusOfReserveSpaceRequest();
  srmStatusOfReserveSpaceRequest(Node &node);
  ~srmStatusOfReserveSpaceRequest();

  virtual void init();
  virtual void finish(Process *proc);
  int exec(Process *proc);
  std::string toString(Process *proc);

private:
};

/*
 * srmStatusOfUpdateSpaceRequest request
 */
struct srmStatusOfUpdateSpaceRequest : public SRM2
{
  /* request (parser/API) */
  std::string *requestToken;

  /* response (parser) */
  std::string *sizeOfTotalSpace;
  std::string *sizeOfGuaranteedSpace;
  std::string *lifetimeGranted;

public:
  srmStatusOfUpdateSpaceRequest();
  srmStatusOfUpdateSpaceRequest(Node &node);
  ~srmStatusOfUpdateSpaceRequest();

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
  std::string *newLifeTime;
  tStorageSystemInfo_ storageSystemInfo;

  /* response (parser) */
  std::string *requestToken;
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

#endif	/* HAVE_SRM22 */

#endif /* _N_SRM_H */
