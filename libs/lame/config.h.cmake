
/* debug define */
#cmakedefine ABORTFP

/* enable VBR bitrate histogram */
#define BRHIST 1

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
#cmakedefine CRAY_STACKSEG_END

/* Define to 1 if using `alloca.c'. */
#cmakedefine C_ALLOCA

/* alot of debug output */
#cmakedefine DEBUG

/* allow to compute a more accurate replaygain value */
#define DECODE_ON_THE_FLY 1

/* double is faster than float on Alpha */
#cmakedefine FLOAT

/* float instead of double */
#cmakedefine FLOAT8

/* Define to 1 if you have `alloca', as a function or macro. */
#cmakedefine HAVE_ALLOCA 1

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#cmakedefine HAVE_ALLOCA_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1


/* Define to 1 if you have the <errno.h> header file. */
#cmakedefine HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H 1

/* Define to 1 if you have the `gettimeofday' function. */
#cmakedefine HAVE_GETTIMEOFDAY 1

/* add ieee754_float32_t type */
/* #undef HAVE_IEEE754_FLOAT32_T */
#ifndef HAVE_IEEE754_FLOAT32_T
	typedef float ieee754_float32_t;
#endif

/* add ieee754_float64_t type */
/* #undef HAVE_IEEE754_FLOAT64_T */
#ifndef HAVE_IEEE754_FLOAT64_T
	typedef double ieee754_float64_t;
#endif

/* system has 80 bit floats */
#define HAVE_IEEE854_FLOAT80 1

/* add ieee854_float80_t type */
/* #undef HAVE_IEEE854_FLOAT80_T */
#ifndef HAVE_IEEE854_FLOAT80_T
	typedef long double ieee854_float80_t;
#endif


/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the <limits.h> header file. */
#cmakedefine HAVE_LIMITS_H 1

/* Define to 1 if you have the <linux/soundcard.h> header file. */
#cmakedefine HAVE_LINUX_SOUNDCARD_H 1

/* Define to 1 if long double works and has more range or precision than
   double. */
#define HAVE_LONG_DOUBLE 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* build with mpglib support */
#undef HAVE_MPGLIB

/* have nasm */
/* #undef HAVE_NASM */

/* Define to 1 if you have the <ncurses/termcap.h> header file. */
/* #undef HAVE_NCURSES_TERMCAP_H */

/* Define to 1 if you have the `socket' function. */
#cmakedefine HAVE_SOCKET 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the `strtol' function. */
#cmakedefine HAVE_STRTOL 1

/* Define to 1 if you have the <sys/soundcard.h> header file. */
#cmakedefine HAVE_SYS_SOUNDCARD_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* add uint16_t type */
#define HAVE_UINT16_T 1
#ifndef HAVE_UINT16_T
	typedef unsigned short uint16_t;
#endif

/* add uint32_t type */
#define HAVE_UINT32_T 1
#ifndef HAVE_UINT32_T
#define A_UINT32_T unsigned int
	typedef A_UINT32_T uint32_t;
#endif

/* add uint64_t type */
#define HAVE_UINT64_T 1
#ifndef HAVE_UINT64_T
#define A_UINT64_T unsigned long long
	typedef A_UINT64_T uint64_t;
#endif

/* add uint8_t type */
#define HAVE_UINT8_T 1
#ifndef HAVE_UINT8_T
	typedef unsigned char uint8_t;
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* winsock */
/* #undef HAVE_WINSOCKET */

/* requested by Frank, seems to be temporary needed for a smooth transition */
#define LAME_LIBRARY_BUILD 1

/* build with libsndfile support */
/* #undef LIBSNDFILE */

/* use MMX version of choose_table */
/* #undef MMX_choose_table */

/* no debug build */
#define NDEBUG 1

/* build without hooks for analyzer */
/* #undef NOANALYSIS */

/* Name of package */
#define PACKAGE "lame"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "l"

/* Define to the full name of this package. */
#define PACKAGE_NAME "lame"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "lame 3.97"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "lame"

/* Define to the version of this package. */
#define PACKAGE_VERSION "3.97"

/* Define to 1 if the C compiler supports function prototypes. */
#define PROTOTYPES 1

/* The size of a `double', as computed by sizeof. */
#define SIZEOF_DOUBLE 8

/* The size of a `float', as computed by sizeof. */
#define SIZEOF_FLOAT 4

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of a `long double', as computed by sizeof. */
/* #undef SIZEOF_LONG_DOUBLE */

/* The size of a `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of a `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* The size of a `unsigned int', as computed by sizeof. */
#define SIZEOF_UNSIGNED_INT 4

/* The size of a `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG 4

/* The size of a `unsigned long long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG_LONG 8

/* The size of a `unsigned short', as computed by sizeof. */
#define SIZEOF_UNSIGNED_SHORT 2

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* IEEE754 compatible machine */
#define TAKEHIRO_IEEE754_HACK 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* faster log implementation with less but enough precission */
#define USE_FAST_LOG 1

/* build with layer 1 decoding */
/* #undef USE_LAYER_1 */

/* build with layer 2 decoding */
#define USE_LAYER_2 1

/* Version number of package */
#define VERSION "3.97"

/* Define if using the dmalloc debugging malloc package */
/* #undef WITH_DMALLOC */

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Define to 1 if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* # undef _ALL_SOURCE */
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* we're on DEC Alpha */
/* #undef __DECALPHA__ */

/* work around a glibc bug */
/* #undef __NO_MATH_INLINES */

/* Define like PROTOTYPES; this can be used by system headers. */
#define __PROTOTYPES 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

