/* cs_config.h.in.  Generated from configure.in by autoheader.  */
/*
 * Copyright 2001-2004 Brandon Long
 * All Rights Reserved.
 *
 * ClearSilver Templating System
 *
 * This code is made available under the terms of the ClearSilver License.
 * http://www.clearsilver.net/license.hdf
 *
 */

/*
 * config file
 */

#ifndef __CS_CONFIG_H_
#define __CS_CONFIG_H_ 1


/* Enable support for HTML Compression (still must be enabled at run time) */
#cmakedefine HTML_COMPRESSION

/* Enable support for X Remote CGI Debugging */
#cmakedefine ENABLE_REMOTE_DEBUG

/********* SYSTEM CONFIG ***************************************************/
/* autoconf/configure should figure all of these out for you */

/* Does your system have the snprintf() call? */
#cmakedefine HAVE_SNPRINTF

/* Does your system have the vsnprintf() call? */
#cmakedefine HAVE_VSNPRINTF

/* Does your system have the strtok_r() call? */
#cmakedefine HAVE_STRTOK_R

/* Does your system have the localtime_r() call? */
#cmakedefine HAVE_LOCALTIME_R

/* Does your system have the gmtime_r() call? */
#cmakedefine HAVE_GMTIME_R

/* Does your system have the mkstemp() call? */
#cmakedefine HAVE_MKSTEMP

/* Does your system have regex.h */
#cmakedefine HAVE_REGEX

/* Does your system have pthreads? */
#cmakedefine HAVE_PTHREADS

/* Does your system have lockf ? */
#cmakedefine HAVE_LOCKF

/* Enable support for gettext message translation */
#cmakedefine ENABLE_GETTEXT


/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_DIRENT_H

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#cmakedefine HAVE_DOPRNT

/* Define to 1 if you have the `drand48' function. */
#cmakedefine HAVE_DRAND48

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H

/* Define to 1 if you have the `gettimeofday' function. */
#cmakedefine HAVE_GETTIMEOFDAY

/* Define to 1 if you have the `gmtime_r' function. */
#cmakedefine HAVE_GMTIME_R

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H

/* Define to 1 if you have the <limits.h> header file. */
#cmakedefine HAVE_LIMITS_H

/* Define to 1 if you have the `localtime_r' function. */
#cmakedefine HAVE_LOCALTIME_R

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H

/* Define to 1 if you have the `mktime' function. */
#cmakedefine HAVE_MKTIME

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#cmakedefine HAVE_NDIR_H

/* Define to 1 if you have the `putenv' function. */
#cmakedefine HAVE_PUTENV

/* Define to 1 if you have the `rand' function. */
#cmakedefine HAVE_RAND

/* Define to 1 if you have the `random' function. */
#cmakedefine HAVE_RANDOM

/* Define to 1 if you have the <stdarg.h> header file. */
#cmakedefine HAVE_STDARG_H

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H

/* Define to 1 if you have the `strerror' function. */
#cmakedefine HAVE_STRERROR

/* Define to 1 if you have the `strftime' function. */
#cmakedefine HAVE_STRFTIME

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H

/* Define to 1 if you have the `strspn' function. */
#cmakedefine HAVE_STRSPN

/* Define to 1 if you have the `strtod' function. */
#cmakedefine HAVE_STRTOD

/* Define to 1 if you have the `strtok_r' function. */
#cmakedefine HAVE_STRTOK_R

/* Define to 1 if you have the `strtol' function. */
#cmakedefine HAVE_STRTOL

/* Define to 1 if you have the `strtoul' function. */
#cmakedefine HAVE_STRTOUL

/* Define to 1 if `tm_zone' is member of `struct tm'. */
#define HAVE_STRUCT_TM_TM_ZONE 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_DIR_H

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#cmakedefine HAVE_SYS_IOCTL_H

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_NDIR_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#cmakedefine HAVE_SYS_WAIT_H

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#cmakedefine HAVE_TM_ZONE

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
#cmakedefine HAVE_TZNAME

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H

/* Define to 1 if you have the <varargs.h> header file. */
#cmakedefine HAVE_VARARGS_H

/* Define to 1 if you have the `vprintf' function. */
#cmakedefine HAVE_VPRINTF

/* Define to 1 if you have the `wait3' system call. Deprecated, you should no
   longer depend upon `wait3'. */
#cmakedefine HAVE_WAIT3

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "clearsilver"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "clearsilver 0.10.3"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "clearsilver-0.10.3.tar.gz"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.10.3"

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#define TM_IN_SYS_TIME 1

/* Define to empty if `const' does not conform to ANSI C. */
#define const const

/* Define to `int' if <sys/types.h> does not define. */
/* define mode_t int */

/* Define to `long' if <sys/types.h> does not define. */
/* #define off_t long */

/* Define to `int' if <sys/types.h> does not define. */
/* #define pid_t int */

/* Define to `unsigned' if <sys/types.h> does not define. */
/* define size_t unsigned int */

#endif /* __CS_CONFIG_H_ */
