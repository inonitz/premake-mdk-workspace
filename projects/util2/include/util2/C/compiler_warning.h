#ifndef __UTIL2_AGNOSTIC_COMPILER_WARNING__
#define __UTIL2_AGNOSTIC_COMPILER_WARNING__
/* 
	Thanks to: https://stackoverflow.com/questions/471935/user-warnings-on-msvc-and-gcc
*/


/*
	Usage:
	#pragma message WARN("your very awesome message")
*/
#if defined( __GNUC__ ) || defined( __MINGW__ ) || defined ( __clang__ )
#   define WARN(exp) ("WARNING: " exp)
#elif defined _MSC_VER
#	define STRINGISE_IMPL(x) #x
#	define STRINGISE(x) STRINGISE_IMPL(x)
#   define FILE_LINE_LINK __FILE__ "(" STRINGISE(__LINE__) ") : "
#   define WARN(exp) (FILE_LINE_LINK "WARNING: " exp)
#else
#	error "Unknown Compiler Detected! Please implement on your own"
#endif


#endif /* __UTIL2_AGNOSTIC_COMPILER_WARNING__ */