#include "demo.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include <util2/C/marker4.h>
#include <util2/time.hpp>
#include "vars.hpp"
#include "render.hpp"


i32 gpugems38_demo()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    u8 alive{true}, paused{false};


    markstr("gpugems38_demo begin");
    cleanup228::initializeLibrary();
    cleanup228::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(cleanup228::getContextID());
    while(alive) 
    {
        cleanup228::g_frameTime.begin();
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(cleanup228::g_beginFrameTime, {
            awc2newframe();
            awc2begin();
        });

        
        if(likely(!paused)) {
            UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(cleanup228::g_renderTime, cleanup228::render());
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }


        alive   = !awc2getContextStatus(cleanup228::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(cleanup228::g_endFrameTime, awc2end());
        cleanup228::g_frameTime.end();
        ++cleanup228::g_frameCounter;
    }
    markstr("Main App Loop End");

    /* return resources */
    cleanup228::destroyGraphics();
    cleanup228::destroyLibrary();
    markstr("gpugems38_demo end");
    return 1;
}