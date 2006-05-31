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

/* simple macros */
#define OPT(s)          (((s) == NULL)? FALSE: (strcmp(opt, (s)) == 0))
#define OPN(s, n)       (((s) == NULL)? FALSE: (strncmp(opt, (s), (n)) == 0))
#define OPI(s)          (((s) == NULL)? FALSE: (strcasecmp(opt, (s)) == 0))
#define OPL(s)          (((s) == NULL)? FALSE: (strncmp(opt, (s), (opt_off = strlen(s))) == 0))

#define UPDATE_MAX(m1, m2)      ((m1) < (m2))? (m1) = (m2): (m1) = (m1)
#define UPDATE_MAXF(m1, m2, f)  if((m1) < (m2 = f)) (m1) = (m2)
#define STRDUP(target,source)\
  if(((target) = strdup(source)) == (char *)NULL) {\
    DM_ERR(ERR_SYSTEM, _("strdup failed\n"));\
    return ERR_SYSTEM;\
  }

/* typedefs */
typedef std::map<std::string, struct nDefun *> TFunctions;

/* extern(al) functions (defined by other modules) */
extern const char* PNAME(void);

/* extern(al) function declarations */
extern int parse(const char *filename, Node **root);

/* global variables */
extern TFunctions gl_fun_tab;	/* table of (global) functions */

#endif /* _PARSE_H */
