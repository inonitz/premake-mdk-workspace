#ifndef __UTIL2_C_DEFINITION_HASH_FUNCTION_MURMURHASH64A__
#define __UTIL2_C_DEFINITION_HASH_FUNCTION_MURMURHASH64A__
#include "util2/C/macro.h"
#include "util2/C/base_type.h"


/* Source: https://github.com/hhrhhr/MurmurHash-for-Lua/blob/master/MurmurHash64A.c */
/*
	[NOTE]:
	I do not understand why 'len' is signed??
*/
__force_inline inline u64 util2_murmurhash64a(const void * key, i32 len, u64 seed)
{
	const u64 m = 0xc6a4a7935bd1e995LLU;
	const int r = 47;

	u64 h = seed ^ (len * m), k = 0;

	const u64 * data = (const u64 *)key;
	const u64 * end = (len >> 3) + data;

	while(data != end)
	{
		k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 

		h ^= k;
		h *= m; 
	}

	const u8* data2 = (const u8*)data;
	switch(len & 7)
	{
	case 7: h ^= (u64)(data2[6]) << 48;
	case 6: h ^= (u64)(data2[5]) << 40;
	case 5: h ^= (u64)(data2[4]) << 32;
	case 4: h ^= (u64)(data2[3]) << 24;
	case 3: h ^= (u64)(data2[2]) << 16;
	case 2: h ^= (u64)(data2[1]) << 8;
	case 1: h ^= (u64)(data2[0]);
			h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}


#endif /* __UTIL2_C_DEFINITION_HASH_FUNCTION_MURMURHASH64A__ */