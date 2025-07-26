#include "ssbo.hpp"
#include "backend16.hpp"
#include <util2/time.hpp>
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include <util2/C/marker4.h>


i32 make_ssbo_work_for_cfl()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    const struct timespec slow_render_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 100 * 1'000'000
    };
    u8 alive {true};
    u8 paused{false}, prevPause{false};


    markstr("making_cfl_work_with_ssbo begin");
    ssbowork::initializeLibrary();
    ssbowork::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(ssbowork::getContextID());
    while(alive) 
    {
        ssbowork::getFrameTime().begin();
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(ssbowork::getTimer0(), {
            awc2newframe();
            awc2begin();
        });
        if(paused) {
            thrd_sleep(&pause_sleep_duration, NULL);
        } else {
            if(ssbowork::getSlowRenderFlag()) {
                thrd_sleep(&slow_render_sleep_duration, NULL);
            }
            UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(ssbowork::getRenderTime(), ssbowork::render());
        }


        alive   = !awc2getContextStatus(ssbowork::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(ssbowork::getTimer1(), awc2end());
        ssbowork::getFrameTime().end();
    }
    markstr("Main App Loop End");

    /* return resources */
    ssbowork::destroyGraphics();
    ssbowork::destroyLibrary();
    markstr("making_cfl_work_with_ssbo end");
    return 1;
}