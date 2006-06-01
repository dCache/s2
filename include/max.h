#ifndef _MAX_H
#define _MAX_H

#define UPDATE_MAX(m1, m2)      ((m1) < (m2))? (m1) = (m2): (m1) = (m1)
#define UPDATE_MAXF(m1, m2, f)  if((m1) < (m2 = f)) (m1) = (m2)

#endif /* _MAX_H */
