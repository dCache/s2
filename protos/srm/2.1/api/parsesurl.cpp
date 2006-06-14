#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include <string.h>             /* strstr(), ... */
#include <errno.h>              /* errno */

#define SRM_EP_PATH "/v2_1_1/srm"

extern int
parsesurl (const char *surl, char **endpoint, char **sfn)
{
    unsigned int len;
    unsigned int lenp;
    char *p;
    static char srm_ep[256];

    if (strncmp (surl, "srm://", 6)) {
        errno = EINVAL;
        return (-1);
    }
    if ((p = strstr (surl + 6, "?SFN="))) {
        *sfn = p + 5;
    }
    else if ((p = strchr (surl + 6, '/'))) {
        *sfn = p;
    }
    else {
        errno = EINVAL;
        return (-1);
    }
#ifdef HAVE_CGSI_PLUGIN
    strcpy (srm_ep, "https://");
    lenp = 8;
#else
    strcpy (srm_ep, "http://");
    lenp = 7;
#endif
    len = p - surl - 6;
    if (lenp + len >= sizeof(srm_ep)) {
        errno = EINVAL;
        return (-1);
    }
    strncpy (srm_ep + lenp, surl + 6, len);
    *(srm_ep + lenp + len) = '\0';
    if (strchr (srm_ep + lenp, '/') == NULL) {
        if (strlen (SRM_EP_PATH) + lenp + len >= sizeof(srm_ep)) {
            errno = EINVAL;
            return (-1);
        }
        strcat (srm_ep, SRM_EP_PATH);
    }
    *endpoint = srm_ep;
    return (0);
}
