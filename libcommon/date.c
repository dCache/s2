#include <sys/time.h>
#include <time.h>

int
week(struct tm *t)
{
  int dow; /* Mon=1... Sun=7 */
  int weeks;
  int yday;

  dow = (t->tm_wday == 0) ? 7 : t->tm_wday;
  yday = (t->tm_yday + 1);
  weeks = yday / 7;
  weeks += (yday % 7) > dow ? 2 : 1 ;

  return weeks;
}
