/* src/config.h.  Generated automatically by configure.  */
/* src/config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if inline functions a la GCC are available.  */
#define HAVE_INLINE 1

/* Define if function attributes a la GCC 2.5 and higher are available.  */
/* #define HAVE_GNUC25_ATTRIB 1 */

/* Define if constant functions a la GCC 2.5 and higher are available.  */
/* #define HAVE_GNUC25_CONST 1 */

/* Define if nonreturning functions a la GCC 2.5 and higher are available.  */
/* #define HAVE_GNUC25_NORETURN 1 */

/* Define if printf-format argument lists a la GCC are available.  */
/* #define HAVE_GNUC25_PRINTFFORMAT 1 */

/* Define if we want to include rpc/types.h.  Crap BSDs put INADDR_LOOPBACK there. */
/* #undef HAVEUSE_RPCTYPES_H */

/* Define if you have the poll function.  */
/* #define HAVE_POLL 1 */

/* Define if you have the <sys/select.h> header file.  */
#define HAVE_SYS_SELECT_H 1

/* Define if you have the nsl library (-lnsl).  */
/* #undef HAVE_LIBNSL */

/* Define if you have the socket library (-lsocket).  */
/* #undef HAVE_LIBSOCKET */

/* Use the definitions: */

#ifndef HAVE_INLINE
#define inline
#endif

#ifdef HAVE_POLL
#include <sys/poll.h>
#else
/* kludge it up */
struct pollfd { int fd; short events; short revents; };
#define POLLIN  1
#define POLLPRI 2
#define POLLOUT 4
#endif

#ifdef HAVEUSE_RPCTYPES_H
#include <rpc/types.h>
#endif

#if !defined(WIN32) && defined(HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif
