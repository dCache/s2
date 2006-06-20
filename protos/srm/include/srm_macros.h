#ifndef _SRM_MACROS_H
#define _SRM_MACROS_H

#ifndef RETURN
#define RETURN(...) do {DM_DBG_O; return __VA_ARGS__;} while(0)
#endif

#define SOAP_INIT(soap)\
  do {\
    if(!soap) {\
      DM_ERR_ASSERT("soap == NULL\n");\
      RETURN(EXIT_FAILURE);\
    } else soap_init(soap);\
  } while(0)

#define SOAP_CALL_SRM(func) \
  DM_DBG(DM_N(1),"soap call: srm"#func "() <---\n");\
  if(srm_endpoint == NULL) { DM_ERR(DM_N(1), "no SRM endpoint\n"); RETURN(EXIT_FAILURE); };\
  if(soap_call_srm__srm ## func (soap, srm_endpoint, "srm" #func, &req, *resp)) {\
     soap_print_fault (soap, stderr);\
     soap_print_fault_location (soap, stderr);\
     std::cerr << "Error: " << \
     *soap_faultcode(soap) << " " <<\
     *soap_faultstring(soap) << " " <<\
     *soap_faultdetail(soap) << std::endl;\
     soap_end (soap);\
     RETURN(EXIT_FAILURE);\
  };\
  DM_DBG(DM_N(1),"soap call: srm"#func "() --->\n");
  
#define NOT_NULL(par)\
  if((par) == NULL) {\
    perror("memory allocation failed");\
    RETURN(EXIT_FAILURE);\
  }

#define NOT_NULL_VEC(v,r) (i < v.r.size() && v.r[i])

#define PINT_VAL_OPT(opt,r)\
  opt = r;\
  if(r) {\
    DM_LOG(DM_N(2), ""#r " == %d\n",*r);\
  } else {\
    DM_LOG(DM_N(2), ""#r " == NULL\n");\
  }
#define PINT_VAL(r) PINT_VAL_OPT(req.r,r)
#define PBOOL_VAL(r) PINT_VAL_OPT(req.r,r)

#define NEW_STDSTRING_OPT(opt,r)\
  if(r) {\
    NOT_NULL(opt = soap_new_std__string(soap, -1));\
    opt->assign(r);\
    DM_LOG(DM_N(2), ""#r " == `%s'\n",opt->c_str());\
  } else {\
    DM_LOG(DM_N(2), ""#r " == NULL\n");\
    opt = NULL;\
  }
#define NEW_STDSTRING(r) NEW_STDSTRING_OPT(req.r,r)

#define NEW_STR_VAL_OPT(opt,r,t)\
  if(r) {\
    NOT_NULL(opt = soap_new_srm__##t(soap, -1));\
    opt->value.assign(r);\
    DM_LOG(DM_N(2), ""#r " == `%s'\n",opt->value.c_str());\
  } else {\
    DM_LOG(DM_N(2), ""#r " == NULL\n");\
    opt = NULL;\
  }
#define NEW_STR_VAL(r,t) NEW_STR_VAL_OPT(req.r,r,t)

#define NEW_INT64_VAL_OPT(opt,r,t)\
  if(r) {\
    NOT_NULL(opt = soap_new_srm__##t(soap, -1));\
    opt->value = *r;\
    DM_LOG(DM_N(2), ""#r " == %"PRIi64"\n",opt->value);\
  } else {\
    DM_LOG(DM_N(2), ""#r " == NULL\n");\
    opt = NULL;\
  }
#define NEW_INT64_VAL(r,t) NEW_INT64_VAL_OPT(req.r,r,t)

#define NEW_PERMISSION_OPT(opt,r,t)\
  if(r) {\
    NOT_NULL(opt = soap_new_srm__##t(soap, -1));\
    opt->mode = (srm__TPermissionMode)*r;\
    DM_LOG(DM_N(2), ""#r " == `%s'\n",getTPermissionMode(opt->mode).c_str());\
  } else {\
    DM_LOG(DM_N(2), ""#r " == NULL\n");\
    opt = NULL;\
  }
#define NEW_PERMISSION(r,t) NEW_PERMISSION_OPT(req.r,r,t)

/* arrays */
#define NEW_ARRAY_OF_STR_VAL(v1,v2,opt,t)\
  NOT_NULL(req.v1 = soap_new_srm__ArrayOf##t(soap, -1));\
  for(uint i = 0; i < v2.size(); i++) {\
    srm__##t *myInfo;\
    if(v2[i]) {\
      NOT_NULL(myInfo = soap_new_srm__##t(soap, -1));\
      myInfo->value.assign(v2[i]->c_str());\
      DM_LOG(DM_N(2), ""#opt "[%u] == `%s'\n", i, myInfo->value.c_str());\
    } else {\
      DM_LOG(DM_N(2), ""#opt "[%u] == NULL\n", i);\
      myInfo = NULL;\
    }\
    req.v1->opt.push_back(myInfo);\
  }

#define ARRAY_OF_TSURL_INFO(v1)\
  DM_LOG(DM_N(2), "path.SURLOrStFN.size() == %d\n", path.SURLOrStFN.size());\
  for (uint i = 0; i < path.SURLOrStFN.size(); i++) {\
    srm__TSURLInfo *myInfo;\
    NOT_NULL(myInfo = soap_new_srm__TSURLInfo(soap, -1));\
\
    if(NOT_NULL_VEC(path,SURLOrStFN)) {\
      NOT_NULL(myInfo->SURLOrStFN = soap_new_srm__TSURL(soap, -1));\
      myInfo->SURLOrStFN->value.assign(path.SURLOrStFN[i]->c_str());\
      DM_LOG(DM_N(2), "SURLOrStFN[%u] == `%s'\n", i, myInfo->SURLOrStFN->value.c_str());\
    } else {\
      myInfo->SURLOrStFN = NULL;\
      DM_LOG(DM_N(2), "SURLOrStFN[%u] == NULL\n", i);\
    }\
    if(NOT_NULL_VEC(path,storageSystemInfo)) {\
      NOT_NULL(myInfo->storageSystemInfo = soap_new_srm__TStorageSystemInfo(soap, -1));\
      myInfo->storageSystemInfo->value.assign(path.storageSystemInfo[i]->c_str());\
      DM_LOG(DM_N(2), "storageSystemInfo[%u] == `%s'\n", i, myInfo->storageSystemInfo->value.c_str());\
    } else {\
      myInfo->storageSystemInfo = NULL;\
      DM_LOG(DM_N(2), "storageSystemInfo[%u] == NULL\n", i);\
    }\
\
    req.v1->surlInfoArray.push_back(myInfo);\
  }

#endif /* _SRM_MACROS_H */
