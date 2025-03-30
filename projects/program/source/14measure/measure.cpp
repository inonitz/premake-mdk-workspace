#include "measure.hpp"
#include "backend14.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <util/marker2.hpp>


i32 measure_diffusion_and_work_distribution_across_shaders_also_revalidate_parallel_reduction()
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


    markstr("measure_diffusion_and_work_distribution_across_shaders_also_revalidate_parallel_reduction begin");
    measure::initializeLibrary();
    measure::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(measure::getContextID());
    while(alive) 
    {
        measure::getFrameTime().begin();
        TIME_NAMESPACE_TIME_CODE_BLOCK(measure::getTimer0(), {
            awc2newframe();
            awc2begin();
        });
        if(paused) {
            thrd_sleep(&pause_sleep_duration, NULL);
        } else {
            if(measure::getSlowRenderFlag()) {
                thrd_sleep(&slow_render_sleep_duration, NULL);
            }
            TIME_NAMESPACE_TIME_CODE_BLOCK(measure::getRenderTime(), measure::render());
        }


        alive   = !awc2getContextStatus(measure::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        
        TIME_NAMESPACE_TIME_CODE_BLOCK(measure::getTimer1(), awc2end());
        measure::getFrameTime().end();
    }
    markstr("Main App Loop End");

    /* return resources */
    measure::destroyGraphics();
    measure::destroyLibrary();
    markstr("measure_diffusion_and_work_distribution_across_shaders_also_revalidate_parallel_reduction end");
    return 1;
}