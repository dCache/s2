#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"                  /* Node, nCmp, ... */

#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "free.h"               /* FREE(), DELETE() */
#include "io.h"                 /* file_ropen(), ... */
#include "str.h"

#include <signal.h>             /* signal() */
#include <stdlib.h>             /* exit(), system() */
#include <stdio.h>              /* stderr */
#include <errno.h>              /* errno */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * nCmp constructor
 */
nCmp::nCmp()
{
  /* initialisation */
  init();
}

/*
 * Initialise nCmp request
 */
void
nCmp::init()
{
  cmp = CMP_EQ;
  lop = NULL;
  rop = NULL;
}

/*
 * nCmp copy constuctor
 */
nCmp::nCmp(Node &node)
{
  Node::init(node);
  init();
}

/*
 * nCmp destructor
 */
nCmp::~nCmp()
{
  DELETE(lop);
  DELETE(rop);
}

int
nCmp::exec()
{
  DM_DBG_I;

  int rval;
  int64_t i_lop;
  int64_t i_rop;
  std::string s_lop;
  std::string s_rop;
  const char *word;
  char *endptr;

  if(lop == NULL && rop == NULL) RETURN(ERR_OK);
  if(lop == NULL || rop == NULL) RETURN(ERR_ERR);

  s_lop = eval_str(lop, TRUE);
  s_rop = eval_str(rop, TRUE);

  word = s_lop.c_str();
  i_lop = get_int64(word, &endptr, FALSE);
  DM_DBG(DM_N(5), "lop=|%s|\n", s_lop.c_str());
  if(endptr == word) goto eval_as_strings;

  word = s_rop.c_str();
  i_rop = get_int64(word, &endptr, FALSE);
  DM_DBG(DM_N(5), "rop=|%s|\n", s_rop.c_str());
  if(endptr == word) goto eval_as_strings;

  switch(cmp) {
    case CMP_EQ: rval = (i_lop == i_rop) ? ERR_OK : ERR_ERR; break;
    case CMP_NE: rval = (i_lop != i_rop) ? ERR_OK : ERR_ERR; break; 
    case CMP_LT: rval = (i_lop < i_rop)  ? ERR_OK : ERR_ERR; break; 
    case CMP_GT: rval = (i_lop > i_rop)  ? ERR_OK : ERR_ERR; break; 
    case CMP_LE: rval = (i_lop <= i_rop) ? ERR_OK : ERR_ERR; break; 
    case CMP_GE: rval = (i_lop >= i_rop) ? ERR_OK : ERR_ERR; break; 
  }
  RETURN(rval);

eval_as_strings:
  DM_DBG(DM_N(5), "Evaluating as strings (%d); |%s|%s|\n", cmp, s_lop.c_str(), s_rop.c_str());
  switch(cmp) {
    case CMP_EQ: rval = (strcmp(s_lop.c_str(),s_rop.c_str()) == 0) ? ERR_OK : ERR_ERR; break;
    case CMP_NE: rval = (strcmp(s_lop.c_str(),s_rop.c_str()) != 0) ? ERR_OK : ERR_ERR; break;
    case CMP_LT: rval = (strcmp(s_lop.c_str(),s_rop.c_str()) < 0)  ? ERR_OK : ERR_ERR; break;
    case CMP_GT: rval = (strcmp(s_lop.c_str(),s_rop.c_str()) > 0)  ? ERR_OK : ERR_ERR; break;
    case CMP_LE: rval = (strcmp(s_lop.c_str(),s_rop.c_str()) <= 0) ? ERR_OK : ERR_ERR; break;
    case CMP_GE: rval = (strcmp(s_lop.c_str(),s_rop.c_str()) >= 0) ? ERR_OK : ERR_ERR; break;
  }
  RETURN(rval);
}

std::string
nCmp::toString(BOOL eval)
{
  BOOL quote = TRUE;
  std::stringstream ss;

  switch(cmp) {
    case CMP_EQ: ss << "EQ"; break;
    case CMP_NE: ss << "NE"; break;
    case CMP_LT: ss << "LT"; break;
    case CMP_GT: ss << "GT"; break;
    case CMP_LE: ss << "LE"; break;
    case CMP_GE: ss << "GE"; break;
  }
  SS_DQ(" ", lop);
  SS_DQ(" ", rop);

  return ss.str();
}
