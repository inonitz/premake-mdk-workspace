#ifndef __UTIL2_ALIGNED_MEMORY_ALLOCATOR__
#define __UTIL2_ALIGNED_MEMORY_ALLOCATOR__
#include "C/util2_api.h"
#include "C/base_type.h"


namespace util2 {


template<u16 alignment> UTIL2_API auto aligned_malloc(u64 bytes) -> void*;
                        UTIL2_API void aligned_free(void* ptr);


} /* namespace util2 */


#endif /* __UTIL2_ALIGNED_MEMORY_ALLOCATOR__ */