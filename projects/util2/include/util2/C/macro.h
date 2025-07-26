#ifndef __UTIL2_BASE_MACRO_DEFINITIONS__
#define __UTIL2_BASE_MACRO_DEFINITIONS__
#include "compiler_warning.h"


/* All credit goes to: https://www.fluentcpp.com/2019/08/30/how-to-disable-a-warning-in-cpp/ */
#if defined( __GNUC__ ) || defined( __MINGW__ ) || defined ( __clang__ )
#   ifndef DO_PRAGMA
#      define DO_PRAGMA(X) _Pragma(#X)
#   endif /* !defined DO_PRAGMA */
#   ifndef DISABLE_WARNING_PUSH
#      define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
#   endif /* !defined DISABLE_WARNING_PUSH */
#   ifndef DISABLE_WARNING_POP
#      define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop) 
#   endif /* !defined DISABLE_WARNING_POP */
#   ifndef DISABLE_WARNING
#      define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)
#   endif /* !defined DISABLE_WARNING */
#   ifndef DISABLE_WARNING_UNUSED_PARAMETER
#      define DISABLE_WARNING_UNUSED_PARAMETER              DISABLE_WARNING(-Wunused-parameter)
#   endif /* !defined DISABLE_WARNING_UNUSED_PARAMETER */
#   ifndef DISABLE_WARNING_UNUSED_FUNCTION
#      define DISABLE_WARNING_UNUSED_FUNCTION               DISABLE_WARNING(-Wunused-function)
#   endif /* !defined DISABLE_WARNING_UNUSED_FUNCTION */
#   ifndef DISABLE_WARNING_NESTED_ANON_TYPES
#      define DISABLE_WARNING_NESTED_ANON_TYPES             DISABLE_WARNING(-Wnested-anon-types)
#   endif /* !defined DISABLE_WARNING_NESTED_ANON_TYPES */
#   ifndef DISABLE_WARNING_GNU_ANON_STRUCT
#      define DISABLE_WARNING_GNU_ANON_STRUCT               DISABLE_WARNING(-Wgnu-anonymous-struct)
#   endif /* !defined DISABLE_WARNING_GNU_ANON_STRUCT */
#   ifndef DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
#      define DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS  DISABLE_WARNING(-Wgnu-zero-variadic-macro-arguments)
#   endif /* !defined DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS */
#   ifndef DISABLE_WARNING_PEDANTIC
#      define DISABLE_WARNING_PEDANTIC                      DISABLE_WARNING(-Wpedantic)
#   endif /* !defined DISABLE_WARNING_PEDANTIC */
#   ifndef DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
#      define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER DISABLE_WARNING_UNUSED_PARAMETER
#   endif /* !defined DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER */
#   ifndef DISABLE_WARNING_UNREFERENCED_FUNCTION
#      define DISABLE_WARNING_UNREFERENCED_FUNCTION         DISABLE_WARNING_UNUSED_FUNCTION
#   endif /* !defined DISABLE_WARNING_UNREFERENCED_FUNCTION */
#   ifndef DISABLE_WARNING_DEPRECATED_FUNCTION
#      define DISABLE_WARNING_DEPRECATED_FUNCTION           DISABLE_WARNING(-Wdeprecated-declarations)
#   endif /* !defined DISABLE_WARNING_DEPRECATED_FUNCTION */


#   ifdef likely
#       pragma message WARN("Macro 'likely' already defined")
#   else
#       if defined __cplusplus && __cplusplus > 202002L /* C++20 Has [[likely]] */
#           define likely(cond) [[likely]]
#       else
#           define likely(cond) __builtin_expect( boolean(cond), 1)
#       endif
#   endif /* ifdef likely */
#   ifdef unlikely
#       pragma message WARN("Macro 'likely' already defined")
#   else
#       if defined __cplusplus && __cplusplus > 202002L /* C++20 Has [[unlikely]] */
#           define unlikely(cond) [[unlikely]]
#       else
#           define unlikely(cond) __builtin_expect( boolean(cond), 0)
#       endif
#   endif /* ifdef unlikely */


#   if defined __cplusplus && __cplusplus > 201703L /* C++17 gives us [[maybe_unused]] */
#      ifndef __util_attribute_unused
#          define __util_attribute_unused [[maybe_unused]]
#      endif
#   else
#      ifndef __util_attribute_unused
#          define __util_attribute_unused __attribute__((unused))
#      endif
#   endif

