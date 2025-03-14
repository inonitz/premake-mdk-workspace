#ifndef __UTIL_BASE_MACRO_DEFINITIONS__
#define __UTIL_BASE_MACRO_DEFINITIONS__
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
#      pragma message WARN("Macro __notused Already defined")
#   else
#      define __unused __util_attribute_unused
#   endif
#   if defined notused /* more appropriate for function parameters */
#      pragma message WARN("Macro __notused Already defined")
#   else
#      define notused __util_attribute_unused
#   endif

#   if defined(_DEBUG) || defined(DEBUG) /* release mode (Usually) omits a lot of code => gives many unused_param errors */
#      define __release_unused
#   else
#      define __release_unused notused
#   endif

#   if defined __hot
#       pragma message WARN("Macro __hot already defined")
#   else
#      define __hot __attribute__((hot))
#   endif /* defined __hot */
#   if defined __cold
#       pragma message WARN("Macro __cold already defined")
#   else
#      define __cold __attribute__((cold))
#   endif /* defined __cold */
#   if defined pack
#       pragma message WARN("Macro pack already defined")
#   else
#      define pack __attribute__((packed))
#   endif /* defined pack */
#   if defined alignpk
#       pragma message WARN("Macro alignpk already defined")
#   else
#      define alignpk(size) __attribute__((packed, aligned(size)))
#   endif /* defined alignpk */
#   if defined alignsz
#       pragma message WARN("Macro alignsz already defined")
#   else
#      define alignsz(size) __attribute__((aligned(size)))
#   endif /* defined alignsz */
#   if defined __force_inline
#       pragma message WARN("Macro __force_inline already defined")
#   else
#      define __force_inline __attribute__((always_inline))
#   endif /* defined __force_inline */
#   if defined alignment_of
#      pragma message WARN("Macro alignment_of already defined")
#   else
#      define alignment_of(__name) __alignof__(__name)
#   endif /* defined alignment_of */



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
#      ifndef __util_attribute_unused
#          define __util_attribute_unused [[maybe_unused]]
#      endif
#   else
#      ifndef __util_attribute_unused
#          define __util_attribute_unused
#      endif
#   endif

#   if defined __notused /* more appropriate for functions	*/
#      pragma message WARN("Macro __notused Already defined")
#   else
#      define __unused __util_attribute_unused
#   endif
#   if defined notused /* more appropriate for function parameters */
#      pragma message WARN("Macro __notused Already defined")
#   else
#      define __unused __util_attribute_unused
#   endif

#   if defined(_DEBUG) /* release mode (Usually) omits a lot of code => gives many unused_param errors */
#      define __release_unused
#   else
#      define __release_unused notused
#   endif

#   if defined __hot
#       pragma message WARN("Macro __hot already defined")
#   else
#      define __hot
#   endif /* defined __hot */
#   if defined __cold
#       pragma message WARN("Macro __cold already defined")
#   else
#      define __cold
#   endif /* defined __cold */
#   if defined pack
#      pragma message WARN("Macro pack already defined")
#   else
#      define pack
#      pragma message WARN("Macro pack has no equivalent on MSVC, Please use UTIL_PACKED_STRUCT instead")
#      ifndef UTIL_PACKED_STRUCT
#          define UTIL_PACKED_STRUCT(struct_name) \
                __pragma(pack(push, 1)) \
                        struct struct_name \
                __pragma(pack(pop))
#      else
#          pragma message WARN("Macro UTIL_PACKED_STRUCT Already Defined")
#      endif
#   endif /* defined pack */
#   if defined alignpk
#       pragma message WARN("Macro alignpk already defined")
#   else
#      define alignpk(size)
#      pragma message WARN("Macro alignpk(size) has no equivalent on MSVC")
#   endif /* defined alignpk */
#   if defined alignsz
#       pragma message WARN("Macro alignsz already defined")
#   else
#      define alignsz(size) __declspec(align(size))
#   endif /* defined alignsz */
#   if defined __force_inline
#       pragma message WARN("Macro __force_inline already defined")
#   else
#      if defined __cplusplus 
#          define __force_inline [[msvc::forceinline]]
#      else
#          define __force_inline
#          pragma message WARN("Macro __force_inline has no equivalent for C on MSVC")
#      endif
#   endif /* defined __force_inline */
#   if defined alignment_of
#      pragma message WARN("Macro alignment_of already defined")
#   else
#      define alignment_of(__name) __alignof(__name)
#   endif /* defined alignment_of */


