#ifndef __UTIL_MARKER_FLAG_MACRO__
#define __UTIL_MARKER_FLAG_MACRO__
// #define MARKER_FLAG_LOG_TO_FILE 1
// #define MARKER_FLAG_KEEP_RELEASE 1
#define MARKER_FLAG_INCLUDE_NEWLINE_AT_END 1

#ifndef MARKER_FLAG_KEEP_RELEASE
    #define MARKER_FLAG_KEEP_RELEASE 0
#endif
#ifndef MARKER_FLAG_DEFINE_IMPLEMENTATION
    #define MARKER_FLAG_DEFINE_IMPLEMENTATION 0
#endif
#ifndef MARKER_FLAG_EXTERNAL_DEFINITION
    #define MARKER_FLAG_EXTERNAL_DEFINITION 0
#endif
#ifndef MARKER_FLAG_LOG_TO_FILE
    #define MARKER_FLAG_LOG_TO_FILE 0
#endif
#ifndef MARKER_FLAG_INCLUDE_NEWLINE_AT_END
    #define MARKER_FLAG_INCLUDE_NEWLINE_AT_END 0
#endif
#if defined(_DEBUG) || defined(DEBUG) || (MARKER_FLAG_KEEP_RELEASE == 1)
    #undef MARKER_FLAG_DEFINE_IMPLEMENTATION
    #undef MARKER_FLAG_EXTERNAL_DEFINITION
    #define MARKER_FLAG_DEFINE_IMPLEMENTATION 1 
    #define MARKER_FLAG_EXTERNAL_DEFINITION 1
#endif


/*
    This needs to be de-coupled into 2 different headers:
	A. one for basic printing
	B. one for marker functions specifically
	Ex. debug_message() & debug_messagefmt() in base.hpp
*/


#if MARKER_FLAG_DEFINE_IMPLEMENTATION == 1
#include "util_api.h"
#include "macro.h"


#if MARKER_FLAG_LOG_TO_FILE == 1
namespace marker {
    UTIL_API void marker_flag_close_logfile_handle();
}
#endif


namespace detail::marker {
    UTIL_API void __begin_exclusion();
    UTIL_API void __end_exclusion();
    UTIL_API unsigned long long __load_atomic_counter();
    UTIL_API void __increment_atomic_counter();
    UTIL_API void __common_print_function_nofmt(const char* str);
    UTIL_API void __common_print_function_fmt(const char* format, ...);
} // namespace detail::marker


DISABLE_WARNING_PUSH
DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
    #define mark_generic(str_or_fmt, append_name_for_str_or_fmt, str, ...) \
        { \
            detail::marker::__begin_exclusion(); \
            detail::marker::__common_print_function_fmt("[%llu] %s:%u", detail::marker::__load_atomic_counter(),  __FILE__, __LINE__); \
            detail::marker::__increment_atomic_counter(); \
            if constexpr (str_or_fmt) { \
                detail::marker::__common_print_function_nofmt(" [ADDITIONAL_INFO] "); \
                detail::marker::__common_print_function##append_name_for_str_or_fmt(str, ##__VA_ARGS__); \
            } \
            if constexpr (MARKER_FLAG_INCLUDE_NEWLINE_AT_END) { \
                detail::marker::__common_print_function_nofmt("\n"); \
            } \
            detail::marker::__end_exclusion(); \
        }
DISABLE_WARNING_POP


#endif


#if MARKER_FLAG_EXTERNAL_DEFINITION == 0
    #define mark()            
    #define markstr(str)      
    #define markfmt(str, ...) 
#elif MARKER_FLAG_EXTERNAL_DEFINITION == 1
    #define mark()            mark_generic(false, _nofmt,   nullptr     )
    #define markstr(str)      mark_generic(true,  _nofmt,   str         )
    #define markfmt(str, ...) mark_generic(true,  _fmt, str, __VA_ARGS__)
#endif


#endif