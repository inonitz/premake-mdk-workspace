#ifndef __AWC2_ENTRY_INTERFACE_HEADER__
#define __AWC2_ENTRY_INTERFACE_HEADER__
#include "C/awc2_api.h"
#include <util2/C/macro.h>


/* 
    [NOTE]: 
    File Rename required, 
    'entry' doesn't describe the purpose of this file fully
*/
namespace AWC2 {
    AWC2_API void init();
    AWC2_API void destroy();


    AWC2_API __hot void newFrame();
} // namespace AWC2


#endif