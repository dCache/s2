#ifndef _FREE_H
#define _FREE_H

#define FREE(p)         if(p) { free(p); p = NULL; }
#define DELETE(p)       if(p) { delete p; p = NULL; }

/* free vector of types (e.g. ints) */
#define FREE_VEC(vec)\
  for (uint __u = 0; __u < vec.size(); __u++) {\
    FREE(vec[__u]);\
  }
/* delete vector of objects (e.g. strings) */
#define DELETE_VEC(vec)\
  for (uint __u = 0; __u < vec.size(); __u++) {\
    DELETE(vec[__u]);\
  }

#endif /* _FREE_H */
