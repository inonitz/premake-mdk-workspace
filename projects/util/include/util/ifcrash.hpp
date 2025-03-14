#ifndef __UTIL_IF_CRASH_MACRO__
#define __UTIL_IF_CRASH_MACRO__
#include "util_api.h"
#include "macro.h"


namespace detail::ifcrash_macro {
	constexpr const char* str_msg_begin = "[IFCRASH_%s] %s:%u";
	constexpr const char* str_msg_mid   = "\n[IFCRASH_%s] ";
	constexpr const char* str_msg_end   = "\n[IFCRASH_%s] ifcrash(...) macro triggered\n";

	UTIL_API void __common_print_fmt(const char* format, ...);
	UTIL_API void __common_print_nofmt(const char* str);
	[[noreturn]] UTIL_API void __common_abort() noexcept;
}


DISABLE_WARNING_PUSH
DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
/* Tested ##__VA_ARGS__ on msvc, clang, gcc and works perfectly (std=C++17) | probably not portable on older versions of msvc */
#define ifcrash_generic(condition, name, str_or_fmt_required, append_name_if_required, action, str, ...) \
if(unlikely( !!(condition)))  \
{ \
	detail::ifcrash_macro::__common_print_fmt(detail::ifcrash_macro::str_msg_begin, name, __FILE__, __LINE__, name); \
	if constexpr (str_or_fmt_required) { \
		detail::ifcrash_macro::__common_print_fmt(detail::ifcrash_macro::str_msg_mid, name); \
		detail::ifcrash_macro::__common_print##append_name_if_required(str, ##__VA_ARGS__); \
	} \
    { action; } \
	detail::ifcrash_macro::__common_print_fmt(detail::ifcrash_macro::str_msg_end, name); \
	detail::ifcrash_macro::__common_abort(); \
}
DISABLE_WARNING_POP


#if defined(_DEBUG)
#define ifcrash_debug(condition) 			  ifcrash_generic(condition, "DBG",      false, _nofmt,   {},     nullptr)
#define ifcrashdo_debug(condition, code)      ifcrash_generic(condition, "CODE_DBG", false, _nofmt, { code }, nullptr)
#define ifcrashstr_debug(condition, str) 	  ifcrash_generic(condition, "STR_DBG",  true,  _nofmt,   {}, str)
#define ifcrashfmt_debug(condition, str, ...) ifcrash_generic(condition, "FMT_DBG",  true,    _fmt,   {}, str, __VA_ARGS__)

#else

#define ifcrash_debug(condition) {}
#define ifcrashstr_debug(condition, str) {}
#define ifcrashfmt_debug(condition, str, ...) {}
#define ifcrashdo_debug(condition, action) {}

#endif


#define ifcrash(condition) 			    ifcrash_generic(condition, "",     false, _nofmt,   {}, nullptr)
#define ifcrashdo(condition, code)      ifcrash_generic(condition, "CODE", false, _nofmt, { code }, nullptr)
#define ifcrashstr(condition, str) 	    ifcrash_generic(condition, "STR",  true,  _nofmt,   {}, str)
#define ifcrashfmt(condition, str, ...) ifcrash_generic(condition, "FMT",  true,    _fmt,   {}, str, __VA_ARGS__)
#define ifcrashfmt_do(condition, str, code, ...) ifcrash_generic(condition, "CODE_FMT", true, _fmt, { code }, str, __VA_ARGS__)


#endif