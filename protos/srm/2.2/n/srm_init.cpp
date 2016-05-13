
#include "globus/globus_module.h"
#include "globus/globus_openssl.h"
#include "globus/globus_gss_assist.h"
#include "globus/gssapi.h"
#include "globus/globus_thread.h"
#include "globus/globus_common.h"

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
