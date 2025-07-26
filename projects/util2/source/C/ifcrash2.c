#include "util2/C/ifcrash2.h"
#include "util2/C/macro.h"
#include "util2/C/print.h"
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>


void util2_ifcrash2(
    const char*  ifcrash_type_str,
    const char*  file_macro,
    u64          line_macro,
    bool_t       condition,
    bool_t       formatStrIsActuallyEmpty,
    const char*  formatstr,
    ...
)  {
    if(!boolean(condition)) {
        return;
    }

    util2_fprintf(stderr, "\n[IFCRASH2_%s] %s:" PRIu32, ifcrash_type_str, file_macro, line_macro);
    if(!formatStrIsActuallyEmpty) {
        util2_fprintf(stderr, "\n[IFCRASH2_%s]", ifcrash_type_str);

        va_list arg_list;
        va_start(arg_list, formatstr);
        util2_va_fprintf(stderr, formatstr, arg_list);
        va_end(arg_list);
    }
    util2_fprintf(stderr, "\n[IFCRASH2_%s] ifcrash2(...) macro triggered\n", ifcrash_type_str);
    exit(0x420);
    return;
}