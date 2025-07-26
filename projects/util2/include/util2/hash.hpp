#ifndef __UTIL2_HASH_STRUCT__
#define __UTIL2_HASH_STRUCT__
#include "util2/C/util2_api.h"
#include "util2/C/base_type.h"
#include "util2/C/murmur64a.h"
#include "util2/random.hpp"


namespace util2 {


struct UTIL2_API Hash { /* Default Hash Struct Overload for flat_hash */
public:
	Hash() : m_seed{random64u()} {}
	
	u64 operator()(u64 key) { 
		return util2_murmurhash64a(&key, 8, m_seed); 
	}

	void refresh() { 
		// markfmt("hash::refresh() switched to new seed=0x%llX\n", seed);
		m_seed = random64u(); 
		return;
	}


public:
	u64 m_seed;
};


} /* namespace util2 */


#endif /* __UTIL2_HASH_STRUCT__ */