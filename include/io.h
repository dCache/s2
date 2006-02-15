#ifndef _IO_H
#define _IO_H

#include <stdio.h>              /* FILE */

/* private function declarations */

/* extern(al) function declarations */
extern int file_ropen(const char* filename, FILE **fin);
extern int file_wopen(const char* filename, FILE **fout);
extern void file_close(const char* filename, FILE **file);
extern char *dfreads(char **s_start, char **s_end, FILE *fin, unsigned int *llen);

#endif /* _IO_H */
