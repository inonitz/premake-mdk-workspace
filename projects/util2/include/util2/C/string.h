#ifndef __UTIL2_C_DEFINITION_STRING_AND_ASSOCIATES__
#define __UTIL2_C_DEFINITION_STRING_AND_ASSOCIATES__
#include "util2/C/macro.h"
#include "util2/C/base_type.h"


/* for whatever reason you may need this */
__force_inline inline u64 util2_readtsc() 
{
    u32 lo, hi;
    __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi) : :);
    return ((u64)hi << 32) | lo;
}


__force_inline inline u64 util2_count_trailing_zeros(u64 ll)
{
#if defined(__GNUC__) || defined(__clang__)
	return ll == 0 ? 0 : __builtin_ctzll(ll);

#elif defined(_MSC_VER)
#   include <intrin.h>

	u64 out;
	_BitScanReverse64(&out, ll);
	return out;

#else
	#warning "Unknown Compiler Used! util2_count_trailing_zeros() will fallback to default implementation"
	u64 cpy = ll;
	u8 bit, notfound = 1;
	for (bit = 0; bit < 63 && notfound; ++bit) {
		notfound = !boolean( (cpy >> bit) & 1 );
	}
	return bit;

#endif
}


__force_inline inline u64 util2_round2(u64 v)
{
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	++v;
	return v;
}


__force_inline inline u64 util2_roundN(u64 powof2, u64 v)
{
	const u64 rem = v & ( powof2 - 1);
	return (v - rem) + boolean(rem) * powof2; 
}


__force_inline inline void util2_memset(byte* pdst, u64 byte_count, u8 value)
{
	for(u64 i = 0; i < byte_count; ++i) {
		*pdst = value;
		++pdst;
	}
	return;
}


__force_inline inline void util2_memcpy(void* pdest, void const* psrc, u64 bytes) {
	byte* 		dest = __rcast(byte*,       pdest);
	byte const* src  = __rcast(byte const*, psrc );
	while(bytes) {
		*dest = *src;
		++dest;
		++src;
		--bytes;
	}
	return;
}


/* 
    Assumptions:
    1. alignment_of(pdest) == alignment_of(psrc) 
    2. alignment_of(pdest) >= 8
    3. Don't use this lmao, it doesn't compare to memcpy
*/
/* 
__force_inline inline UTIL2_API void util2_aligned_memcpy(void* pdest, void const* psrc, u64 bytes) {
	byte* 		dest = __rcast(byte      *, __assume_aligned(pdest, 8) );
	byte const* src  = __rcast(byte const*, __assume_aligned(psrc,  8) );
	while(bytes) {
		*dest = *src;
		++dest;
		++src;
		--bytes;
	}
	return;
}
*/


#endif /* __UTIL2_C_DEFINITION_STRING_AND_ASSOCIATES__ */