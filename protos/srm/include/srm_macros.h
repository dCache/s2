#ifndef _SRM_MACROS_H
#define _SRM_MACROS_H

void wrap_fposthdr(struct soap *);

#ifndef RETURN
#define RETURN(...) do {DM_DBG_O; return __VA_ARGS__;} while(0)
#endif

#define CSTR(s)		(s)? s->c_str(): (const char *)NULL	/* no SEGVs if s is NULL */

#define DO_SOAP_INIT(soap)\
  do {\
    if(!soap) {\
      DM_ERR_ASSERT("soap == NULL\n");\
      RETURN(EXIT_FAILURE);\
    } else {					\
      soap_init(soap);				\
      wrap_fposthdr(soap);			\
    }						\
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
  do {\
    if((par) == NULL) {\
      perror("memory allocation failed");\
      RETURN(EXIT_FAILURE);\
    }\
  } while(0)

#define NOT_0(v,par,n)\
  do {\
    if(!v.size()) par = NULL;\
    else NOT_NULL(par = n);\
  } while(0)

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
#define MV_INT(t,s)\
  do {\
    t = s;\
    DM_LOG(DM_N(2), ""#t " == %d\n",t);\
  } while(0)

#define MV_UINT(t,s)\
  do {\
    t = s;\
    DM_LOG(DM_N(2), ""#t " == %u\n",t);\
  } while(0)

#define MV_INT64(t,s)\
  do {\
    t = s;\
    DM_LOG(DM_N(2), ""#t " == %"PRIi64"\n",t);\
  } while(0)

#define MV_UINT64(t,s)\
  do {\
    t = s;\
    DM_LOG(DM_N(2), ""#t " == %"PRIu64"\n",t);\
  } while(0)

#define MV_PINT(t,s)\
  do {\
    t = s;\
    if(t) {\
      DM_LOG(DM_N(2), ""#t " == %d\n",*t);\
    } else {\
      DM_LOG(DM_N(2), ""#t " == NULL\n");\
    }\
  } while(0)

#define MV_PUINT(t,s)\
  do {\
    t = s;\
    if(t) {\
      DM_LOG(DM_N(2), ""#t " == %u\n",*t);\
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

#define MV_CSTR2STR(t,s)\
  do {\
    if(s) {\
      t.assign(s);\
    }\
    DM_LOG(DM_N(2), ""#t " == `%s'\n", t.c_str());\
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

#define MV_PSTR2PSTR(t,s)\
  do {\
    t = s;\
    DM_LOG(DM_N(2), ""#t " == `%s'\n", CSTR(t));\
  } while(0)

/* SRM types */
#define MV_SOAP(type,t,s)\
  do {\
    t = (srm__T##type)s;\
    DM_LOG(DM_N(2), ""#t " == %s\n", (t)? getT##type(t).c_str() : NULL);\
  } while(0)

#define MV_PSOAP(type,t,s)\
  do {\
    t = (srm__T##type *)s;\
    DM_LOG(DM_N(2), ""#t " == %s\n", (t)? getT##type(*(t)).c_str() : NULL);\
  } while(0)

#define MV_ARRAY_OF_STR_VAL(v1,v2,opt,t)\
  do {\
    NOT_0(v2,v1,soap_new_srm__ArrayOf##t(soap, -1));\
    for(uint u = 0; u < v2.size(); u++) {\
      if(v2[u]) {\
        DM_LOG(DM_N(2), ""#opt "[%u] == `%s'\n", u, v2[u]->c_str());\
        v1->opt.push_back(v2[u]->c_str());\
      } else {\
        DM_LOG(DM_N(2), ""#opt "[%u] == NULL\n", u);\
        v1->opt.push_back("");\
      }\
    }\
  } while(0)

#define MV_STORAGE_SYSTEM_INFO(v1,v2)\
  do {\
    NOT_0(v2.key,v1,soap_new_srm__ArrayOfTExtraInfo(soap, -1));\
    for (uint u = 0; u < v2.key.size(); u++) {\
      DM_LOG(DM_N(2), "storageSystemInfo.key[%u]\n", u);\
      srm__TExtraInfo *extraInfo;\
      NOT_NULL(extraInfo = soap_new_srm__TExtraInfo(soap, -1));\
      MV_CSTR2STR(extraInfo->key,CSTR(v2.key[u]));\
      MV_PSTR2PSTR(extraInfo->value, NOT_NULL_VEC(v2,value)? v2.value[u]: NULL);\
      v1->extraInfoArray.push_back(extraInfo);\
    }\
  } while(0)

#define MV_TRANSFER_PARAMETERS(r)\
  /* Transfer parameters */\
  do {\
    if(!accessPattern && !connectionType && !clientNetworks.size() && !transferProtocols.size()) r = NULL;\
    else {\
      NOT_NULL(r = soap_new_srm__TTransferParameters(soap, -1));\
      MV_PSOAP(AccessPattern,r->accessPattern,accessPattern);\
      MV_PSOAP(ConnectionType,r->connectionType,connectionType);\
      /*   client networks */\
      NOT_0(clientNetworks,r->arrayOfClientNetworks,soap_new_srm__ArrayOfString(soap, -1));\
      for(uint u = 0; u < clientNetworks.size(); u++) {\
        DM_LOG(DM_N(2), "clientNetworks[%u]\n", u);\
        if(clientNetworks[u]) {\
          r->arrayOfClientNetworks->stringArray.push_back(CSTR(clientNetworks[u]));\
          DM_LOG(DM_N(2), "clientNetworks[%u] == `%s'\n", u, r->arrayOfClientNetworks->stringArray.back().c_str());\
        } else {\
          DM_LOG(DM_N(2), "clientNetworks[%u] == NULL\n", u);\
        }\
      }\
      /*   transfer protocols */\
      NOT_0(transferProtocols,r->arrayOfTransferProtocols,soap_new_srm__ArrayOfString(soap, -1));\
      for(uint u = 0; u < transferProtocols.size(); u++) {\
        DM_LOG(DM_N(2), "transferProtocols[%u]\n", u);\
        if(transferProtocols[u]) {\
          r->arrayOfTransferProtocols->stringArray.push_back(CSTR(transferProtocols[u]));\
          DM_LOG(DM_N(2), "transferProtocols[%u] == `%s'\n", u, r->arrayOfTransferProtocols->stringArray.back().c_str());\
        } else {\
          DM_LOG(DM_N(2), "transferProtocols[%u] == NULL\n", u);\
        }\
      }\
    }\
  } while(0)

#define MV_RETENTION_POLICY(r,rp,al)\
  do {\
    if(rp) {\
      NOT_NULL(r = soap_new_srm__TRetentionPolicyInfo(soap, -1));\
      MV_SOAP(RetentionPolicy,r->retentionPolicy,*rp);\
      MV_PSOAP(AccessLatency,r->accessLatency,al);\
    } else {\
      DM_LOG(DM_N(2), ""#r " == NULL\n");\
      r = NULL;\
    }\
  } while(0)

#define MV_DIR_OPTION_VEC(r)\
  do {\
    if(fileRequests.isSourceADirectory.size() != 0\
       || fileRequests.allLevelRecursive.size() != 0\
       || fileRequests.numOfLevels.size() != 0) {\
      NOT_NULL(r->dirOption = soap_new_srm__TDirOption(soap, -1));\
\
      if(NOT_NULL_VEC(fileRequests,isSourceADirectory)) {\
        r->dirOption->isSourceADirectory = fileRequests.isSourceADirectory[u];\
        DM_LOG(DM_N(2), "isSourceADirectory[%u] = %d\n", u, r->dirOption->isSourceADirectory);\
      } else {\
        r->dirOption->isSourceADirectory = 0;\
        DM_LOG(DM_N(2), "isSourceADirectory[%u] == 0\n", u);\
      }\
      if(NOT_NULL_VEC(fileRequests,allLevelRecursive)) {\
        r->dirOption->allLevelRecursive = (bool *)fileRequests.allLevelRecursive[u];\
        DM_LOG(DM_N(2), "allLevelRecursive[%u] = %d\n", u, *(r->dirOption->allLevelRecursive));\
      } else {\
        r->dirOption->allLevelRecursive = NULL;\
        DM_LOG(DM_N(2), "allLevelRecursive[%u] == NULL\n", u);\
      }\
      if(NOT_NULL_VEC(fileRequests,numOfLevels)) {\
        r->dirOption->numOfLevels = fileRequests.numOfLevels[u];\
        DM_LOG(DM_N(2), "numOfLevels[%u] = %d\n", u, *(r->dirOption->numOfLevels));\
      } else {\
        r->dirOption->numOfLevels = NULL;\
        DM_LOG(DM_N(2), "numOfLevels[%u] == NULL\n", u);\
      }\
    } else {\
      DM_DBG(DM_N(3), "dirOption = NULL\n");\
      r->dirOption = NULL;\
    }\
  } while(0)

#endif	/* HAVE_SRM22 */

#endif /* _SRM_MACROS_H */
