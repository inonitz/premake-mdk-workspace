#include "cfl.hpp"
#include "backend12.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <util/marker2.hpp>


i32 compute_shader_2d_fluid_also_cfl()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    const struct timespec slow_render_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 300 * 1000000
    };
    u8 alive {true};
    u8 paused{false}, prevPause{false};


    markstr("compute_shader_2d_fluid_also_cfl begin");
    program3::initializeLibrary();
    program3::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(program3::getContextID());
    while(alive) 
    {
        program3::getFrameTime().begin();
        awc2newframe();
        awc2begin();
        if(paused) {
            thrd_sleep(&pause_sleep_duration, NULL);
        } else {
            if(program3::getSlowRenderFlag()) {
                thrd_sleep(&slow_render_sleep_duration, NULL);
            }
            program3::render();
        }
        alive   = !awc2getContextStatus(program3::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        awc2end();
        program3::getFrameTime().end();
    }
    markstr("Main App Loop End");

    /* return resources */
    program3::destroyGraphics();
    program3::destroyLibrary();
    markstr("compute_shader_2d_fluid_also_cfl end");
    return 1;
}