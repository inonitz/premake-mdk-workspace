#ifndef __UTIL2_STRING_AND_ASSOCIATES__
#define __UTIL2_STRING_AND_ASSOCIATES__
#include "util2/C/string.h"
#include "util2/type_trait.hpp"


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
/*
#define UTIL_CONDITIONAL_SET(var, val, cond) \
	var *= !boolean(cond); \
	var += boolean(cond) * (val); \

inline uintptr_t __outv = 0;
#define UTIL_CONDITIONAL_SET_PTR(ptr, ptr_val, cond) \
	__outv = __rcast(uintptr_t, ptr); \
	__outv *= !boolean(cond); \
	__outv += boolean(cond) * __rcast(uintptr_t, ptr_val); \
	ptr = __rcast(decltype(ptr), __outv); \
*/


#define UTIL2_SET_BIT_AT(to_set, bit_index, bool_val) \
    to_set &= ~(1 << bit_index); \
    to_set |= ( __scast(  decltype( sizeof(to_set) ), bool_val  ) << bit_index); \


/* 
	[NOTE]:
	no need to use UTIL2_API:
	* all functions are inline (ODR) in a header file,
	* using dllimport/dllexport on a function 'inline void foo()' will "create 2 versions":
		1. the 'inline' version - will be optimized away in the dll itself, 
			you won't be able to find its definition in the symbol table 
		2. the non-inline version - a copy, for users of the dll/shared-lib, this will have a function body
			and will also not be inlined, ofcourse, since it needs to have a defined pointer
	* read more: 
		https://devblogs.microsoft.com/oldnewthing/20140109-00/?p=2123
		https://stackoverflow.com/questions/8876279/c-inline-functions-with-dllimport-dllexport?noredirect=1&lq=1
*/
namespace util2 {


__force_inline inline u64 readtsc() { return util2_readtsc(); } 


__force_inline inline u64 __count_trailing_zeros(u64 ll)
{
#if defined(__cplusplus) && __cplusplus > 202002L
#include <bit>
return std::countr_zero(ll);

#else
return util2_count_trailing_zeros(ll);

#endif
}



template<
	typename T
> constexpr T round2(T v)
{
	static_assert(util2::type_trait::__is_integral_type<T>::value, 
		"Value must be an Integral Type! (Value v belongs to group N [0 -> +inf]\n"
	);
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	++v;
	return v;
}

template<
	typename T
> constexpr T roundN(T powof2, T v)
{
	static_assert(type_trait::__is_integral_type<T>::value, 
		"Value must be an Integral Type! (Value v belongs to group N [0 -> +inf]\n"
	);

	const auto rem = v & ( powof2 - 1);
	return (v - rem) + boolean(rem) * powof2; 
}


template<
	typename T
> 
__force_inline inline void memset(
	T*  ptr, 
	u64 count, 
	T   val = T()
) {
	for(u64 i = 0; i < count; ++i) {
		*ptr = val;
		++ptr;
	}
	return;
}


template<
	typename T
> 
__force_inline inline void memcpy(
	T* 		 pdest, 
	T const* psrc, 
	u64 	 count
) {
	// while(count) {
	// 	*pdest = *psrc;
	// 	++pdest;
	// 	++psrc;
	// 	--count;
	// }
	for(; --count; ) {
		*pdest = *psrc;
		++psrc;
		++pdest;
	}
	return;
}


template<> 
__force_inline inline void memcpy<void>(
	void* 		pdest, 
	void const* psrc, 
	u64 		bytes
) {
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


} /* namespace util2 */


#endif /* __UTIL2_STRING_AND_ASSOCIATES__ */