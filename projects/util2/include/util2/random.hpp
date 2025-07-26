#ifndef __UTIL2_RANDOM_DEFINITION__
#define __UTIL2_RANDOM_DEFINITION__
#include "util2/C/util2_api.h"
#include "util2/C/base_type.h"

namespace util2 {


UTIL2_API u32 random32u();
UTIL2_API i32 random32i();
UTIL2_API f32 random32f();
UTIL2_API f32 randnorm32f();
UTIL2_API u64 random64u();
UTIL2_API i64 random64i();
UTIL2_API f64 random64f();
UTIL2_API f32 randnorm64f();


} /* namespace util2 */

#endif /* __UTIL2_RANDOM_DEFINITION__ */