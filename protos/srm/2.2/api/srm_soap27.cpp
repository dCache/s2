#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "free.h"
#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "str.h"
#include "io.h"                 /* file_ropen(), ... */

#include "srm2api.h"
#include "srm_soap27.h"

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

//SOAP_FMAC1 long SOAP_FMAC2 soap_int_code(const struct soap_code_map*, const char *str, long other);
//SOAP_FMAC1 const char* SOAP_FMAC2 soap_str_code(const struct soap_code_map*, long code);

#define GET_SOAP_C(table)\
extern const long * \
getT##table(const char *string)\
{\
  uint i;\
\
  for(i = 0; string && soap_codes_srm__T##table[i].string && strcmp(string, soap_codes_srm__T##table[i].string) != 0; i++)\
    ;\
\
  if(!string || soap_codes_srm__T##table[i].string == NULL)\
    DM_WARN(ERR_WARN, _("returning a default value `%d' for `%s'\n"), soap_codes_srm__T##table[0].code, string);\
\
  return &soap_codes_srm__T##table[i].code;\
}

#define GET_SOAP_S(table)\
extern std::string \
getT##table(long code)\
{\
  uint i;\
  std::stringstream ss;\
\
  for(i = 0; soap_codes_srm__T##table[i].string && code != soap_codes_srm__T##table[i].code; i++)\
    ;\
\
  if(soap_codes_srm__T##table[i].string == NULL) {\
    ss << S2_UNDEF_STR << code;\
    DM_WARN(ERR_WARN, _("returning a default value `%s' for `%ld'\n"), ss.str().c_str(), code);\
  } else {\
    ss << soap_codes_srm__T##table[i].string;\
  }\
\
  return ss.str();\
}

#define GET_SOAP(table)\
  GET_SOAP_C(table);\
  GET_SOAP_S(table);

GET_SOAP(StatusCode);
GET_SOAP(ConnectionType);
GET_SOAP(AccessPattern);
GET_SOAP(FileLocality);
GET_SOAP(OverwriteMode);
GET_SOAP(RequestType);
GET_SOAP(PermissionType);
GET_SOAP(PermissionMode);
GET_SOAP(AccessLatency);
GET_SOAP(RetentionPolicy);
GET_SOAP(FileType);
GET_SOAP(FileStorageType);
#undef GET_SOAP_C
#undef GET_SOAP_S
#undef GET_SOAP
