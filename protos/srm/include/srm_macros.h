#ifndef _SRM_MACROS_H
#define _SRM_MACROS_H

#ifndef RETURN
#define RETURN(...) do {DM_DBG_O; return __VA_ARGS__;} while(0)
#endif

#define CSTR(s)		(s)? s->c_str(): (const char *)NULL	/* no SEGVs if s is NULL */

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

#define NOT_NULL_VEC(v,r) (u < v.r.size() && v.r[u])
#define NOT_NULL_VEC1(v)  (u < v.size() && v[u])

#ifdef HAVE_SRM21
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

#define NEW_ARRAY_OF_STR_VAL(v1,v2,opt,t)\
  NOT_NULL(req.v1 = soap_new_srm__ArrayOf##t(soap, -1));\
  for(uint u = 0; u < v2.size(); u++) {\
    srm__##t *myInfo;\
    if(v2[u]) {\
      NOT_NULL(myInfo = soap_new_srm__##t(soap, -1));\
      myInfo->value.assign(v2[u]->c_str());\
      DM_LOG(DM_N(2), ""#opt "[%u] == `%s'\n", u, myInfo->value.c_str());\
    } else {\
      DM_LOG(DM_N(2), ""#opt "[%u] == NULL\n", u);\
      myInfo = NULL;\
    }\
    req.v1->opt.push_back(myInfo);\
  }

#define ARRAY_OF_TSURL_INFO(v1)\
  DM_LOG(DM_N(2), "path.SURLOrStFN.size() == %d\n", path.SURLOrStFN.size());\
  for (uint u = 0; u < path.SURLOrStFN.size(); u++) {\
    srm__TSURLInfo *myInfo;\
    NOT_NULL(myInfo = soap_new_srm__TSURLInfo(soap, -1));\
\
    if(NOT_NULL_VEC(path,SURLOrStFN)) {\
      NOT_NULL(myInfo->SURLOrStFN = soap_new_srm__TSURL(soap, -1));\
      myInfo->SURLOrStFN->value.assign(path.SURLOrStFN[u]->c_str());\
      DM_LOG(DM_N(2), "SURLOrStFN[%u] == `%s'\n", u, myInfo->SURLOrStFN->value.c_str());\
    } else {\
      myInfo->SURLOrStFN = NULL;\
      DM_LOG(DM_N(2), "SURLOrStFN[%u] == NULL\n", u);\
    }\
    if(NOT_NULL_VEC(path,storageSystemInfo)) {\
      NOT_NULL(myInfo->storageSystemInfo = soap_new_srm__TStorageSystemInfo(soap, -1));\
      myInfo->storageSystemInfo->value.assign(path.storageSystemInfo[u]->c_str());\
      DM_LOG(DM_N(2), "storageSystemInfo[%u] == `%s'\n", u, myInfo->storageSystemInfo->value.c_str());\
    } else {\
      myInfo->storageSystemInfo = NULL;\
      DM_LOG(DM_N(2), "storageSystemInfo[%u] == NULL\n", u);\
    }\
\
    req.v1->surlInfoArray.push_back(myInfo);\
  }
#endif	/* HAVE_SRM21 */

#ifdef HAVE_SRM22
#define MV_PINT(t,s)\
  do {\
    t = s;\
    if(t) {\
      DM_LOG(DM_N(2), ""#t " == %d\n",*t);\
    } else {\
      DM_LOG(DM_N(2), ""#t " == NULL\n");\
    }\
  } while(0)

#define MV_PBOOL(t,s) MV_PINT(t,s)

#define MV_PINT64(t,s)\
  do {\
    t = s;\
    if(t) {\
      DM_LOG(DM_N(2), ""#t " == %"PRIi64"\n",*t);\
    } else {\
      DM_LOG(DM_N(2), ""#t " == NULL\n");\
    }\
  } while(0)

#define MV_PUINT64(t,s)\
  do {\
    t = s;\
    if(t) {\
      DM_LOG(DM_N(2), ""#t " == %"PRIu64"\n",*t);\
    } else {\
      DM_LOG(DM_N(2), ""#t " == NULL\n");\
    }\
  } while(0)

#define MV_CSTR2PSTR(t,s)\
  if(s) {\
    NOT_NULL(t = soap_new_std__string(soap, -1));\
    t->assign(s);\
    DM_LOG(DM_N(2), ""#t " == `%s'\n",t->c_str());\
  } else {\
    DM_LOG(DM_N(2), ""#t " == NULL\n");\
    t = NULL;\
  }

#define MV_CSTR2STR(t,s)\
  do {\
    t.assign(s);\
    DM_LOG(DM_N(2), ""#t " == `%s'\n", t.c_str());\
  } while(0)

#define MV_PSTR2PSTR(t,s)\
  do {\
    t = s;\
    DM_LOG(DM_N(2), ""#t " == `%s'\n", CSTR(s));\
  } while(0)

/* SRM types */
#define MV_SOAP(type,t,s)\
  do {\
    t = (srm__T##type)s;\
    DM_LOG(DM_N(2), ""#type " == %s\n", (t)? getT##type(t).c_str() : NULL);\
  } while(0)

#define MV_PSOAP(type,t,s)\
  do {\
    t = (srm__T##type *)s;\
    DM_LOG(DM_N(2), ""#type " == %s\n", (t)? getT##type(*(t)).c_str() : NULL);\
  } while(0)

#define MV_ARRAY_OF_STR_VAL(v1,v2,opt,t)\
  NOT_NULL(v1 = soap_new_srm__ArrayOf##t(soap, -1));\
  for(uint u = 0; u < v2.size(); u++) {\
    if(v2[u]) {\
      DM_LOG(DM_N(2), ""#opt "[%u] == `%s'\n", u, v2[u]->c_str());\
      v1->opt.push_back(v2[u]->c_str());\
    } else {\
      DM_LOG(DM_N(2), ""#opt "[%u] == NULL\n", u);\
      v1->opt.push_back("");\
    }\
  }

#define MV_STORAGE_SYSTEM_INFO(v1,v2)\
  NOT_NULL(v1 = soap_new_srm__ArrayOfTExtraInfo(soap, -1));\
  for (uint u = 0; u < v2.key.size(); u++) {\
    DM_LOG(DM_N(2), "storageSystemInfo.key[%u]\n", u);\
    srm__TExtraInfo *extraInfo;\
    NOT_NULL(extraInfo = soap_new_srm__TExtraInfo(soap, -1));\
    MV_CSTR2STR(extraInfo->key,CSTR(v2.key[u]));\
    MV_PSTR2PSTR(extraInfo->value,v2.value[u]);\
    v1->extraInfoArray.push_back(extraInfo);\
  }

#endif	/* HAVE_SRM22 */

#endif /* _SRM_MACROS_H */
