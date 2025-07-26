#ifndef __UTIL2_API_EXTERN_C_DEFINITION_MACRO
#define __UTIL2_API_EXTERN_C_DEFINITION_MACRO
#ifdef __cplusplus
#   define UTIL2_EXTERNC extern "C"
#   define UTIL2_EXTERNC_DECL_BEGIN UTIL2_EXTERNC {
#   define UTIL2_EXTERNC_DECL_END   }
#else
#   define UTIL2_EXTERNC
#   define UTIL2_EXTERNC_DECL_BEGIN
#   define UTIL2_EXTERNC_DECL_END
#endif
#endif