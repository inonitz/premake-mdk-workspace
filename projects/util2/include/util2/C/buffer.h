#ifndef __UTIL2_C_DEFINITION_GENERIC_VECTOR_BUFFER__
#define __UTIL2_C_DEFINITION_GENERIC_VECTOR_BUFFER__
#include "util2/C/util2_api.h"
#include "util2/C/util2_extern.h"
#include "util2/C/macro.h"
#include "util2/C/base_type.h"
#include <stdlib.h>
#include <stdio.h>


UTIL2_EXTERNC_DECL_BEGIN


#if defined UTILITY2_BUFFER_GENERATE_TEMPLATE_TYPE_DEFINITION
#   error "Macro Named UTILITY22_BUFFER_GENERATE_TEMPLATE_TYPE_DEFINITION already exists"
#endif
#if defined UTILITY2_BUFFER_GENERATE_TEMPLATE_FUNCTION_DEFINITION
#   error "Macro Named UTILITY2_BUFFER_GENERATE_TEMPLATE_DEFINITION already exists"
#endif
#if defined UTILITY2_BUFFER_GENERATE_TEMPLATE_FUNCTION_IMPL
#   error "Macro Named UTILITY2_BUFFER_GENERATE_TEMPLATE_IMPL already exists"
#endif


#define UTILITY2_BUFFER_GENERATE_TEMPLATE_TYPE_DEFINITION(Type, __StructureName) \
typedef struct __generic_buffer_vector_like_##Type##__StructureName \
{ \
    Type* ptr; \
    u32 elem_size; \
    u32 currentSize; \
    u32 maximumSize; \
    u8  ownptr; /* 1 = I am responsible for ptr, 0 = I'm not */ \
    u8  reserved[3]; \
} __StructureName; \


#define UTILITY2_BUFFER_GENERATE_TEMPLATE_FUNCTION_DEFINITION(Type, __StructureName) \
\
\
UTIL2_API __StructureName __StructureName##CreateWith( \
    void* initialData, \
    u32   initialSize, \
    u8    bool_ObjectShouldOwnInitialDataPointer \
); \
UTIL2_API __StructureName __StructureName##CreateWithAndSet( \
    void* initialData, \
    u32   initialSize, \
    u8    bool_ObjectShouldOwnInitialDataPointer, \
    Type  initialValue \
); \
UTIL2_API __StructureName __StructureName##Create(u32 size); \
UTIL2_API __StructureName __StructureName##CreateAndSet( \
    u32  size, \
    Type initialValue \
); \
UTIL2_API __StructureName __StructureName##CreateWithMax(u32 size, u32 maxSize); \
UTIL2_API void        __StructureName##Copy(__StructureName* dest, __StructureName* src); \
UTIL2_API void        __StructureName##Destroy(__StructureName* buf); \
UTIL2_API void        __StructureName##Clear(__StructureName* buf); \
UTIL2_API void        __StructureName##Set( \
    __StructureName* buf, \
    Type*            value \
); \
UTIL2_API Type        __StructureName##At( \
    __StructureName* buf, \
    u32 index \
); \
UTIL2_API Type*       __StructureName##AtPtr( \
    __StructureName* buf, \
    u32 index \
); \
UTIL2_API const Type* __StructureName##AtConstPtr( \
    __StructureName* buf, \
    u32 index \
); \
UTIL2_API void __StructureName##Expand( \
    __StructureName* buf, \
    u32              newMax \
); \
UTIL2_API void __StructureName##Push_back( \
    __StructureName* buf, \
    Type*            val \
); \
UTIL2_API Type*       __StructureName##Begin(__StructureName* buf); \
UTIL2_API Type*       __StructureName##End(__StructureName* buf); \
UTIL2_API const Type* __StructureName##BeginConst(__StructureName* buf); \
UTIL2_API const Type* __StructureName##EndConst(__StructureName* buf); \




