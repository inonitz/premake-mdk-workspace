#include "features.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <util/marker2.hpp>
#include "util/time.hpp"
#include "vars.hpp"
#include "render.hpp"


i32 add_features_and_then_optimize()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    u8 alive{true}, paused{false};


    markstr("add_features_and_then_optimize begin");
    features18::initializeLibrary();
    features18::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(features18::getContextID());
    while(alive) 
    {
        features18::getFrameTime().begin();
        TIME_NAMESPACE_TIME_CODE_BLOCK(features18::getTimer0(), {
            awc2newframe();
            awc2begin();
        });

        
        if(likely(!paused)) {
            TIME_NAMESPACE_TIME_CODE_BLOCK(features18::getRenderTime(), features18::render());
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }


        alive   = !awc2getContextStatus(features18::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        TIME_NAMESPACE_TIME_CODE_BLOCK(features18::getTimer1(), awc2end());
        features18::getFrameTime().end();
        ++features18::getFrameCounter();
    }
    markstr("Main App Loop End");

    /* return resources */
    features18::destroyGraphics();
    features18::destroyLibrary();
    markstr("add_features_and_then_optimize end");
    return 1;
}