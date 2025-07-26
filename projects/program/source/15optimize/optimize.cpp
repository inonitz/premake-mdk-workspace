#include "optimize.hpp"
#include "backend15.hpp"
#include <util2/time.hpp>
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include <util2/C/marker4.h>


i32 after_measuring_you_cut_the_dead_wood()
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


    markstr("after_measuring_you_cut_the_dead_wood begin");
    optimize::initializeLibrary();
    optimize::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(optimize::getContextID());
    while(alive) 
    {
        optimize::getFrameTime().begin();
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(optimize::getTimer0(), {
            awc2newframe();
            awc2begin();
        });
        if(paused) {
            thrd_sleep(&pause_sleep_duration, NULL);
        } else {
            if(optimize::getSlowRenderFlag()) {
                thrd_sleep(&slow_render_sleep_duration, NULL);
            }
            UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(optimize::getRenderTime(), optimize::render());
        }


        alive   = !awc2getContextStatus(optimize::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(optimize::getTimer1(), awc2end());
        optimize::getFrameTime().end();
    }
    markstr("Main App Loop End");

    /* return resources */
    optimize::destroyGraphics();
    optimize::destroyLibrary();
    markstr("after_measuring_you_cut_the_dead_wood end");
    return 1;
}