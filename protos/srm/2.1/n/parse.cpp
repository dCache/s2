#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "parse.h"

#include "n.h"			/* Node */
#include "sysdep.h"		/* BOOL */
#include "free.h"		/* FREE(), DELETE() */

#include "n_srm.h"
#include "soapH.h"              /* soap_codes_srm__TSpaceType, ... */

int
Parser::ENDPOINT(std::string **target)
{
  std::string _val;

  WS();
  P_DQ_PARAM(dq_param,_val,*target,"endpoint\n");
  
  /* parsing succeeded */
  return ERR_OK;
}

int
Parser::srmAbortFilesR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmAbortFiles *r = new srmAbortFiles(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("surlArray",r->surlArray) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmAbortFilesR */

int
Parser::srmAbortRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmAbortRequest *r = new srmAbortRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else

    /* response */
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmAbortRequestR */

int
Parser::srmChangeFileStorageTypeR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmChangeFileStorageType *r = new srmChangeFileStorageType(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_ARRAY("path.SURLOrStFN",r->path.SURLOrStFN) else
    P_OPL_ARRAY("path.storageSystemInfo",r->path.storageSystemInfo) else
    P_OPL_EQ_PARAM("desiredStorageType",r->desiredStorageType) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmChangeFileStorageTypeR */

int
Parser::srmCheckPermissionR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmCheckPermission *r = new srmCheckPermission(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_ARRAY("path.SURLOrStFN",r->path.SURLOrStFN) else
    P_OPL_ARRAY("path.storageSystemInfo",r->path.storageSystemInfo) else
    P_OPL_EQ_PARAM("checkInLocalCacheOnly",r->checkInLocalCacheOnly) else

    /* response */
    P_OPL_EQ_PARAM("permissions",r->permissions) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmCheckPermissionR */

int
Parser::srmCompactSpaceR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmCompactSpace *r = new srmCompactSpace(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("spaceToken",r->spaceToken) else
    P_OPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    P_OPL_EQ_PARAM("doDynamicCompactFromNowOn",r->doDynamicCompactFromNowOn) else

    /* response */
    P_OPL_EQ_PARAM("newSizeOfThisSpace",r->newSizeOfThisSpace) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmCompactSpaceR */

int
Parser::srmCopyR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmCopy *r = new srmCopy(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_ARRAY("arrayOfFileRequests.allLevelRecursive",r->arrayOfFileRequests.allLevelRecursive) else
    P_OPL_ARRAY("arrayOfFileRequests.isSourceADirectory",r->arrayOfFileRequests.isSourceADirectory) else
    P_OPL_ARRAY("arrayOfFileRequests.numOfLevels",r->arrayOfFileRequests.numOfLevels) else
    P_OPL_ARRAY("arrayOfFileRequests.fileStorageType",r->arrayOfFileRequests.fileStorageType) else
    P_OPL_ARRAY("arrayOfFileRequests.fromSURLOrStFN",r->arrayOfFileRequests.fromSURLOrStFN) else
    P_OPL_ARRAY("arrayOfFileRequests.fromStorageSystemInfo",r->arrayOfFileRequests.fromStorageSystemInfo) else
    P_OPL_ARRAY("arrayOfFileRequests.lifetime",r->arrayOfFileRequests.lifetime) else
    P_OPL_ARRAY("arrayOfFileRequests.overwriteMode",r->arrayOfFileRequests.overwriteMode) else
    P_OPL_ARRAY("arrayOfFileRequests.spaceToken",r->arrayOfFileRequests.spaceToken) else
    P_OPL_ARRAY("arrayOfFileRequests.toSURLOrStFN",r->arrayOfFileRequests.toSURLOrStFN) else
    P_OPL_ARRAY("arrayOfFileRequests.toStorageSystemInfo",r->arrayOfFileRequests.toStorageSystemInfo) else
    P_OPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else
    P_OPL_EQ_PARAM("overwriteOption",r->overwriteOption) else
    P_OPL_EQ_PARAM("removeSourceFiles",r->removeSourceFiles) else
    P_OPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    P_OPL_EQ_PARAM("totalRetryTime",r->totalRetryTime) else

    /* response */
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmCopyR */

int
Parser::srmExtendFileLifeTimeR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmExtendFileLifeTime *r = new srmExtendFileLifeTime(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_EQ_PARAM("siteURL",r->siteURL) else
    P_OPL_EQ_PARAM("newLifeTime",r->newLifeTime) else

    /* response */
    P_OPL_EQ_PARAM("newTimeExtended",r->newTimeExtended) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmExtendFileLifeTimeR */

int
Parser::srmGetRequestIDR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetRequestID *r = new srmGetRequestID(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else

    /* response */
    P_OPL_EQ_PARAM("requestTokens",r->requestTokens) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetRequestIDR */

int
Parser::srmGetRequestSummaryR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetRequestSummary *r = new srmGetRequestSummary(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_ARRAY("arrayOfRequestToken",r->arrayOfRequestToken) else

    /* response */
    P_OPL_EQ_PARAM("requestSummary",r->requestSummary) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetRequestSummaryR */

int
Parser::srmGetSpaceMetaDataR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetSpaceMetaData *r = new srmGetSpaceMetaData(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_ARRAY("arrayOfSpaceToken",r->arrayOfSpaceToken) else

    /* response */
    P_OPL_EQ_PARAM("spaceDetails",r->spaceDetails) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetSpaceMetaDataR */

int
Parser::srmGetSpaceTokenR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetSpaceToken *r = new srmGetSpaceToken(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("userSpaceTokenDescription",r->userSpaceTokenDescription) else

    /* response */
    P_OPL_EQ_PARAM("possibleSpaceTokens",r->possibleSpaceTokens) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetSpaceTokenR */

int
Parser::srmLsR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmLs *r = new srmLs(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_ARRAY("path.SURLOrStFN",r->path.SURLOrStFN) else
    P_OPL_ARRAY("path.storageSystemInfo",r->path.storageSystemInfo) else
    P_OPL_EQ_PARAM("fileStorageType",r->fileStorageType) else
    P_OPL_EQ_PARAM("fullDetailedList",r->fullDetailedList) else
    P_OPL_EQ_PARAM("allLevelRecursive",r->allLevelRecursive) else
    P_OPL_EQ_PARAM("numOfLevels",r->numOfLevels) else
    P_OPL_EQ_PARAM("offset",r->offset) else
    P_OPL_EQ_PARAM("count",r->count) else

    /* response */
    P_OPL_EQ_PARAM("pathDetails",r->pathDetails) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmLsR */

int
Parser::srmMkdirR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmMkdir *r = new srmMkdir(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("SURLOrStFN",r->SURLOrStFN) else
    P_OPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else

    /* response */
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmMkdirR */

int
Parser::srmMvR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmMv *r = new srmMv(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("fromSURLOrStFN",r->fromSURLOrStFN) else
    P_OPL_EQ_PARAM("fromStorageSystemInfo",r->fromStorageSystemInfo) else
    P_OPL_EQ_PARAM("toSURLOrStFN",r->toSURLOrStFN) else
    P_OPL_EQ_PARAM("toStorageSystemInfo",r->toStorageSystemInfo) else

    /* response */
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmMvR */

int
Parser::srmPrepareToGetR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmPrepareToGet *r = new srmPrepareToGet(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_ARRAY("arrayOfFileRequests.allLevelRecursive",r->arrayOfFileRequests.allLevelRecursive) else
    P_OPL_ARRAY("arrayOfFileRequests.isSourceADirectory",r->arrayOfFileRequests.isSourceADirectory) else
    P_OPL_ARRAY("arrayOfFileRequests.numOfLevels",r->arrayOfFileRequests.numOfLevels) else
    P_OPL_ARRAY("arrayOfFileRequests.fileStorageType",r->arrayOfFileRequests.fileStorageType) else
    P_OPL_ARRAY("arrayOfFileRequests.SURLOrStFN",r->arrayOfFileRequests.SURLOrStFN) else
    P_OPL_ARRAY("arrayOfFileRequests.storageSystemInfo",r->arrayOfFileRequests.storageSystemInfo) else
    P_OPL_ARRAY("arrayOfFileRequests.lifetime",r->arrayOfFileRequests.lifetime) else
    P_OPL_ARRAY("arrayOfFileRequests.spaceToken",r->arrayOfFileRequests.spaceToken) else
    P_OPL_ARRAY("arrayOfTransferProtocols",r->arrayOfTransferProtocols) else
    P_OPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else
    P_OPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    P_OPL_EQ_PARAM("totalRetryTime",r->totalRetryTime) else

    /* response */
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmPrepareToGetR */

int
Parser::srmPrepareToPutR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmPrepareToPut *r = new srmPrepareToPut(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_ARRAY("arrayOfFileRequests.fileStorageType",r->arrayOfFileRequests.fileStorageType) else
    P_OPL_ARRAY("arrayOfFileRequests.knownSizeOfThisFile",r->arrayOfFileRequests.knownSizeOfThisFile) else
    P_OPL_ARRAY("arrayOfFileRequests.lifetime",r->arrayOfFileRequests.lifetime) else
    P_OPL_ARRAY("arrayOfFileRequests.spaceToken",r->arrayOfFileRequests.spaceToken) else
    P_OPL_ARRAY("arrayOfFileRequests.SURLOrStFN",r->arrayOfFileRequests.SURLOrStFN) else
    P_OPL_ARRAY("arrayOfFileRequests.storageSystemInfo",r->arrayOfFileRequests.storageSystemInfo) else
    P_OPL_ARRAY("arrayOfTransferProtocols",r->arrayOfTransferProtocols) else
    P_OPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else
    P_OPL_EQ_PARAM("overwriteOption",r->overwriteOption) else
    P_OPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    P_OPL_EQ_PARAM("totalRetryTime",r->totalRetryTime) else

    /* response */
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmPrepareToPutR */

int
Parser::srmPutDoneR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmPutDone *r = new srmPutDone(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("surlArray",r->surlArray) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmPutDoneR */

int
Parser::srmReassignToUserR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmReassignToUser *r = new srmReassignToUser(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("assignedUser",r->assignedUser) else
    P_OPL_EQ_PARAM("lifeTimeOfThisAssignment",r->lifeTimeOfThisAssignment) else
    P_OPL_EQ_PARAM("SURLOrStFN",r->SURLOrStFN) else
    P_OPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else

    /* response */
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmReassignToUserR */

int
Parser::srmReleaseFilesR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmReleaseFiles *r = new srmReleaseFiles(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("surlArray",r->surlArray) else
    P_OPL_EQ_PARAM("keepFiles",r->keepFiles) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmReleaseFilesR */

int
Parser::srmReleaseSpaceR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmReleaseSpace *r = new srmReleaseSpace(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("spaceToken",r->spaceToken) else
    P_OPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    P_OPL_EQ_PARAM("forceFileRelease",r->forceFileRelease) else

    /* response */
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmReleaseSpaceR */

int
Parser::srmRemoveFilesR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmRemoveFiles *r = new srmRemoveFiles(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("surlArray",r->surlArray) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmRemoveFilesR */

int
Parser::srmReserveSpaceR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmReserveSpace *r = new srmReserveSpace(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("typeOfSpace",r->typeOfSpace) else
    P_OPL_EQ_PARAM("userSpaceTokenDescription",r->userSpaceTokenDescription) else
    P_OPL_EQ_PARAM("sizeOfTotalSpaceDesired",r->sizeOfTotalSpaceDesired) else
    P_OPL_EQ_PARAM("sizeOfGuaranteedSpaceDesired",r->sizeOfGuaranteedSpaceDesired) else
    P_OPL_EQ_PARAM("lifetimeOfSpaceToReserve",r->lifetimeOfSpaceToReserve) else
    P_OPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else

    /* response */
    P_OPL_EQ_PARAM("typeOfReservedSpace",r->typeOfReservedSpace) else
    P_OPL_EQ_PARAM("sizeOfTotalReservedSpace",r->sizeOfTotalReservedSpace) else
    P_OPL_EQ_PARAM("sizeOfGuaranteedReservedSpace",r->sizeOfGuaranteedReservedSpace) else
    P_OPL_EQ_PARAM("lifetimeOfReservedSpace",r->lifetimeOfReservedSpace) else
    P_OPL_EQ_PARAM("referenceHandleOfReservedSpace",r->referenceHandleOfReservedSpace) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmReserveSpaceR */

int
Parser::srmResumeRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmResumeRequest *r = new srmResumeRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else

    /* response */
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmResumeRequestR */

int
Parser::srmRmR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmRm *r = new srmRm(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_ARRAY("path.SURLOrStFN",r->path.SURLOrStFN) else
    P_OPL_ARRAY("path.storageSystemInfo",r->path.storageSystemInfo) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmRmR */

int
Parser::srmRmdirR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmRmdir *r = new srmRmdir(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("SURLOrStFN",r->SURLOrStFN) else
    P_OPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    P_OPL_EQ_PARAM("recursive",r->recursive) else

    /* response */
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmRmdirR */

int
Parser::srmSetPermissionR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmSetPermission *r = new srmSetPermission(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("SURLOrStFN",r->SURLOrStFN) else
    P_OPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else
    P_OPL_EQ_PARAM("permissionType",r->permissionType) else
    P_OPL_EQ_PARAM("ownerPermission",r->ownerPermission) else
    P_OPL_ARRAY("userPermissionArray.mode",r->userPermissionArray.mode) else
    P_OPL_ARRAY("userPermissionArray.ID",r->userPermissionArray.ID) else
    P_OPL_ARRAY("groupPermissionArray.mode",r->groupPermissionArray.mode) else
    P_OPL_ARRAY("groupPermissionArray.ID",r->groupPermissionArray.ID) else
    P_OPL_EQ_PARAM("otherPermission",r->otherPermission) else

    /* response */
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmSetPermissionR */

int
Parser::srmStatusOfCopyRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmStatusOfCopyRequest *r = new srmStatusOfCopyRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("fromSurlArray",r->fromSurlArray) else
    P_OPL_ARRAY("toSurlArray",r->toSurlArray) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmStatusOfCopyRequestR */

int
Parser::srmStatusOfGetRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmStatusOfGetRequest *r = new srmStatusOfGetRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("surlArray",r->surlArray) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmStatusOfGetRequestR */

int
Parser::srmStatusOfPutRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmStatusOfPutRequest *r = new srmStatusOfPutRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("surlArray",r->surlArray) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmStatusOfPutRequestR */

int
Parser::srmSuspendRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmSuspendRequest *r = new srmSuspendRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else

    /* response */
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmSuspendRequestR */

int
Parser::srmUpdateSpaceR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmUpdateSpace *r = new srmUpdateSpace(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("userID",r->userID) else
    P_OPL_EQ_PARAM("spaceToken",r->spaceToken) else
    P_OPL_EQ_PARAM("newSizeOfTotalSpaceDesired",r->newSizeOfTotalSpaceDesired) else
    P_OPL_EQ_PARAM("newSizeOfGuaranteedSpaceDesired",r->newSizeOfGuaranteedSpaceDesired) else
    P_OPL_EQ_PARAM("newLifeTimeFromCallingTime",r->newLifeTimeFromCallingTime) else
    P_OPL_EQ_PARAM("storageSystemInfo",r->storageSystemInfo) else

    /* response */
    P_OPL_EQ_PARAM("sizeOfTotalSpace",r->sizeOfTotalSpace) else
    P_OPL_EQ_PARAM("sizeOfGuaranteedSpace",r->sizeOfGuaranteedSpace) else
    P_OPL_EQ_PARAM("lifetimeGranted",r->lifetimeGranted) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmUpdateSpaceR */
