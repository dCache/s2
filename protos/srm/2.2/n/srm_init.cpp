
#include "globus/globus_module.h"
#include "globus/globus_openssl.h"
#include "globus/globus_gss_assist.h"
#include "globus/gssapi.h"
#include "globus/globus_thread.h"
#include "globus/globus_common.h"

#include "stdsoap2.h"

#define USER_AGENT_PREFIX "s2"

using namespace std;

/**
 *  Function to be called exactly once, before any other SRM
 *  interaction.
 */
void srm_init()
{
  globus_thread_set_model("pthread");
  globus_module_activate(GLOBUS_COMMON_MODULE);
  globus_module_activate(GLOBUS_OPENSSL_MODULE);
}


static int
s2_http_post_header(struct soap *soap, const char *key, const char *val)
{
  // HACK: recover the wrapped fposthdr function.
  int (*fposthdr)(struct soap*, const char*, const char*) = (int (*)(struct soap*, const char*, const char*)) soap->fdimewrite;

  if (key && !strcmp(key, "User-Agent")) {
    char *id = (char *)malloc(sizeof(USER_AGENT_PREFIX) + strlen(val) + 4);
    sprintf(id, "%s (%s)", USER_AGENT_PREFIX, val);
    int ret = fposthdr(soap, key, id);
    free(id);
    char *client_info = getenv("CLIENT_INFO");

    if (ret == 0 && client_info != NULL) {
      ret = fposthdr(soap, "ClientInfo", client_info);
    }

    return ret;
  } else {
    return fposthdr(soap, key, val);
  }
}

/**
 *  Function that updates a soap structure so that it points to our
 *  custom fposthdr command.
 */
void wrap_fposthdr(struct soap *soap)
{
  // HACK: We store the original fposthdr value as fdimewrite.  Given
  // we are not doing DIME, it isn't going to be used.
  soap->fdimewrite = (int (*)(struct soap*, void*, const char*, size_t)) soap->fposthdr;

  soap->fposthdr = s2_http_post_header;
}
