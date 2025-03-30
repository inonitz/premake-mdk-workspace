#ifndef __UTIL_VECTOR_MATH_LIBRARY3__
#define __UTIL_VECTOR_MATH_LIBRARY3__
#include "util.hpp"
#include "ifcrash.hpp"
#include <initializer_list>
#include <nmmintrin.h>
#include <array>


namespace util::math2 {


template<typename T, u8 length> class Vector
{
private:
    T __data[util::round2<u8>(length)];
    
public:
	void zero() { 
        for(u32 i = 0; i < __carraysize(__data); ++i) {
            __data[i] = __scast(T, 0x00); 
        }
        return;
    }


	constexpr Vector() { zero(); }
	constexpr Vector(const_ref<T> defaultVal) 
	{
		for(u32 i = 0; i < __carraysize(__data); i += 2)  {
			__data[i    ] = defaultVal;
			__data[i + 1] = defaultVal;
		}
		return;
	}
	constexpr Vector(std::initializer_list<T> const& values) {
		zero();
		u32 limit = (length < values.size()) ? length : values.size();
		memcpy(&__data[0], values.begin(), limit);
		return;
	}


	T& 		 operator[](uint8_t idx) 	   { ifcrash_debug((u32)idx >= length); return __data[idx]; }
	const T& operator[](uint8_t idx) const { ifcrash_debug((u32)idx >= length); return __data[idx]; }

	      T* begin()       { return &__data[0];      }
	      T* end()         { return &__data[length]; }
	const T* begin() const { return &__data[0];      }
	const T* end()   const { return &__data[length]; }
	constexpr size_t bytes()  const { return sizeof(T) * length;  }
	constexpr size_t len()    const { return length;		      }
};


using vec3f = util::math2::Vector<f32, 4>;
using vec4f = util::math2::Vector<f32, 4>;
using vec3u = util::math2::Vector<u32, 4>;
using vec4u = util::math2::Vector<u32, 4>;
using vec3i = util::math2::Vector<i32, 4>;
using vec4i = util::math2::Vector<i32, 4>;


} /* namespace util::math2 */


#endif /* __UTIL_VECTOR_MATH_LIBRARY3__ */
