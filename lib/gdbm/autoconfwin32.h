/* special autoconf.h fitting the needs of Windows */
#ifdef WIN32
#include <io.h> /* needed in nearly all files */
#endif

#ifdef STATIC_BUILD
# define DLLIMPORT
# define DLLEXPORT
#else
#ifdef WIN32
# define DLLEXPORT __declspec(dllexport)      // dll-creation
# define DLLIMPORT __declspec(dllimport)
#else
# define DLLIMPORT                            // static lib
# define DLLEXPORT
#endif
#endif

#ifdef USE_DLL
# define IMEXPORT DLLIMPORT
#else
# define IMEXPORT DLLEXPORT
#endif

/* Define to empty if platform is not WINDOWS (which has nearly nothing by default) */
#define HAVE_NOTHING 1

/* Define to 1 if you have the `bcopy' function.  */
/* #undef HAVE_BCOPY */

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `flock' function. */
/* #undef HAVE_FLOCK */

/* Define to 1 if you have the `fsync' function. */
/* #undef HAVE_FSYNC */

/* Define to 1 if you have the `ftruncate' function. */
/* #undef HAVE_FTRUNCATE */

/* Define to 1 if you have the `getopt' function. */
#define HAVE_GETOPT 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `dbm' library (-ldbm). */
/* #undef HAVE_LIBDBM */

/* Define to 1 if you have the `ndbm' library (-lndbm). */
/* #undef HAVE_LIBNDBM */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `rename' function.  */
/* #undef HAVE_RENAME */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if `st_blksize' is member of `struct stat'. */
/* #undef HAVE_STRUCT_STAT_ST_BLKSIZE */

/* Define to 1 if your `struct stat' has `st_blksize'. Deprecated, use
   `HAVE_STRUCT_STAT_ST_BLKSIZE' instead. */
/* #undef HAVE_ST_BLKSIZE */

/* Define to 1 if you have the <sys/file.h> header file. */
/* #undef HAVE_SYS_FILE_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
/* #undef HAVE_SYS_STAT_H */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
/* #undef HAVE_UNISTD_H */

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "gdbm"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "gdbm 1.8.3"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "gdbm"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.8.3"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `long' if <sys/types.h> does not define. */
/* #undef off_t */
