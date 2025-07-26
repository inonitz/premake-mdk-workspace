#include "util2/C/marker4.h"


#if UTIL2_MARKER_FLAG_DEFINE_IMPLEMENTATION == 1
#   if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
#       include <time.h>
#   else
#       include "util2/C/compiler_warning.h"
#       pragma message WARN("Cannot use <time.h> - No support available for it")
#   endif
#   include "util2/C/macro.h"
#   include <stdatomic.h>
#   include <stdio.h>
#   include <stdlib.h>
#   include <stdarg.h>
#   include <pthread.h>
#if defined __clang__ && defined _WIN32 && defined __MINGW64__
#   include <pthread_time.h>
#endif


#define __UTIL2_GENERIC_FORMAT_BUFFER_MAX_SIZE 32768
typedef struct generic_format_char_buffer 
{
    char mem[__UTIL2_GENERIC_FORMAT_BUFFER_MAX_SIZE];
} util2_format_buffer;


/*
    [NOTE]:
    static  objects - PTHREAD_MUTEX_INITIALIZER
    runtime objects - pthread_mutex_init
*/
static alignsz(64) pthread_mutex_t       s_write_lock = PTHREAD_MUTEX_INITIALIZER;
static alignsz(64) atomic_uint_least64_t s_markflag;
static FILE*                             s_logbuffer;


__force_inline static inline void __begin_exclusion()
{
    const struct timespec sleep_request = (struct timespec){
        .tv_sec = 0,
        .tv_nsec = 5000000 /* 5 milliseconds */
    };
    while( pthread_mutex_trylock(&s_write_lock) != 0)
        nanosleep(&sleep_request, NULL);

    return;
}

__force_inline static inline void __end_exclusion()
{
    pthread_mutex_unlock(&s_write_lock);
    return;
}

static void __util2_internal_printfmt_va_list(
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
        fputs("\n[marker3.cpp] => __util2_internal_printfmt_va_list() __VA_ARGS__ too large\n", s_logbuffer);
        invalid_state = BOOL_TRUE;
    }
    if (!invalid_state) { /* on success iterate-over/use the given arg_list inside vsnprintf, va_end() will be called outside the function */
        done = vsnprintf(fmtbuf.mem, size, format, arg_list);
    }


    if (invalid_state || done < 0) {
        fputs("\n[marker3.cpp] => __util2_internal_printfmt_va_list() Couldn't format __VA_ARGS__\n", s_logbuffer);
        fmtbuf.mem[0] = '\0'; /* if fputs encounters an eol it'll stop */
    }
    fputs(fmtbuf.mem, s_logbuffer);
    return;
}


__force_inline static inline void __util2_internal_printstr(const char* str) {
    fputs(str, s_logbuffer);
    return;
}

/* variable argument-list can't be inlined */
static void __util2_internal_printfmt(const char* format, ...) 
{
    va_list arg_list;


    va_start(arg_list, format);
    __util2_internal_printfmt_va_list(format, arg_list);
    va_end(arg_list);
    return;
}




static void __util2_marker_flag_destroy_state()
{
#if UTIL2_MARKER_FLAG_LOG_TO_FILE == 1
    fclose(s_logbuffer);
#endif
    pthread_mutex_destroy(&s_write_lock);
    return;
}

__force_inline static inline void __util2_marker_flag_create_state()
{
    s_logbuffer = (UTIL2_MARKER_FLAG_LOG_TO_FILE == 1) ? fopen("__marker_log.txt", "w") : stdout;
    atexit(&__util2_marker_flag_destroy_state);
    return;
}


void util2_marker_flag(
    const char*   file_macro, 
    unsigned long line_macro, 
    const char*   formatstr, 
    ...
) {
    __begin_exclusion();
    if(unlikely( atomic_load(&s_markflag) == 0 )) 
        __util2_marker_flag_create_state();

    __util2_internal_printfmt("[%llu] %s:%u", atomic_load(&s_markflag), file_macro, line_macro);
    __util2_internal_printstr(" [ADDITIONAL_INFO] ");
    va_list fmt_list;
    va_start(fmt_list, formatstr);
    __util2_internal_printfmt_va_list(formatstr, fmt_list);
    va_end(fmt_list);


#if UTIL2_MARKER_FLAG_INCLUDE_NEWLINE_AT_END == 1
    __util2_internal_printstr("\n");
#endif
    atomic_fetch_add_explicit(&s_markflag, 1, memory_order_relaxed);
    __end_exclusion();
    return;
}


#endif /* UTIL2_MARKER_FLAG_DEFINE_IMPLEMENTATION */