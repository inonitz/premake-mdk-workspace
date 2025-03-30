#include "dye.hpp"
#include "backend13.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <util/marker2.hpp>


i32 fluid_sim_2d_also_dye()
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
    program4::initializeLibrary();
    program4::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(program4::getContextID());
    while(alive) 
    {
        program4::getFrameTime().begin();
        awc2newframe();
        awc2begin();
        if(paused) {
            thrd_sleep(&pause_sleep_duration, NULL);
        } else {
            if(program4::getSlowRenderFlag()) {
                thrd_sleep(&slow_render_sleep_duration, NULL);
            }
            program4::render();
        }
        alive   = !awc2getContextStatus(program4::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        awc2end();
        program4::getFrameTime().end();
    }
    markstr("Main App Loop End");

    /* return resources */
    program4::destroyGraphics();
    program4::destroyLibrary();
    markstr("compute_shader_2d_fluid_also_cfl end");
    return 1;
}