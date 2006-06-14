#ifndef _PARSE_H
#define _PARSE_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include "n.h"          /* Node */
#include "sysdep.h"     /* BOOL */

/* parser constants */
#define CH_COMMENT      ';'                     /* script comment character */
#define CH_PREPROC      '#'                     /* script preprocessor character */
#define CH_EOL          -1                      /* EOL character (parsing purposes) */
#define CH_INV_UGC      0                       /* invalid unget() character */
#define MAX_ID          63                      /* maximum length of an identifier */
#define MAX_IFS         63                      /* maximum number of nested #if directives */
#define MAX_INCS	63			/* maximum number of files to include */

#define STRDUP(target,source)\
  if(((target) = strdup(source)) == (char *)NULL) {\
    DM_ERR(ERR_SYSTEM, _("strdup failed\n"));\
    return ERR_SYSTEM;\
  }

/* typedefs */
typedef std::map<std::string, struct nDefun *> TFunctions;
typedef std::map<std::string, std::string> TDefines;

/* extern(al) functions (defined by other modules) */
extern const char* PNAME(void);

/* extern(al) function declarations */
extern int parse(const char *filename, Node **root);

/* global variables */
extern TFunctions gl_fun_tab;	/* table of (global) functions */

#endif /* _PARSE_H */
