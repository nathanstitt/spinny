
#ifndef _HASH_MAP_H_
#define _HASH_MAP_H_


#ifdef __GNUC__
    #include <ext/hash_map>
    namespace stdext = ::__gnu_cxx;       // GCC 3.1 and later
#else      // ...  there are other compilers, right?
    namespace stdext = std;
#endif


#include "stlext/hasher.hpp"

#endif // _HASH_MAP_