#   if defined __unused /* more appropriate for functions	*/
#       pragma message WARN("Macro __notused Already defined")
#   else
#       define __unused __util_attribute_unused
#   endif
#   if defined notused /* more appropriate for function parameters */
#       pragma message WARN("Macro __notused Already defined")
#   else
#       define notused __util_attribute_unused
#   endif

#   if defined(_DEBUG) || defined(DEBUG) /* release mode (Usually) omits a lot of code => gives many unused_param errors */
#       define __release_unused
#   else
#       define __release_unused notused
#   endif

#   if defined __hot
#       pragma message WARN("Macro __hot already defined")
#   else
#       define __hot __attribute__((hot))
#   endif /* defined __hot */
#   if defined __cold
#       pragma message WARN("Macro __cold already defined")
#   else
#       define __cold __attribute__((cold))
#   endif /* defined __cold */
#   if defined pack
#       pragma message WARN("Macro pack already defined")
#   else
#       define pack __attribute__((packed))
#   endif /* defined pack */
#   if defined alignpk
#       pragma message WARN("Macro alignpk already defined")
#   else
#       define alignpk(size) __attribute__((packed, aligned(size)))
#   endif /* defined alignpk */
#   if defined alignsz
#       pragma message WARN("Macro alignsz already defined")
#   else
#       define alignsz(size) __attribute__((aligned(size)))
#   endif /* defined alignsz */
#   if defined __force_inline
#       pragma message WARN("Macro __force_inline already defined")
#   else
#       define __force_inline __attribute__((always_inline))
// #       if defined( __GNUC__ )
// #           define __force_inline __attribute__((always_inline)) inline
// #       elif defined( __clang__ )
// #           define __force_inline __attribute__((always_inline))
// #       endif
#   endif /* defined __force_inline */
#   if defined alignment_of
#       pragma message WARN("Macro alignment_of already defined")
#   else
#       if __cplusplus >= 201103L
#           define alignment_of(__name) ( alignof(__name) )
#       elif __STDC_VERSION__ >= 202311L
#           define alignment_of(__name) ( alignof(__name) )
#       elif __STDC_VERSION__ >= 201112L
#           define alignment_of(__name) _Alignof(__name)
#       else
#           define alignment_of(__name) __alignof__(__name)
#       endif
#   endif /* defined alignment_of */
#   if defined __assume_aligned
#       pragma message WARN("Macro __assume_aligned already defined")
#   else
#       if defined(__cplusplus) & __cplusplus >= 202002L
#           pragma message WARN("Macro __assume_aligned is empty! use std::assume_aligned in <memory>")
#           define __assume_aligned(ptr, alignment_size_bytes)
#       else
#           define __assume_aligned(ptr, alignment_size_bytes) (__builtin_assume_aligned(ptr, alignment_size_bytes))
#       endif
#   endif /* defined __assume_aligned */


#elif defined(_MSC_VER)
#   ifndef DISABLE_WARNING_PUSH
#      define DISABLE_WARNING_PUSH           __pragma(warning(  push  ))
#   endif /* !defined DISABLE_WARNING_PUSH */
#   ifndef DISABLE_WARNING_POP
#      define DISABLE_WARNING_POP            __pragma(warning(  pop  )) 
#   endif /* !defined DISABLE_WARNING_POP */
#   ifndef DISABLE_WARNING
#      define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))
#   endif /* !defined DISABLE_WARNING */
#   ifndef DISABLE_WARNING_UNUSED_PARAMETER
#      define DISABLE_WARNING_UNUSED_PARAMETER DISABLE_WARNING(4100)
#   endif /* !defined DISABLE_WARNING_UNUSED_PARAMETER */
#   ifndef DISABLE_WARNING_UNUSED_FUNCTION
#      define DISABLE_WARNING_UNUSED_FUNCTION  DISABLE_WARNING(4505)
#   endif /* !defined DISABLE_WARNING_UNUSED_FUNCTION */
#   ifndef DISABLE_WARNING_NESTED_ANON_TYPES
#      define DISABLE_WARNING_NESTED_ANON_TYPES
#   endif /* !defined DISABLE_WARNING_NESTED_ANON_TYPES */
#   ifndef DISABLE_WARNING_GNU_ANON_STRUCT
#      define DISABLE_WARNING_GNU_ANON_STRUCT
#   endif /* !defined DISABLE_WARNING_GNU_ANON_STRUCT */
#   ifndef DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
#      define DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
#   endif /* !defined DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS */
#   ifndef DISABLE_WARNING_PEDANTIC
#      define DISABLE_WARNING_PEDANTIC
#   endif /* !defined DISABLE_WARNING_PEDANTIC */
#   ifndef DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
#      define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    DISABLE_WARNING_UNUSED_PARAMETER
#   endif /* !defined DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER */
#   ifndef DISABLE_WARNING_UNREFERENCED_FUNCTION
#      define DISABLE_WARNING_UNREFERENCED_FUNCTION            DISABLE_WARNING_UNUSED_FUNCTION
#   endif /* !defined DISABLE_WARNING_UNREFERENCED_FUNCTION */
#   ifndef DISABLE_WARNING_DEPRECATED_FUNCTION
#      define DISABLE_WARNING_DEPRECATED_FUNCTION              DISABLE_WARNING(4996)
#   endif /* !defined DISABLE_WARNING_DEPRECATED_FUNCTION */


