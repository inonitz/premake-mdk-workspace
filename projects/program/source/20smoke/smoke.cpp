#include "smoke.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include <util2/C/marker4.h>
#include <util2/time.hpp>
#include "vars.hpp"
#include "render.hpp"


i32 smoke_sim()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    u8 alive{true}, paused{false};


    markstr("smoke_sim begin");
    smoke20::initializeLibrary();
    smoke20::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(smoke20::getContextID());
    while(alive) 
    {
        smoke20::g_frameTime.begin();
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(smoke20::g_beginFrameTime, {
            awc2newframe();
            awc2begin();
        });

        
        if(likely(!paused)) {
            UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(smoke20::g_renderTime, smoke20::render());
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }


        alive   = !awc2getContextStatus(smoke20::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(smoke20::g_endFrameTime, awc2end());
        smoke20::g_frameTime.end();
        ++smoke20::g_frameCounter;
    }
    markstr("Main App Loop End");

    /* return resources */
    smoke20::destroyGraphics();
    smoke20::destroyLibrary();
    markstr("smoke_sim end");
    return 1;
}