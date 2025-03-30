#include "aa.hpp"
#include <threads.h>
#include <util/marker2.hpp>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
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
        TIME_NAMESPACE_TIME_CODE_BLOCK(anotherattempt26::g_beginFrameTime, {
            awc2newframe();
            awc2begin();
        });

        
        if(likely(!paused)) {
            TIME_NAMESPACE_TIME_CODE_BLOCK(anotherattempt26::g_renderTime, anotherattempt26::render());
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }


        alive   = !awc2getContextStatus(anotherattempt26::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        TIME_NAMESPACE_TIME_CODE_BLOCK(anotherattempt26::g_endFrameTime, awc2end());
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