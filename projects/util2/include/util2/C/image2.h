#ifndef __UTIL2_C_DEFINITION_LOAD_IMAGE__
#define __UTIL2_C_DEFINITION_LOAD_IMAGE__
#include "util2/C/util2_api.h"
#include "util2/C/util2_extern.h"
#include "util2/C/base_type.h"


UTIL2_EXTERNC_DECL_BEGIN


typedef struct __util2_lib_loaded_image_pod_type {
    u16 m_x;
    u16 m_y;
    u8  m_channels;
    u8  m_normfloat;
    u8  m_reserved[2];
    u8* m_data;
} util2Image;


UTIL2_API bool_t util2_load_image(
    const char* fileName, 
    u8          convert_to_float,
    util2Image* result
);

UTIL2_API void util2_unload_image(util2Image* img);


UTIL2_EXTERNC_DECL_BEGIN


#endif