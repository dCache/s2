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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("SURL",r->SURL) else

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
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
Parser::srmBringOnlineR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmBringOnline *r = new srmBringOnline(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_ARRAY("fileRequests.SURL",r->fileRequests.SURL) else
    P_OPL_ARRAY("fileRequests.isSourceADirectory",r->fileRequests.isSourceADirectory) else
    P_OPL_ARRAY("fileRequests.allLevelRecursive",r->fileRequests.allLevelRecursive) else
    P_OPL_ARRAY("fileRequests.numOfLevels",r->fileRequests.numOfLevels) else
    P_OPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else
    P_OPL_EQ_PARAM("desiredFileStorageType",r->desiredFileStorageType) else
    P_OPL_EQ_PARAM("desiredTotalRequestTime",r->desiredTotalRequestTime) else
    P_OPL_EQ_PARAM("desiredLifeTime",r->desiredLifeTime) else
    P_OPL_EQ_PARAM("targetSpaceToken",r->targetSpaceToken) else
    P_OPL_EQ_PARAM("retentionPolicy",r->retentionPolicy) else
    P_OPL_EQ_PARAM("accessLatency",r->accessLatency) else
    P_OPL_EQ_PARAM("accessPattern",r->accessPattern) else
    P_OPL_EQ_PARAM("connectionType",r->connectionType) else
    P_OPL_ARRAY("clientNetworks",r->clientNetworks) else
    P_OPL_ARRAY("transferProtocols",r->transferProtocols) else
    P_OPL_EQ_PARAM("deferredStartTime",r->deferredStartTime) else

    /* response */
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("remainingTotalRequestTime",r->remainingTotalRequestTime) else
    P_OPL_EQ_PARAM("remainingDeferredStartTime",r->remainingDeferredStartTime) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmBringOnlineR */

int
Parser::srmChangeSpaceForFilesR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmChangeSpaceForFiles *r = new srmChangeSpaceForFiles(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("spaceToken",r->spaceToken) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else
    P_OPL_ARRAY("SURL",r->SURL) else

    /* response */
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_EQ_PARAM("estimatedProcessingTime",r->estimatedProcessingTime) else
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmChangeSpaceForFilesR */

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_ARRAY("SURL",r->SURL) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else

    /* response */
    P_OPL_EQ_PARAM("permissionArray",r->permissionArray) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmCheckPermissionR */

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else

    P_OPL_ARRAY("sourceSURL",r->sourceSURL) else
    P_OPL_ARRAY("targetSURL",r->targetSURL) else
    P_OPL_ARRAY("isSourceADirectory",r->isSourceADirectory) else
    P_OPL_ARRAY("allLevelRecursive",r->allLevelRecursive) else
    P_OPL_ARRAY("numOfLevels",r->numOfLevels) else

    P_OPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else
    P_OPL_EQ_PARAM("overwriteOption",r->overwriteOption) else
    P_OPL_EQ_PARAM("desiredTotalRequestTime",r->desiredTotalRequestTime) else
    P_OPL_EQ_PARAM("desiredTargetSURLLifeTime",r->desiredTargetSURLLifeTime) else
    P_OPL_EQ_PARAM("targetFileStorageType",r->targetFileStorageType) else
    P_OPL_EQ_PARAM("targetSpaceToken",r->targetSpaceToken) else
    P_OPL_EQ_PARAM("targetFileRetentionPolicyInfo",r->targetFileRetentionPolicyInfo) else
    P_OPL_EQ_PARAM("retentionPolicy",r->retentionPolicy) else
    P_OPL_EQ_PARAM("accessLatency",r->accessLatency) else

    P_OPL_ARRAY("sourceStorageSystemInfo.key",r->sourceStorageSystemInfo.key) else
    P_OPL_ARRAY("sourceStorageSystemInfo.value",r->sourceStorageSystemInfo.value) else

    P_OPL_ARRAY("targetStorageSystemInfo.key",r->targetStorageSystemInfo.key) else
    P_OPL_ARRAY("targetStorageSystemInfo.value",r->targetStorageSystemInfo.value) else

    /* response */
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("remainingTotalRequestTime",r->remainingTotalRequestTime) else

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("SURL",r->SURL) else
    P_OPL_EQ_PARAM("newFileLifeTime",r->newFileLifeTime) else
    P_OPL_EQ_PARAM("newPinLifeTime",r->newPinLifeTime) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmExtendFileLifeTimeR */

int
Parser::srmExtendFileLifeTimeInSpaceR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmExtendFileLifeTimeInSpace *r = new srmExtendFileLifeTimeInSpace(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("spaceToken",r->spaceToken) else
    P_OPL_ARRAY("SURL",r->SURL) else
    P_OPL_EQ_PARAM("newLifeTime",r->newLifeTime) else

    /* response */
/*    P_OPL_EQ_PARAM("newTimeExtended",r->newTimeExtended) else */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmExtendFileLifeTimeInSpaceR */

int
Parser::srmGetPermissionR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetPermission *r = new srmGetPermission(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_ARRAY("SURL",r->SURL) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else

    /* response */
    P_OPL_EQ_PARAM("permissionArray",r->permissionArray) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetPermissionR */

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_ARRAY("requestToken",r->requestToken) else

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
Parser::srmGetRequestTokensR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetRequestTokens *r = new srmGetRequestTokens(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else

    /* response */
    P_OPL_EQ_PARAM("requestTokens",r->requestTokens) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetRequestTokensR */

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_ARRAY("spaceTokens",r->spaceTokens) else

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
Parser::srmGetSpaceTokensR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetSpaceTokens *r = new srmGetSpaceTokens(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("userSpaceTokenDescription",r->userSpaceTokenDescription) else

    /* response */
    P_OPL_EQ_PARAM("spaceTokens",r->spaceTokens) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetSpaceTokensR */

int
Parser::srmGetTransferProtocolsR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmGetTransferProtocols *r = new srmGetTransferProtocols(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else

    /* response */
    P_OPL_EQ_PARAM("transferProtocols",r->transferProtocols) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmGetTransferProtocolsR */

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("SURL",r->SURL) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("fromSURL",r->fromSURL) else
    P_OPL_EQ_PARAM("toSURL",r->toSURL) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else

    /* response */
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmMvR */

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_ARRAY("SURL",r->SURL) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else
    P_OPL_EQ_PARAM("fileStorageType",r->fileStorageType) else
    P_OPL_EQ_PARAM("fullDetailedList",r->fullDetailedList) else
    P_OPL_EQ_PARAM("allLevelRecursive",r->allLevelRecursive) else
    P_OPL_EQ_PARAM("numOfLevels",r->numOfLevels) else
    P_OPL_EQ_PARAM("offset",r->offset) else
    P_OPL_EQ_PARAM("count",r->count) else

    /* response */
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_EQ_PARAM("pathDetails",r->pathDetails) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmLsR */

int
Parser::srmPingR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmPing *r = new srmPing(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else

    /* response */
    P_OPL_EQ_PARAM("versionInfo",r->versionInfo) else
    P_OPL_EQ_PARAM("otherInfo",r->otherInfo) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmPingR */

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_ARRAY("fileRequests.SURL",r->fileRequests.SURL) else
    P_OPL_ARRAY("fileRequests.isSourceADirectory",r->fileRequests.isSourceADirectory) else
    P_OPL_ARRAY("fileRequests.allLevelRecursive",r->fileRequests.allLevelRecursive) else
    P_OPL_ARRAY("fileRequests.numOfLevels",r->fileRequests.numOfLevels) else
    P_OPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else
    P_OPL_EQ_PARAM("desiredFileStorageType",r->desiredFileStorageType) else
    P_OPL_EQ_PARAM("desiredTotalRequestTime",r->desiredTotalRequestTime) else
    P_OPL_EQ_PARAM("desiredPinLifeTime",r->desiredPinLifeTime) else
    P_OPL_EQ_PARAM("targetSpaceToken",r->targetSpaceToken) else
    P_OPL_EQ_PARAM("retentionPolicy",r->retentionPolicy) else
    P_OPL_EQ_PARAM("accessLatency",r->accessLatency) else
    P_OPL_EQ_PARAM("accessPattern",r->accessPattern) else
    P_OPL_EQ_PARAM("connectionType",r->connectionType) else
    P_OPL_ARRAY("clientNetworks",r->clientNetworks) else
    P_OPL_ARRAY("transferProtocols",r->transferProtocols) else

    /* response */
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("remainingTotalRequestTime",r->remainingTotalRequestTime) else
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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else

    P_OPL_ARRAY("fileRequests.SURL",r->fileRequests.SURL) else
    P_OPL_ARRAY("fileRequests.expectedFileSize",r->fileRequests.expectedFileSize) else
    P_OPL_EQ_PARAM("userRequestDescription",r->userRequestDescription) else
    P_OPL_EQ_PARAM("overwriteOption",r->overwriteOption) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else
    P_OPL_EQ_PARAM("desiredTotalRequestTime",r->desiredTotalRequestTime) else
    P_OPL_EQ_PARAM("desiredPinLifeTime",r->desiredPinLifeTime) else
    P_OPL_EQ_PARAM("desiredFileLifeTime",r->desiredFileLifeTime) else
    P_OPL_EQ_PARAM("desiredFileStorageType",r->desiredFileStorageType) else
    P_OPL_EQ_PARAM("targetSpaceToken",r->targetSpaceToken) else
    P_OPL_EQ_PARAM("retentionPolicy",r->retentionPolicy) else
    P_OPL_EQ_PARAM("accessLatency",r->accessLatency) else
    P_OPL_EQ_PARAM("accessPattern",r->accessPattern) else
    P_OPL_EQ_PARAM("connectionType",r->connectionType) else
    P_OPL_ARRAY("clientNetworks",r->clientNetworks) else
    P_OPL_ARRAY("transferProtocols",r->transferProtocols) else
      
    /* response */
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("remainingTotalRequestTime",r->remainingTotalRequestTime) else

    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else

    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmPrepareToPutR */

int
Parser::srmPurgeFromSpaceR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmPurgeFromSpace *r = new srmPurgeFromSpace(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_ARRAY("SURL",r->SURL) else
    P_OPL_EQ_PARAM("spaceToken",r->spaceToken) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmPurgeFromSpaceR */

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("SURL",r->SURL) else

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("SURL",r->SURL) else
    P_OPL_EQ_PARAM("doRemove",r->doRemove) else

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("spaceToken",r->spaceToken) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else
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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("userSpaceTokenDescription",r->userSpaceTokenDescription) else
    P_OPL_EQ_PARAM("retentionPolicy",r->retentionPolicy) else
    P_OPL_EQ_PARAM("accessLatency",r->accessLatency) else
    P_OPL_EQ_PARAM("desiredSizeOfTotalSpace",r->desiredSizeOfTotalSpace) else
    P_OPL_EQ_PARAM("desiredSizeOfGuaranteedSpace",r->desiredSizeOfGuaranteedSpace) else
    P_OPL_EQ_PARAM("desiredLifetimeOfReservedSpace",r->desiredLifetimeOfReservedSpace) else
    P_OPL_ARRAY("expectedFileSizes",r->expectedFileSizes) else
    P_OPL_EQ_PARAM("accessPattern",r->accessPattern) else
    P_OPL_EQ_PARAM("connectionType",r->connectionType) else
    P_OPL_ARRAY("clientNetworks",r->clientNetworks) else
    P_OPL_ARRAY("transferProtocols",r->transferProtocols) else

    /* response */
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_EQ_PARAM("estimatedProcessingTime",r->estimatedProcessingTime) else
    P_OPL_EQ_PARAM("respRetentionPolicy",r->respRetentionPolicy) else
    P_OPL_EQ_PARAM("respAccessLatency",r->respAccessLatency) else
    P_OPL_EQ_PARAM("sizeOfTotalReservedSpace",r->sizeOfTotalReservedSpace) else
    P_OPL_EQ_PARAM("sizeOfGuaranteedReservedSpace",r->sizeOfGuaranteedReservedSpace) else
    P_OPL_EQ_PARAM("lifetimeOfReservedSpace",r->lifetimeOfReservedSpace) else
    P_OPL_EQ_PARAM("spaceToken",r->spaceToken) else
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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_ARRAY("SURL",r->SURL) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("SURL",r->SURL) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else
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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("SURL",r->SURL) else
    P_OPL_EQ_PARAM("permissionType",r->permissionType) else
    P_OPL_EQ_PARAM("ownerPermission",r->ownerPermission) else
    P_OPL_ARRAY("userPermission.ID",r->userPermission.ID) else
    P_OPL_ARRAY("userPermission.mode",r->userPermission.mode) else
    P_OPL_ARRAY("groupPermission.ID",r->groupPermission.ID) else
    P_OPL_ARRAY("groupPermission.mode",r->groupPermission.mode) else
    P_OPL_EQ_PARAM("otherPermission",r->otherPermission) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else

    /* response */
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmSetPermissionR */


int
Parser::srmStatusOfBringOnlineRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmStatusOfBringOnlineRequest *r = new srmStatusOfBringOnlineRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("SURL",r->SURL) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("remainingTotalRequestTime",r->remainingTotalRequestTime) else
    P_OPL_EQ_PARAM("remainingDeferredStartTime",r->remainingDeferredStartTime) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmStatusOfBringOnlineRequestR */

int
Parser::srmStatusOfChangeSpaceForFilesRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmStatusOfChangeSpaceForFilesRequest *r = new srmStatusOfChangeSpaceForFilesRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else

    /* response */
    P_OPL_EQ_PARAM("estimatedProcessingTime",r->estimatedProcessingTime) else
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmStatusOfChangeSpaceForFilesRequestR */

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("sourceSURL",r->sourceSURL) else
    P_OPL_ARRAY("targetSURL",r->targetSURL) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("remainingTotalRequestTime",r->remainingTotalRequestTime) else
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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("SURL",r->SURL) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("remainingTotalRequestTime",r->remainingTotalRequestTime) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmStatusOfGetRequestR */

int
Parser::srmStatusOfLsRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmStatusOfLsRequest *r = new srmStatusOfLsRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
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
} /* srmStatusOfLsRequestR */

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
    P_OPL_ARRAY("SURL",r->SURL) else

    /* response */
    P_OPL_EQ_PARAM("fileStatuses",r->fileStatuses) else
    P_OPL_EQ_PARAM("remainingTotalRequestTime",r->remainingTotalRequestTime) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmStatusOfPutRequestR */

int
Parser::srmStatusOfReserveSpaceRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmStatusOfReserveSpaceRequest *r = new srmStatusOfReserveSpaceRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else

    /* response */
    P_OPL_EQ_PARAM("estimatedProcessingTime",r->estimatedProcessingTime) else
    P_OPL_EQ_PARAM("respRetentionPolicy",r->respRetentionPolicy) else
    P_OPL_EQ_PARAM("respAccessLatency",r->respAccessLatency) else
    P_OPL_EQ_PARAM("sizeOfTotalReservedSpace",r->sizeOfTotalReservedSpace) else
    P_OPL_EQ_PARAM("sizeOfGuaranteedReservedSpace",r->sizeOfGuaranteedReservedSpace) else
    P_OPL_EQ_PARAM("lifetimeOfReservedSpace",r->lifetimeOfReservedSpace) else
    P_OPL_EQ_PARAM("spaceToken",r->spaceToken) else
    P_OPL_EQ_PARAM("returnStatus.explanation",r->returnStatus.explanation) else
    P_OPL_EQ_PARAM("returnStatus.statusCode",r->returnStatus.statusCode) else
    P_OPL_ERR;
  }

  /* parsing succeeded */
  return ERR_OK;
} /* srmStatusOfReserveSpaceRequestR */

int
Parser::srmStatusOfUpdateSpaceRequestR(void)
{
  int rval;
  char *opt;
  char *end = NULL;
  std::string _val;
  
  srmStatusOfUpdateSpaceRequest *r = new srmStatusOfUpdateSpaceRequest(parser_node);
  new_node = r;

  EAT(ENDPOINT, &r->srm_endpoint);

  while(col < llen) {
    _val.clear();

    WS_COMMENT; /* allow whitespace, leave if comment char hit */
    AZaz_dot(opt = line + col, &end);   /* get options */

    /* request */
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else

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
} /* srmStatusOfUpdateSpaceRequestR */

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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
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
    P_OPL_EQ_PARAM("authorizationID",r->authorizationID) else
    P_OPL_EQ_PARAM("spaceToken",r->spaceToken) else
    P_OPL_EQ_PARAM("newSizeOfTotalSpaceDesired",r->newSizeOfTotalSpaceDesired) else
    P_OPL_EQ_PARAM("newSizeOfGuaranteedSpaceDesired",r->newSizeOfGuaranteedSpaceDesired) else
    P_OPL_EQ_PARAM("newLifeTime",r->newLifeTime) else
    P_OPL_ARRAY("storageSystemInfo.key",r->storageSystemInfo.key) else
    P_OPL_ARRAY("storageSystemInfo.value",r->storageSystemInfo.value) else

    /* response */
    P_OPL_EQ_PARAM("requestToken",r->requestToken) else
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
