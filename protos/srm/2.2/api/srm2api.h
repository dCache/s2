#ifndef _SRM2API_H
#define _SRM2API_H

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#include "soapH.h"              /* srm__*Response_, srm__TStatusCode, srm__TSpaceType__Volatile, ... */

/* type definitions */
typedef struct tArrayOfPutFileRequests
{ 
  std::vector <std::string *> targetSURL;
  std::vector <uint64_t *> expectedFileSize;
};

typedef struct tStorageSystemInfo
{ 
  std::vector <std::string *> key;
  std::vector <std::string *> value;
};

/* extern(al) function declarations */
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

#endif /* _SRM2API_H */
