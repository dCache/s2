#ifndef _FREE_H
#define _FREE_H

#define FREE(p)         if(p) { free(p); p = NULL; }
#define DELETE(p)       if(p) { delete p; p = NULL; }

/* free vector of types (e.g. ints) */
#define FREE_VEC(vec)\
  for (uint __i = 0; __i < vec.size(); __i++) {\
    FREE(vec[__i]);\
  }
/* delete vector of objects (e.g. strings) */
#define DELETE_VEC(vec)\
  for (uint __i = 0; __i < vec.size(); __i++) {\
    DELETE(vec[__i]);\
  }

#endif /* _FREE_H */
