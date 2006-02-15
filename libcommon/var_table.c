/*
 * TODO: insertion of NULL strings + destructor
 */
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "var_table.h"

#include "free.h"
#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "str.h"
#include "io.h"                 /* file_ropen(), ... */

#include <signal.h>             /* signal() */
#include <stdlib.h>             /* exit() */
#include <stdio.h>              /* stderr */

#include <iostream>             /* std::string, cout, endl, ... */

using namespace std;

/* typedefs */
typedef std::map<std::string, std::string> TVariables;

/* local variables */
static TVariables var_table;    /* table of variables */

extern void
WriteVariable(const char *name, const char *value)
{
  TVariables::iterator varIter;
 
  if(!name) {
    DM_ERR_ASSERT(_("name == NULL\n"));
    return;
  }
 
  DM_DBG(DM_N(0), _("writing variable `%s' with value `%s'\n"), name, value);

  if ((varIter = var_table.find(name)) != var_table.end()) {
    /* variable `name' exists, change its value */
    varIter->second = value;
  } else {
    /* we have a new variable */
    var_table.insert(std::pair<std::string, std::string>(name, value));   
  }
} /* WriteVariable */

extern void
WriteVariable(const char *name, const char *value, int vlen)
{
  TVariables::iterator varIter;

  if(!name) {
    DM_ERR_ASSERT(_("name == NULL\n"));
    return;
  }
 
  DM_DBG(DM_N(0), _("writing variable `%s' with value `%.*s'\n"), name, vlen, value);

  if ((varIter = var_table.find(name)) != var_table.end()) {
    /* variable `name' exists, change its value */
    varIter->second = std::string(value, vlen);
  } else {
    /* we have a new variable */
    var_table.insert(std::pair<std::string, std::string>(name, std::string(value, vlen)));
  }
} /* WriteVariable */

extern const char *
ReadVariable(const char *name)
{
  TVariables::iterator varIter;

  if(!name) {
    DM_ERR_ASSERT(_("name == NULL\n"));
    return (const char *)NULL;
  }
 
  if ((varIter = var_table.find(name)) != var_table.end()) {
    /* variable exists */
    return varIter->second.c_str();
  } else {
    /* variable doesn't exist */
    return (const char *)NULL;
  }
} /* ReadVariable */
