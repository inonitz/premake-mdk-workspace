#include "improve.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include "vars.hpp"
#include "render.hpp"
#include "fluid.hpp"


namespace cleanup329 {


i32 gpugems38_demo_last()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    u8 alive{true}, paused{false};


    markstr("gpugems38_demo_last begin");
    init::initializeLibrary();
    init::initializeGraphics();


    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(init::getContextID());
    while(alive) 
    {
        g_frameTime.begin();
        TIME_NAMESPACE_TIME_CODE_BLOCK(g_beginFrameTime, {
            awc2newframe();
            awc2begin();
        });

        
        render::clear();
        if(likely(!paused)) {
            TIME_NAMESPACE_TIME_CODE_BLOCK(g_fluidUpdateTime, fluid::update());
            TIME_NAMESPACE_TIME_CODE_BLOCK(g_renderImGuiTime, render::render_imgui());
            TIME_NAMESPACE_TIME_CODE_BLOCK(g_renderBlitTime,  render::render());
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }


        alive   = !awc2getContextStatus(init::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        TIME_NAMESPACE_TIME_CODE_BLOCK(g_endFrameTime, awc2end());
        g_frameTime.end();
        ++g_frameCounter;
    }
    markstr("Main App Loop End");

    /* return resources */
    init::destroyGraphics();
    init::destroyLibrary();
    markstr("gpugems38_demo_last end");
    return 1;
}


} /* namespace cleanup329 */