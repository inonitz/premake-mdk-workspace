#include "mb.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <util/marker2.hpp>
#include "util/time.hpp"
#include "vars.hpp"
#include "render.hpp"


i32 no_multigrid_for_now_more_buttons()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    u8 alive{true}, paused{false};


    markstr("no_multigrid_for_now_more_buttons begin");
    morebutton24::initializeLibrary();
    morebutton24::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(morebutton24::getContextID());
    while(alive) 
    {
        morebutton24::g_frameTime.begin();
        TIME_NAMESPACE_TIME_CODE_BLOCK(morebutton24::g_beginFrameTime, {
            awc2newframe();
            awc2begin();
        });

        
        if(likely(!paused)) {
            TIME_NAMESPACE_TIME_CODE_BLOCK(morebutton24::g_renderTime, morebutton24::render());
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }


        alive   = !awc2getContextStatus(morebutton24::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        TIME_NAMESPACE_TIME_CODE_BLOCK(morebutton24::g_endFrameTime, awc2end());
        morebutton24::g_frameTime.end();
        ++morebutton24::g_frameCounter;
    }
    markstr("Main App Loop End");

    /* return resources */
    morebutton24::destroyGraphics();
    morebutton24::destroyLibrary();
    markstr("no_multigrid_for_now_more_buttons end");
    return 1;
}