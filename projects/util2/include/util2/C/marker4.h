#ifndef __UTIL2_C_DEFINITION_MARKER_FLAG_MACRO__
#define __UTIL2_C_DEFINITION_MARKER_FLAG_MACRO__
// #define UTIL2_MARKER_FLAG_DISABLE 1
#define UTIL2_MARKER_FLAG_KEEP_RELEASE           1
#define UTIL2_MARKER_FLAG_INCLUDE_NEWLINE_AT_END 1
// #define UTIL2_MARKER_FLAG_LOG_TO_FILE 1


#ifndef UTIL2_MARKER_FLAG_DISABLE
#	define UTIL2_MARKER_FLAG_DISABLE 0
#endif
#ifndef UTIL2_MARKER_FLAG_KEEP_RELEASE
#	define UTIL2_MARKER_FLAG_KEEP_RELEASE 0
#endif
#ifndef UTIL2_MARKER_FLAG_INCLUDE_NEWLINE_AT_END
#	define UTIL2_MARKER_FLAG_INCLUDE_NEWLINE_AT_END 0
#endif
#ifndef UTIL2_MARKER_FLAG_LOG_TO_FILE
#	define UTIL2_MARKER_FLAG_LOG_TO_FILE 0
#endif
#if !(UTIL2_MARKER_FLAG_DISABLE) & (defined(_DEBUG) | defined(DEBUG) | (UTIL2_MARKER_FLAG_KEEP_RELEASE))
#	undef UTIL2_MARKER_FLAG_DEFINE_IMPLEMENTATION
#	define UTIL2_MARKER_FLAG_DEFINE_IMPLEMENTATION 1
#endif


#if UTIL2_MARKER_FLAG_DEFINE_IMPLEMENTATION == 1
#	include "util2/C/util2_api.h"
#	include "util2/C/util2_extern.h"


/* 
    [NOTE]: The only reason the impl file is a .cpp file
    is due to clangd incorrectly linting <stdatomic.h> on my windows 10 machine 
    currently waiting for 'BuiltinHeaders' feature on clangd21 to see if fixed
*/
UTIL2_API UTIL2_EXTERNC void util2_marker_flag(
    const char*   file_macro,
    unsigned long line_macro,
    const char*   formatstr,
    ...
);


// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, cppcoreguidelines-macro-usage)
#	define mark()            util2_marker_flag(__FILE__, __LINE__, "");
#	define markstr(str)      util2_marker_flag(__FILE__, __LINE__, str);
#	define markfmt(str, ...) util2_marker_flag(__FILE__, __LINE__, str, __VA_ARGS__);
// NOLINTEND(cppcoreguidelines-pro-type-vararg, cppcoreguidelines-macro-usage)


#endif /* UTIL2_MARKER_FLAG_DISABLE */


#endif /* __UTIL2_C_DEFINITION_MARKER_FLAG_MACRO__ */