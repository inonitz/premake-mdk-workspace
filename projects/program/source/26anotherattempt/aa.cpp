#include "aa.hpp"
#include <threads.h>
#include <util2/C/marker4.h>
#include <awc2/C/awc2.h>
#include "vars.hpp"
#include "render.hpp"


i32 another_boundary_implementation_attempt()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    u8 alive{true}, paused{false};


    markstr("another_boundary_implementation_attempt begin");
    anotherattempt26::initializeLibrary();
    anotherattempt26::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(anotherattempt26::getContextID());
    while(alive) 
    {
        anotherattempt26::g_frameTime.begin();
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(anotherattempt26::g_beginFrameTime, {
            awc2newframe();
            awc2begin();
        });

        
        if(likely(!paused)) {
            UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(anotherattempt26::g_renderTime, anotherattempt26::render());
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }


        alive   = !awc2getContextStatus(anotherattempt26::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(anotherattempt26::g_endFrameTime, awc2end());
        anotherattempt26::g_frameTime.end();
        ++anotherattempt26::g_frameCounter;
    }
    markstr("Main App Loop End");

    /* return resources */
    anotherattempt26::destroyGraphics();
    anotherattempt26::destroyLibrary();
    markstr("another_boundary_implementation_attempt end");
    return 1;
}