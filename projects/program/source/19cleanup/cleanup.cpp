#include "cleanup.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include <util2/C/marker4.h>
#include <util2/time.hpp>
#include "vars.hpp"
#include "render.hpp"


i32 cleanup_old()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    u8 alive{true}, paused{false};


    markstr("cleanup_old begin");
    cleanup19::initializeLibrary();
    cleanup19::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(cleanup19::getContextID());
    while(alive) 
    {
        cleanup19::g_frameTime.begin();
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(cleanup19::g_beginFrameTime, {
            awc2newframe();
            awc2begin();
        });

        
        if(likely(!paused)) {
            UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(cleanup19::g_renderTime, cleanup19::render());
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }


        alive   = !awc2getContextStatus(cleanup19::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(cleanup19::g_endFrameTime, awc2end());
        cleanup19::g_frameTime.end();
        ++cleanup19::g_frameCounter;
    }
    markstr("Main App Loop End");

    /* return resources */
    cleanup19::destroyGraphics();
    cleanup19::destroyLibrary();
    markstr("cleanup_old end");
    return 1;
}