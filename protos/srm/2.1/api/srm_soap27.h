#ifndef _SRM_SOAP27_H
#define _SRM_SOAP27_H

static const struct soap_code_map soap_codes_srm__TStatusCode[] =
{
  { (long)srm__TStatusCode__SRM_USCORESUCCESS, "SRM_SUCCESS" },
  { (long)srm__TStatusCode__SRM_USCOREFAILURE, "SRM_FAILURE" },
  { (long)srm__TStatusCode__SRM_USCOREAUTHENTICATION_USCOREFAILURE, "SRM_AUTHENTICATION_FAILURE" },
  { (long)srm__TStatusCode__SRM_USCOREUNAUTHORIZED_USCOREACCESS, "SRM_UNAUTHORIZED_ACCESS" },
  { (long)srm__TStatusCode__SRM_USCOREINVALID_USCOREREQUEST, "SRM_INVALID_REQUEST" },
  { (long)srm__TStatusCode__SRM_USCOREINVALID_USCOREPATH, "SRM_INVALID_PATH" },
  { (long)srm__TStatusCode__SRM_USCOREFILE_USCORELIFETIME_USCOREEXPIRED, "SRM_FILE_LIFETIME_EXPIRED" },
  { (long)srm__TStatusCode__SRM_USCORESPACE_USCORELIFETIME_USCOREEXPIRED, "SRM_SPACE_LIFETIME_EXPIRED" },
  { (long)srm__TStatusCode__SRM_USCOREEXCEED_USCOREALLOCATION, "SRM_EXCEED_ALLOCATION" },
  { (long)srm__TStatusCode__SRM_USCORENO_USCOREUSER_USCORESPACE, "SRM_NO_USER_SPACE" },
  { (long)srm__TStatusCode__SRM_USCORENO_USCOREFREE_USCORESPACE, "SRM_NO_FREE_SPACE" },
  { (long)srm__TStatusCode__SRM_USCOREDUPLICATION_USCOREERROR, "SRM_DUPLICATION_ERROR" },
  { (long)srm__TStatusCode__SRM_USCORENON_USCOREEMPTY_USCOREDIRECTORY, "SRM_NON_EMPTY_DIRECTORY" },
  { (long)srm__TStatusCode__SRM_USCORETOO_USCOREMANY_USCORERESULTS, "SRM_TOO_MANY_RESULTS" },
  { (long)srm__TStatusCode__SRM_USCOREINTERNAL_USCOREERROR, "SRM_INTERNAL_ERROR" },
  { (long)srm__TStatusCode__SRM_USCOREFATAL_USCOREINTERNAL_USCOREERROR, "SRM_FATAL_INTERNAL_ERROR" },
  { (long)srm__TStatusCode__SRM_USCORENOT_USCORESUPPORTED, "SRM_NOT_SUPPORTED" },
  { (long)srm__TStatusCode__SRM_USCOREREQUEST_USCOREQUEUED, "SRM_REQUEST_QUEUED" },
  { (long)srm__TStatusCode__SRM_USCOREREQUEST_USCOREINPROGRESS, "SRM_REQUEST_INPROGRESS" },
  { (long)srm__TStatusCode__SRM_USCOREREQUEST_USCORESUSPENDED, "SRM_REQUEST_SUSPENDED" },
  { (long)srm__TStatusCode__SRM_USCOREABORTED, "SRM_ABORTED" },
  { (long)srm__TStatusCode__SRM_USCORERELEASED, "SRM_RELEASED" },
  { (long)srm__TStatusCode__SRM_USCOREFILE_USCOREPINNED, "SRM_FILE_PINNED" },
  { (long)srm__TStatusCode__SRM_USCOREFILE_USCOREIN_USCORECACHE, "SRM_FILE_IN_CACHE" },
  { (long)srm__TStatusCode__SRM_USCORESPACE_USCOREAVAILABLE, "SRM_SPACE_AVAILABLE" },
  { (long)srm__TStatusCode__SRM_USCORELOWER_USCORESPACE_USCOREGRANTED, "SRM_LOWER_SPACE_GRANTED" },
  { (long)srm__TStatusCode__SRM_USCOREDONE, "SRM_DONE" },
  { (long)srm__TStatusCode__SRM_USCORECUSTOM_USCORESTATUS, "SRM_CUSTOM_STATUS" },
  { 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TOverwriteMode[] =
{
  { (long)srm__TOverwriteMode__Never, "Never" },
  { (long)srm__TOverwriteMode__Always, "Always" },
  { (long)srm__TOverwriteMode__WhenFilesAreDifferent, "WhenFilesAreDifferent" },
  { 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TRequestType[] =
{
  { (long)srm__TRequestType__PrepareToGet, "PrepareToGet" },
  { (long)srm__TRequestType__PrepareToPut, "PrepareToPut" },
  { (long)srm__TRequestType__Copy, "Copy" },
  { 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TPermissionType[] =
{
  { (long)srm__TPermissionType__ADD, "ADD" },
  { (long)srm__TPermissionType__REMOVE, "REMOVE" },
  { (long)srm__TPermissionType__CHANGE, "CHANGE" },
  { 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TPermissionMode[] =
{
  { (long)srm__TPermissionMode__None, "None" },
  { (long)srm__TPermissionMode__X, "X" },
  { (long)srm__TPermissionMode__W, "W" },
  { (long)srm__TPermissionMode__WX, "WX" },
  { (long)srm__TPermissionMode__R, "R" },
  { (long)srm__TPermissionMode__RX, "RX" },
  { (long)srm__TPermissionMode__RW, "RW" },
  { (long)srm__TPermissionMode__RWX, "RWX" },
  { 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TFileType[] =
{
  { (long)srm__TFileType__File, "File" },
  { (long)srm__TFileType__Directory, "Directory" },
  { (long)srm__TFileType__Link, "Link" },
  { 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TFileStorageType[] =
{
  { (long)srm__TFileStorageType__Volatile, "Volatile" },
  { (long)srm__TFileStorageType__Durable, "Durable" },
  { (long)srm__TFileStorageType__Permanent, "Permanent" },
  { 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TSpaceType[] =
{
  { (long)srm__TSpaceType__Volatile, "Volatile" },
  { (long)srm__TSpaceType__Durable, "Durable" },
  { (long)srm__TSpaceType__Permanent, "Permanent" },
  { 0, NULL }
};

/* extern(al) function declarations */
#define GET_SOAP(table)\
  extern const long *getT##table(const char *string);\
  extern std::string getT##table(long code);

GET_SOAP(StatusCode);
GET_SOAP(OverwriteMode);
GET_SOAP(RequestType);
GET_SOAP(PermissionType);
GET_SOAP(PermissionMode);
GET_SOAP(FileType);
GET_SOAP(FileStorageType);
GET_SOAP(SpaceType);
#undef GET_SOAP


#endif /* _SRM_SOAP27_H */