#   ifdef likely
#       pragma message WARN("Macro 'likely' already defined")
#   else
#       if defined __cplusplus && __cplusplus > 202002L /* C++20 Has [[likely]] */
#           define likely(cond) [[likely]]
#       else
#           define likely(cond) /* No equivalent in MSVC */
#       endif
#   endif /* ifdef likely */
#   ifdef unlikely
#       pragma message WARN("Macro 'likely' already defined")
#   else
#       if defined __cplusplus && __cplusplus > 202002L /* C++20 Has [[unlikely]] */
#           define unlikely(cond) [[unlikely]]
#       else
#           define unlikely(cond) /* No equivalent in MSVC */
#       endif
#   endif /* ifdef unlikely */


#   if defined __cplusplus && __cplusplus > 201703L /* C++17 gives us [[maybe_unused]] */
#       ifndef __util_attribute_unused
#           define __util_attribute_unused [[maybe_unused]]
#       endif
#   else
#       ifndef __util_attribute_unused
#           define __util_attribute_unused
#       endif
#   endif

#   if defined __notused /* more appropriate for functions	*/
#       pragma message WARN("Macro __notused Already defined")
#   else
#       define __unused __util_attribute_unused
#   endif
#   if defined notused /* more appropriate for function parameters */
#       pragma message WARN("Macro __notused Already defined")
#   else
#       define __unused __util_attribute_unused
#   endif

#   if defined(_DEBUG) /* release mode (Usually) omits a lot of code => gives many unused_param errors */
#       define __release_unused
#   else
#       define __release_unused notused
#   endif

#   if defined __hot
#       pragma message WARN("Macro __hot already defined")
#   else
#       define __hot
#   endif /* defined __hot */
#   if defined __cold
#       pragma message WARN("Macro __cold already defined")
#   else
#       define __cold
#   endif /* defined __cold */
#   if defined pack
#       pragma message WARN("Macro pack already defined")
#   else
#       define pack
#       pragma message WARN("Macro pack has no equivalent on MSVC, Please use UTIL_PACKED_STRUCT instead")
#       ifndef UTIL_PACKED_STRUCT
#           define UTIL_PACKED_STRUCT(struct_name) __pragma(pack(push, 1)) struct struct_name __pragma(pack(pop))
#       else
#           pragma message WARN("Macro UTIL_PACKED_STRUCT Already Defined")
#   endif
#   endif /* defined pack */
#   if defined alignpk
#       pragma message WARN("Macro alignpk already defined")
#   else
#       define alignpk(size)
#       pragma message WARN("Macro alignpk(size) has no equivalent on MSVC")
#   endif /* defined alignpk */
#   if defined alignsz
#       pragma message WARN("Macro alignsz already defined")
#   else
#       define alignsz(size) __declspec(align(size))
#   endif /* defined alignsz */
#   if defined __force_inline
#       pragma message WARN("Macro __force_inline already defined")
#   else
#       if defined __cplusplus 
#           define __force_inline [[msvc::forceinline]]
#       else
#           define __force_inline
#           pragma message WARN("Macro __force_inline has no equivalent for C on MSVC")
#       endif
#   endif /* defined __force_inline */
#   if defined alignment_of
#       pragma message WARN("Macro alignment_of already defined")
#   else
#       if defined __cplusplus | __STDC_VERSION__ >= 202311L
#           define alignment_of(__name) ( alignof(__name) )
#       elif __STDC_VERSION__ >= 201112L
#           define alignment_of(__name) _Alignof(__name)
#       else
#           define alignment_of(__name) __alignof(__name)
#       endif
#   endif /* defined alignment_of */
#   if defined __assume_aligned
#       pragma message WARN("Macro __assume_aligned already defined")
#   else
#       if defined(__cplusplus) & __cplusplus >= 202002L
#           pragma message WARN("Macro __assume_aligned is empty! use std::assume_aligned in <memory>")
#           define __assume_aligned(ptr, alignment_size_bytes)
#       else
#           pragma message WARN("Macro __assume_aligned has no equivalent on MSVC")
#           define __assume_aligned(ptr, alignment_size_bytes)
#       endif
#   endif /* defined __assume_aligned */


