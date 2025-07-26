#include "util2/random.hpp"
#include "util2/C/macro.h"
#include <random>


template<class Generator = std::mt19937>
struct NumberGenerator
{
    using int_result    = std::uniform_int_distribution<int32_t>::result_type;
    using uint_result   = std::uniform_int_distribution<uint32_t>::result_type;
    using int64_result  = std::uniform_int_distribution<int64_t>::result_type;
    using uint64_result = std::uniform_int_distribution<uint64_t>::result_type;
    using fdist_result  = std::uniform_real_distribution<f32>::result_type;
    using ddist_result  = std::uniform_real_distribution<f64>::result_type;


    u64                                     seed;
    Generator                               generator;
    std::uniform_int_distribution<uint32_t> uintdist;
    std::uniform_int_distribution<int32_t>  intdist;
    std::uniform_int_distribution<uint64_t> luintdist;
    std::uniform_int_distribution<int64_t>  lintdist;
    std::uniform_real_distribution<f32>     floatdist;
    std::uniform_real_distribution<f32>     normfloatdist{0.0F, 1.0F};
    std::uniform_real_distribution<f64>     doubledist;
    std::uniform_real_distribution<f32>     normdoubledist{0.0, 1.0};


    explicit NumberGenerator(u64 initialSeed = 0)
    {
        initialSeed += __scast(std::random_device::result_type, initialSeed == 0) * std::random_device()();
        seed = initialSeed;
        generator.seed(initialSeed);
        return;
    }


    uint_result   randu()   { return uintdist(generator);       }
    int_result    randi()   { return intdist(generator);        }
    fdist_result  randf()   { return floatdist(generator);      }
    ddist_result  randd()   { return doubledist(generator);     }
    fdist_result  normf()   { return normfloatdist(generator);  }
    ddist_result  normd()   { return normdoubledist(generator); }
    uint64_result randu64() { return luintdist(generator);      }
    int64_result  randi64() { return lintdist(generator);       }
};


static NumberGenerator<> default_random{0};


u32 util2::random32u() { return default_random.randu();   }
i32 util2::random32i() { return default_random.randi();   }
f32 util2::random32f() { return default_random.randf();   }
f32 util2::randnorm32f() { return default_random.normf(); }
u64 util2::random64u() { return default_random.randu64(); }
i64 util2::random64i() { return default_random.randi64(); }
f64 util2::random64f() { return default_random.randd();   }
f32 util2::randnorm64f() { return default_random.normd(); }