#define UTILITY2_BUFFER_GENERATE_TEMPLATE_FUNCTION_IMPL(Type, __StructureName) \
__StructureName __StructureName##CreateWith(void* initialData, u32 initialSize, u8 shouldOwn) \
{ \
    __StructureName to_ret; \
    to_ret = (__StructureName){ \
        ((Type*)initialData), \
        sizeof(Type), \
        0, \
        initialSize, \
        shouldOwn, \
        {DEFAULT8} \
    };  \
    return to_ret; \
} \
__StructureName __StructureName##CreateWithAndSet(void* initialData, u32 initialSize, u8 shouldOwn, Type initialValue) \
{ \
    __StructureName to_ret; \
    to_ret = (__StructureName){ \
        ((Type*)initialData), \
        sizeof(Type), \
        initialSize, \
        initialSize, \
        shouldOwn, \
        {DEFAULT8} \
    }; \
    for(u32 i = 0; i < initialSize; ++i) { \
        to_ret.ptr[i] = initialValue; \
    } \
    return to_ret; \
} \
__StructureName __StructureName##Create(u32 size) { \
    return __StructureName##CreateWith( malloc(sizeof(Type) * size), size, boolean(1)); \
} \
__StructureName __StructureName##CreateAndSet(u32 size, Type initialValue) { \
    return __StructureName##CreateWithAndSet( malloc(sizeof(Type) * size), size, initialValue, boolean(1)); \
} \
__StructureName __StructureName##CreateWithMax(u32 size, u32 maxSize) { \
    __StructureName to_ret = __StructureName##Create(maxSize); \
    to_ret.currentSize = size; \
    return to_ret; \
} \
void __StructureName##Copy(__StructureName* dest, __StructureName* src) { \
    util2_debug({ \
        if(dest->currentSize < src->currentSize) \
            fputs("__StructureName##Copy() => Invalid Copy Operation [dest.currentSize < src.currentSize]\n", stderr); \
            return; \
    }); \
    for(u32 i = 0; i < src->currentSize; ++i) { \
        dest->ptr[i] = src->ptr[i]; \
    } \
    return; \
} \
void __StructureName##Destroy(__StructureName* buf) \
{ \
    if(buf->ownptr) { \
        free(buf->ptr); \
    } \
    buf->ptr         = NULLPTR_TYPE(Type); \
    buf->elem_size   = 0; \
    buf->currentSize = 0; \
    buf->maximumSize = 0; \
    buf->ownptr      = 0; \
    return; \
} \
void __StructureName##Set(__StructureName* buf, Type* value) \
{ \
    Type __local = *value; \
    for(u32 i = 0; i < buf->currentSize; ++i) { \
        buf->ptr[i] = __local; \
    } \
    return; \
} \
void __StructureName##Clear(__StructureName* buf) \
{ \
    Type __default_value; \
    for(u32 i = 0; i < sizeof(Type); ++i) { \
        ((u8*)(&__default_value))[i] = DEFAULT8; \
    } \
    __StructureName##Set(buf, &__default_value); \
    return; \
} \
Type* __StructureName##AtPtr(__StructureName* buf, u32 index) \
{ \
    util2_debug({ \
        u8 __ret_null = (index >= buf->maximumSize); \
        if(index >= buf->currentSize) { \
            fputs("__StructureName##At() => Invalid Memory Access [givenIndex > currentSize]\n", stderr); \
        } \
        if(__ret_null) { \
            fputs("__StructureName##At() => Invalid Memory Access [givenIndex > maxmiumSize]\n", stderr); \
        } \
        return (__ret_null) ? NULLPTR_TYPE(Type) : &buf->ptr[index]; \
    }); \
    util2_releasenobr({ \
        return &buf->ptr[index]; \
    }); \
} \
Type __StructureName##At(__StructureName* buf, u32 index)  \
{ \
    return *__StructureName##AtPtr(buf, index); \
} \
const Type* __StructureName##AtConstPtr(__StructureName* buf, u32 index)  \
{ \
    return __StructureName##AtPtr(buf, index); \
} \
void __StructureName##Expand( \
    __StructureName* buf, \
    u32              newMax \
) { \
    if(!buf->ownptr) { \
        fputs("__StructureName##Expand() => Buffer can't free the old memory it doesn't own\n", stderr); \
        return; \
    } \
    buf->maximumSize = newMax; \
    __StructureName new_buf = __StructureName##Create(newMax); \
    __StructureName##Copy(&new_buf, buf); /* will copy to new_buf from buf[0] -> buf[buf->currentSize] */ \
    free(buf->ptr);                       /* free the old small buffer */ \
    buf->ptr = new_buf.ptr;               /* transfer of pointers, we don't want to destroy buf */ \
    return; \
} \
void __StructureName##Push_back( \
    __StructureName* buf, \
    Type*            val \
) { \
    if(buf->currentSize == buf->maximumSize) \
        __StructureName##Expand( buf, buf->currentSize + (buf->currentSize / 2) ); \
    \
    buf->ptr[buf->currentSize] = *val; \
    ++buf->currentSize; \
    return; \
} \
Type* __StructureName##Begin(__StructureName* buf) \
{ \
    return &buf->ptr[0]; \
} \
Type* __StructureName##End(__StructureName* buf) \
{ \
    return &buf->ptr[buf->currentSize - 1]; \
} \
const Type* __StructureName##BeginConst(__StructureName* buf) \
{ \
    return __StructureName##Begin(buf); \
} \
const Type* __StructureName##EndConst(__StructureName* buf) \
{ \
    return __StructureName##End(buf); \
} \


UTILITY2_BUFFER_GENERATE_TEMPLATE_TYPE_DEFINITION(u8, util2BufferByte)
UTILITY2_BUFFER_GENERATE_TEMPLATE_FUNCTION_DEFINITION(u8, util2BufferByte)


UTIL2_EXTERNC_DECL_END


#endif