/*
    aligned_malloc already defined in most major compilers/stdlibs 
    Nevermind that, just use aligned_malloc.h (the macros below should be refactored)
    also, aligned_alloc/free are defined since C11/C++17
*/
/*
#   ifdef __cplusplus
#       include <cstdlib>
#   else 
#       include <stdlib.h>
#   endif
#   ifndef aligned_alloc
#       define aligned_alloc(size, align) _aligned_malloc(size, align)
#   endif
#   ifndef aligned_free
#       define aligned_free(ptr) _aligned_free(ptr)
#   endif
*/


#else
#   error "Unknown Compiler, Dunno What Attributes to define"
#   ifndef DISABLE_WARNING_PUSH
#       define DISABLE_WARNING_PUSH
#   endif
#   ifndef DISABLE_WARNING_POP
#       define DISABLE_WARNING_POP
#   endif
#   ifndef DISABLE_WARNING
#       define DISABLE_WARNING
#   endif
#   ifndef DISABLE_WARNING_UNUSED_PARAMETER
#       define DISABLE_WARNING_UNUSED_PARAMETER
#   endif
#   ifndef DISABLE_WARNING_UNUSED_FUNCTION 
#       define DISABLE_WARNING_UNUSED_FUNCTION
#   endif
#   ifndef DISABLE_WARNING_NESTED_ANON_TYPES
#       define DISABLE_WARNING_NESTED_ANON_TYPES
#   endif
#   ifndef DISABLE_WARNING_GNU_ANON_STRUCT
#       define DISABLE_WARNING_GNU_ANON_STRUCT
#   endif
#   ifndef DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
#       define DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
#   endif
#   ifndef DISABLE_WARNING_PEDANTIC
#       define DISABLE_WARNING_PEDANTIC
#   endif
#   ifndef DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
#       define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
#   endif
#   ifndef DISABLE_WARNING_UNREFERENCED_FUNCTION
#       define DISABLE_WARNING_UNREFERENCED_FUNCTION
#   endif
#   ifndef DISABLE_WARNING_DEPRECATED_FUNCTION
#       define DISABLE_WARNING_DEPRECATED_FUNCTION
#   endif
#   ifndef likely
#       define likely
#   endif
#   ifndef unlikely
#       define unlikely
#   endif
#   ifndef __notused
#       define __notused
#   endif
#   ifndef notused
#       define notused
#   endif
#   ifndef __release_unused
#       define __release_unused
#   endif
#   ifndef __hot
#       define __hot
#   endif
#   ifndef __cold
#       define __cold
#   endif
#   ifndef pack
#       define pack
#   endif
#   ifndef alignpk
#       define alignpk
#   endif
#   ifndef alignsz
#       define alignpz
#   endif
#   ifndef __force_inline
#       define __force_inline
#   endif
#   ifndef alignment_of
#       define alignment_of
#   endif
#   ifndef __assume_aligned
#       define __assume_aligned
#   endif
#   ifndef aligned_alloc
#       define aligned_alloc
#   endif
#   if defined aligned_alloc
#       pragma message WARN("aligned_alloc(size, align) is empty! Don't manage ANY memory with it")
#   endif
#   define aligned_free(size, align)
#   if defined aligned_free
#       pragma message WARN("aligned_free(size, align) is empty! Don't manage ANY memory with it")
#   endif
#endif


