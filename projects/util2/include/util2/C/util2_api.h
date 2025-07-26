#ifndef _UTIL2_API_H_
#define _UTIL2_API_H_


#ifdef UTIL2_STATIC_DEFINE /* Define if compiling as a static library (-DUTIL2_STATIC_DEFINE) */
#	define UTIL2_API
#	define UTIL2_NO_EXPORT
#else
#	ifndef UTIL2_API
#		ifdef UTIL2_EXPORTS /* We are building this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				if defined __GNUC__ || defined __clang__
#					define UTIL2_API __attribute__ ((dllexport))
#				else
#					define UTIL2_API __declspec(dllexport)
#				endif
#			else 
#				if (defined __GNUC__ && __GNUC__ >= 4) || defined __clang__
#					define UTIL2_API __attribute__ ((visibility ("default")))
#				endif
#			endif
#    	else /* We are using this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				if defined __GNUC__ || defined __clang__
#					define UTIL2_API __attribute__ ((dllimport))
#				else
#					define UTIL2_API __declspec(dllimport)
#				endif
#			else 
#				if defined __GNUC__ && __GNUC__ >= 4
#					define UTIL2_API
#				endif
#			endif
#       endif
#	else /* Should Only reach here for non-*nix, un-supported platforms */
#       warning "Platform Unsupported - Either Not a derivative of Unix // Not Windows"
#		define UTIL2_API
#   endif
#	ifndef UTIL2_NO_EXPORT
#       if defined __GNUC__ && __GNUC__ >= 4 /* Symbols exported by default on *nix systems */
#           define UTIL2_NO_EXPORT __attribute__((visibility ("hidden")))
#       else /* (DLL) Symbols on platforms like windows must be exported manually [__declspec(dllexport)] */
#		    define UTIL2_NO_EXPORT 
#       endif
#	endif
#endif


#ifndef UTIL2_DEPRECATED
#   if defined(__cplusplus)
#       if __cplusplus >= 201402L /* [[deprecated]] Supported since C++14 */
#           define UTIL2_DEPRECATED [[deprecated]]
#           define UTIL2_DEPRECATED_MSG(MSG) [[deprecated(MSG)]]
#       endif
#   else
#       if defined _WIN32 || defined _WIN64
#           if defined __GNUC__ || defined __clang__ /* Cygwin, MinGW32/64 */
#               define UTIL2_DEPRECATED          __attribute__((deprecated))
#               define UTIL2_DEPRECATED_MSG(MSG) __attribute__((deprecated(MSG)))
#           else
#               define UTIL2_DEPRECATED          __declspec(deprecated)
#               define UTIL2_DEPRECATED_MSG(MSG) __declspec(deprecated(MSG))
#           endif
#       elif defined __GNUC__ || defined __clang__
#           define UTIL2_DEPRECATED __attribute__((deprecated))
#           define UTIL2_DEPRECATED_MSG(MSG) __attribute__((deprecated(MSG)))
#       else /* Should Only reach here for non-*nix, un-supported platforms */
#           define UTIL2_DEPRECATED
#           define UTIL2_DEPRECATED_MSG(MSG)
#       endif
#   endif
#endif


#ifndef UTIL2_DEPRECATED_EXPORT
#  define UTIL2_DEPRECATED_EXPORT UTIL2_API UTIL2_DEPRECATED
#endif


#ifndef UTIL2_DEPRECATED_NO_EXPORT
#  define UTIL2_DEPRECATED_NO_EXPORT UTIL2_NO_EXPORT UTIL2_DEPRECATED
#endif


/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef UTIL2_NO_DEPRECATED
#    define UTIL2_NO_DEPRECATED
#  endif
#endif

#endif /* _UTIL2_API_H_ */