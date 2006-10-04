#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"                  /* Node, nDelay, ... */

#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "s2.h"			/* opts (s2 options), progress(), ... */
#include "free.h"               /* FREE(), DELETE() */
#include "io.h"                 /* file_ropen(), ... */
#include "str.h"

#include <errno.h>		/* errno */

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
  init();
  Node::init(node);
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
nDelay::exec(Process *proc)
{
  DM_DBG_I;

  std::string s_sec;
  std::string s_nsec;
  const char *word;
  char *endptr;
  struct timeval now;			/* timeval uses micro-seconds */
  timespec req, rem;			/* timespec uses nano-seconds */
  uint64_t sec_add;

  /* seconds */
  s_sec = Process::eval_str(nDelay::sec, proc);
  word = s_sec.c_str();

  req.tv_sec = get_uint64(s_sec.c_str(), &endptr, FALSE);
  if(endptr == word) {
    DM_ERR(ERR_ERR, _("cannot evaluate number of seconds to sleep `%s': %s\n"), word, _(strerror(errno)));
    RETURN(ERR_ERR);
  }

  /* nanoseconds */
  s_nsec = Process::eval_str(nDelay::nsec, proc);
  word = s_nsec.c_str();

  req.tv_nsec = get_uint64(s_nsec.c_str(), &endptr, FALSE);
  if(endptr == word) {
    DM_ERR(ERR_ERR, _("cannot evaluate number of nanoseconds to sleep `%s': %s\n"), word, _(strerror(errno)));
    RETURN(ERR_ERR);
  }

  sec_add = req.tv_nsec / 1000000000U;
  req.tv_sec += sec_add;
  req.tv_nsec %= 1000000000U;

  if(gettimeofday(&now, NULL)) {
    DM_ERR(ERR_SYSTEM, _("gettimeofday: %s\n"), strerror(errno));
    RETURN(ERR_SYSTEM);
  }

  DM_DBG(DM_N(2), _("sleeping sec=%ld seconds, nsec=%ld [now: sec=%ld, usec=%ld]\n"), req.tv_sec, req.tv_nsec, now.tv_sec, now.tv_usec);
  
  /* show 's' => sleeping */
  Process::progress(-2, proc);

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
nDelay::toString(Process *proc)
{
  BOOL quote = TRUE;
  std::stringstream ss;

  ss << "SLEEP";
  if(sec) ss << " " << dq_param(Process::eval_str(sec, proc), quote);
  else ss << " 0";
  if(nsec && strncmp(Process::eval_str(nsec, proc).c_str(),"0",1))
    ss << " " << dq_param(Process::eval_str(nsec, proc), quote);

  return ss.str();
}
