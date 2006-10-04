#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>  /* explicit include for SL4 */

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "io.h"

#include "constants.h"
#include "i18.h"
#include "sysdep.h"     /* BOOL, STD_BUF, ... */

#include <errno.h>      /* errno */
#include <string.h>     /* strerror() */
#include <stdlib.h>     /* malloc(), realloc() */

/*
 * Open `filename' for reading and return its file descriptor in *fin.
 * If `filename' is NULL, returns stdin in *fin.
 */
extern int 
file_ropen(const char* filename, FILE **fin)
{
  if(fin == NULL) {
    DM_ERR_ASSERT(_("fin == NULL\n"));
    goto err;
  }

  if(filename) {
    /* we have a filename of a file to open */
    *fin = fopen(filename, "rb");
    if(!*fin)
    {
err:
      DM_ERR(ERR_SYSTEM, _("failed to open `%s' for reading: %s\n"), filename, _(strerror(errno)));

      return ERR_SYSTEM;
    }
  } else
    /* use standard input */
    *fin = stdin;
  
  return 0;
} /* open_rfile */

/*
 * Open `filename' for writing and return its file descriptor in *fout.
 * If `filename' is NULL, returns stdin in *fout.
 */
extern int 
file_wopen(const char* filename, FILE **fout)
{
  if(fout == NULL) {
    DM_ERR_ASSERT(_("fout == NULL\n"));
    goto err;
  }

  if(filename) {
    /* we have a filename of a file to open */
    *fout = fopen(filename, "wb");
    if(!*fout)
    {
err:
      DM_ERR(ERR_SYSTEM, _("failed to open `%s' for writing: %s\n"), filename, _(strerror(errno)));

      return ERR_SYSTEM;
    }
  } else
    /* use standard output */
    *fout = stdout;
  
  return 0;
} /* open_wfile */

/*
 * Close file `filename' with descriptor *fin.  Issue an error message if this fails.
 */
extern void 
file_close(const char* filename, FILE **file)
{
  if(file == NULL) {
    DM_ERR_ASSERT(_("fin == NULL\n"));
    return;
  }
  if(*file == NULL)
    /* nothing to close */
    return;

  if(filename && fclose(*file) != 0)
    DM_ERR(ERR_SYSTEM, _("failed to close `%s': %s\n"), filename, _(strerror(errno)));
  else
    *file = NULL;
} /* close_file */

/*
 * Read a '\n' terminated line from a file.
 *
 * It also dynamically allocates more memory if we are trying to write
 * past `*s_end' (s_end is the last memory cell which can be written
 * without reallocation).
 *
 * Returns
 *   Returns a pointer to a '\n\0' or '\0' terminated string read from `fin'.
 *   NULL if there was an error
 *
 */
extern char *
dfreads(char **s_start, char **s_end, FILE *fin, unsigned int *llen)
{
#define REALLOC_BYTES   STD_BUF

  char *s = NULL;
  int c;

#ifndef FAST_CODE
  if(s_start == NULL) {
    DM_ERR_ASSERT(_("s_start == NULL\n"));
    return NULL;
  }

  if(s_end == NULL) {
    DM_ERR_ASSERT(_("s_end == NULL\n"));
    return NULL;
  }
#endif

  s = *s_start;

  if(s == NULL) {
    /* we have an unallocated string */
    s = *s_start = (char *)malloc(STD_BUF + 1);
    if(s == NULL)
    {
      DM_ERR(ERR_SYSTEM, _("malloc failed\n"));
      return NULL;
    }
    *s_end = *s_start + STD_BUF;
  }

/* Assign with bounds checking and Realloc */
#ifdef FAST_CODE
#define AR(lval, val)\
  (lval) = (val)
#else
#define AR(lval, val)\
  if(s > *s_end) {\
    REALLOC;\
  }\
  (lval) = (val);
#define REALLOC\
  int s_off = s - *s_start;\
  int s_size = *s_end - *s_start;\
  errno = 0;\
  if((*s_start = (char *)realloc(*s_start, s_size + REALLOC_BYTES + 1)) == NULL) {\
    DM_ERR(ERR_SYSTEM, _("realloc failed: %s\n"), _(strerror(errno)));\
    return NULL;\
  };\
  /*Verbose(1, _("%s: realloc OK\n"), __FUNCTION__);*/\
  *s_end = *s_start + s_size + REALLOC_BYTES;\
  s = *s_start + s_off;
#endif

  while((c = fgetc(fin)) != EOF) {
    AR(*s++, c);
    if(c == '\n') break;
  }
  AR(*s, '\0');

  if(llen) *llen = s - *s_start;

  return (c == EOF && *llen == 0)? NULL: *s_start;

#undef AR
#undef REALLOC
#undef REALLOC_BYTES
} /* dfreads */

