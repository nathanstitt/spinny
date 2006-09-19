// Public domain, by Christopher Diggins
// http://www.cdiggins.com

#ifndef _HASHER_HPP_
#define _HASHER_HPP_ 1
#include <string>

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned long u4;

#define get16bits(d) (*((const u2*)(d)))

// Paul Hseih's hash algorithm
// from http://www.azillionmonkeys.com/qed/hash.html

u4 hseih_hash(const char* key, u4 len) 
{  
	u4 hash = len;
	u4 tmp;
	int rem;
	if (len <= 0 || key == NULL) return 0;
	rem = len & 3;
	len >>= 2;
	
	// Main loop 
	for (;len > 0; --len) {
		hash  += get16bits (key);
		tmp    = (get16bits (key+2) << 11) ^ hash;
		hash   = (hash << 16) ^ tmp;
		key  += 4;
		hash  += hash >> 11;
	}

	// Handle end cases 
	switch (rem) {
	case 3: hash += get16bits (key);
		hash ^= hash << 16;
		hash ^= key[2] << 18;
		hash += hash >> 11;
		break;
	case 2: hash += get16bits(key);
		hash ^= hash << 11;
		hash += hash >> 17;
		break;
	case 1: hash += *key;
		hash ^= hash << 10;
		hash += hash >> 1;
	}

	// Force "avalanching" of final bits 
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 2;
	hash += hash >> 15;
	hash ^= hash << 10;
  
	return hash;
}

#undef get16bits

namespace stbd {

template<typename T>
struct hash {
  u4 operator()(const T& x) const { 

    return hseih_hash(reinterpret_cast<const char*>(&x), sizeof(T));
  }  
};

template<>
struct hash<const char*> {
  u4 operator()(const char* x) const { 
    return hseih_hash(x, (u4)strlen(x));
  }  
};

template<>
struct hash<const std::string> {
	u4 operator()(const std::string &x) const { 
		return hseih_hash( x.c_str(), x.size() );
	}
};

} // namespace std

#endif // _HASHER_HPP_
