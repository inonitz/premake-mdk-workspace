#include "util2/C/print.h"


#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
#   include <time.h>
#else
#   include "util2/C/compiler_warning.h"
#   pragma message WARN("Cannot use <time.h> - No support available for it")
#endif
#include "util2/C/macro.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#if defined __clang__ && defined _WIN32
#   include <pthread_time.h>
#endif


#define __UTIL2_GENERIC_FORMAT_BUFFER_MAX_SIZE 32768
typedef struct __generic_format_char_buffer 
{
    char mem[__UTIL2_GENERIC_FORMAT_BUFFER_MAX_SIZE];
} util2_format_buffer;


/*
    [NOTE]:
    static  objects - PTHREAD_MUTEX_INITIALIZER
    runtime objects - pthread_mutex_init
*/
static alignsz(64) pthread_mutex_t __write_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned char               __called_once = 0;
static FILE*                       __log_buffer;


__force_inline static inline void __util2_begin_exclusion()
{
    const struct timespec sleep_request = (struct timespec){
        .tv_sec = 0,
        .tv_nsec = 5000000 /* 5 milliseconds */
    };
    while( pthread_mutex_trylock(&__write_lock) != 0)
        nanosleep(&sleep_request, NULL);

    return;
}

__force_inline static inline void __util2_end_exclusion()
{
    pthread_mutex_unlock(&__write_lock);
    return;
}


void util2_va_fprintf(
    FILE*       write_into,
    const char* format, 
    va_list     arg_list
) {
    static util2_format_buffer fmtbuf;
    va_list arglistcopy;
    int size = 0;
    int done = 1;
    unsigned char invalid_state = BOOL_FALSE;


    fmtbuf.mem[__UTIL2_GENERIC_FORMAT_BUFFER_MAX_SIZE - 1] = '\0'; /* incase of overflow */

    /* use arglistcopy to calculate the total bytes needed */
    va_copy(arglistcopy, arg_list);
    size = 1 + vsnprintf(NULL, 0, format, arglistcopy);
    va_end(arglistcopy);

    if (size > __UTIL2_GENERIC_FORMAT_BUFFER_MAX_SIZE) {
        fputs("\n[print.c] => util2_va_fprintf() __VA_ARGS__ too large\n", write_into);
        invalid_state = BOOL_TRUE;
    }
    if (!invalid_state) { /* on success iterate-over/use the given arg_list inside vsnprintf, va_end() will be called outside the function */
        done = vsnprintf(fmtbuf.mem, size, format, arg_list);
    }


    if (invalid_state || done < 0) {
        fputs("\n[print.c] => util2_va_fprintf() Couldn't format __VA_ARGS__\n", write_into);
        fmtbuf.mem[0] = '\0'; /* if fputs encounters an eol it'll stop */
    }
    fputs(fmtbuf.mem, write_into);
    return;
}




__force_inline static inline void __util2_init_logbuf()
{
    __log_buffer = stdout;
    return;
}


void util2_printf(const char* format, ...)
{
    __util2_begin_exclusion();
    
    /* Initialize output buffer on first call */
    if(unlikely(!boolean(__called_once))) {
        __util2_init_logbuf();
        ++__called_once;
    }

    va_list arg_list;
    va_start(arg_list, format);
    util2_va_fprintf(__log_buffer, format, arg_list);
    va_end(arg_list);
    __util2_end_exclusion();
    return;
}

void util2_fprintf(FILE* write_to, const char* format, ...)
{
    __util2_begin_exclusion();
    va_list arg_list;
    va_start(arg_list, format);
    util2_va_fprintf(write_to, format, arg_list);
    va_end(arg_list);
    __util2_end_exclusion();
    return;
}
