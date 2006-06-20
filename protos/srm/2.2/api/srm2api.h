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


#endif /* _SRM2API_H */
