#include "optimize2.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <util/marker2.hpp>
#include "vars.hpp"
#include "render.hpp"


i32 optimize_again_and_again_and_again_and_again()
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


    markstr("optimize_again_and_again_and_again_and_again begin");
    optimize17::initializeLibrary();
    optimize17::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(optimize17::getContextID());
    while(alive) 
    {
        optimize17::getFrameTime().begin();
        TIME_NAMESPACE_TIME_CODE_BLOCK(optimize17::getTimer0(), {
            awc2newframe();
            awc2begin();
        });
        if(paused) {
            thrd_sleep(&pause_sleep_duration, NULL);
        } else {
            if(optimize17::getSlowRenderFlag()) {
                thrd_sleep(&slow_render_sleep_duration, NULL);
            }
            TIME_NAMESPACE_TIME_CODE_BLOCK(optimize17::getRenderTime(), optimize17::render());
        }


        alive   = !awc2getContextStatus(optimize17::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        TIME_NAMESPACE_TIME_CODE_BLOCK(optimize17::getTimer1(), awc2end());
        optimize17::getFrameTime().end();
    }
    markstr("Main App Loop End");

    /* return resources */
    optimize17::destroyGraphics();
    optimize17::destroyLibrary();
    markstr("optimize_again_and_again_and_again_and_again end");
    return 1;
}