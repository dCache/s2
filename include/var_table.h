#ifndef _VAR_TABLE_H
#define _VAR_TABLE_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include "sysdep.h"             /* BOOL */

#include "constants.h"

#include <iostream>             /* std::string, cout, endl, ... */
#include <fstream>              /* ifstream */
#include <map>                  /* std::map */

/* extern(al) function declarations */
extern const char *ReadVariable(const char *name);
extern void WriteVariable(const char *name, const char *value);
extern void WriteVariable(const char *name, const char *value, int vlen);

#endif /* _VAR_TABLE_H */
