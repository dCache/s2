#ifndef _SYSDEP_H
#define _SYSDEP_H

/* Note: BUFSIZ is defined in stdio.h, 8192 on my system (Linux), 512 on MinGW Win98/XP/...! */
#define STD_BUF         8191                    /* my standard buffer size */
//#define STD_BUF       10                      /* my standard buffer size (checking purposes) */

/* type definitions */
#ifndef BOOL
  typedef unsigned char BOOL;
#endif

typedef int int_t;						/* type alias */
typedef unsigned int uint_t;					/* type alias */

#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif

#ifdef __STDC__
#define MAX32U 0xFFFFFFFFU
#else
#define MAX32U 0xFFFFFFFF
#endif
#define MAX32  0x8FFFFFFF

#if defined(unix) || defined(__unix) || defined (__unix__)
/* Code for Unix.  Any Unix compiler should define one of the above three
 * symbols. */

#define PATH_CHAR '/'
#define PATH_STR "/"

/* end of 'if unix' */

#elif defined(WIN32) || defined(__WIN32__)

#include <winsock2.h>  /* For NT socket */
#include <ws2tcpip.h>  /* IP_ADD_MEMBERSHIP */
#include <windows.h>
#include <time.h>      /* time_t */
#include <utilNT.h>    /* For function and struct in UNIX but not in NT */

#define PATH_CHAR '\\'
#define PATH_STR "\\"

#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL

#endif

#define NOLONGLONG
#define RTP_LITTLE_ENDIAN 1
#define nextstep

/* Determine if the C(++) compiler requires complete function prototype  */
#ifndef __P
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#define __P(x) x
#else
#define __P(x) ()
#endif
#endif

#ifdef __BORLANDC__
#include <io.h>
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif /* __BORLANDC__ */

#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define open _open
#define write _write
#define close _close
#define ftime _ftime
#define timeb _timeb
#endif /* _MSC_VER */

#ifndef SIGBUS
#define SIGBUS SIGINT
#endif

#ifndef SIGHUP
#define SIGHUP SIGINT
#endif

#ifndef SIGPIPE
#define SIGPIPE SIGINT
#endif

#if 0
typedef int     ssize_t;
typedef long pid_t;
typedef long gid_t;
typedef long uid_t;
typedef unsigned long u_long;
typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned char u_char;
#endif

typedef char *   caddr_t;        /* core address */
typedef long  fd_mask;
#define NBBY  8   /* number of bits in a byte */
#define NFDBITS (sizeof(fd_mask) * NBBY)  /* bits per mask */
#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif

struct msghdr {
        caddr_t msg_name;               /* optional address */
        int     msg_namelen;            /* size of address */
        struct  iovec *msg_iov;         /* scatter/gather array */
        int     msg_iovlen;             /* # elements in msg_iov */
        caddr_t msg_accrights;          /* access rights sent/received */
        int     msg_accrightslen;
};

struct passwd {
        char    *pw_name;
        char    *pw_passwd;
        uid_t   pw_uid;
        gid_t   pw_gid;
        char    *pw_age;
        char    *pw_comment;
        char    *pw_gecos;
        char    *pw_dir;
        char    *pw_shell;
};

#if 0
struct ip_mreq {
        struct in_addr  imr_multiaddr;  /* IP multicast address of group */
        struct in_addr  imr_interface;  /* local IP address of interface */
};
#endif

#define  ITIMER_REAL     0       /* Decrements in real time */

#ifndef _TIMESPEC_T
#define _TIMESPEC_T
typedef struct  timespec {              /* definition per POSIX.4 */
        time_t          tv_sec;         /* seconds */
        long            tv_nsec;        /* and nanoseconds */
} timespec_t;
#endif  /* _TIMESPEC_T */

struct  itimerval {
        struct  timeval it_interval;    /* timer interval */
        struct  timeval it_value;       /* current value */
};

#ifndef ETIME
#define ETIME 1
#endif

#ifndef SIGKILL
#define SIGKILL SIGTERM
#endif

#define fork() 0
#define setsid() {}

#ifndef FILE_SOCKET
#define FILE_SOCKET int
#endif

#ifndef fdopen_socket
#define fdopen_socket(f, g) &f
#endif

#ifndef fclose_socket
#define fclose_socket(f) closesocket(*f)
#endif

extern int winfd_dummy;  /* for WinNT see unitNT.c by Akira 12/27/01 */
extern char getc_socket(FILE_SOCKET *f);
extern ssize_t write_socket(int fildes, const void *buf, size_t nbyte);
extern int sendmsg(int s, const struct msghdr *msg, int flags);

/* end of 'ifdef WIN32' */
#else
#error "Not Unix or WIN32 -- what system is this?"
#endif

#if !defined(sun4) && !defined(hp) && !defined(nextstep) && !defined(linux)
#include <sys/select.h>  /* select() */
#endif

#endif /* end of ifdef _SYSDEP_H */
