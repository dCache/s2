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

