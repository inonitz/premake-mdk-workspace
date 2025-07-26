#include "arbitrary.hpp"
#include <threads.h>
#include <util2/C/marker4.h>
#include <awc2/C/awc2.h>
#include <util2/time.hpp>
#include "vars.hpp"
#include "render.hpp"


i32 arbitrary_boundaries_in_sim()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    u8 alive{true}, paused{false};


    markstr("arbitrary_boundaries_in_sim begin");
    boundary21::initializeLibrary();
    boundary21::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(boundary21::getContextID());
    while(alive) 
    {
        boundary21::g_frameTime.begin();
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(boundary21::g_beginFrameTime, {
            awc2newframe();
            awc2begin();
        });

        
        if(likely(!paused)) {
            UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(boundary21::g_renderTime, boundary21::render());
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }


        alive   = !awc2getContextStatus(boundary21::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(boundary21::g_endFrameTime, awc2end());
        boundary21::g_frameTime.end();
        ++boundary21::g_frameCounter;
    }
    markstr("Main App Loop End");

    /* return resources */
    boundary21::destroyGraphics();
    boundary21::destroyLibrary();
    markstr("arbitrary_boundaries_in_sim end");
    return 1;
}