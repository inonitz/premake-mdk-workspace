#ifndef __UTIL_HEADER__
#define __UTIL_HEADER__
#include "util_api.h"
#include "macro.h"
#include "types.hpp"


/*
	[NOTE]: 
		Just use the ternary operator [?], 
		it'll be optimized to a conditional move which is way
		better than this, which is ~5 instructions (atleast on x86)
	if cond:
		var *= false       => var = 0;
		var += true * val  => var = val;
	else:
		var *= true 	   => var = var;
		var += false * val => var += 0;
*/
#define CONDITIONAL_SET(var, val, cond) \
	var *= !boolean(cond); \
	var += boolean(cond) * (val); \

inline uintptr_t __outv = 0;
#define CONDITIONAL_SET_PTR(ptr, ptr_val, cond) \
	__outv = __rcast(uintptr_t, ptr); \
	__outv *= !boolean(cond); \
	__outv += boolean(cond) * __rcast(uintptr_t, ptr_val); \
	ptr = __rcast(decltype(ptr), __outv); \

#define SET_BIT_AT(to_set, bit_index, bool_val) \
    to_set &= ~(1 << bit_index); \
    to_set |= ( __scast(  decltype( sizeof(to_set) ), bool_val  ) << bit_index); \


#define __remove_const(val) const_cast< remove_const<decltype(val)> >(val)


namespace util {


/* 
	for whatever reason you may need this 
*/
__force_inline inline u64 __readtsc() {
    u32 lo, hi;
    __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
    return ((u64)hi << 32) | lo;
}


__force_inline inline u64 __count_trailing_zeros(u64 ll)
{
#if defined(__cplusplus) && __cplusplus > 202002L
#include <bit>
return std::countr_zero(ll);

#elif defined(__GNUC__) || defined(__clang__)
	return ll == 0 ? 0 : __builtin_ctzll(ll);

#elif defined(_MSC_VER)
	u64 out;
	_BitScanReverse64(&out, ll);
	return out;

#else
	#warning "Unknown Compiler Used! __count_trailing_zeros() will fallback to default implementation
	u64 cpy = ll;
	u8 bit, notfound = 1;
	for (bit = 0; bit < 63 && notfound; ++bit) {
		notfound = !boolean( (cpy >> bit) & 1 );
	}
	return bit;
#endif
}



template<typename T> constexpr UTIL_API T round2(T v)
{
	static_assert(type_trait::__is_integral_type<T>::value, 
	"Value must be an Integral Type! (Value v belongs to group N [0 -> +inf]. ");
	
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	++v;
	return v;
}

template<typename T> constexpr UTIL_API T roundN(T powof2, T v)
{
	static_assert(type_trait::__is_integral_type<T>::value, 
		"Value must be an Integral Type! (Value v belongs to group N [0 -> +inf]. "
	);

	const auto rem = v & ( powof2 - 1);
	return (v - rem) + boolean(rem) * powof2; 
}


__force_inline inline void internal_memset(byte* dest, byte* src, u64 src_obj_size, u64 obj_to_set)
{
	for(u64 i = 0; i < obj_to_set; ++i) {
		byte* tmp = src;
		for(u64 j = 0; j < src_obj_size; ++j) {
			*dest = *tmp;
			++tmp;
			++dest;
		}
	}
	return;
}


template<typename T> __force_inline inline UTIL_API void __memset(T* p, u64 count, T val = T())
{
	for(u64 i = 0; i < count; ++i) {
		*p = val;
		++p;
	}
	// internal_memset(
	// 	__scast(byte*, p),
	// 	&val,
	// 	sizeof(T),
	// 	count
	// );
	return;
}


template<typename T> __force_inline inline UTIL_API void __memcpy(T* pdest, T const* psrc, u64 count)
{
	while(count) {
		*pdest = *psrc;
		++pdest;
		++psrc;
		--count;
	}
	return;
}


template<> __force_inline inline UTIL_API void __memcpy<void>(void* pdest, void const* psrc, u64 bytes) {
	byte* 		dest = __rcast(byte *, 		pdest);
	byte const* src  = __rcast(byte const*, psrc );
	while(bytes) {
		*dest = *src;
		++dest;
		++src;
		--bytes;
	}
	return;
}


} // namespace util


#endif