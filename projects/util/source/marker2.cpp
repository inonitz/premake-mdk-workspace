#include "util/marker2.hpp"


#if MARKER_FLAG_DEFINE_IMPLEMENTATION == 1
#include <cstdarg>
#include <cstdio>

/* 
    The only reason I'm not using C++11 atomics 
    is because msvc keeps giving me compiling errors (their <memory>, <mutex> headers are broken...)
    ( for std=C++17 )
*/
#include <threads.h>
#include <stdatomic.h>


namespace detail::marker {
    using write_to_file_one_at_a_time = mtx_t;
    using write_lock_type = write_to_file_one_at_a_time;
    

    static atomic_uint_least64_t __markflag = 0;
    static write_lock_type __write_lock;
    static FILE* __output_buf = (MARKER_FLAG_LOG_TO_FILE) ? fopen("__debug_output.txt", "w") : stdout;


    void __begin_exclusion() {
        /* init mutex on first call of __begin_exclusion */
        uint_least64_t expected = 0;
        if(atomic_compare_exchange_strong(&__markflag, &expected, 1))
            mtx_init(&__write_lock, memory_order_acq_rel);
        
        /* Wait if other thread is already printing to __output_buf */
        while(thrd_success != mtx_trylock(&__write_lock)) {}
        return;
    }
    void __end_exclusion() {
        mtx_unlock(&__write_lock);
        return;
    }

    unsigned long long __load_atomic_counter() { 
        return atomic_load(&__markflag);
    }

    void __increment_atomic_counter() { 
        atomic_fetch_add_explicit(&__markflag, 1, memory_order_seq_cst);
        return;
    }


    void __common_print_function_nofmt(const char* stri)
    {
        std::fputs(stri, __output_buf);
        return;
    }

    void __common_print_function_fmt(const char* format, ...) 
    {
#define __GENERIC_FORMAT_BUFFER_MAX_SIZE 32768
        static struct __generic_format_buffer {
            char mem[__GENERIC_FORMAT_BUFFER_MAX_SIZE];
        } __format_buffer;
        va_list arg, argcopy;
        int size, done = 1;
        bool invalid_state = false;


        __format_buffer.mem[__GENERIC_FORMAT_BUFFER_MAX_SIZE - 1] = '\0'; /* incase of overflow */
        va_start(arg, format);
        va_copy(argcopy, arg);
        size = 1 + vsnprintf(NULL, 0, format, arg);
        va_end(arg);
        if(size > __GENERIC_FORMAT_BUFFER_MAX_SIZE) {
            std::fputs("\n[marker2.cpp] => __common_print_function_fmt() __VA_ARGS__ too large\n", __output_buf);
            invalid_state = true;
        }
        if(!invalid_state) {
            done = vsnprintf(__format_buffer.mem, size, format, argcopy);
        }
        va_end(argcopy);


        if (invalid_state || done < 0) {
            std::fputs("\n[marker2.cpp] => __common_print_function_fmt() Couldn't format __VA_ARGS__\n", __output_buf);
            __format_buffer.mem[0] = '\0'; /* if fputs encounters an eol it'll stop */
        }
        std::fputs(__format_buffer.mem, __output_buf);
        return;
#undef __GENERIC_FORMAT_BUFFER_MAX_SIZE
    }


} // namespace detail::marker


#if MARKER_FLAG_LOG_TO_FILE == 1
namespace marker 
{
    void marker_flag_close_logfile_handle() {
        std::fclose(detail::marker::__output_buf);
        mtx_destroy(&__write_lock);
        return;
    }
}
#endif


#endif