#include "mg.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include <util2/C/marker4.h>
#include <util2/time.hpp>
#include "vars.hpp"
#include "render.hpp"


i32 multigrid_method_also_no_internal_boundaries_for_now()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    u8 alive{true}, paused{false};


    markstr("multigrid_method_also_no_internal_boundaries_for_now begin");
    multigrid23::initializeLibrary();
    multigrid23::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(multigrid23::getContextID());
    while(alive) 
    {
        multigrid23::g_frameTime.begin();
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(multigrid23::g_beginFrameTime, {
            awc2newframe();
            awc2begin();
        });

        
        if(likely(!paused)) {
            UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(multigrid23::g_renderTime, multigrid23::render());
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }


        alive   = !awc2getContextStatus(multigrid23::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(multigrid23::g_endFrameTime, awc2end());
        multigrid23::g_frameTime.end();
        ++multigrid23::g_frameCounter;
    }
    markstr("Main App Loop End");

    /* return resources */
    multigrid23::destroyGraphics();
    multigrid23::destroyLibrary();
    markstr("multigrid_method_also_no_internal_boundaries_for_now end");
    return 1;
}