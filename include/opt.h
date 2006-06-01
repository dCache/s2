#ifndef _OPT_H
#define _OPT_H

/* simple macros */
#define OPT(s)          (((s) == NULL)? FALSE: (strcmp(opt, (s)) == 0))
#define OPN(s, n)       (((s) == NULL)? FALSE: (strncmp(opt, (s), (n)) == 0))
#define OPI(s)          (((s) == NULL)? FALSE: (strcasecmp(opt, (s)) == 0))
#define OPL(s)          (((s) == NULL)? FALSE: (strncmp(opt, (s), (opt_off = strlen(s))) == 0))

#endif /* _OPT_H */
