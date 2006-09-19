#ifndef _CONFIG_H_
#define _CONFIG_H_ 1

#define GOOGLE_NAMESPACE google
#undef HASH_NAMESPACE

#cmakedefine HAVE_U_INT16_T
#cmakedefine HAVE___UINT16
#cmakedefine HAVE_LONG_LONG
#cmakedefine HAVE_SYS_TYPES_H
#cmakedefine HAVE_STDINT_H
#cmakedefine HAVE_INTTYPES_H
#cmakedefine HAVE_MEMCPY
#cmakedefine HAVE_STDINT_H
#define STL_NAMESPACE std

#define UNDERSTANDS_ITERATOR_TAGS 1
#define UNDERSTANDS_TYPE_TRAITS 1

#undef HAVE_TYPE_TRAITS
#define _END_GOOGLE_NAMESPACE_ }
#define _START_GOOGLE_NAMESPACE_ namespace google {


#endif // _CONFIG_H_