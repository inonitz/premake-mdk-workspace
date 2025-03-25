#include "ba.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include "util/time.hpp"
#include "vars.hpp"
#include "render.hpp"


i32 no_multigrid_for_now_until_internal_boundary_works()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    u8 alive{true}, paused{false};


    markstr("no_multigrid_for_now_until_internal_boundary_works begin");
    makebelievebound25::initializeLibrary();
    makebelievebound25::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(makebelievebound25::getContextID());
    while(alive) 
    {
        makebelievebound25::g_frameTime.begin();
        TIME_NAMESPACE_TIME_CODE_BLOCK(makebelievebound25::g_beginFrameTime, {
            awc2newframe();
            awc2begin();
        });

        
        if(likely(!paused)) {
            TIME_NAMESPACE_TIME_CODE_BLOCK(makebelievebound25::g_renderTime, makebelievebound25::render());
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }


        alive   = !awc2getContextStatus(makebelievebound25::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        TIME_NAMESPACE_TIME_CODE_BLOCK(makebelievebound25::g_endFrameTime, awc2end());
        makebelievebound25::g_frameTime.end();
        ++makebelievebound25::g_frameCounter;
    }
    markstr("Main App Loop End");

    /* return resources */
    makebelievebound25::destroyGraphics();
    makebelievebound25::destroyLibrary();
    markstr("no_multigrid_for_now_until_internal_boundary_works end");
    return 1;
}