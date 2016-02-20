#ifndef _CONSTANTS_H
#define _CONSTANTS_H

/* return values (error codes) */
#define ERR_OK          0                       /* success */
#define ERR_WARN        1                       /* warning(s) */
#define ERR_ERR         2                       /* general function failure */
#define ERR_NEXEC       3                       /* program could not be executed for some reason */
#define ERR_ASSERT      4                       /* assertion failed error */
#define ERR_SYSTEM      5                       /* system error (malloc failed, etc.) */

/* limits ((U)INT_MAX/MIN defined in limits.h) */
#ifndef UINT_MIN
#define UINT_MIN	0U
#endif
#ifndef INT8_MAX
#define INT8_MAX	127
#endif
#ifndef INT8_MIN
#define INT8_MIN	(-INT8_MAX - 1)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX	255U
#endif
#ifndef UINT8_MIN
#define UINT8_MIN	0U
#endif
#ifndef INT16_MAX
#define INT16_MAX	32767
#endif
#ifndef INT16_MIN
#define INT16_MIN	(-INT16_MAX - 1)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX	65535U
#endif
#ifndef UINT16_MIN
#define UINT16_MIN	0U
#endif
#ifndef INT32_MAX
#define INT32_MAX	2147483647
#endif
#ifndef INT32_MIN
#define INT32_MIN	(-INT32_MAX - 1)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX	4294967295U
#endif
#ifndef UINT32_MIN
#define UINT32_MIN	0U
#endif
#ifndef INT64_MAX
#define INT64_MAX	9223372036854775807LL
#endif
#ifndef INT64_MIN
#define INT64_MIN	(-INT64_MAX - 1LL)
#endif
#ifndef UINT64_MAX
#define UINT64_MAX	18446744073709551615ULL
#endif
#ifndef UINT64_MIN
#define UINT64_MIN	0ULL
#endif

/* S2 specific constants */
#define S2_UNDEF_STR    "(undef)"	/* undefined */
#define S2_NULL_STR     "(nil)"		/* NULL */

/* Simple macros */
#ifndef RETURN
#define RETURN(...) do {DM_DBG_O; return __VA_ARGS__;} while(0)
#endif

/* type definitions */
typedef enum S2_condition {
  S2_COND_NONE = 0,
  S2_COND_OR = 1,
  S2_COND_AND = 2,
  S2_COND_SEQ = 3,
} S2_condition;

typedef enum S2_repeat {
  S2_REPEAT_NONE = 0,
  S2_REPEAT_OR = 1,
  S2_REPEAT_AND = 2,
  S2_REPEAT_SEQ = 3,
  S2_REPEAT_PAR = 4,
  S2_REPEAT_WHILE = 5,
} S2_repeat;

/* S2 defaults */
#define S2_EVAL         0                       /* default evaluation level */
#define S2_TIMEOUT      0                       /* default timeout value */


#endif /* _CONSTANTS_H */
