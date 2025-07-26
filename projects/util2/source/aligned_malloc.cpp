#include "util2/aligned_malloc.hpp"
#include "util2/C/aligned_malloc.h"


namespace util2 {


template<u16 alignment> void* aligned_malloc(u64 bytes)
{
    return util2_aligned_malloc(bytes, alignment);
}


void aligned_free(void* ptr)
{
    util2_aligned_free(ptr);
    return;
}


template UTIL2_API void* aligned_malloc<0x008>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0010>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0018>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0020>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0028>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0030>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0038>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0040>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0048>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0050>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0058>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0060>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0068>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0070>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0078>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0080>(u64 bytes);

template UTIL2_API void* aligned_malloc<0x00c0>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0100>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0140>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0180>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x01c0>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0200>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0400>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x0800>(u64 bytes);
template UTIL2_API void* aligned_malloc<0x1000>(u64 bytes);


} // namespace util2