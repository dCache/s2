#ifndef _FREE_H
#define _FREE_H

/* private function declarations */

/* extern(al) function declarations */
#define FREE(p)         if(p) { free(p); p = NULL; }
#define DELETE(p)       if(p) { delete p; p = NULL; }

#endif /* _FREE_H */