#if defined _DEBUG || defined DEBUG
#    define util2_debug(...) { __VA_ARGS__ }
#    define util2_debugnobr(...) __VA_ARGS__
#    define util2_debug_declaration_nobr(...) __VA_ARGS__
#    define util2_release(...)
#    define util2_releasenobr(...)
#    define util2_release_declaration_nobr(...)
#else
#    define util2_debug(...)
#    define util2_debugnobr(...)
#    define util2_debug_declaration_nobr(...)
#    define util2_release(...) { __VA_ARGS__ }
#    define util2_releasenobr(...) __VA_ARGS__
#    define util2_release_declaration_nobr(...) __VA_ARGS__
#endif


#ifndef __carraysize
#    define __carraysize(__c_array) ( sizeof(__c_array) / sizeof(__c_array[0]) )
#endif
#ifndef __ccast
#    define __ccast(type, val) ((type)val)
#endif
#ifdef __cplusplus
#    ifndef __scast
#        define __scast(type, val) static_cast<type>((val))
#    endif
#    ifndef __rcast
#        define __rcast(type, val) reinterpret_cast<type>((val))
#    endif
#    ifndef isaligned
#        define isaligned(ptr, alignment) boolean( (  __rcast(unsigned long long, ptr) & ( __scast(unsigned long long, alignment) - 1llu )  ) == 0 )
#    endif
#else
#    define __scast(type, val) (__ccast(type, val))
#    define __rcast(type, val) (__ccast(type, val))
#endif



#ifndef __rconce
#    define __rconce(__finished, ...) \
        if(!boolean(__finished)) {{ \
            __VA_ARGS__; \
            ++__finished; \
        }} \

#endif
#ifndef __rcblock
#    define __rcblock(times, __finished, code_block) \
        if(boolean((uint16_t)times - __finished)) {{ \
            code_block; \
            ++__finished; \
        }} \

#endif


#ifndef boolean
#    define boolean(arg) !!(arg)
#endif
#ifndef KB
#    define KB   (1024llu)
#endif
#ifndef MB
#    define MB   (KB*KB)
#endif
#ifndef GB
#    define GB   (MB*MB)
#endif
#ifndef PAGE
#    define PAGE (4 * KB)
#endif
#ifndef __M64_ALIGN_BYTES
#    define __M64_ALIGN_BYTES  (0x08llu)
#endif
#ifndef __M128_ALIGN_BYTES
#    define __M128_ALIGN_BYTES (0x0fllu)
#endif
#ifndef __M256_ALIGN_BYTES
#    define __M256_ALIGN_BYTES (0x1fllu)
#endif
#ifndef __M512_ALIGN_BYTES
#    define __M512_ALIGN_BYTES (0x3fllu)
#endif
#ifndef __M64_SIZE_BYTES
#    define __M64_SIZE_BYTES   (0x08llu)
#endif
#ifndef __M128_SIZE_BYTES
#    define __M128_SIZE_BYTES  (0x10llu)
#endif
#ifndef __M256_SIZE_BYTES
#    define __M256_SIZE_BYTES  (0x20llu)
#endif
#ifndef __M512_SIZE_BYTES
#    define __M512_SIZE_BYTES  (0x40llu)
#endif
#ifndef CACHE_LINE_BYTES
#    define CACHE_LINE_BYTES   (64ul)
#endif
#ifndef DEFAULT8
#    define DEFAULT8           (0xAA)
#endif
#ifndef DEFAULT16
#    define DEFAULT16          (0xF00D)
#endif
#ifndef DEFAULT32
#    define DEFAULT32          (0xBABEBABE)
#endif
#ifndef DEFAULT64
#    define DEFAULT64          (0xFACADE00FACADE00)
#endif
#ifndef DEFAULT128
#    define DEFAULT128         (0xAAAC0FFEEAC1DAAA)
#endif
#ifndef DEFAULTVOIDPTR
#    define DEFAULTVOIDPTR __rcast(void*, DEFAULT64)
#endif
#ifndef NULLPTR
#    define NULLPTR ((void*)0)
#endif
#ifndef NULLPTR_TYPE
#    define NULLPTR_TYPE(type) ( __rcast(type*, (void*)0) )
#endif
#ifndef BOOL_FALSE
#    define BOOL_FALSE (boolean(0))
#endif
#ifndef BOOL_TRUE
#    define BOOL_TRUE (boolean(1))
#endif


#endif /* __UTIL2_BASE_MACRO_DEFINITIONS__ */