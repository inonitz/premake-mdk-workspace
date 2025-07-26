#include "util2/C/thread_sleep.h"


#if defined(linux) || defined(__linux__)
#   if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
#       include <time.h>
#   else
#       include "util2/C/compiler_warning.h"
#       pragma message WARN("Cannot use <time.h> - No support available for it")
#   endif
#   include <pthread.h>
#   define __UTIL2_DEFINITION_THREAD_SLEEP_USE_POSIX__ 1
#elif defined(_WIN32) || defined(_WIN64)
#   if defined(__MINGW64__)
#       include <pthread.h>
#       include <pthread_time.h>
#       define __UTIL2_DEFINITION_THREAD_SLEEP_USE_POSIX__ 1
#   else
#       include <threads.h>
#       define __UTIL2_DEFINITION_THREAD_SLEEP_USE_POSIX__ 0
#   endif
#endif




void util2_thread_sleep(i32 duration_nanoseconds)
{
    const struct timespec sleep_request = (struct timespec){
        .tv_sec = 0,
        .tv_nsec = duration_nanoseconds
    };


#if __UTIL2_DEFINITION_THREAD_SLEEP_USE_POSIX__ == 1
    nanosleep(&sleep_request, NULL);
#else
    thrd_sleep(&sleep_request, NULL);
#endif


    return;
}


#undef __UTIL2_DEFINITION_THREAD_SLEEP_USE_POSIX__
