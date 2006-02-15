#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"                  /* Node, nDelay, ... */

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
#include <time.h>               /* nanosleep() */

#include <iostream>             /* std::string, cout, endl, ... */
#include <sstream>              /* ostringstream */

using namespace std;

/*
 * nDelay constructor
 */
nDelay::nDelay()
{
  /* initialisation */
  init();
}

/*
 * Initialise nDelay request
 */
void
nDelay::init()
{
  sec = NULL;
  nsec = NULL;
}

/*
 * nDelay copy constuctor
 */
nDelay::nDelay(Node &node)
{
  Node::init(node);
  init();
}

/*
 * nDelay destructor
 */
nDelay::~nDelay()
{
  DELETE(sec);
  DELETE(nsec);
}

int
nDelay::exec()
{
  DM_DBG_I;

  std::string s_sec;
  std::string s_nsec;
  const char *word;
  char *endptr;
  timespec req, rem;
  uint64_t sec_add;

  /* seconds */
  s_sec = eval_str(nDelay::sec, TRUE);
  word = s_sec.c_str();

  req.tv_sec = get_uint64(s_sec.c_str(), &endptr, FALSE);
  if(endptr == word) {
    DM_ERR(ERR_ERR, _("cannot evaluate number of seconds to sleep `%s': %s\n"), word, _(strerror(errno)));
    RETURN(ERR_ERR);
  }

  /* nanoseconds */
  s_nsec = eval_str(nDelay::nsec, TRUE);
  word = s_nsec.c_str();

  req.tv_nsec = get_uint64(s_nsec.c_str(), &endptr, FALSE);
  if(endptr == word) {
    DM_ERR(ERR_ERR, _("cannot evaluate number of nanoseconds to sleep `%s': %s\n"), word, _(strerror(errno)));
    RETURN(ERR_ERR);
  }

  sec_add = req.tv_nsec / 1000000000U;
  req.tv_sec += sec_add;
  req.tv_nsec %= 1000000000U;

  DM_DBG(DM_N(2), _("sleeping %d seconds, %d nanoseconds\n"), req.tv_sec, req.tv_nsec);

  while(nanosleep(&req, &rem) != 0) {
    if(errno == EINTR) {
      /* nanosleep interrupted by a signal, continue sleeping */
      req = rem;
      continue;
    }
    DM_ERR(ERR_SYSTEM, _("nanosleep failed: %s\n"), _(strerror(errno)));
    RETURN(ERR_SYSTEM);
  }

  RETURN(ERR_OK);
}

std::string
nDelay::toString(BOOL eval)
{
  BOOL quote = TRUE;
  std::stringstream ss;

  ss << "SLEEP";
  if(sec) ss << " " << dq_param(eval_str(sec, eval), quote);
  else ss << " 0";
  if(nsec && strncmp(eval_str(nsec,eval).c_str(),"0",1))
    ss << " " << dq_param(eval_str(nsec, eval), quote);

  return ss.str();
}
