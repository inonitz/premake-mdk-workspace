#include "forces_advection_diffusion.hpp"
#include "backend9.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <util/marker2.hpp>


i32 compute_shader_external_forces_advection_diffusion_draw()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    const struct timespec slow_render_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 300 * 1000000
    };
    constexpr u8 slowRender{false};
    u8 alive {true};
    u8 paused{false};


    markstr("compute_shader_external_forces_advection_diffusion_draw begin");
    
    program0::initializeLibrary();
    program0::initializeGraphics();


    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(program0::getContextID());
    while(alive) 
    {
        program0::getFrameTime().begin();
        awc2newframe();
        awc2begin();
        if(paused) {
            thrd_sleep(&pause_sleep_duration, NULL);
        } else {
            if constexpr (slowRender) {
                thrd_sleep(&slow_render_sleep_duration, NULL);
            }
            program0::render();
        }
        alive   = !awc2getContextStatus(program0::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        awc2end();
        program0::getFrameTime().end();
    }
    markstr("Main App Loop End");


    /* return resources */
    program0::destroyGraphics();
    program0::destroyLibrary();

    markstr("compute_shader_external_forces_advection_diffusion_draw end  ");
    return 1;
}