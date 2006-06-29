#ifndef _SRM_SOAP27_H
#define _SRM_SOAP27_H

static const struct soap_code_map soap_codes_srm__TStatusCode[] =
{	{ (long)srm__TStatusCode__SRM_USCORESUCCESS, "SRM_SUCCESS" },
	{ (long)srm__TStatusCode__SRM_USCOREFAILURE, "SRM_FAILURE" },
	{ (long)srm__TStatusCode__SRM_USCOREAUTHENTICATION_USCOREFAILURE, "SRM_AUTHENTICATION_FAILURE" },
	{ (long)srm__TStatusCode__SRM_USCOREAUTHORIZATION_USCOREFAILURE, "SRM_AUTHORIZATION_FAILURE" },
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
	{ (long)srm__TStatusCode__SRM_USCOREPARTIAL_USCORESUCCESS, "SRM_PARTIAL_SUCCESS" },
	{ (long)srm__TStatusCode__SRM_USCOREREQUEST_USCORETIMED_USCOREOUT, "SRM_REQUEST_TIMED_OUT" },
	{ (long)srm__TStatusCode__SRM_USCORELAST_USCORECOPY, "SRM_LAST_COPY" },
	{ (long)srm__TStatusCode__SRM_USCOREFILE_USCOREBUSY, "SRM_FILE_BUSY" },
	{ (long)srm__TStatusCode__SRM_USCOREFILE_USCORELOST, "SRM_FILE_LOST" },
	{ (long)srm__TStatusCode__SRM_USCOREFILE_USCOREUNAVAILABLE, "SRM_FILE_UNAVAILABLE" },
	{ (long)srm__TStatusCode__SRM_USCORECUSTOM_USCORESTATUS, "SRM_CUSTOM_STATUS" },
	{ 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TConnectionType[] =
{	{ (long)srm__TConnectionType__WAN, "WAN" },
	{ (long)srm__TConnectionType__LAN, "LAN" },
	{ 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TAccessPattern[] =
{	{ (long)srm__TAccessPattern__TRANSFER_USCOREMODE, "TRANSFER_MODE" },
	{ (long)srm__TAccessPattern__PROCESSING_USCOREMODE, "PROCESSING_MODE" },
	{ 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TFileLocality[] =
{	{ (long)srm__TFileLocality__ONLINE, "ONLINE" },
	{ (long)srm__TFileLocality__NEARLINE, "NEARLINE" },
	{ (long)srm__TFileLocality__ONLINE_USCOREAND_USCORENEARLINE, "ONLINE_AND_NEARLINE" },
	{ (long)srm__TFileLocality__LOST, "LOST" },
	{ (long)srm__TFileLocality__NONE, "NONE" },
	{ (long)srm__TFileLocality__UNAVAILABLE, "UNAVAILABLE" },
	{ 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TOverwriteMode[] =
{	{ (long)srm__TOverwriteMode__NEVER, "NEVER" },
	{ (long)srm__TOverwriteMode__ALWAYS, "ALWAYS" },
	{ (long)srm__TOverwriteMode__WHEN_USCOREFILES_USCOREARE_USCOREDIFFERENT, "WHEN_FILES_ARE_DIFFERENT" },
	{ 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TRequestType[] =
{	{ (long)srm__TRequestType__PREPARE_USCORETO_USCOREGET, "PREPARE_TO_GET" },
	{ (long)srm__TRequestType__PREPARE_USCORETO_USCOREPUT, "PREPARE_TO_PUT" },
	{ (long)srm__TRequestType__COPY, "COPY" },
	{ (long)srm__TRequestType__BRING_USCOREONLINE, "BRING_ONLINE" },
	{ (long)srm__TRequestType__RESERVE_USCORESPACE, "RESERVE_SPACE" },
	{ (long)srm__TRequestType__UPDATE_USCORESPACE, "UPDATE_SPACE" },
	{ (long)srm__TRequestType__CHANGE_USCORESPACE_USCOREFOR_USCOREFILES, "CHANGE_SPACE_FOR_FILES" },
	{ (long)srm__TRequestType__LS, "LS" },
	{ 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TPermissionType[] =
{	{ (long)srm__TPermissionType__ADD, "ADD" },
	{ (long)srm__TPermissionType__REMOVE, "REMOVE" },
	{ (long)srm__TPermissionType__CHANGE, "CHANGE" },
	{ 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TPermissionMode[] =
{	{ (long)srm__TPermissionMode__NONE, "NONE" },
	{ (long)srm__TPermissionMode__X, "X" },
	{ (long)srm__TPermissionMode__W, "W" },
	{ (long)srm__TPermissionMode__WX, "WX" },
	{ (long)srm__TPermissionMode__R, "R" },
	{ (long)srm__TPermissionMode__RX, "RX" },
	{ (long)srm__TPermissionMode__RW, "RW" },
	{ (long)srm__TPermissionMode__RWX, "RWX" },
	{ 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TAccessLatency[] =
{	{ (long)srm__TAccessLatency__ONLINE, "ONLINE" },
	{ (long)srm__TAccessLatency__NEARLINE, "NEARLINE" },
	{ 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TRetentionPolicy[] =
{	{ (long)srm__TRetentionPolicy__REPLICA, "REPLICA" },
	{ (long)srm__TRetentionPolicy__OUTPUT, "OUTPUT" },
	{ (long)srm__TRetentionPolicy__CUSTODIAL, "CUSTODIAL" },
	{ 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TFileType[] =
{	{ (long)srm__TFileType__FILE_, "FILE" },
	{ (long)srm__TFileType__DIRECTORY, "DIRECTORY" },
	{ (long)srm__TFileType__LINK, "LINK" },
	{ 0, NULL }
};

static const struct soap_code_map soap_codes_srm__TFileStorageType[] =
{	{ (long)srm__TFileStorageType__VOLATILE, "VOLATILE" },
	{ (long)srm__TFileStorageType__DURABLE, "DURABLE" },
	{ (long)srm__TFileStorageType__PERMANENT, "PERMANENT" },
	{ 0, NULL }
};

/* extern(al) function declarations */
#define GET_SOAP(table)\
  extern const long *getT##table(const char *string);\
  extern const long getT##table(const char *string, char warn);\
  extern std::string getT##table(long code);

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
#undef GET_SOAP

#endif /* _SRM_SOAP27_H */
