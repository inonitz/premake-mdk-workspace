#ifndef __UTIL2_C_DEFINITION_PRINT__
#define __UTIL2_C_DEFINITION_PRINT__
#include "util2/C/util2_api.h"
#include "util2/C/util2_extern.h"
#include <stdio.h>
#include <stdarg.h>


UTIL2_EXTERNC_DECL_BEGIN


UTIL2_API void util2_printf(const char* formatstr, ...);
UTIL2_API void util2_fprintf(FILE* write_to, const char* formatstr, ...);
UTIL2_API void util2_va_fprintf(
    FILE*       write_into,
    const char* format, 
    va_list     arg_list
); 

UTIL2_EXTERNC_DECL_END


#endif /* __UTIL2_C_DEFINITION_PRINT__ */