/* aligned_malloc already defined in most major compilers. */
#   ifdef __cplusplus
#      include <cstdlib>
#   else 
#      include <malloc.h>
#   endif
#   ifndef aligned_alloc
#      define aligned_alloc(size, align) _aligned_malloc(size, align)
#   endif
#   ifndef aligned_free
#      define aligned_free(ptr) _aligned_free(ptr)
#   endif




#else
#   error "Unknown Compiler, Dunno What Attributes to define"
#   define DISABLE_WARNING_PUSH
#   define DISABLE_WARNING_POP
#   define DISABLE_WARNING(warningNumber)
#   define DISABLE_WARNING_UNUSED_PARAMETER
#   define DISABLE_WARNING_UNUSED_FUNCTION 
#   define DISABLE_WARNING_NESTED_ANON_TYPES
#   define DISABLE_WARNING_GNU_ANON_STRUCT
#   define DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
#   define DISABLE_WARNING_PEDANTIC
#   define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
#   define DISABLE_WARNING_UNREFERENCED_FUNCTION
#   define DISABLE_WARNING_DEPRECATED_FUNCTION
#   define likely(cond)
#   define unlikely(cond)
#   define __notused
#   define notused
#   define __release_unused
#   define __hot
#   define __hot
#   define __cold
#   define pack
#   define alignpk(align)
#   define alignsz(size)
#   define __force_inline
#   define alignment_of(typename)
#   define aligned_alloc(size, align)
#   if defined aligned_alloc
#       pragma message WARN("aligned_alloc(size, align) are empty macros! Don't manage ANY memory with them")
#   endif
#   define aligned_free(size, align)
#   if defined aligned_free
#       pragma message WARN("aligned_free(size, align) are empty macros! Don't manage ANY memory with them")
#   endif
#endif


#if defined _DEBUG || defined DEBUG
#    define debug(...) { __VA_ARGS__ }
#    define debugnobr(...) __VA_ARGS__
#    define debug_declaration_nobr(...) __VA_ARGS__
#    define release(...)
#    define releasenobr(...)
#    define release_declaration_nobr(...)
#else
#    define debug(...)
#    define debugnobr(...)
#    define debug_declaration_nobr(...)
#    define release(...) { __VA_ARGS__ }
#    define releasenobr(...) __VA_ARGS__
#    define release_declaration_nobr(...) __VA_ARGS__
#endif


#define __carraysize(__c_array) ( sizeof(__c_array) / sizeof(__c_array[0]) )
#ifdef __cplusplus
#   define isaligned(ptr, alignment) boolean( (  reinterpret_cast<size_t>(ptr) & (static_cast<size_t>(alignment) - 1llu)  ) == 0 )
#   define __scast(type, val) static_cast<type>((val))
#   define __rcast(type, val) reinterpret_cast<type>((val))
#else
#   define isaligned(ptr, alignment) boolean( (  ((char*)(ptr) & ((u64)alignment) - 1llu)  ) == 0 )
#   define __scast(type, val)
#   define __rcast(type, val)
#endif


#define __rconce(__finished, ...) \
        if(!boolean(__finished)) { \
                { \
                        __VA_ARGS__; \
                        ++__finished; \
                } \
        } \


#define __rcblock(times, __finished, code_block) \
        if(boolean((uint16_t)times - __finished)) { \
                { \
                        code_block; \
                        ++__finished; \
                } \
        } \


#define boolean(arg) !!(arg)
#define KB           	   (1024llu)
#define MB           	   (KB*KB)
#define GB           	   (MB*MB)
#define PAGE         	   (4 * KB)
#define __M64_ALIGN_BYTES  (0x08llu)
#define __M128_ALIGN_BYTES (0x0fllu)
#define __M256_ALIGN_BYTES (0x1fllu)
#define __M512_ALIGN_BYTES (0x3fllu)
#define __M64_SIZE_BYTES   (0x08llu)
#define __M128_SIZE_BYTES  (0x10llu)
#define __M256_SIZE_BYTES  (0x20llu)
#define __M512_SIZE_BYTES  (0x40llu)
#define CACHE_LINE_BYTES   (64ul)
#define DEFAULT8           (0xAA)
#define DEFAULT16          (0xF00D)
#define DEFAULT32          (0xBABEBABE)
#define DEFAULT64          (0xFACADE00FACADE00)
#define DEFAULT128         (0xAAAC0FFEEAC1DAAA)
#ifdef __cplusplus
#   define DEFAULTVOIDPTR __rcast(void*, DEFAULT64)
#else
#   define DEFAULTVOIDPTR ((void*)DEFAULT64)
#endif


#endif /* __UTIL_BASE_MACRO_DEFINITIONS__ */