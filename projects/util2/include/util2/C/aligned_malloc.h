#ifndef __UTIL2_C_DEFINITION_ALIGNED_MEMORY_ALLOCATOR__
#define __UTIL2_C_DEFINITION_ALIGNED_MEMORY_ALLOCATOR__
#include "util2/C/util2_api.h"
#include "util2/C/util2_extern.h"
#include "util2/C/base_type.h"
#include "util2_extern.h"


UTIL2_EXTERNC_DECL_BEGIN


UTIL2_API void* util2_aligned_malloc(u64 bytes, u16 alignment);
UTIL2_API void  util2_aligned_free(void* ptr);


UTIL2_EXTERNC_DECL_END


#endif /* __UTIL2_C_DEFINITION_ALIGNED_MEMORY_ALLOCATOR__ */